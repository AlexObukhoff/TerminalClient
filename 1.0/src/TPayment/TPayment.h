//---------------------------------------------------------------------------

#ifndef TPaymentH
#define TPaymentH

#include "TWConfig.h"
#include "LogClass.h"
#include "XMLPacket.h"
#include "TFileMap.h"
#include "PacketSender.h"
#include "CSPacketSender.h"
#include "SSPacketSender.h"
#include "TConnectThread.h"
#include "XMLInfo.h"
#include "TLocationParser.h"

const int cnNewSession=0;
const int cnWaitingFor7thStatus=1;

enum PaymentDoneCmd {cnPDCmdCancel, cnPDCmdStore};

enum PaymentCompleteCodes{PCCTimedOut = 254, PCCTimedOutWSession = 255, PCCCancelledFromServer = 33};

//de_errors.js
namespace db_errors
{
  const int noValidPhone = -1;
  const int noOperatorCode = 4;
  const int errPacket0 = -3;
  const int noNumberInNumCapacity = 23;
}

class TPayment
{
private:
    int m_error;
    int m_result;
    bool m_xmlParseError;
protected:
    TDateTime PaymentCreateDT;
    TXMLInfo* InfoFile;
    TFileMap *FileMap;
    TWConfig* Cfg;
    TLogClass* Log;
    TPacketSender *PS, *InitPS;
    AnsiString MessageText;
    bool InnerLog;
    double ForcedSum;
    //int PaymentState;

    virtual AnsiString Connect(AnsiString URL,bool bProcessMessages = false);
    void PrepareAnswer(AnsiString&, TStringList*);
    AnsiString PrepareString(AnsiString);
    AnsiString GetAnswerValue(TStringList*, const AnsiString&);
    bool HasAnswerValue(TStringList *slSrc, const AnsiString& AName);
    int GetAnswerIntegerValue(TStringList*, AnsiString);
    AnsiString FormatAnswerForLog(const char*, TStringList*, int, int);
    AnsiString FormatStatusAnswerForLog(AnsiString, TStringList*, int, int);
    virtual AnsiString GetErrorDescr(int);
    virtual int GetStatus(bool StatPacketSendingAllowed, bool bTestingOnly = false);
    virtual bool Payment(bool bProcessMessages, bool bSendStatusChange = true);
    void Clear();
    AnsiString ConvertFieldsString(AnsiString Fields);
    virtual void ParseLocation(AnsiString);
    virtual AnsiString GetMessageTextForCheck(bool, const std::string& existInquiry);
    virtual AnsiString GetMessageTextForPayment(const std::string& existInquiry);
    virtual void SetSum(double Sum);
    virtual void ParseAnswer(TStringList*);
    virtual void PostProcessFirstCheck(int,int);
    virtual void PostProcessPayment(int,int);
    std::string getStatFields(std::string);
    bool PacketLoadError;
    AnsiString ConnectResult;
    void SalonPacketSend();
    bool isErrorIgnored(int);
public:
    virtual AnsiString GetMessageText(AnsiString);
    std::string AddInfo;
    TPayment(AnsiString, TWConfig*, TLogClass*, TFileMap*, TXMLInfo*);
    virtual ~TPayment();

    TPaymentPacket *XMLP;
    bool RetryAllowed;
    bool RenamePackets();
    bool RenamePacketsToUnprocess();
    std::string GetSessionNumber();
    AnsiString PostPaymentInfo;
    AnsiString AFieldsForInterface;
    std::string FieldsFromInterface;
    AnsiString AFieldsForCheque;
    AnsiString AUnnamedFieldsForCheque;
    AnsiString Session;
    AnsiString SalonCardNum;
    TDateTime PaymentProcessedDT;
    virtual bool Process();
    virtual bool Check(bool bFirstCheck, AnsiString AForcedOffline = "");
    double Sum;
    AnsiString SalonCard;  //номер карты салона для регистрации платежей по карте этого салона (напр., связной)
    
    double MinSum;
    virtual double GetComission();
    double GetComission(double Sum);
    virtual double GetPaymentMinSum();
    int Recepient;
    AnsiString StringForPaymentsLog();
    virtual bool InitPayment(AnsiString Location);
    virtual void AddNote(int, double);
    virtual bool InitDone(PaymentDoneCmd Command = cnPDCmdStore);
    virtual void Update();
    void UpdateNotes(int, double);
    bool IsOnTime();
    bool IsTimedOut();
    int StatusError;
    int PaymentErrorCode;
    bool IsCancelled();
    void CancelPayment();
    virtual double GetLimMax();
    bool IndyError;
    bool ResurrectPayment(AnsiString Parameters);
    AnsiString CheckConnection();
    int CheckErrorCode;
    bool CanCancel();
    bool TransportError;
    virtual bool CreateMenu();
    bool CancelPaymentReq;
    virtual bool PreInit(int OperatorId, AnsiString Login = "", AnsiString Password = "");
    virtual bool ChangePassword(const char* Login,const char* OldPassword,const std::string NewPassword, const std::string NewPassword1) { return false;};
    bool PayProcessStarted;
    bool GetAmount(AnsiString &Result);
    bool getXmlParseError() const;
};
#endif
