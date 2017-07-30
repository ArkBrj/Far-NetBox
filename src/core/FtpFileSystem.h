
#pragma once

#ifndef NO_FILEZILLA

#include <time.h>
#include <rdestl/map.h>
#include <FileSystems.h>

class TFileZillaIntf;
class TFileZillaImpl;
class TMessageQueue;
class TFTPServerCapabilities;
struct TOverwriteFileParams;
struct TListDataEntry;
struct TFileTransferData;
struct TFtpsCertificateData;
struct TRemoteFileTime;

class TFTPFileSystem : public TCustomFileSystem
{
friend class TFileZillaImpl;
friend class TFTPFileListHelper;
NB_DISABLE_COPY(TFTPFileSystem)
public:
  static inline bool classof(const TObject * Obj) { return Obj->is(OBJECT_CLASS_TFTPFileSystem); }
  virtual bool is(TObjectClassId Kind) const override { return (Kind == OBJECT_CLASS_TFTPFileSystem) || TCustomFileSystem::is(Kind); }
public:
  explicit TFTPFileSystem(TTerminal * ATerminal);
  virtual ~TFTPFileSystem();

  virtual void Init(void *) override;

  virtual void Open() override;
  virtual void Close() override;
  virtual bool GetActive() const override;
  virtual void CollectUsage() override;
  virtual void Idle() override;
  virtual UnicodeString GetAbsolutePath(UnicodeString APath, bool Local) override;
  virtual UnicodeString GetAbsolutePath(UnicodeString APath, bool Local) const override;
  virtual void AnyCommand(UnicodeString Command,
    TCaptureOutputEvent OutputEvent) override;
  virtual void ChangeDirectory(UnicodeString ADirectory) override;
  virtual void CachedChangeDirectory(UnicodeString ADirectory) override;
  virtual void AnnounceFileListOperation() override;
  virtual void ChangeFileProperties(UnicodeString AFileName,
    const TRemoteFile * AFile, const TRemoteProperties * Properties,
    TChmodSessionAction & Action) override;
  virtual bool LoadFilesProperties(TStrings * AFileList) override;
  virtual void CalculateFilesChecksum(UnicodeString Alg,
    TStrings * AFileList, TStrings * Checksums,
    TCalculatedChecksumEvent OnCalculatedChecksum) override;
  virtual void CopyToLocal(const TStrings * AFilesToCopy,
    UnicodeString TargetDir, const TCopyParamType * CopyParam,
    intptr_t Params, TFileOperationProgressType * OperationProgress,
    TOnceDoneOperation & OnceDoneOperation) override;
  virtual void CopyToRemote(const TStrings * AFilesToCopy,
    UnicodeString ATargetDir, const TCopyParamType * CopyParam,
    intptr_t Params, TFileOperationProgressType * OperationProgress,
    TOnceDoneOperation & OnceDoneOperation) override;
  virtual void RemoteCreateDirectory(UnicodeString ADirName) override;
  virtual void CreateLink(UnicodeString AFileName, UnicodeString PointTo, bool Symbolic) override;
  virtual void RemoteDeleteFile(UnicodeString AFileName,
    const TRemoteFile * AFile, intptr_t Params, TRmSessionAction & Action) override;
  virtual void CustomCommandOnFile(UnicodeString AFileName,
    const TRemoteFile * AFile, UnicodeString Command, intptr_t Params, TCaptureOutputEvent OutputEvent) override;
  virtual void DoStartup() override;
  virtual void HomeDirectory() override;
  virtual bool IsCapable(intptr_t Capability) const override;
  virtual void LookupUsersGroups() override;
  virtual void ReadCurrentDirectory() override;
  virtual void ReadDirectory(TRemoteFileList * FileList) override;
  virtual void ReadFile(UnicodeString AFileName,
    TRemoteFile *& AFile) override;
  virtual void ReadSymlink(TRemoteFile * SymlinkFile,
    TRemoteFile *& AFile) override;
  virtual void RemoteRenameFile(UnicodeString AFileName,
    UnicodeString ANewName) override;
  virtual void RemoteCopyFile(UnicodeString AFileName,
    UnicodeString ANewName) override;
  virtual TStrings * GetFixedPaths() const override;
  virtual void SpaceAvailable(UnicodeString APath,
    TSpaceAvailable & ASpaceAvailable) override;
  virtual const TSessionInfo & GetSessionInfo() const override;
  virtual const TFileSystemInfo & GetFileSystemInfo(bool Retrieve) override;
  virtual bool TemporaryTransferFile(UnicodeString AFileName) override;
  virtual bool GetStoredCredentialsTried() const override;
  virtual UnicodeString RemoteGetUserName() const override;
  virtual void GetSupportedChecksumAlgs(TStrings * Algs) override;
  virtual void LockFile(UnicodeString AFileName, const TRemoteFile * AFile) override;
  virtual void UnlockFile(UnicodeString AFileName, const TRemoteFile * AFile) override;
  virtual void UpdateFromMain(TCustomFileSystem * MainFileSystem) override;

protected:
#if 0
  enum TOverwriteMode { omOverwrite, omResume, omComplete };
#endif // #if 0

