
#pragma once

#include <FileSystems.h>

typedef int32_t SSH_FX_TYPES;
typedef uint32_t SSH_FXP_TYPES;
typedef uint32_t SSH_FILEXFER_ATTR_TYPES;
typedef uint8_t SSH_FILEXFER_TYPES;
typedef uint32_t SSH_FXF_TYPES;
typedef uint32_t ACE4_TYPES;

class TSFTPPacket;
struct TOverwriteFileParams;
struct TSFTPSupport;
class TSecureShell;

#if 0
enum TSFTPOverwriteMode { omOverwrite, omAppend, omResume };
#endif // #if 0

class TSFTPFileSystem : public TCustomFileSystem
{
NB_DISABLE_COPY(TSFTPFileSystem)
friend class TSFTPPacket;
friend class TSFTPQueue;
friend class TSFTPAsynchronousQueue;
friend class TSFTPUploadQueue;
friend class TSFTPDownloadQueue;
friend class TSFTPLoadFilesPropertiesQueue;
friend class TSFTPCalculateFilesChecksumQueue;
friend class TSFTPBusy;
public:
  static inline bool classof(const TObject * Obj)
  {
    return
      Obj->GetKind() == OBJECT_CLASS_TSFTPFileSystem;
  }
public:
  explicit TSFTPFileSystem(TTerminal * ATerminal);
  virtual ~TSFTPFileSystem();

  virtual void Init(void * Data /*TSecureShell* */);
  virtual void FileTransferProgress(int64_t /*TransferSize*/, int64_t /*Bytes*/) {}

