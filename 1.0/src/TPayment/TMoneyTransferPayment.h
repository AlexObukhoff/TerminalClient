//---------------------------------------------------------------------------

#ifndef TMoneyTransferPaymentH
#define TMoneyTransferPaymentH

#include <TPinPayment.h>
#include "gz_stream.h"
#include "localize.h"
#include <curl.h>
#include <easy.h>
#include <fstream>
#include <math>
#include <boost\lexical_cast.hpp>
#include <boost\format.hpp>
#include <boost\algorithm\string.hpp>

typedef CURL *(*Tcurl_easy_init)(void);
typedef void (*Tcurl_easy_cleanup)(CURL *curl);
typedef CURLcode (*Tcurl_easy_perform)(CURL *curl);
typedef CURLcode (*Tcurl_easy_setopt)(CURL *curl, CURLoption option, ...);
typedef const char* (*Tcurl_easy_strerror)(CURLcode);

enum XMLMessageType
{
    mtMTLogin,
    mtMTChangePassword,
    mtMTCalculateTransferSumsAmount,
    mtMTCalculateTransferSumsAmountAll,
    mtMTCheckPaymentInfo,
    mtMTProcessPayment,
    mtGetErrorsList
};

struct login_info
{
    std::string CardNumber;
    std::string PasswordMD5;
    std::string NewPasswordMD5;
};

struct _sender_info
{
    std::string id;
    std::string last_name;
    std::string first_name;
    std::string middle_name;
    std::string birth_date;
    std::string birth_place;
    std::string country;
    std::string address;
    std::string resident;
    std::string phone;
    std::string email;
    std::string mobile;
};

struct _sender_id_info
{
    std::string type;
    std::string serial;
    std::string number;
    std::string issue_date;
    std::string country;
    std::string authority;
    std::string authority_code;
};

struct _recipient_info
{
    std::string last_name;
    std::string first_name;
    std::string middle_name;
    std::string country;
    std::string resident;
    std::string email;
    std::string mobile;
    std::string recipient_bank_id;
    std::string mt_system;
    std::string mt_system_name;
    std::string bank_address;
    std::string id;
    int currencyId;
};

struct _transfer_info
{
    int mtsystem;
    std::string trans_id;
    std::string transfer_code;
    std::string currency;
    double amount;
    double comiss;
    double amount_total;
    std::string recipient_bank_id;
    std::string recipient_bank_parent_id;
};

struct _error_message_info
{
    std::string Language;
    std::string SystemMessage;
    std::string UserMessage;
};

class TMoneyTransferPayment : public TPayment
{
private:
    TCriticalSection* CS;

    void saveConnect(const XMLMessageType type,const double sum,AnsiString* Result);
    virtual bool Check(bool bFirstCheck, AnsiString AForcedOffline = "");
    virtual bool Payment(bool bProcessMessages, bool bSendStatusChange = true);
    virtual void ParseLocation(AnsiString LocationString);
    virtual bool Process();

    std::string m_errorsListFileName;
    const static int m_rurId = 810;
    const int cWaitFlagTimeoutSecond;

    _sender_info SenderInfo;
    _sender_id_info SenderIdInfo;
    std::vector<_recipient_info> LastRecipients;
    _transfer_info TransferInfo;
    _recipient_info RecipientInfo;

    std::map <int, std::string> MessageType;
    std::map <int, _error_message_info> ErrorMessages;

    std::string MakeXMLMessage(XMLMessageType,double summ=-1);
    void MakeMTLoginContent(xmlGuard <_di_IXMLNode> NdContent);
    void MakeMTChangePasswordContent(xmlGuard <_di_IXMLNode> NdContent);
    void MakeCalculateTransferSumsContentAmount(xmlGuard <_di_IXMLNode> NdContent,double summ);
    void MakeCalculateTransferSumsContentAmountAll(xmlGuard <_di_IXMLNode> NdContent,double summ);
    void MakeMTPaymentInfoContent(xmlGuard <_di_IXMLNode> NdContent, XMLMessageType MessageType);
    bool ParseXMLAnswer(AnsiString AnswerBody, XMLMessageType MessageType,bool test);
    std::string GetMessageTypeString(XMLMessageType MType);
    void GetMTLoginContent(xmlGuard <_di_IXMLNode> NdContent);
    void GetMTCalculateTransferSumsInfo(xmlGuard <_di_IXMLNode> NdContent,bool test);
    void GetMTCheckPaymentInfo(xmlGuard <_di_IXMLNode> NdContent);
    void GetMTProcessPaymentInfo(xmlGuard <_di_IXMLNode> NdContent);
    bool GetMTGetErrorsListInfo(xmlGuard <_di_IXMLNode> NdContent);
    void getMTStatus(xmlGuard <_di_IXMLNode> NdContent);
    void getErrorChangePasswordContent(xmlGuard <_di_IXMLNode> NdContent);
    void MakePostLoginJSFile();
    void MakePostGetErrorsListJSFile();
    bool UseAmountTotal;

    void InitCurlLib();
    HINSTANCE CURL_WC_DLL;
    Tcurl_easy_init curl_easy_init;
    Tcurl_easy_cleanup curl_easy_cleanup;
    Tcurl_easy_setopt curl_easy_setopt;
    Tcurl_easy_perform curl_easy_perform;
    Tcurl_easy_strerror curl_easy_strerror;
    AnsiString GetTimeStamp(const char* FileName);
    bool ReadAll(const char* FileName,AnsiString& Data);
    std::string m_amount; // Сумма Перевода за вычитом
    std::string m_amount_all; // Внесенная сумма
    std::string m_system_commission; //Коммиссия системы для получателя
    std::string m_rent_commission; //Остаток в терминале

    std::string m_rec_amount,m_rec_system_commission,m_rec_amount_all; // receive_amounts, amount, amount_total, fee

    std::map<int,std::string> m_IdName; //Код валюты, буквенное обозначение

    int statusCode;
    bool RenamePacketsToCanceled();
    bool sendTransfer(bool bProcessMessages);
    bool sendTransferConfirm(bool bProcessMessages);
    std::string replaceLF(const std::string& value);
    std::string currencyToStr(const char* paramName);

    const char* cEmpty;
    const char* cSendTransfer;
    const char* cSendTransferConfirm;
    const char* cCancel;
protected:
    virtual void SetSum(double Sum);
public:
    TMoneyTransferPayment(AnsiString _fileName, TWConfig *_Cfg, TLogClass *_Log, TFileMap* _FileMap, TXMLInfo* _InfoFile);
    virtual ~TMoneyTransferPayment();

    virtual bool PreInit(int OperatorId, AnsiString Login, AnsiString Password);
    virtual double GetComission();
    bool GetErrorsList();
    void setRecepientID(int Recepient);

    std::string getCalculateAmountAll();
    std::string getCalculateAmount();
    std::string getCalculateSystemComission();
    std::string getCalculateRentComission();

    bool CalculateCommission(const char* location,const double sumTransfer);
    login_info LoginInfo;
    int LastErrorCode;
    virtual bool ChangePassword(const char* Login,const char* OldPassword,const std::string NewPassword,const std::string NewPassword1);
    virtual void AddNote(int, double);
    void clearLoginData();//Очищаем данные полученные при логине
};

#endif