  virtual UnicodeString RemoteGetCurrentDirectory() const override;

  const wchar_t * GetOption(intptr_t OptionID) const;
  intptr_t GetOptionVal(intptr_t OptionID) const;

  enum
  {
    REPLY_CONNECT      = 0x01,
    REPLY_2XX_CODE     = 0x02,
    REPLY_ALLOW_CANCEL = 0x04,
    REPLY_3XX_CODE     = 0x08,
    REPLY_SINGLE_LINE  = 0x10,
  };

  bool FTPPostMessage(uintptr_t Type, WPARAM wParam, LPARAM lParam);
  bool ProcessMessage();
  void DiscardMessages();
  void WaitForMessages();
  uintptr_t WaitForReply(bool Command, bool WantLastCode);
  uintptr_t WaitForCommandReply(bool WantLastCode = true);
  void WaitForFatalNonCommandReply();
  void PoolForFatalNonCommandReply();
  void GotNonCommandReply(uintptr_t Reply);
  UnicodeString GotReply(uintptr_t Reply, uintptr_t Flags = 0,
    UnicodeString Error = L"", uintptr_t * Code = nullptr,
    TStrings ** Response = nullptr);
  void ResetReply();
  void HandleReplyStatus(UnicodeString Response);
  void DoWaitForReply(uintptr_t &ReplyToAwait, bool WantLastCode);
  bool KeepWaitingForReply(uintptr_t &ReplyToAwait, bool WantLastCode) const;
  inline bool NoFinalLastCode() const;

  bool HandleStatus(const wchar_t * AStatus, int Type);
  bool HandleAsynchRequestOverwrite(
    wchar_t * FileName1, size_t FileName1Len, const wchar_t * FileName2,
    const wchar_t * Path1, const wchar_t * Path2,
    int64_t Size1, int64_t Size2, time_t LocalTime,
    bool HasLocalTime, const TRemoteFileTime & RemoteTime, void * AUserData,
    HANDLE & ALocalFileHandle,
    int & RequestResult);
  bool HandleAsynchRequestVerifyCertificate(
    const TFtpsCertificateData & Data, int & RequestResult);
  bool HandleAsynchRequestNeedPass(
    struct TNeedPassRequestData & Data, int & RequestResult) const;
  bool HandleListData(const wchar_t * Path, const TListDataEntry * Entries,
    uintptr_t Count);
  bool HandleTransferStatus(bool Valid, int64_t TransferSize,
    int64_t Bytes, bool FileTransfer);
  bool HandleReply(intptr_t Command, uintptr_t Reply);
  bool HandleCapabilities(TFTPServerCapabilities * ServerCapabilities);
  bool CheckError(intptr_t ReturnCode, const wchar_t * Context);
  void PreserveDownloadFileTime(HANDLE AHandle, void * UserData) const;
  bool GetFileModificationTimeInUtc(const wchar_t * FileName, struct tm & Time);
  void EnsureLocation(UnicodeString ADirectory, bool Log);
  void EnsureLocation();
  UnicodeString GetActualCurrentDirectory() const;
  void Discard();
  void DoChangeDirectory(UnicodeString Directory);
  bool DoQuit();

