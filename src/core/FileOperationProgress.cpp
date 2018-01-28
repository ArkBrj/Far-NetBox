
#include <vcl.h>
#pragma hdrstop

#include <Common.h>

#include "FileOperationProgress.h"
#include "CoreMain.h"
//---------------------------------------------------------------------------
#define TRANSFER_BUF_SIZE 32 * 1024
//---------------------------------------------------------------------------
__fastcall TFileOperationProgressType::TFileOperationProgressType() :
  FParent(nullptr),
  FOnProgress(nullptr),
  FOnFinished(nullptr),
  FReset(false)
{
  Init();
  Clear();
}
//---------------------------------------------------------------------------
__fastcall TFileOperationProgressType::TFileOperationProgressType(
  TFileOperationProgressEvent AOnProgress, TFileOperationFinishedEvent AOnFinished,
  TFileOperationProgressType *Parent) :
  FParent(Parent),
  FOnProgress(AOnProgress),
  FOnFinished(AOnFinished),
  FReset(false)
{
  Init();
  Clear();
}
//---------------------------------------------------------------------------
__fastcall TFileOperationProgressType::~TFileOperationProgressType()
{
  DebugAssert(!GetInProgress() || FReset);
  DebugAssert(!GetSuspended() || FReset);
  SAFE_DESTROY_EX(TCriticalSection, FSection);
  SAFE_DESTROY_EX(TCriticalSection, FUserSelectionsSection);
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::Init()
{
  FSection = new TCriticalSection();
  FUserSelectionsSection = new TCriticalSection();
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::Assign(const TFileOperationProgressType &Other)
{
  volatile TValueRestorer<TCriticalSection *> SectionRestorer(FSection);
  volatile TValueRestorer<TCriticalSection *> UserSelectionsSectionRestorer(FUserSelectionsSection);
  volatile TGuard Guard(*FSection);
  volatile TGuard OtherGuard(*Other.FSection);

  *this = Other;
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::AssignButKeepSuspendState(const TFileOperationProgressType &Other)
{
  volatile TGuard Guard(*FSection);
  volatile TValueRestorer<uintptr_t> SuspendTimeRestorer(FSuspendTime);
  volatile TValueRestorer<bool> SuspendedRestorer(FSuspended);

  Assign(Other);
}
//---------------------------------------------------------------------------
__fastcall void TFileOperationProgressType::Clear()
{
  FFileName = L"";
  FFullFileName = L"";
  FDirectory = L"";
  FAsciiTransfer = false;
  FCount = -1;
  FFilesFinished = 0;
  FFilesFinishedSuccessfully = 0;
  FStartTime = Now();
  FSuspended = false;
  FSuspendTime = 0;
  FInProgress = false;
  FDone = false;
  FFileInProgress = false;
  FTotalTransferred = 0;
  FTotalSkipped = 0;
  FTotalSize = 0;
  FSkippedSize = 0;
  FTotalSizeSet = false;
  FFileStartTime = 0.0;
  FFilesFinished = 0;
  FReset = false;
  FLastSecond = 0;
  FRemainingCPS = 0;
  FTicks.clear();
  FTotalTransferredThen.clear();
  FCounterSet = false;
  FOperation = foNone;
  FSide = osLocal;
  FFileName.Clear();
  FDirectory.Clear();
  FAsciiTransfer = false;
  FLocalSize = 0;
  FLocallyUsed = 0;
  FOperation = foNone;
  FTemp = false;
  FSkipToAll = false;
  FBatchOverwrite = boNo;
  // to bypass check in ClearTransfer()
  FTransferSize = 0;
  FCPSLimit = 0;
  FTicks.clear();
  FTotalTransferredThen.clear();
  FCounterSet = false;
  ClearTransfer();
  FTransferredSize = 0;
  FCancel = csContinue;
  FCount = 0;
  FStartTime = Now();
  FCPSLimit = 0;
  FSuspended = false;

  ClearTransfer();
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::ClearTransfer()
{
  if ((FTransferSize > 0) && (FTransferredSize < FTransferSize))
  {
    TGuard Guard(*FSection);
    int64_t RemainingSize = (FTransferSize - FTransferredSize);
    AddSkipped(RemainingSize);
  }
  FLocalSize = 0;
  FTransferSize = 0;
  FLocallyUsed = 0;
  FSkippedSize = 0;
  FTransferredSize = 0;
  FTransferringFile = false;
  FLastSecond = 0;
}
//---------------------------------------------------------------------------
void TFileOperationProgressType::Start(TFileOperation AOperation,
  TOperationSide ASide, intptr_t ACount)
{
  Start(AOperation, ASide, ACount, false, L"", 0);
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::Start(TFileOperation AOperation,
  TOperationSide ASide, intptr_t ACount, bool ATemp,
  const UnicodeString ADirectory, uintptr_t ACPSLimit)
{

  {
    TGuard Guard(*FSection); // not really needed, just for consistency
    Clear();
    FOperation = AOperation;
    FSide = ASide;
    FCount = ACount;
    FInProgress = true;
    FCancel = csContinue;
    FDirectory = ADirectory;
    FTemp = ATemp;
    FCPSLimit = ACPSLimit;
  }

  try
  {
    DoProgress();
  }
  catch (...)
  {
    // connection can be lost during progress callbacks
    ClearTransfer();
    FInProgress = false;
    throw;
  }
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::Reset()
{
  FReset = true;
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::Stop()
{
  // added to include remaining bytes to TotalSkipped, in case
  // the progress happens to update before closing
  ClearTransfer();
  FInProgress = false;
  DoProgress();
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::SetDone()
{
  FDone = true;
  DoProgress();
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::Suspend()
{

  {
    TGuard Guard(*FSection);
    DebugAssert(!FSuspended);
    FSuspended = true;
    FSuspendTime = ::GetTickCount();
  }

  DoProgress();
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::Resume()
{

  {
    TGuard Guard(*FSection);
    DebugAssert(FSuspended);
    FSuspended = false;

    // shift timestamps for CPS calculation in advance
    // by the time the progress was suspended
    intptr_t Stopped = static_cast<intptr_t>(::GetTickCount() - FSuspendTime);
    size_t Index = 0;
    while (Index < FTicks.size())
    {
      FTicks[Index] += Stopped;
      ++Index;
    }
  }

  DoProgress();
}
//---------------------------------------------------------------------------
intptr_t __fastcall TFileOperationProgressType::OperationProgress() const
{
  intptr_t Result;
  if (FCount > 0)
  {
    Result = (FFilesFinished * 100) / FCount;
  }
  else
  {
    Result = 0;
  }
  return Result;
}
//---------------------------------------------------------------------------
intptr_t __fastcall TFileOperationProgressType::TransferProgress() const
{
  intptr_t Result;
  if (FTransferSize)
  {
    Result = static_cast<intptr_t>((FTransferredSize * 100) / FTransferSize);
  }
  else
  {
    Result = 0;
  }
  return Result;
}
//---------------------------------------------------------------------------
intptr_t __fastcall TFileOperationProgressType::TotalTransferProgress() const
{
  TGuard Guard(*FSection);
  DebugAssert(FTotalSizeSet);
  intptr_t Result = FTotalSize > 0 ? static_cast<intptr_t>(((GetTotalTransferred() + FTotalSkipped) * 100) / FTotalSize) : 0;
  return Result < 100 ? Result : 100;
}
//---------------------------------------------------------------------------
intptr_t __fastcall TFileOperationProgressType::OverallProgress() const
{
  if (FTotalSizeSet)
  {
    DebugAssert((FOperation == foCopy) || (FOperation == foMove));
    return TotalTransferProgress();
  }
  return OperationProgress();
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::Progress()
{
  DoProgress();
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::DoProgress()
{
  SetThreadExecutionState(ES_SYSTEM_REQUIRED);
  FOnProgress(*this);
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::Finish(UnicodeString AFileName,
  bool Success, TOnceDoneOperation &OnceDoneOperation)
{
  DebugAssert(FInProgress);

  // Cancel reader is guarded
  FOnFinished(FOperation, FSide, FTemp, AFileName,
    Success && (FCancel == csContinue), OnceDoneOperation);
  FFilesFinished++;
  if (Success)
  {
    FFilesFinishedSuccessfully++;
  }
  DoProgress();
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::SetFile(UnicodeString AFileName, bool AFileInProgress)
{
  FFullFileName = AFileName;
  if (FSide == osRemote)
  {
    // historically set were passing filename-only for remote site operations,
    // now we need to collect a full paths, so we pass in full path,
    // but still want to have filename-only in FileName
    AFileName = base::UnixExtractFileName(AFileName);
  }
  FFileName = AFileName;
  FFileInProgress = AFileInProgress;
  ClearTransfer();
  FFileStartTime = Now();
  DoProgress();
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::SetFileInProgress()
{
  DebugAssert(!FFileInProgress);
  FFileInProgress = true;
  DoProgress();
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::SetLocalSize(int64_t ASize)
{
  FLocalSize = ASize;
  DoProgress();
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::AddLocallyUsed(int64_t ASize)
{
  FLocallyUsed += ASize;
  if (FLocallyUsed > FLocalSize)
  {
    FLocalSize = FLocallyUsed;
  }
  DoProgress();
}
//---------------------------------------------------------------------------
bool __fastcall TFileOperationProgressType::IsLocallyDone() const
{
  DebugAssert(FLocallyUsed <= FLocalSize);
  return (FLocallyUsed == FLocalSize);
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::SetSpeedCounters()
{
  if ((FCPSLimit > 0) && !FCounterSet)
  {
    FCounterSet = true;
    __removed Configuration->Usage->Inc(L"SpeedLimitUses");
  }
}
//---------------------------------------------------------------------------
// Used in WebDAV protocol
void __fastcall TFileOperationProgressType::ThrottleToCPSLimit(
  intptr_t Size)
{
  intptr_t Remaining = Size;
  while (Remaining > 0)
  {
    Remaining -= AdjustToCPSLimit(Remaining);
  }
}
//---------------------------------------------------------------------------
intptr_t __fastcall TFileOperationProgressType::AdjustToCPSLimit(
  intptr_t Size)
{
  SetSpeedCounters();

  // CPSLimit reader is guarded, we cannot block whole method as it can last long.
  if (FCPSLimit > 0)
  {
    // we must not return 0, hence, if we reach zero,
    // we wait until the next second
    do
    {
      uintptr_t Second = (::GetTickCount() / MSecsPerSec);

      if (Second != FLastSecond)
      {
        FRemainingCPS = FCPSLimit;
        FLastSecond = Second;
      }

      if (FRemainingCPS == 0)
      {
        SleepEx(100, true);
        DoProgress();
      }
    }
    while ((FCPSLimit > 0) && (FRemainingCPS == 0));

    // CPSLimit may have been dropped in DoProgress
    if (FCPSLimit > 0)
    {
      if (FRemainingCPS < Size)
      {
        Size = FRemainingCPS;
      }

      FRemainingCPS -= Size;
    }
  }
  return Size;
}
//---------------------------------------------------------------------------
// Use in SCP protocol only
uintptr_t __fastcall TFileOperationProgressType::LocalBlockSize()
{
  uintptr_t Result = TRANSFER_BUF_SIZE;
  if (FLocallyUsed + static_cast<int64_t>(Result) > FLocalSize)
  {
    Result = static_cast<uintptr_t>(FLocalSize - FLocallyUsed);
  }
  Result = AdjustToCPSLimit(Result);
  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::SetTotalSize(int64_t ASize)
{
  TGuard Guard(*FSection); // not really needed, just for consistency

  FTotalSize = ASize;
  FTotalSizeSet = true;
  // parent has its own totals
  if (FParent != nullptr)
  {
    DebugAssert(FParent->GetTotalSizeSet());
  }
  DoProgress();
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::SetTransferSize(int64_t ASize)
{
  FTransferSize = ASize;
  DoProgress();
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::SetTransferringFile(bool ATransferringFile)
{
  FTransferringFile = ATransferringFile;
}
//---------------------------------------------------------------------------
bool __fastcall TFileOperationProgressType::PassCancelToParent(TCancelStatus ACancel)
{
  bool Result;
  if (ACancel < csCancel)
  {
    // do not propagate csCancelFile,
    // though it's not supported for queue atm, so we do not expect it here
    DebugFail();
    Result = false;
  }
  else if (ACancel == csCancel)
  {
    Result = true;
  }
  else
  {
    // csCancelTransfer and csRemoteAbort are used with SCP only, which does not use parallel transfers
    DebugFail();
    Result = false;
  }
  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::SetCancel(TCancelStatus ACancel)
{
  TGuard Guard(*FSection);
  FCancel = ACancel;

  if ((FParent != nullptr) && PassCancelToParent(ACancel))
  {
    FParent->SetCancel(ACancel);
  }
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::SetCancelAtLeast(TCancelStatus ACancel)
{
  TGuard Guard(*FSection);
  if (FCancel < ACancel)
  {
    FCancel = ACancel;
  }

  if ((FParent != nullptr) && PassCancelToParent(ACancel))
  {
    FParent->SetCancelAtLeast(ACancel);
  }
}
//---------------------------------------------------------------------------
TCancelStatus __fastcall TFileOperationProgressType::GetCancel() const
{
  TCancelStatus Result = FCancel;
  if (FParent != nullptr)
  {
    TGuard Guard(*FSection);
    TCancelStatus ParentCancel = FParent->GetCancel();
    if (ParentCancel > Result)
    {
      Result = ParentCancel;
    }
  }
  return Result;
}
//---------------------------------------------------------------------------
bool __fastcall TFileOperationProgressType::ClearCancelFile()
{
  TGuard Guard(*FSection);
  // Not propagated to parent, as this is local flag, see also PassCancelToParent
  bool Result = (GetCancel() == csCancelFile);
  if (Result)
  {
    FCancel = csContinue;
  }
  return Result;
}
//---------------------------------------------------------------------------
intptr_t __fastcall TFileOperationProgressType::GetCPSLimit() const
{
  intptr_t Result;
  if (FParent != nullptr)
  {
    Result = FParent->GetCPSLimit();
  }
  else
  {
    TGuard Guard(*FSection);
    Result = FCPSLimit;
  }
  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::SetCPSLimit(intptr_t ACPSLimit)
{
  if (FParent != nullptr)
  {
    FParent->SetCPSLimit(ACPSLimit);
  }
  else
  {
    TGuard Guard(*FSection);
    FCPSLimit = ACPSLimit;
  }
}
//---------------------------------------------------------------------------
TBatchOverwrite __fastcall TFileOperationProgressType::GetBatchOverwrite() const
{
  TBatchOverwrite Result;
  if (FParent != nullptr)
  {
    Result = FParent->GetBatchOverwrite();
  }
  else
  {
    TGuard Guard(*FSection); // not really needed
    Result = FBatchOverwrite;
  }
  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::SetBatchOverwrite(TBatchOverwrite ABatchOverwrite)
{
  if (FParent != nullptr)
  {
    FParent->SetBatchOverwrite(ABatchOverwrite);
  }
  else
  {
    TGuard Guard(*FSection); // not really needed
    FBatchOverwrite = ABatchOverwrite;
  }
}
//---------------------------------------------------------------------------
bool __fastcall TFileOperationProgressType::GetSkipToAll() const
{
  bool Result;
  if (FParent != nullptr)
  {
    Result = FParent->GetSkipToAll();
  }
  else
  {
    TGuard Guard(*FSection); // not really needed
    Result = FSkipToAll;
  }
  return Result;
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::SetSkipToAll()
{
  if (FParent != nullptr)
  {
    FParent->SetSkipToAll();
  }
  else
  {
    TGuard Guard(*FSection); // not really needed
    FSkipToAll = true;
  }
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::ChangeTransferSize(int64_t ASize)
{
  // reflect change on file size (due to text transfer mode conversion particularly)
  // on total transfer size
  if (GetTotalSizeSet())
  {
    AddTotalSize(ASize - FTransferSize);
  }
  FTransferSize = ASize;
  DoProgress();
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::RollbackTransferFromTotals(int64_t ATransferredSize, int64_t ASkippedSize)
{
  TGuard Guard(*FSection);

  DebugAssert(ATransferredSize <= FTotalTransferred);
  DebugAssert(ASkippedSize <= FTotalSkipped);
  FTotalTransferred -= ATransferredSize;
  FTicks.clear();
  FTotalTransferredThen.clear();
  FTotalSkipped -= ASkippedSize;

  if (FParent != nullptr)
  {
    FParent->RollbackTransferFromTotals(ATransferredSize, ASkippedSize);
  }
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::RollbackTransfer()
{
  FTransferredSize -= FSkippedSize;
  RollbackTransferFromTotals(FTransferredSize, FSkippedSize);
  FSkippedSize = 0;
  FTransferredSize = 0;
  FTransferSize = 0;
  FLocallyUsed = 0;
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::AddTransferredToTotals(int64_t ASize)
{
  TGuard Guard(*FSection);

  FTotalTransferred += ASize;
  if (ASize >= 0)
  {
    intptr_t Ticks = static_cast<intptr_t>(::GetTickCount());
    if (FTicks.empty() ||
        (FTicks.back() > Ticks) || // ticks wrap after 49.7 days
        ((Ticks - FTicks.back()) >= MSecsPerSec))
    {
      FTicks.push_back(Ticks);
      FTotalTransferredThen.push_back(FTotalTransferred);
    }

    if (FTicks.size() > 10)
    {
      FTicks.erase(FTicks.begin());
      FTotalTransferredThen.erase(FTotalTransferredThen.begin());
    }
  }
  else
  {
    FTicks.clear();
  }

  if (FParent != nullptr)
  {
    FParent->AddTransferredToTotals(ASize);
  }
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::AddTotalSize(int64_t ASize)
{
  if (ASize != 0)
  {
    TGuard Guard(*FSection);
    FTotalSize += ASize;

    if (FParent != nullptr)
    {
      FParent->AddTotalSize(ASize);
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::AddTransferred(int64_t ASize,
  bool AddToTotals)
{
  FTransferredSize += ASize;
  if (FTransferredSize > FTransferSize)
  {
    // this can happen with SFTP when downloading file that
    // grows while being downloaded
    if (FTotalSizeSet)
    {
      // we should probably guard this with AddToTotals
      AddTotalSize(FTransferredSize - FTransferSize);
    }
    FTransferSize = FTransferredSize;
  }
  if (AddToTotals)
  {
    AddTransferredToTotals(ASize);
  }
  DoProgress();
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::AddSkipped(int64_t ASize)
{
  TGuard Guard(*FSection);

  FTotalSkipped += ASize;

  if (FParent != nullptr)
  {
    FParent->AddSkipped(ASize);
  }
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::AddResumed(int64_t ASize)
{
  AddSkipped(ASize);
  FSkippedSize += ASize;
  AddTransferred(ASize, false);
  AddLocallyUsed(ASize);
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::AddSkippedFileSize(int64_t ASize)
{
  AddSkipped(ASize);
  DoProgress();
}
//---------------------------------------------------------------------------
// Use in SCP protocol only
uintptr_t __fastcall TFileOperationProgressType::TransferBlockSize()
{
  uintptr_t Result = TRANSFER_BUF_SIZE;
  if (FTransferredSize + static_cast<int64_t>(Result) > FTransferSize)
  {
    Result = static_cast<uintptr_t>(FTransferSize - FTransferredSize);
  }
  Result = AdjustToCPSLimit(Result);
  return Result;
}
//---------------------------------------------------------------------------
uintptr_t __fastcall TFileOperationProgressType::StaticBlockSize()
{
  return TRANSFER_BUF_SIZE;
}
//---------------------------------------------------------------------------
bool __fastcall TFileOperationProgressType::IsTransferDone() const
{
  DebugAssert(FTransferredSize <= FTransferSize);
  return (FTransferredSize == FTransferSize);
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::SetAsciiTransfer(bool AAsciiTransfer)
{
  FAsciiTransfer = AAsciiTransfer;
  DoProgress();
}
//---------------------------------------------------------------------------
TDateTime __fastcall TFileOperationProgressType::TimeElapsed() const
{
  return Now() - GetStartTime();
}
//---------------------------------------------------------------------------
uintptr_t __fastcall TFileOperationProgressType::CPS() const
{
  TGuard Guard(*FSection);
  return GetCPS();
}
//---------------------------------------------------------------------------
// Has to be called from a guarded method
uintptr_t __fastcall TFileOperationProgressType::GetCPS() const
{
  uintptr_t Result;
  if (FTicks.empty())
  {
    Result = 0;
  }
  else
  {
    intptr_t Ticks = (GetSuspended() ? FSuspendTime : ::GetTickCount());
    intptr_t TimeSpan;
    if (Ticks < FTicks.front())
    {
      // clocks has wrapped, guess 10 seconds difference
      TimeSpan = 10000;
    }
    else
    {
      TimeSpan = (Ticks - FTicks.front());
    }

    if (TimeSpan == 0)
    {
      Result = 0;
    }
    else
    {
      int64_t Transferred = (GetTotalTransferred() - FTotalTransferredThen.front());
      Result = static_cast<uintptr_t>(Transferred * MSecsPerSec / TimeSpan);
    }
  }
  return Result;
}
//---------------------------------------------------------------------------
TDateTime __fastcall TFileOperationProgressType::TimeExpected() const
{
  uintptr_t CurCps = CPS();
  if (CurCps)
  {
    return TDateTime(ToDouble((ToDouble(FTransferSize - FTransferredSize)) / CurCps) / SecsPerDay);
  }
  return TDateTime(0.0);
}
//---------------------------------------------------------------------------
TDateTime __fastcall TFileOperationProgressType::TotalTimeLeft() const
{
  TGuard Guard(*FSection);
  DebugAssert(FTotalSizeSet);
  uintptr_t CurCps = GetCPS();
  // sanity check
  if ((CurCps > 0) && (FTotalSize > FTotalSkipped + FTotalTransferred))
  {
    return TDateTime(ToDouble(ToDouble(FTotalSize - FTotalSkipped) / CurCps) /
        SecsPerDay);
  }
  return TDateTime(0.0);
}
//---------------------------------------------------------------------------
int64_t __fastcall TFileOperationProgressType::GetTotalTransferred() const
{
  TGuard Guard(*FSection);
  return FTotalTransferred;
}
//---------------------------------------------------------------------------
int64_t __fastcall TFileOperationProgressType::GetTotalSize() const
{
  TGuard Guard(*FSection);
  return FTotalSize;
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::LockUserSelections()
{
  if (FParent != nullptr)
  {
    FParent->LockUserSelections();
  }
  else
  {
    FUserSelectionsSection->Enter();
  }
}
//---------------------------------------------------------------------------
void __fastcall TFileOperationProgressType::UnlockUserSelections()
{
  if (FParent != nullptr)
  {
    FParent->UnlockUserSelections();
  }
  else
  {
    FUserSelectionsSection->Leave();
  }
}
//---------------------------------------------------------------------------
UnicodeString __fastcall TFileOperationProgressType::GetLogStr(bool Done) const
{
  UnicodeString Transferred = FormatSize(FTotalTransferred);
//  UnicodeString Left;
  TDateTime Time;
  UnicodeString TimeLabel;
  if (!Done && FTotalSizeSet)
  {
    Time = TotalTimeLeft();
    TimeLabel = L"Left";
  }
  else
  {
    Time = TimeElapsed();
    TimeLabel = L"Elapsed";
  }
  UnicodeString TimeStr = FormatDateTimeSpan(GetConfiguration()->TimeFormat(), Time);
  UnicodeString CPSStr = FormatSize(CPS());
  return FORMAT("Transferred: %s, %s: %s, CPS: %s/s", Transferred, TimeLabel, TimeStr, CPSStr);
}
