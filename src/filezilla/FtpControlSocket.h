
#pragma once

#include "structures.h"
#include "stdafx.h"
#include "FileZillaApi.h"
#include "FileZillaIntf.h"

class CTransferSocket;
class CMainThread;

class CAsyncProxySocketLayer;
class CMainThread;

#define CSMODE_NONE             0x0000
#define CSMODE_CONNECT          0x0001
#define CSMODE_COMMAND          0x0002
#define CSMODE_LIST             0x0004
#define CSMODE_TRANSFER         0x0008
#define CSMODE_DOWNLOAD         0x0010
#define CSMODE_UPLOAD           0x0020
#define CSMODE_TRANSFERERROR    0x0040
#define CSMODE_TRANSFERTIMEOUT  0x0080
#define CSMODE_DELETE           0x0100
#define CSMODE_RMDIR            0x0200
#define CSMODE_DISCONNECT       0x0400
#define CSMODE_MKDIR            0x0800
#define CSMODE_RENAME           0x1000
#define CSMODE_CHMOD            0x2000
#define CSMODE_LISTFILE         0x4000

struct t_transferdata
{
CUSTOM_MEM_ALLOCATION_IMPL
  t_transferdata() :
    transfersize(0), transferleft(0),
    localFileHandle(INVALID_HANDLE_VALUE),
    bResume(FALSE), bResumeAppend(FALSE), bType(FALSE)
  {
  }
  int64_t transfersize, transferleft;
  HANDLE localFileHandle;
  BOOL bResume, bResumeAppend, bType;
};

class CFtpControlSocket : public CAsyncSocketEx, public CApiLog
{
  friend class CTransferSocket;

public:
  CFtpControlSocket(CMainThread * pMainThread, CFileZillaTools * pTools);
  virtual ~CFtpControlSocket();

public:
  virtual void Connect(t_server & server);
  virtual void OnTimer();
  virtual BOOL IsReady();
  virtual void List(BOOL bFinish, int nError = 0, CServerPath path = CServerPath(), CString subdir = L"", int nListMode = 0);
  virtual void ListFile(const CString & filename, const CServerPath & path);
  virtual void FtpCommand(LPCTSTR pCommand);
  virtual void Disconnect();
  virtual void FileTransfer(t_transferfile * transferfile = 0, BOOL bFinish = FALSE, int nError = 0);
  virtual void Delete(const CString & filename, const CServerPath & path, bool filenameOnly);
  virtual void Rename(const CString & oldName, const CString & newName, const CServerPath & path, const CServerPath & newPath);
  virtual void MakeDir(const CServerPath & path);
  virtual void RemoveDir(const CString & dirname, const CServerPath & path);
  virtual void Chmod(const CString & filename, const CServerPath & path, int nValue);

  virtual void ProcessReply();
  virtual void TransferEnd(int nMode);
  virtual void Cancel(BOOL bQuit = FALSE);

  virtual void SetAsyncRequestResult(int nAction, CAsyncRequestData * pData);
  
  int CheckOverwriteFile();
  virtual BOOL Create();
  void TransfersocketListenFinished(unsigned int ip, unsigned short port);

  BOOL m_bKeepAliveActive;
  BOOL m_bDidRejectCertificate;

  // Some servers are broken. Instead of an empty listing, some MVS servers
  // for example they return something "550 no members found"
  // Other servers return "550 No files found."
  bool IsMisleadingListResponse();

  virtual bool UsingMlsd();
  virtual bool UsingUtf8();
  virtual std::string GetTlsVersionStr();
  virtual std::string GetCipherName();
  bool HandleSize(int code, int64_t & size);
  bool HandleMdtm(int code, t_directory::t_direntry::t_date & date);
  void TransferHandleListError();

  enum transferDirection
  {
    download = 0,
    upload = 1
  };

  BOOL RemoveActiveTransfer();
  BOOL SpeedLimitAddTransferredBytes(enum transferDirection direction, _int64 nBytesTransferred);

  _int64 GetSpeedLimit(enum transferDirection direction, CTime & time);

  _int64 GetAbleToTransferSize(enum transferDirection direction, bool &beenWaiting, int nBufSize = 0);

  t_server GetCurrentServer();

public:
  virtual void OnReceive(int nErrorCode);
  virtual void OnConnect(int nErrorCode);
  virtual void OnClose(int nErrorCode);
  virtual void OnSend(int nErrorCode);

protected:
  // Called by OnTimer()
  void ResumeTransfer();
  void CheckForTimeout();
  void SendKeepAliveCommand();