  void Sink(UnicodeString AFileName,
    const TRemoteFile * AFile, UnicodeString TargetDir,
    const TCopyParamType * CopyParam, intptr_t AParams,
    TFileOperationProgressType * OperationProgress, uintptr_t Flags,
    TDownloadSessionAction & Action);
  void SinkRobust(UnicodeString AFileName,
    const TRemoteFile * AFile, UnicodeString TargetDir,
    const TCopyParamType * CopyParam, intptr_t Params,
    TFileOperationProgressType * OperationProgress, uintptr_t Flags);
  void SinkFile(UnicodeString AFileName, const TRemoteFile * AFile, void * Param);
  void SourceRobust(UnicodeString AFileName,
    const TRemoteFile * AFile,
    UnicodeString TargetDir, const TCopyParamType * CopyParam, intptr_t Params,
    TFileOperationProgressType * OperationProgress, uintptr_t Flags);
  void Source(UnicodeString AFileName,
    const TRemoteFile * AFile,
    UnicodeString TargetDir, const TCopyParamType * CopyParam, intptr_t Params,
    TOpenRemoteFileParams * OpenParams,
    TOverwriteFileParams * FileParams,
    TFileOperationProgressType * OperationProgress, uintptr_t Flags,
    TUploadSessionAction & Action);
  void DirectorySource(UnicodeString DirectoryName,
    UnicodeString TargetDir, intptr_t Attrs, const TCopyParamType * CopyParam,
    intptr_t Params, TFileOperationProgressType * OperationProgress, uintptr_t Flags);
  bool ConfirmOverwrite(UnicodeString ASourceFullFileName, UnicodeString & ATargetFileName,
    intptr_t Params, TFileOperationProgressType * OperationProgress,
    bool AutoResume,
    const TOverwriteFileParams * FileParams,
    const TCopyParamType * CopyParam,
    TOverwriteMode & OverwriteMode);
  void ReadDirectoryProgress(int64_t Bytes);
  void ResetFileTransfer();
  virtual void DoFileTransferProgress(int64_t TransferSize, int64_t Bytes);
  virtual void FileTransferProgress(int64_t TransferSize, int64_t Bytes) override;
  void ResetCaches();
  void CaptureOutput(UnicodeString Str);
  void DoReadDirectory(TRemoteFileList * FileList);
  void DoReadFile(UnicodeString AFileName, TRemoteFile *& AFile);
  void FileTransfer(UnicodeString AFileName, UnicodeString LocalFile,
    UnicodeString RemoteFile, UnicodeString RemotePath, bool Get,
    int64_t Size, intptr_t Type, TFileTransferData & UserData,
    TFileOperationProgressType * OperationProgress);
  TDateTime ConvertLocalTimestamp(time_t Time);
  void RemoteFileTimeToDateTimeAndPrecision(const TRemoteFileTime & Source,
    TDateTime & DateTime, TModificationFmt & ModificationFmt) const;
  void SetLastCode(intptr_t Code);
  void StoreLastResponse(UnicodeString Text);
  void SetCPSLimit(TFileOperationProgressType * OperationProgress);
  bool VerifyCertificateHostName(const TFtpsCertificateData & Data);
  bool SupportsReadingFile() const;
  void AutoDetectTimeDifference(TRemoteFileList * FileList);
  void AutoDetectTimeDifference(UnicodeString Directory, const TCopyParamType * CopyParam, intptr_t Params);
  void ApplyTimeDifference(TRemoteFile * File);
  void ApplyTimeDifference(
    UnicodeString FileName, TDateTime & Modification, TModificationFmt & ModificationFmt);
  void DummyReadDirectory(UnicodeString Directory);
  bool IsEmptyFileList(TRemoteFileList * FileList) const;
  void CheckTimeDifference();
  inline bool NeedAutoDetectTimeDifference() const;
  bool LookupUploadModificationTime(
    UnicodeString FileName, TDateTime & Modification, TModificationFmt ModificationFmt);
  UnicodeString DoCalculateFileChecksum(bool UsingHashCommand, UnicodeString Alg, TRemoteFile * File);
  void DoCalculateFilesChecksum(bool UsingHashCommand, UnicodeString Alg,
    TStrings * FileList, TStrings * Checksums,
    TCalculatedChecksumEvent OnCalculatedChecksum,
    TFileOperationProgressType * OperationProgress, bool FirstLevel);
  void HandleFeatReply();
  void ResetFeatures();
  bool SupportsSiteCommand(UnicodeString Command) const;
  bool SupportsCommand(UnicodeString Command) const;
  void RegisterChecksumAlgCommand(UnicodeString Alg, UnicodeString Command);
  void SendCommand(UnicodeString Command);
  bool CanTransferSkipList(intptr_t Params, uintptr_t Flags, const TCopyParamType * CopyParam) const;

