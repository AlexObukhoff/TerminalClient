//---------------------------------------------------------------------------

#include <vcl.h>
#include <memory>
#include <vector>
#pragma hdrstop

#include "TConnectThread.h"
#define ZLIB_WINAPI
#include "gz_stream.h"
#include "globals.h"
#include "boost/format.hpp"
#include "zlib.h"
#include <assert.h>
#include <string>
#include <sstream>
#pragma package(smart_init)
//---------------------------------------------------------------------------

//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall TConnectThread::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------
using namespace std;

__fastcall TConnectThread::TConnectThread(TLogClass *_Log, TWConfig *_Cfg, AnsiString _URL, AnsiString _SignedMessage, const int keyID, AnsiString _DetachedSignature)
		: TThread(true), m_keyID(keyID)
{
    try
    {
        CoInitializeEx(NULL, COINIT_MULTITHREADED);
        FreeOnTerminate = false;
        Finished=false;
        InnerLog=false;
        if (_Log==NULL)
        {
            Log = new TLogClass("TConnectThread");
            InnerLog=true;
        }
        else
        {
            Log=_Log;
        }
        //Log->Write("TConnectThread.Init started...");
        Cfg=_Cfg;
        URL=_URL;
        SignedMessage=_SignedMessage;
        DetachedSignature=_DetachedSignature;
        IndyError = false;
        AnswerContentLength=0;
        ConnectOK = false;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

__fastcall TConnectThread::~TConnectThread(void)
{
    if (InnerLog)
    {
        delete Log;
        Log = NULL;
    }
}

//---------------------------------------------------------------------------

void __fastcall TConnectThread::Execute()
{
    try
    {
        Process();
        Finished=true;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

//---------------------------------------------------------------------------

void TConnectThread::Process()
{
//Log->Write("Message: "+PrepareString(SignedMessage));
    bool bOK = true;
    try
    {
        TransportError = false;
        ConnectResult = "";
        if (Terminated)
            return;

        std::auto_ptr <TMemoryStream> Data;
        std::auto_ptr <TMemoryStream> Answer;
        std::auto_ptr <TIdHTTP> IdHTTPC;
        std::auto_ptr <TIdConnectionInterceptOpenSSL> IdSSL;

        try
        {
            IdHTTPC = std::auto_ptr <TIdHTTP> ( new TIdHTTP(Application) );
            Data = std::auto_ptr <TMemoryStream> ( new TMemoryStream() );
            Answer = std::auto_ptr <TMemoryStream> ( new TMemoryStream() );
        }
        catch(Exception& error)
        {
            IndyError=true;
            Log->Write((boost::format("TConnectThread::Process: Init Exception: %1%") % error.Message.c_str()).str().c_str());
            bOK=false;
            ConnectResult += "\r\nInit Exception: "+AnsiString(error.Message);
        }
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
            bOK = false;
        }

        if ((bOK)&&(!Terminated))
        {
            try
            {
                if (URL.Pos("https://")!=0)
                {
                    IdSSL = std::auto_ptr <TIdConnectionInterceptOpenSSL> (new TIdConnectionInterceptOpenSSL(Application));
                    if(!IdSSL.get())
                    {
                        IndyError=true;
                        Log->Write("TConnectThread::Process: Error creating object TIdConnectionInterceptOpenSSL.");
                        bOK=false;
                        ConnectResult += "\r\nError creating object(s).";
                        throw Exception("Error creating object TIdConnectionInterceptOpenSSL");
                    }
                    IdHTTPC->Intercept = IdSSL.get();
                    IdHTTPC->InterceptEnabled=true;
                    IdHTTPC->Port=443;
                }
                else
                {
                    IdHTTPC->Port=80;
                }

                if (Cfg->Connection().HTTPProxy.Type=="http")
                {
                    IdHTTPC->Request->ProxyServer=Cfg->Connection().HTTPProxy.Host.c_str();
                    IdHTTPC->Request->ProxyPort=Cfg->Connection().HTTPProxy.Port;
                    IdHTTPC->Request->ProxyUsername=Cfg->Connection().HTTPProxy.UserName.c_str();
                    IdHTTPC->Request->ProxyPassword=Cfg->Connection().HTTPProxy.Password.c_str();
                }
                else
                {
                    if (Cfg->Connection().HTTPProxy.Type=="socks")
                        IdHTTPC->SocksInfo->Assign(Cfg->Connection().Proxy);
                }
            }
            catch(Exception& error)
            {
                IndyError=true;
                Log->Write((boost::format("TConnectThread::Process: ProxyInit Exception: %1%") % error.Message.c_str()).str().c_str());
                bOK=false;
                ConnectResult += "\r\nProxy Init Exception: "+AnsiString(error.Message);
            }
            catch(...)
            {
                ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                bOK=false;
            }
        }

        if((!Data.get())||(!Answer.get())||(!IdHTTPC.get()))
        {
            IndyError=true;
            Log->Write("TConnectThread::Process: Error creating object(s).");
            bOK=false;
            ConnectResult += "\r\nError creating object(s).";
        }

        if ((bOK)&&(!Terminated))
        {
            if (DetachedSignature == "")
            {
              //SignedMessage = ChangeChars(SignedMessage,"+","%2b");
              std::string strSignedMessage = SignedMessage.c_str();
              SignedMessage = encodeLocaleString(strSignedMessage, "=").c_str();
            }

            try
            {
                Data->SetSize(SignedMessage.Length());
                Data->Clear();
                Data->Write(SignedMessage.c_str(), SignedMessage.Length());
            }
            catch(Exception& error)
            {
                Log->Write((boost::format("TConnectThread::Process: TMemoryStream Exception: %1%") % error.Message.c_str()).str().c_str());
                bOK=false;
                ConnectResult += "\r\nTMemoryStream Exception: "+AnsiString(error.Message);
            }
            catch(...)
            {
                ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                bOK = false;
            }
        }
        ConnectResult += "\r\nConnecting to "+URL+"...";
        //Log->Write("  Connecting to "+URL+"...");

        if ((bOK)&&(!Terminated))
        {
            try
            {
                _keys_info CurrentKeys = Cfg->Keys[Cfg->GetKeysNum(m_keyID)];
                IdHTTPC->Request->UserAgent = (boost::format("terminal module version: %1%, SD: %2%, AP: %3%, OP: %4%")
                      % FileVersion % CurrentKeys.SD % CurrentKeys.AP % CurrentKeys.OP).str().c_str();
                if (DetachedSignature!="")
                {
                    Log->Write("   Using detached signature...");
                    AnsiString XSignature = ChangeChars(ChangeChars(ChangeChars(ChangeChars(URLEncode(DetachedSignature),"+","%2b"),"\r\n","%0d%0a"),"=","%3d"),"/","%2f");
                    //std::string strDetachedSignature = DetachedSignature.c_str();
                    //AnsiString XSignature = encodeLocaleString(strDetachedSignature, "=").c_str();
                    IdHTTPC->Request->ExtraHeaders->Add("X-signature: "+XSignature+"");
                    /*
                    Log->Write("o.X-signature : { "+XSignature+" }");
                    Log->Write("o.DetachedSignature : { "+DetachedSignature+" }");
                    Log->Write("o.SignedMessage : { "+SignedMessage+" }");
                    */
                    IdHTTPC->Request->ContentType = "text/xml; charset=utf-8";
                    IdHTTPC->Request->UserAgent = "TRM_mt";
                }

                Log->Write((boost::format("   Posting data to %1%...") % URL.c_str()).str().c_str());

                IdHTTPC->Post(URL, Data.get(), Answer.get());
                int size=Data->Size;
                Log->Append((boost::format("OK, %1%B sent") % size).str().c_str());
                ConnectResult += "\r\nPost done...";
            }
            catch (EIdProtocolReplyError &ErrData)
            {
                Log->Append((boost::format(" Error: Server reply: #%1%: %2%.") % ErrData.ReplyErrorCode % ErrData.Message.c_str()).str().c_str());
                bOK=false;
                ConnectResult += "\r\nError: Server reply: #"+AnsiString(ErrData.ReplyErrorCode)+": "+ErrData.Message+".";
                IdHTTPC->Disconnect();
            }
            catch (Exception &exception)
            {
                Log->Append((boost::format(" Post exception: %1%") % exception.Message.c_str()).str().c_str());
                bOK=false;
                ConnectResult += "\r\nPost exception: "+AnsiString(exception.Message);
                TransportError = true;
                IdHTTPC->Disconnect();
            }
            catch (...)
            {
                ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                Log->Append(" exception: ?");
                bOK=false;
                ConnectResult += "\r\nexception: ?";
                IdHTTPC->Disconnect();
            }
        }

        if ((bOK)&&(!Terminated))
        {
            Log->Append((boost::format(", result: %1%") % IdHTTPC->ResponseText.c_str()).str().c_str());
            try
            {
                IdHTTPC->Disconnect();
            }
            catch(Exception& error)
            {
                Log->Write((boost::format("TConnectThread::Process: Close connection exception: %1%") % error.Message.c_str()).str().c_str());
                ConnectResult += "\r\nDisconnect exception: "+AnsiString(error.Message);
            }
            catch(...)
            {
                ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
            }
            ConnectOK = true;
            if (!Terminated)
            {
                if ((Answer->Size>0)&&(IdHTTPC->ResponseCode==200))
                {
                    long AnswerLength = Answer->Size;

                    std::vector<char> Buffer( AnswerLength+1, 0 );
                    Answer->Position=0;
                    Answer->Read(Buffer.begin(),AnswerLength);
                    if (0 != IdHTTPC->Response->ContentType.Pos("application/x-gzip"))
                    {
                        std::string m_answer;
                        m_answer.append(Buffer.begin(),AnswerLength);
                        stringstream output( ios_base::in | ios_base::out | ios_base::binary );
                        stringstream input(m_answer, ios_base::in | ios_base::out | ios_base::binary );

                        if( GZ::gz_decompress(input,output) )
                            m_answer = output.str();
                        AnswerMessage = AnsiString(m_answer.c_str(),m_answer.size());
                        int ratio = 100 - double(AnswerLength*100)/AnswerMessage.Length();
                        Log->Append((boost::format(", %1%B rcvd, msg size: %2%B, compression: %3%%%.") % AnswerLength % AnswerMessage.Length() % ratio).str().c_str());
                    }
                    else
                    {
                        AnswerMessage = AnsiString(Buffer.begin(), AnswerLength);
                        Log->Append((boost::format(", %1%B rcvd.") % AnswerMessage.Length()).str().c_str());
                    }

                    if (DetachedSignature!="")
                        DetachedSignature = URLDecode(IdHTTPC->Response->ExtraHeaders->Values["X-signature"]);

                    Answer->Clear();
                }
                ConnectResult += "\r\nPost result: "+AnsiString(IdHTTPC->ResponseCode);
            }
        }
    }
    catch(Exception& error)
    {
        Log->Write((boost::format("Exception occured in TConnectThread::Process: %1%") % error.Message.c_str()).str().c_str());
        ConnectResult += "\r\nConnect exception: "+AnsiString(error.Message);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    Finished = true;
}

//---------------------------------------------------------------------------

std::string encodeLocaleString(std::string source, std::string exceptionChars)
{
  std::string result;

  for(int i = 0; i < source.length(); i++)
  {
    int code = static_cast<int>(source[i]);

    //RFC 1738: Uniform Resource Locators (URL) specification
    //ranges: 32-47, 58-64, 91-96, 123-126, 128-255
    if ((((code > 31)  && (code < 48)) ||
         ((code > 57)  && (code < 65)) ||
         ((code > 90)  && (code < 97)) ||
         ((code > 122) && (code < 127)) ||
         ((code > 127) && (code < 256))) &&
        (exceptionChars.find(source[i]) == std::string::npos))
      result += (boost::format("%%%02X") % code).str();
    else
      result += source[i];
  }
  return result;
}