  virtual int OnLayerCallback(rde::list<t_callbackMsg> & callbacks);
  void SetFileExistsAction(int nAction, COverwriteRequestData * pData);
  void SetVerifyCertResult(int nResult, t_SslCertData * pData);
  void ResetOperation(int nSuccessful = -1);

  virtual void DoClose(int nError = 0);
  int GetReplyCode();
  CString GetReply();
  void LogOnToServer(BOOL bSkipReply = FALSE);
  BOOL Send(CString str);

  BOOL ParsePwdReply(CString & rawpwd);
  BOOL ParsePwdReply(CString & rawpwd, CServerPath & realPath);
  BOOL SendAuthSsl();

  void DiscardLine(CStringA line);
  int FileTransferListState(bool get);
  bool NeedModeCommand();
  bool NeedOptsCommand();
  CString GetListingCmd();

  bool InitConnect();
  int InitConnectState();

  bool IsRoutableAddress(const CString & host);
  bool CheckForcePasvIp(CString & host);
  void TransferFinished(bool preserveFileTimeForUploads);

  virtual void LogSocketMessageRaw(int nMessageType, LPCTSTR pMsg);
  virtual bool LoggingSocketMessage(int nMessageType);

  void ShowStatus(UINT nID, int type) const;
  void ShowStatus(CString status,int type) const;
  void ShowTimeoutError(UINT nID) const;

  void Close();
  BOOL Connect(CString hostAddress, UINT nHostPort);
  CString ConvertDomainName(CString domain);
  bool ConnectTransferSocket(const CString & host, UINT port);

  struct t_ActiveList
  {
  CUSTOM_MEM_ALLOCATION_IMPL
    CFtpControlSocket * pOwner;
    int64_t nBytesAvailable;
    int64_t nBytesTransferred;
  };
  static rde::list<t_ActiveList> m_InstanceList[2];
  static CTime m_CurrentTransferTime[2];
  static _int64 m_CurrentTransferLimit[2];
  static CCriticalSection m_SpeedLimitSync;
  _int64 GetAbleToUDSize(bool & beenWaiting, CTime & curTime, _int64 & curLimit, rde::list<t_ActiveList>::iterator & iter, enum transferDirection direction, int nBufSize);
  _int64 GetSpeedLimit(CTime & time, int valType, int valValue);

  void SetDirectoryListing(t_directory * pDirectory, bool bSetWorkingDir = true);
  t_directory * m_pDirectoryListing;

  CMainThread * m_pOwner;
  CFileZillaTools * m_pTools;

  CFile * m_pDataFile;
  CTransferSocket * m_pTransferSocket;
  CStringA m_MultiLine;
  CTime m_LastSendTime;

  CString m_ServerName;
  rde::list<CStringA> m_RecvBuffer;
  CTime m_LastRecvTime;
  class CLogonData;
  class CListData;
  class CListFileData;
  class CFileTransferData;
  class CMakeDirData;

#ifndef MPEXT_NO_ZLIB
  bool m_useZlib;
  bool m_zlibSupported;
  int m_zlibLevel;
#endif

  bool m_bUTF8;
  bool m_bAnnouncesUTF8;
  int m_nCodePage;
  bool m_hasClntCmd;
  TFTPServerCapabilities m_serverCapabilities;
  CStringA m_ListFile;
  int64_t m_ListFileSize;
  bool m_isFileZilla;

  bool m_awaitsReply;
  bool m_skipReply;

  char * m_sendBuffer;
  size_t m_sendBufferLen;

  bool m_bProtP;

  bool m_mayBeMvsFilesystem;
  bool m_mayBeBS2000Filesystem;

  struct t_operation
  {
  CUSTOM_MEM_ALLOCATION_IMPL
    int nOpMode;
    int nOpState;
    class COpData //: public TObject //Base class which will store operation specific parameters.
    {
    CUSTOM_MEM_ALLOCATION_IMPL
    public:
      COpData() {}
      virtual ~COpData() {}
    };
    COpData * pData;
  };

  t_operation m_Operation;

  CAsyncProxySocketLayer * m_pProxyLayer;
  CAsyncSslSocketLayer * m_pSslLayer;
#ifndef MPEXT_NO_GSS
  CAsyncGssSocketLayer * m_pGssLayer;
#endif
  t_server m_CurrentServer;

private:
  BOOL m_bCheckForTimeout;
};

