//---------------------------------------------------------------------------
#ifndef notebookH
#define notebookH
#include "LogClass.h"
#include "TWConfig.h"
#include "TFileMap.h"
#include "XMLInfo.h"
#include "TJSMaker.h"
#include "TConnectThread.h"
#include <list>
#include <algorithm>
#include <boost\format.hpp>
#include <boost\regex.hpp>
#include <boost\lexical_cast.hpp>
#include <boost\algorithm\string.hpp>
#include <boost\shared_ptr.hpp>

#include <map.h>

//---------------------------------------------------------------------------
typedef enum
{
  parseNoError    = 0,
  parseMainError     = -1,
  parsePropertyError = -2,
  parseCommentError  = -3,
  parseUnknownError  = -4,
  parseNoRecords     = -5,
  parseNoOperator    = -6,
  parseInvalidData   = -7,
  parseToManyRecord  = -8

} ParseRecordErrorCodes;
//---------------------------------------------------------------------------
typedef enum
{
  UnknownError = -1,

  NoError     = 0,

  badCommand  = 1,
  badOperator = 2,
  badUser     = 3,
  badRec_Id   = 4,
  badPin      = 5

} PaymentBookErrorCodes;
//---------------------------------------------------------------------------
typedef enum
{
  parseURLNoError = 0,
  parseURLBaseError    = -10,
  parseURLFieldsError  = -11,
  parseURLUnknownError = -12,
  parseURLRecIdError   = -13,
  parseURLOperIdError  = -14

} ParseURLErrorCodes;
//---------------------------------------------------------------------------
typedef enum
{
  NoRequest = 0,
  Registration    = 1,
  CheckLogin      = 2,
  GetBook         = 3,
  AddRecord       = 4,
  ChangeRecord    = 5,
  DeleteRecord    = 6,
  SendPasswordSms = 7,
  PayToRecord     = 8

} PaymentBookRequestCodes;
//---------------------------------------------------------------------------
namespace pbook
{
    typedef enum
    {
      Unknown   = -1,
      NoCommand = 0,

      CheckLogin    = 1,
      EnterPin      = 2,
      GetPin        = 3,
      SendSmsPin    = 4,
      RestoreSmsPin = 5,
      CheckAccount  = 6,
      ShowRecords   = 7,
      Checking      = 8,
      Payment       = 9,
      AllThanks     = 10,
      Add_rec       = 11,
      Delete_rec    = 12,
      Change_rec    = 13

    } PaymentBookInterfaceCommand;
}
//---------------------------------------------------------------------------

typedef class TPaymentRecord
{
public:
  int Id;
  int OperatorId;
  std::vector<property> pr_Properties;
  std::string pr_ShowName;
  int pr_Color;
  bool pr_empty;

  TPaymentRecord(int tId = 0): Id(tId), OperatorId(-1), pr_ShowName(""), pr_empty(true), pr_Color(0){};
  TPaymentRecord(int, int, std::vector<property>, std::string, int);
  bool operator == (const TPaymentRecord& rhs);
  bool operator != (const TPaymentRecord& rhs);
  int FindField(std::string);
  TPaymentRecord& operator=(const TPaymentRecord&);
  ~TPaymentRecord();            
                           
}TPaymentRecord;

typedef std::list<TPaymentRecord> TPaymentRecords;
typedef std::map<int, TPaymentRecords> TPaymentNotes;
typedef std::pair<int, TPaymentRecords::iterator> TRecordNavi;
//---------------------------------------------------------------------------

class TPrivateData
{
private:
  void swap(const TPrivateData& rhs);

public:
  AnsiString Nick;
  AnsiString Password;
  AnsiString EMail;

  AnsiString Name;
  AnsiString Patronymic;
  AnsiString SurName;

  void clear();
  TPrivateData(AnsiString, AnsiString, AnsiString, AnsiString, AnsiString, AnsiString);
  TPrivateData(AnsiString, AnsiString, AnsiString tEMail = "");
  TPrivateData();
  TPrivateData(const TPrivateData& rhs);
  TPrivateData& operator=(const TPrivateData& rhs);
};

//---------------------------------------------------------------------------

typedef std::pair<std::string, pbook::PaymentBookInterfaceCommand> PaymentBookInterfaceElement;
typedef std::map<std::string,  pbook::PaymentBookInterfaceCommand> PaymentBookInterface;

