
#pragma once

#include "FileZillaIntern.h"

class CApiLog : public TObject
{
public:
  static bool classof(const TObject * Obj)
  {
    TObjectClassId Kind = Obj->GetKind();
    return
      Kind == OBJECT_CLASS_CApiLog;
  }
public:
  CApiLog();
  virtual ~CApiLog();

  void InitIntern(TFileZillaIntern * Intern);
  TFileZillaIntern * GetIntern();

  bool LoggingMessageType(int nMessageType) const;

  void LogMessage(int nMessageType, LPCTSTR pMsgFormat, ...) const;
  void LogMessageRaw(int nMessageType, LPCTSTR pMsg) const;

  CString GetOption(int OptionID) const;
  int GetOptionVal(int OptionID) const;

protected:
  void SendLogMessage(int nMessageType, LPCTSTR pMsg) const;

  TFileZillaIntern * FIntern;
};