  virtual void Open();
  virtual void Close();
  virtual bool GetActive() const;
  virtual void CollectUsage();
  virtual void Idle();
  virtual UnicodeString GetAbsolutePath(UnicodeString APath, bool Local) override;
  virtual UnicodeString GetAbsolutePath(UnicodeString APath, bool Local) const override;
  virtual void AnyCommand(UnicodeString Command,
    TCaptureOutputEvent OutputEvent) override;
  virtual void ChangeDirectory(UnicodeString Directory) override;
  virtual void CachedChangeDirectory(UnicodeString Directory) override;
  virtual void AnnounceFileListOperation() override;
  virtual void ChangeFileProperties(UnicodeString AFileName,
    const TRemoteFile * AFile, const TRemoteProperties * AProperties,
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
    UnicodeString TargetDir, const TCopyParamType * CopyParam,
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
  TSecureShell * FSecureShell;
  TFileSystemInfo FFileSystemInfo;
  bool FFileSystemInfoValid;
  intptr_t FVersion;
  UnicodeString FCurrentDirectory;
  UnicodeString FDirectoryToChangeTo;
  UnicodeString FHomeDirectory;
  AnsiString FEOL;
  TList * FPacketReservations;
  rde::vector<uintptr_t> FPacketNumbers;
  SSH_FXP_TYPES FPreviousLoggedPacket;
  int FNotLoggedPackets;
  int FBusy;
  void * FBusyToken;
  bool FAvoidBusy;
  TStrings * FExtensions;
  TSFTPSupport * FSupport;
  TAutoSwitch FUtfStrings;
  bool FUtfDisablingAnnounced;
  bool FSignedTS;
  TStrings * FFixedPaths;
  uint32_t FMaxPacketSize;
  bool FSupportsStatVfsV2;
  uintptr_t FCodePage;
  bool FSupportsHardlink;
  std::unique_ptr<TStringList> FChecksumAlgs;
  std::unique_ptr<TStringList> FChecksumSftpAlgs;

  void SendCustomReadFile(TSFTPPacket * Packet, TSFTPPacket * Response,
    uint32_t Flags);
  void CustomReadFile(UnicodeString AFileName,
    TRemoteFile *& AFile, SSH_FXP_TYPES Type, TRemoteFile * ALinkedByFile = nullptr,
    SSH_FX_TYPES AllowStatus = -1);
  virtual UnicodeString RemoteGetCurrentDirectory() const override;
  UnicodeString GetHomeDirectory();
  SSH_FX_TYPES GotStatusPacket(TSFTPPacket * Packet, SSH_FX_TYPES AllowStatus);
  bool RemoteFileExists(UnicodeString FullPath, TRemoteFile ** AFile = nullptr);
  TRemoteFile * LoadFile(TSFTPPacket * Packet,
    TRemoteFile * ALinkedByFile, UnicodeString AFileName,
    TRemoteFileList * TempFileList = nullptr, bool Complete = true);
  void LoadFile(TRemoteFile * AFile, TSFTPPacket * Packet,
    bool Complete = true);
  UnicodeString LocalCanonify(UnicodeString APath) const;
  UnicodeString Canonify(UnicodeString APath);
  UnicodeString GetRealPath(UnicodeString APath);
  UnicodeString GetRealPath(UnicodeString APath, UnicodeString ABaseDir);
  void ReserveResponse(const TSFTPPacket * Packet,
    TSFTPPacket * Response);
  SSH_FX_TYPES ReceivePacket(TSFTPPacket * Packet, SSH_FXP_TYPES ExpectedType = -1,
    SSH_FX_TYPES AllowStatus = -1, bool TryOnly = false);
  bool PeekPacket();
  void RemoveReservation(intptr_t Reservation);
  void SendPacket(const TSFTPPacket * Packet);
  SSH_FX_TYPES ReceiveResponse(const TSFTPPacket * Packet,
    TSFTPPacket * AResponse, SSH_FXP_TYPES ExpectedType = -1, SSH_FX_TYPES AllowStatus = -1, bool TryOnly = false);
  SSH_FX_TYPES SendPacketAndReceiveResponse(const TSFTPPacket * Packet,
    TSFTPPacket * Response, SSH_FXP_TYPES ExpectedType = -1, SSH_FX_TYPES AllowStatus = -1);
  void UnreserveResponse(TSFTPPacket * Response);
  void TryOpenDirectory(UnicodeString Directory);
  bool SupportsExtension(UnicodeString Extension) const;
  void ResetConnection();
  void DoCalculateFilesChecksum(
    UnicodeString Alg, UnicodeString SftpAlg,
    TStrings * AFileList, TStrings * Checksums,
    TCalculatedChecksumEvent OnCalculatedChecksum,
    TFileOperationProgressType * OperationProgress, bool FirstLevel);
  void RegisterChecksumAlg(UnicodeString Alg, UnicodeString SftpAlg);
  void DoDeleteFile(UnicodeString AFileName, SSH_FXP_TYPES Type);

  void SFTPSourceRobust(UnicodeString AFileName,
    const TRemoteFile * AFile,
    UnicodeString TargetDir, const TCopyParamType * CopyParam, intptr_t Params,
    TFileOperationProgressType * OperationProgress, uintptr_t Flags);
  void SFTPSource(UnicodeString AFileName,
    const TRemoteFile * AFile,
    UnicodeString TargetDir, const TCopyParamType * CopyParam, intptr_t Params,
    TOpenRemoteFileParams & OpenParams,
    TOverwriteFileParams & FileParams,
    TFileOperationProgressType * OperationProgress, uintptr_t Flags,
    TUploadSessionAction & Action, bool & ChildError);
  RawByteString SFTPOpenRemoteFile(UnicodeString AFileName,
    SSH_FXF_TYPES OpenType, int64_t Size = -1);
  intptr_t SFTPOpenRemote(void * AOpenParams, void * Param2);
  void SFTPCloseRemote(const RawByteString & Handle,
    UnicodeString AFileName, TFileOperationProgressType * OperationProgress,
    bool TransferFinished, bool Request, TSFTPPacket * Packet);
  void SFTPDirectorySource(UnicodeString DirectoryName,
    UnicodeString TargetDir, uintptr_t LocalFileAttrs, const TCopyParamType * CopyParam,
    intptr_t Params, TFileOperationProgressType * OperationProgress, uintptr_t Flags);
  void SFTPConfirmOverwrite(UnicodeString ASourceFullFileName, UnicodeString & ATargetFileName,
    const TCopyParamType * CopyParam, intptr_t AParams, TFileOperationProgressType * OperationProgress,
    const TOverwriteFileParams * FileParams,
    OUT TOverwriteMode & OverwriteMode);
  bool SFTPConfirmResume(UnicodeString DestFileName, bool PartialBiggerThanSource,
    TFileOperationProgressType * OperationProgress);
  void SFTPSinkRobust(UnicodeString AFileName,
    const TRemoteFile * AFile, UnicodeString TargetDir,
    const TCopyParamType * CopyParam, intptr_t Params,
    TFileOperationProgressType * OperationProgress, uintptr_t Flags);
  void SFTPSink(UnicodeString AFileName,
    const TRemoteFile * AFile, UnicodeString TargetDir,
    const TCopyParamType * CopyParam, intptr_t Params,
    TFileOperationProgressType * OperationProgress, uintptr_t Flags,
    TDownloadSessionAction & Action, bool & ChildError);
  void SFTPSinkFile(UnicodeString AFileName,
    const TRemoteFile * AFile, void * Param);
  char * GetEOL() const;
  inline void BusyStart();
  inline void BusyEnd();
  uint32_t TransferBlockSize(uint32_t Overhead,
    TFileOperationProgressType * OperationProgress,
    uint32_t MinPacketSize = 0,
    uint32_t MaxPacketSize = 0) const;
  uint32_t UploadBlockSize(const RawByteString & Handle,
    TFileOperationProgressType * OperationProgress) const;
  uint32_t DownloadBlockSize(
    TFileOperationProgressType * OperationProgress) const;
  intptr_t PacketLength(uint8_t * LenBuf, SSH_FXP_TYPES ExpectedType) const;
  void Progress(TFileOperationProgressType * OperationProgress);

private:
  const TSessionData * GetSessionData() const;
};