class CICommand
{
private:
    static PaymentBookInterface m_icommands;
    static void setValues()
    {
      if(m_icommands.empty())
      {
          PaymentBookInterfaceElement ICommands[] = {
              make_pair<std::string,  pbook::PaymentBookInterfaceCommand>("unknown",       pbook::Unknown),
              make_pair<std::string,  pbook::PaymentBookInterfaceCommand>("nocommand",     pbook::NoCommand),

              make_pair<std::string,  pbook::PaymentBookInterfaceCommand>("checklogin",    pbook::CheckLogin),
              make_pair<std::string,  pbook::PaymentBookInterfaceCommand>("sendsmspin",    pbook::SendSmsPin),
              make_pair<std::string,  pbook::PaymentBookInterfaceCommand>("restoresmspin", pbook::RestoreSmsPin),
              make_pair<std::string,  pbook::PaymentBookInterfaceCommand>("enterpin",      pbook::EnterPin),
              make_pair<std::string,  pbook::PaymentBookInterfaceCommand>("getpin",        pbook::GetPin),
              make_pair<std::string,  pbook::PaymentBookInterfaceCommand>("checkaccount",  pbook::CheckAccount),
              make_pair<std::string,  pbook::PaymentBookInterfaceCommand>("getpb",         pbook::ShowRecords),
              make_pair<std::string,  pbook::PaymentBookInterfaceCommand>("checking",      pbook::Checking),
              make_pair<std::string,  pbook::PaymentBookInterfaceCommand>("payment",       pbook::Payment),
              make_pair<std::string,  pbook::PaymentBookInterfaceCommand>("thanks",        pbook::AllThanks),

              make_pair<std::string,  pbook::PaymentBookInterfaceCommand>("add",           pbook::Add_rec),
              make_pair<std::string,  pbook::PaymentBookInterfaceCommand>("delete",        pbook::Delete_rec),
              make_pair<std::string,  pbook::PaymentBookInterfaceCommand>("update",        pbook::Change_rec),
          };
          m_icommands = PaymentBookInterface(ICommands, ICommands + sizeof(ICommands) / sizeof(ICommands[0]));
      }
    }
public:
  static pbook::PaymentBookInterfaceCommand getICommand(const std::string& str)
  {
    setValues();
    return (m_icommands.find(str) != m_icommands.end()) ? m_icommands[str] : pbook::Unknown;
  }

  //---------------------------------------------------------------------------

  std::string GetIDescription(pbook::PaymentBookInterfaceCommand code)
  {
    std::string result = "";

    setValues();
    for(std::map<std::string, pbook::PaymentBookInterfaceCommand>::iterator itCode = m_icommands.begin(); itCode != m_icommands.end(); itCode++)
      if (itCode->second == code)
        result = itCode->first;

    return result;
  }
};

//---------------------------------------------------------------------------

class TPaymentBook
{
private:
    TWConfig* Cfg;
    TXMLInfo* InfoFile;
    TFileMap *FileMap;
    TLogClass* Log;
    TXMLInfo* Info;

    TPaymentNotes PaymentNote;
    PaymentBookRequestCodes CurrentRequestCode;
    PaymentBookErrorCodes CurrentError;
    std::string CurrentRequest;
    AnsiString CurrentAnswer;

    ParseRecordErrorCodes CurrentParseRecordError;
    ParseURLErrorCodes CurrentParseURLError;

    std::string AddDelimiter(){return CurrentRequest += std::string("\r\n");};
    AnsiString RemoveDelimiter(bool IsRequest = true);
    template<typename T1, typename T2> std::string AddRequestString(T1, T2, bool isEnd = false);
    AnsiString CreateProperties(std::vector<property>&);
    bool IndyError;
    void Connect(TPaymentRecord* CurrentRecord = NULL);
    ParseRecordErrorCodes ParseRecord(std::string, int&, int&, std::vector<property>&, AnsiString&, int&);
    ParseURLErrorCodes ParseURL(std::string, int&, std::vector<property>&, std::string&, int& tempRecId);
    ParseRecordErrorCodes ProcessError(std::string, ParseRecordErrorCodes);
    TRecordNavi GetRecordById(int rRecId);
    void MakeJS();
    int MakeColor();

    bool invalidChar(char c);
public:
    TPaymentBook(TWConfig*, TLogClass*, TFileMap*, TXMLInfo*);
    int GetCapacity(int tGroup = -1);

    ~TPaymentBook();
    TPrivateData PrivateData;
    bool Enter(bool IsNeedCreate = false);
    void Exit();
    void Add(std::string);
    void Delete(std::string);
    void Change(std::string);
    bool CheckAccount();
    bool GetPassword();
    bool CreatePacket(const std::list<TPaymentRecord>::iterator CurrentRecord = NULL);
    ParseRecordErrorCodes ProcessAnswer(const std::list<TPaymentRecord>::iterator CurrentRecord = NULL);
    bool IsChecked(){return checked == true;};
    void clear(){PaymentNote.clear(); PrivateData.clear();};

    bool LastConnectionOK;
    bool payment;
    bool checked;
    bool entered;
};

AnsiString str_replace(AnsiString&, const char*, const char*);
int getFieldsCount(AnsiString str, const char* ch);

#endif                                                             