  static bool Unquote(UnicodeString & Str);

private:
  enum TCommand
  {
    CMD_UNKNOWN,
    PASS,
    SYST,
    FEAT
  };

  mutable TFileZillaIntf * FFileZillaIntf;
  TCriticalSection FQueueCriticalSection;
  TCriticalSection FTransferStatusCriticalSection;
  TMessageQueue * FQueue;
  HANDLE FQueueEvent;
  TSessionInfo FSessionInfo;
  TFileSystemInfo FFileSystemInfo;
  bool FFileSystemInfoValid;
  uintptr_t FReply;
  uintptr_t FCommandReply;
  TCommand FLastCommand;
  bool FPasswordFailed;
  bool FStoredPasswordTried;
  bool FMultineResponse;
  intptr_t FLastCode;
  intptr_t FLastCodeClass;
  intptr_t FLastReadDirectoryProgress;
  UnicodeString FTimeoutStatus;
  UnicodeString FDisconnectStatus;
  TStrings * FLastResponse;
  TStrings * FLastErrorResponse;
  TStrings * FLastError;
  UnicodeString FSystem;
  TStrings * FFeatures;
  UnicodeString FCurrentDirectory;
  bool FReadCurrentDirectory;
  UnicodeString FHomeDirectory;
  TRemoteFileList * FFileList;
  TRemoteFileList * FFileListCache;
  UnicodeString FFileListCachePath;
  UnicodeString FWelcomeMessage;
  bool FActive;
  bool FOpening;
  bool FWaitingForReply;
  enum
  {
    ftaNone,
    ftaSkip,
    ftaCancel,
  } FFileTransferAbort;
  bool FIgnoreFileList;
  bool FFileTransferCancelled;
  int64_t FFileTransferResumed;
  bool FFileTransferPreserveTime;
  bool FFileTransferRemoveBOM;
  bool FFileTransferNoList;
  uintptr_t FFileTransferCPSLimit;
  bool FAwaitingProgress;
  TCaptureOutputEvent FOnCaptureOutput;
  UnicodeString FUserName;
  TAutoSwitch FListAll;
  bool FDoListAll;
  TFTPServerCapabilities * FServerCapabilities;
  TDateTime FLastDataSent;
  bool FDetectTimeDifference;
  int64_t FTimeDifference;
  std::unique_ptr<TStrings> FChecksumAlgs;
  std::unique_ptr<TStrings> FChecksumCommands;
  std::unique_ptr<TStrings> FSupportedCommands;
  std::unique_ptr<TStrings> FSupportedSiteCommands;
  std::unique_ptr<TStrings> FHashAlgs;
  typedef rde::map<UnicodeString, TDateTime> TUploadedTimes;
  TUploadedTimes FUploadedTimes;
  bool FSupportsAnyChecksumFeature;
  UnicodeString FLastCommandSent;
  X509 * FCertificate;
  EVP_PKEY * FPrivateKey;
  bool FTransferActiveImmediately;
  bool FWindowsServer;
  int64_t FBytesAvailable;
  bool FBytesAvailableSupported;
  bool FMVS;
  bool FVMS;
  bool FFileTransferAny;
  mutable UnicodeString FOptionScratch;
};

UnicodeString GetOpenSSLVersionText();

#endif // NO_FILEZILLA

