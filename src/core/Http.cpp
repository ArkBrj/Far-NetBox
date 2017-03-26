
#include <vcl.h>
#pragma hdrstop

#include <ne_request.h>
#include <openssl/ssl.h>

#include "Http.h"
#include "NeonIntf.h"
#include "Exceptions.h"
#include "TextsCore.h"

THttp::THttp() :
  FProxyPort(0),
  FResponseLimit(-1),
  FOnDownload(nullptr),
  FOnError(nullptr),
  FRequestHeaders(nullptr),
  FResponseHeaders(new TStringList())
{
}

THttp::~THttp()
{
  delete FResponseHeaders;
  delete FRequestHeaders;
}

void THttp::SendRequest(const char * Method, const UnicodeString & Request)
{
  std::unique_ptr<TStringList> AttemptedUrls(CreateSortedStringList());
  AttemptedUrls->Add(GetURL());
  UnicodeString RequestUrl = GetURL();
  bool WasTlsUri = false; // shut up

  bool Retry;

  do
  {
    ne_uri uri;
    NeonParseUrl(RequestUrl, uri);

    bool IsTls = IsTlsUri(uri);
    if (RequestUrl == GetURL())
    {
      WasTlsUri = IsTls;
    }
    else
    {
      if (!IsTls && WasTlsUri)
      {
        throw Exception(LoadStr(UNENCRYPTED_REDIRECT));
      }
    }

    FHostName = StrFromNeon(uri.host);

    UnicodeString Uri = StrFromNeon(uri.path);
    if (uri.query != nullptr)
    {
      Uri += L"?" + StrFromNeon(uri.query);
    }

    FResponse.SetLength(0);
    FCertificateError.SetLength(0);
    FException.reset(nullptr);

    TProxyMethod ProxyMethod = GetProxyHost().IsEmpty() ? ::pmNone : pmHTTP;

    ne_session_s * NeonSession =
      CreateNeonSession(
        uri, ProxyMethod, GetProxyHost(), GetProxyPort(), UnicodeString(), UnicodeString());

    try__finally
    {
      SCOPE_EXIT
      {
        DestroyNeonSession(NeonSession);
        ne_uri_free(&uri);
      };

      if (IsTls)
      {
        SetNeonTlsInit(NeonSession, InitSslSession);

        ne_ssl_set_verify(NeonSession, NeonServerSSLCallback, this);

        ne_ssl_trust_default_ca(NeonSession);
      }

      ne_request_s * NeonRequest = ne_request_create(NeonSession, Method, StrToNeon(Uri));
      try__finally
      {
        SCOPE_EXIT
        {
          ne_request_destroy(NeonRequest);
        };

        if (FRequestHeaders != nullptr)
        {
          for (intptr_t Index = 0; Index < FRequestHeaders->GetCount(); Index++)
          {
            ne_add_request_header(
              NeonRequest, StrToNeon(FRequestHeaders->GetName(Index)), StrToNeon(FRequestHeaders->GetValueFromIndex(Index)));
          }
        }

        UTF8String RequestUtf;
        if (!Request.IsEmpty())
        {
          RequestUtf = UTF8String(Request);
          ne_set_request_body_buffer(NeonRequest, RequestUtf.c_str(), RequestUtf.Length());
        }

        ne_add_response_body_reader(NeonRequest, ne_accept_2xx, NeonBodyReader, this);

        int Status = ne_request_dispatch(NeonRequest);

        // Exception has precedence over status as status will always be NE_ERROR,
        // as we returned 1 from NeonBodyReader
        if (FException.get() != nullptr)
        {
          RethrowException(FException.get());
        }

        if (Status == NE_REDIRECT)
        {
          Retry = true;
          RequestUrl = GetNeonRedirectUrl(NeonSession);
          CheckRedirectLoop(RequestUrl, AttemptedUrls.get());
        }
        else
        {
          Retry = false;
          CheckNeonStatus(NeonSession, Status, FHostName, FCertificateError);

          const ne_status * NeonStatus = ne_get_status(NeonRequest);
          if (NeonStatus->klass != 2)
          {
            int StatusCode = NeonStatus->code;
            UnicodeString Message = StrFromNeon(NeonStatus->reason_phrase);
            if (GetOnError() != nullptr)
            {
              GetOnError()(this, StatusCode, Message);
            }
            throw Exception(FMTLOAD(HTTP_ERROR2, StatusCode, Message.c_str(), FHostName.c_str()));
          }

          void * Cursor = nullptr;
          const char * HeaderName;
          const char * HeaderValue;
          while ((Cursor = ne_response_header_iterate(NeonRequest, Cursor, &HeaderName, &HeaderValue)) != nullptr)
          {
            FResponseHeaders->SetValue(StrFromNeon(HeaderName), StrFromNeon(HeaderValue));
          }
        }
      }
      __finally
      {
/*
        ne_request_destroy(NeonRequest);
*/
      };
    }
    __finally
    {
/*
      DestroyNeonSession(NeonSession);
      ne_uri_free(&uri);
*/
    };
  }
  while (Retry);
}

void THttp::Get()
{
  SendRequest("GET", UnicodeString());
}

void THttp::Post(const UnicodeString & Request)
{
  SendRequest("POST", Request);
}

UnicodeString THttp::GetResponse() const
{
  UTF8String UtfResponse(FResponse.c_str());
  return UnicodeString(UtfResponse);
}

int THttp::NeonBodyReaderImpl(const char * Buf, size_t Len)
{
  bool Result = true;
  if ((FResponseLimit < 0) ||
      (FResponse.Length() + static_cast<intptr_t>(Len) <= FResponseLimit))
  {
    FResponse += RawByteString(Buf, Len);

    if (FOnDownload != nullptr)
    {
      bool Cancel = false;

      try
      {
        FOnDownload(this, GetResponseLength(), Cancel);
      }
      catch (Exception & E)
      {
        FException.reset(CloneException(&E));
        Result = false;
      }

      if (Cancel)
      {
        FException.reset(new EAbort(UnicodeString()));
        Result = false;
      }
    }
  }

  // neon wants 0 for success
  return Result ? 0 : 1;
}

int THttp::NeonBodyReader(void * UserData, const char * Buf, size_t Len)
{
  THttp * Http = static_cast<THttp *>(UserData);
  return Http->NeonBodyReaderImpl(Buf, Len);
}

int64_t THttp::GetResponseLength() const
{
  return FResponse.Length();
}

void THttp::InitSslSession(ssl_st * Ssl, ne_session * /*Session*/)
{
  int Options = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1;
  SSL_ctrl(Ssl, SSL_CTRL_OPTIONS, Options, nullptr);
}

int THttp::NeonServerSSLCallback(void * UserData, int Failures, const ne_ssl_certificate * Certificate)
{
  THttp * Http = static_cast<THttp *>(UserData);
  return Http->NeonServerSSLCallbackImpl(Failures, Certificate);
}

int THttp::NeonServerSSLCallbackImpl(int Failures, const ne_ssl_certificate * Certificate)
{
  AnsiString AsciiCert = NeonExportCertificate(Certificate);

  UnicodeString WindowsCertificateError;
  if (Failures != 0)
  {
    NeonWindowsValidateCertificate(Failures, AsciiCert, WindowsCertificateError);
  }

  if (Failures != 0)
  {
    FCertificateError = NeonCertificateFailuresErrorStr(Failures, FHostName);
    AddToList(FCertificateError, WindowsCertificateError, L"\n");
  }

  return (Failures == 0) ? NE_OK : NE_ERROR;
}

bool THttp::IsCertificateError() const
{
  return !FCertificateError.IsEmpty();
}

