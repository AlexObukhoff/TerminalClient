//---------------------------------------------------------------------------

#ifndef TWConfigH
#define TWConfigH
//---------------------------------------------------------------------------
#include <system.hpp>
#include <registry.hpp>
#include <algorith.h>
#include <XMLDoc.hpp>
#include <Classes.hpp>
#include <ComCtrls.hpp>
#include <IdTCPClient.hpp>
#include <vector>
#include <memory>
#include <map>
#include <set>
#include <string>

#include <boost\algorithm\string\case_conv.hpp>

#include "TJSMaker.h"

#include "LogClass.h"
#include "common.h"
#include "TFileMap.h"
#include "VirtualTrees.hpp"
//---------------------------------------------------------------------------

const int cnMonitoringServer = 0;
const int cnCyberPlatServer = 1;

const int cnReadMenu = 0;
const int cnClearData = 1;

const int cnMIRoot = 0;
const int cnMIGroup = 1;
const int cnMIOperator = 2;

const int cnSOIncassation = 0;
const int cnSOServiceMenu = 1;

const int cnIncassationOpen  = 0;
const int cnIncassationClose = 1;     //инкассатор подходит сзади

typedef std::vector<int> TIntVector;     //для хранения int-овых переменных, записанных в строку в конфиге через "," или " "
typedef std::vector<std::string> TStringVector; 

//const int KeysMaxCount = 10;
const int MaxStatServersAdresses = 3;

enum RoundModes { RNone ,RDown, RBankers, RUp };

enum EMailTypes{cnETHWErr, cnETHWOK, cnETValFull, cnETIncass, cnETProgMsg, cnETMdmMsg, cnETFile};

enum restState {eNo = -1, eImplement = 0, eYes = 1};

class comm_info
{
private:
    void swap(const comm_info& rhs);
public:
    bool Relative;
    double Value;
    int Min;
    int MinTime;
    int MinDay;
    comm_info();
    comm_info(bool _Relative, double _Value, int _Min, int _MinTime, int _MinDay);
    comm_info(const comm_info& ob);
    comm_info& operator = (const comm_info& ob);
    AnsiString GetString();
};

class field
{
private:
    void swap(const field& rhs);
public:
    std::string Id;
    std::string Name;
    std::string Mask;
    bool SendUnmasked;
    std::string Type;
    std::map<std::string, std::string> Enum;
    std::string Regexp,Format;
    std::string URL1;

    field();
    field(const char* _Id, const char* _Name, const char* _Mask, bool _SendUnmasked, const char* _Type, const std::map <std::string, std::string>& _Enum,const char* regexp,const char* format,const char* tURL1);
    field(const field& ob);
    field& operator = (const field& ob);
    AnsiString GetString();
    AnsiString GetEnumText(const char* Value);
    bool HasEnumValue(std::string Value);
};

class property
{
public:
    std::string Name;
    std::string FieldId;
    property();
    property(const char* _Name, const char* _FieldId);
    property(const property& ob);
    property& operator = (const property& ob);
    bool operator == (const property& ob);
    bool operator != (const property& ob);
    AnsiString GetString();
};

struct _currency_info
{
    std::string Currency;
    double ExchangeRate;
    std::string CurrencyName;
    int currencyId;
    TIntVector nominals;
};

class _money_transfers_info
{
public:
    TDateTime MTInfoUpdateTime;
    _money_transfers_info();
    _money_transfers_info(const _money_transfers_info& ob);
    _money_transfers_info& operator = (const _money_transfers_info& ob);
    AnsiString GetString();
};

class receive_property
{
public:
    std::string Name;
    std::string Description;
    bool Encrypted;
    receive_property();
    receive_property(const char* _Name, const char* _Description, bool _Encrypted);
    receive_property(const receive_property& ob);
    receive_property& operator = (const receive_property& ob);
};

struct _groups_info
{
  int Id;
  std::string title;
  std::set<int> OperatorIds;
};

struct _salon_info
{
  int Id;
  int FieldId;
  std::string name;
  AnsiString URL;
};

typedef class OpInfo
{
private:
    void swap(const OpInfo& rhs);
public:
    OpInfo(){}
    OpInfo(const OpInfo& rhs);
    OpInfo& operator=(const OpInfo& rhs);

    int Id;
    int GroupId;
    std::string Name;
    std::string Image;
    std::string RootMenuImage;
    std::string CheckURL;
    int CheckAmount;
    std::string CheckAmountFieldId;
    std::string PaymentURL;
    std::string StatusURL;
    std::string GetCardsURL;
    int SalonField;
    std::string LoginURL;
    int Offline;
    short showOnLineComment;
    double LimMin;
    double LimMax;
    int fix;   //надо ли на данного оператора вносить фиксированную сумму
    int LimSetCount;
    std::string ProcessorType;
    std::string ACardsInfo;
    std::string DTCardsInfo;

    std::string ChequeFileName;
    std::string ErrorChequeFileName;
    std::string ErrorMoneyChequeFileName;
    bool PaymentBeforeChequePrinting;
    std::string MNVO_ID;
    int KeysId;
    RoundModes RoundAmount;
    bool PrinterOkOnly;
    bool GetCardsAllowed;
    std::string MaxSavedCardsCount;
    int SignatureType;
    std::string NameForCheque;
    std::string INNForCheque;
    std::string CurrencyId;
    std::string MarketingOperatorId;

    std::vector <comm_info> CommissionInfo;
    std::vector <field> Fields;
    std::vector <property> Properties;
    _salon_info Salon;

    std::map <std::string, receive_property> ReceiveProperties;
    bool HasReceiveProperty(const char* PropName);
    bool ShowAddInfo;
    field getFieldById(std::string);

    std::string ServiceGuid;
} TOpInfo;

typedef class MenuItemData
{
private:
    void swap(const MenuItemData& rhs);
public:
    MenuItemData();
    MenuItemData(const MenuItemData& rhs);
    MenuItemData& MenuItemData::operator=(const MenuItemData& rhs);

    int Type;
    int Id;
    std::string Name;
    std::string Image;
    std::string TitleImage;
    int Columns;
    int Rows;
} TMenuItemData;

class _dealer_info
{
private:
    void swap(const _dealer_info& rhs);
public:
    std::string DealerName;
    std::string DealerAddress;
    std::string BusinessDealerAddress;
    std::string DealerINN;
    std::string DealerPhone;
    std::string PointAddress;
    std::string ContractNumber;
    std::string BankName;
    std::string BankBIK;
    std::string BankPhone;
    std::string BankAddress;
    std::string BankINN;

    _dealer_info(const _dealer_info& rhs);
    _dealer_info& operator=(const _dealer_info& rhs);
    _dealer_info();
};

struct _proxy_info
{
    std::string CfgType;
    std::string Type;
    std::string Host;
    int Port;
    std::string UserName;
    std::string Password;
    _proxy_info();
    _proxy_info(const _proxy_info& ob);
    _proxy_info& operator = (const _proxy_info& ob);
};

//typedef
class ConnInfo
{
private:
    void swap(const ConnInfo& ob);
public:
    std::string Name;
    int DisconnectTime;
    std::string InitCmd;
    std::string InitString[2];
    std::string Login;
    std::string Password;
    _proxy_info HTTPProxy;
    TSocksInfo *Proxy;

    ConnInfo();
    ConnInfo(const ConnInfo& ob);
    ConnInfo& operator = (const ConnInfo& ob);
    ~ConnInfo();
};

class _dirs_info
{
private:
    void swap(const _dirs_info& rhs);
public:
    std::string WorkDir;

    std::string PaymentsOutbound;
    std::string PaymentsOutboundTemp;
    std::string PaymentsOutboundCanceled;
    std::string PaymentsUnprocessed;
    std::string PaymentsBad;

    std::string StatOutbound;
    std::string StatOutboundTemp;
    std::string StatOutboundBad;
    
    std::string EMailOutbound;
    std::string EMailOutboundExt;
    std::string EMailOutboundTemp;
    std::string EMailOutboundExtTemp;

    std::string SMSOutbound;
    std::string SMSOutboundTemp;
    
    std::string CommandsInbound;
    std::string StatusFileName;
    std::string InterfaceDir;
    std::string InterfaceSkinName;
    std::string DBNumCapacityUpdateAddress;
    std::string MTRoot;

    _dirs_info();
    _dirs_info(const _dirs_info& rhs);
    _dirs_info& operator=(const _dirs_info& rhs);
};

class _email_info
{
private:
    void swap(const _email_info& rhs);
public:
    std::string SMailHost;
    std::string FromAddress;
    std::string UserId;
    std::string Password;
    bool Ext;
    std::string ToHWErrAddr;
    std::string ToHWOKAddr;
    std::string ToValFullAddr;
    std::string ToIncassAddr;
    std::string ToProgMsgAddr;
    std::string ToMdmMsgAddr;
    std::string ToFileAddr;
    std::string IgnoredErrorsList;
    int SendInterval;
    AnsiString GetRecipientAddress(EMailTypes MsgType);
    _email_info();
    _email_info(const _email_info& rhs);
    _email_info& operator=(const _email_info& rhs);
};

class _sms_info
{
private:
    void swap(const _sms_info& rhs);
public:
    int Interval;
    std::string PhoneNumber;
    std::string Comment;
    bool SendStartUpSMS;
    _sms_info();
    _sms_info(const _sms_info& rhs);
    _sms_info& operator=(const _sms_info& rhs);
};

struct _commands_info
{
    int CommandDeleteTime;
    _commands_info() { CommandDeleteTime = 0;};
};

struct _stat_info
{
    std::string StatServerSrc;
    std::string Host[MaxStatServersAdresses];
    int Port[MaxStatServersAdresses];
    long StatServerPollInterval;
    int StatServersAdressesCount;
    int CurrentStatServersAdressNum;
    bool Inhibit;
    bool InhibitHeartBeats;
    bool NoFileSending;
    bool CompressFiles;
    TDateTime DTServerTimeOutDiff;
    TDateTime DTLastSuccessfullPacketSending;
    int ProcessorType;
    std::string HeartBeatURL;
    std::string PaymentCompleteURL;
    std::string IncassationURL;
    std::string DownloadURLPrefix;
    bool IsSignStatistics;
};

struct _service
{
    std::string Number;
    int OperatorId;
    std::string ServiceMenuPasswordMaskSrc;
    std::string ServiceMenuPasswordMask;
    std::string PaymentBookURL;
    std::string ServiceMenuItems;
    std::string ServiceShortMenuPasswordMaskSrc;
    std::string ServiceShortMenuPasswordMask;
    std::string ServiceShortMenuItems;
    std::string IncassationNumberMaskSrc;
    std::string IncassationNumberMask;
    std::string BalanceReportFileName;
    std::string TestChequeFileName;
    std::string IncassReportFileName;
    int IncassReportCount;
    std::string IncassGetAmountURL;
    std::string OutOfOrderPassword;
};

struct _payments_info
{
    restState restOperator;
    void SetRestState(restState tempRestState);
    int Offline;
    double ErrorPaymentsDeleteHours;
    TDateTime OfflinePeriodBegin;
    TDateTime OfflinePeriodEnd;
    TDateTime OfflineMoneyPeriodBegin;
    TDateTime OfflineMoneyPeriodEnd;
    int UpdateCardsInfo;
    __property restState rState = {read = restOperator, write = SetRestState};

    std::set<int> IgnoredCheckErrorsList;
    double nextTry;// Следующая попытка в минутах при разрыве связи, если меньше нуля прежняя логика
    double Rest; //остаток после платежа фиксированной суммы, если внесли >, чем надо
};

struct _terminal_info
{
    TDateTime NightModePeriodBegin;
    TDateTime NightModePeriodEnd;
    bool ShowCursor;
    std::string SrcNumber;
    bool WriteInCheque;
    std::string SupportString;
    std::string MainMenuMarqueeString;
    bool RebootAllowed;
    bool DetectWriteErrors;
    bool SetWebclientHighPriority;
    //bool StayOnTop;
    unsigned int ChequeCounter;
    int InterfaceSoundVolume;
    bool NoChangeMessage;
    bool ShowPB;
    std::string Number;
    std::string NumberForCheque;
    double NoteMinAmount;

    _terminal_info();
};

struct _debug_log
{
    bool url;
    bool full;
    bool sound;

    void operator=(const bool);
    _debug_log();
};

struct _cdebug
{
    short PeripheralsState;
    bool sound;
    bool explorerProcess;
    bool connProcess;
    _debug_log Logs;

    _cdebug();
};

struct _keys_info
{
    int Id;
    int Hasp;
    std::string SD;
    std::string AP;
    std::string OP;
    std::string PubKeyPath;
    int PubKeySerial;
    std::string SecKeyPath;
    std::string SecKeyPassword;
    _keys_info(int _Id, int _Hasp, const char* _SD, const char* _AP, const char* _OP, const char* _PubKeyPath, int _PubKeySerial, const char* _SecKeyPath, const char* _SecKeyPassword);
    _keys_info();
    _keys_info(int _Id);
};

struct _bar_code_template
{
    int terminal;
    int validator;
    int cheque;
    int total;
    std::string date;
    std::string time;
    int nominal;
    int count;
    std::string templateStr;
    std::string filler;

    _bar_code_template();
};

struct _logs_delete_info
{
    int Payments;
    int Main;
    int Other;
    int UnprocessedPayments;
    _logs_delete_info();
};

struct _printer_info
{
    std::string Type;
    int Port;
    int PortBaudRate;
    bool AutoOpenShift;
    bool PrintUnprintedCheques;
    bool SaveUnprintedCheques;
    bool FreeseOnError;
    bool ShowMessageOnError;
    int MinLinesCount;
    std::string Font;
    int PresenterCommand;
    int ZReportWithIncassation;
    bool IncassBarCode;
    std::size_t TapeLength;

    _printer_info();
};

struct _validator_info
{
    std::string Type;
    std::string Protocol;
    int Port;
    int PortBaudRate;
    int StackerOpenCommand;
    int IncassOpenStacker;
    std::string BillsSensitivity;
    std::string ReportBillCount;
};

struct _coin_acceptor_info
{
	std::string Type;
	int Port;
    int PortBaudRate;
    double MinCash;
    double nominalMultiply;
    std::string ReportCoinCount;
    _coin_acceptor_info() {
        Type = "";
        Port = 0;
        PortBaudRate = 0;
        MinCash = 0;
        ReportCoinCount = "";
        nominalMultiply = 1;
    }
    _coin_acceptor_info* operator ->() { return this; };
};
struct _modem_info
{
    int AutoDial;
    int Port;
    int ServiceInfoPort;
    int PortBaudRate;
    std::string ServiceNumber;
    int GetServiceInfoInterval;
    int FailuresBeforeModemReset;
    int FailuresBeforeTerminalReboot;
    int SendSMSInterval;
    AnsiString Hosts[2];
    int Interval;
    TDateTime ConnectionCheckTimeOutDT;
    std::string ServiceAnswerMask;
};

struct _watchdog_info
{
    std::string Type;
    int Port;
    int PortBaudRate;
};

struct _keyb_info
{
    std::string Type;
    int Port;
    int PortBaudRate;
};

struct _scanner_info
{
    std::string Type;
    int Port;
    int PortBaudRate;
};

struct _cardreader_info
{
    std::string Type;
    int Port;
    int PortBaudRate;
    long SystemCode;
    long DeviceCode;
    bool FreeseOnError;
};

struct _peripherals_info
{
    _printer_info Printer;
    _validator_info Validator;
    _coin_acceptor_info CoinAcceptor;
    _modem_info Modem;
    _watchdog_info WatchDog;
    _keyb_info Keyboard;
    _cardreader_info CardReader;
    _scanner_info Scanner;
};

class TWConfig
{
private:
    TLogClass *Log;
    bool InnerLog;

    AnsiString __fastcall Strip(AnsiString S);
    AnsiString md5ToString(AnsiString S);

    AnsiString GetPath(void);
    void __fastcall CheckDir(AnsiString);
    AnsiString GetHostFromRegistry(int);
    AnsiString ConfigFileName;
    AnsiString OperatorsFileName;
    void ProcessMenuGroup(xmlGuard <_di_IXMLNode> &ProcessedNode, TTreeNode* ParentNode = NULL, AnsiString Margin = "");
    void SortKeys();
    TFileStream *CfgFile;
    TFileStream *OperFile;
    TOpInfo BlankOper;
    _groups_info BlankGroup;
    void SetOperDefaults(TOpInfo& BlankOper);
    ConnInfo DefaultConnInfo;

    void SortComission(TOpInfo& OperatorInfo);
    TTreeNodes* Menu;
    void GetComission(TOpInfo& OperatorInfo, xmlGuard <_di_IXMLNode> &xmlComissionNode);

    void ClearOperatorsInfo();

    void ProcessOperatorNode(xmlGuard <_di_IXMLNode> &xmlNode);
    void postProcessOperatorNode(xmlGuard <_di_IXMLNode> &xmlNode, TOpInfo& OperatorInfo);
    void ProcessProcessorNode(TOpInfo& OperatorInfo, xmlGuard <_di_IXMLNode> &xmlOpParamNode);
    void postProcessProcessorNode(TOpInfo& OperatorInfo, xmlGuard <_di_IXMLNode> &xmlOpParamNode);
    void ProcessFieldsNode(TOpInfo& OperatorInfo, xmlGuard <_di_IXMLNode> &xmlOpParamNode);
    void postProcessFieldsNode(xmlGuard <_di_IXMLNode> &xmlOpParamNode);
    void processNestedEnum(xmlGuard <_di_IXMLNode> &enumNode, std::map<std::string, std::string>& Enum, const std::string &branchName);

    std::map <std::string, std::string> CountryCodes;

    TCriticalSection* CS;

    std::vector <ConnInfo> m_RASConnections;

    std::set<int> m_allOperators;
    std::set<int> m_allOperatorsInMenu;
public:
    bool OpenFile(TFileStream* &File, AnsiString FileName, AnsiString &FileData);
    bool SaveFile(TFileStream* &File, AnsiString SourceData);

    bool IsExist(AnsiString, xmlGuard <_di_IXMLNode> &Node);
    int GetInt(bool bOverWrite, const char* NodeName, xmlGuard <_di_IXMLNode> &xmlParentNode, int Default);
    double GetDouble(bool bOverWrite, const char* NodeName, xmlGuard <_di_IXMLNode>& xmlParentNode, double Default);

    std::string GetChildNodeValue(bool bOverWrite, const char* NodeName, xmlGuard <_di_IXMLNode>& xmlParentNode,  const char* Default, bool isNeedTrim = true);
    std::string GetChildNodeValue(bool bOverWrite, const char* NodeName, xmlGuard <_di_IXMLNode>& xmlParentNode,  std::string Default, bool isNeedTrim = true);
    std::string GetChildNodeValue(bool bOverWrite, const char* NodeName, xmlGuard <_di_IXMLNode>& xmlParentNode,  int Default, bool isNeedTrim = true);
    std::string GetChildNodeValue(bool bOverWrite, const char* NodeName, xmlGuard <_di_IXMLNode>& xmlParentNode,  double Default, bool isNeedTrim = true);

    std::string GetNodeValue(xmlGuard <_di_IXMLNode>& xmlNode, const std::string& defaultValue = "", bool isNeedTrim = true);
            int GetNodeValue(xmlGuard <_di_IXMLNode>& xmlNode, const int defaultValue = -1, bool isNeedTrim = true);
         double GetNodeValue(xmlGuard <_di_IXMLNode>& xmlNode, const double defaultValue = -1, bool isNeedTrim = true);
           bool GetNodeValue(xmlGuard <_di_IXMLNode>& xmlNode, const bool defaultValue = false, bool isNeedTrim = true);

    _di_IXMLNode CreateNode(AnsiString NodeName, xmlGuard <_di_IXMLNode>& xmlParentNode, AnsiString Value);
    void DeleteNode(AnsiString NodeName, xmlGuard <_di_IXMLNode>& xmlParentNode);
    _di_IXMLNode GetNode(AnsiString NodeName, xmlGuard <_di_IXMLNode>& xmlParentNode, bool bNoCreate = false);
    bool SetNodeValue(xmlGuard <_di_IXMLNode>& xmlNode, AnsiString Value);
    bool SetAttribute(AnsiString AttribName, xmlGuard <_di_IXMLNode>& xmlParentNode, AnsiString Value);
    bool DeleteAttribute(AnsiString AttribName, xmlGuard <_di_IXMLNode>& xmlNode);
    bool IsAttributeExists(AnsiString AttribName, xmlGuard <_di_IXMLNode>& xmlParentNode);
    std::string GetStringAttribute(bool, const char* AttribName, xmlGuard <_di_IXMLNode>& xmlParentNode, const char* Default, bool bNoCreate = false);
    int GetIntAttribute(bool, const char* AttribName, xmlGuard <_di_IXMLNode>& xmlParentNode, int Default, bool bNoCreate = false);
    long GetLongAttribute(bool bOverWrite, const char* AttribName, xmlGuard <_di_IXMLNode>& xmlParentNode, long Default, bool bNoCreate = false);
    double GetDoubleAttribute(bool, const char* AttribName, xmlGuard <_di_IXMLNode>& xmlParentNode, double Default, bool bNoCreate = false);
    TDateTime GetTimeAttribute(bool, const char* AttribName, xmlGuard <_di_IXMLNode>& xmlParentNode, TDateTime Default, bool bNoCreate = false);

    __fastcall TWConfig(AnsiString _ConfigFileName, AnsiString _OperatorsFileName, TLogClass *_Log);
    __fastcall ~TWConfig();

    AnsiString Md5Str(AnsiString src);
    int ConnectionTimeOut;
    TJSMaker *JSMaker;
    std::vector <ConnInfo> RASConnections;

    ConnInfo Connection(int i = -1);
    int ActiveConnection;

    std::string m_LocaleName;
    std::string GetKeysURL;

    //std::auto_ptr <TJSMaker> JSMaker;
    short InternalVersion;
    std::string ConfigMD5;
    std::string OperatorsMD5;
    bool Prepared;

    _bar_code_template BarCodeTemplate;
    _terminal_info Terminal;
    _payments_info Payments;
    _commands_info Commands;
    _email_info EMailInfo;
    _sms_info SMSInfo;
    _stat_info StatInfo;
    _service ServiceInfo;
    _logs_delete_info LogsDelete;
    _peripherals_info Peripherals;
    _dirs_info Dirs;
    _dealer_info DealerInfo;
    _money_transfers_info MoneyTransfers;
    _currency_info CurrencyInfo;
    _cdebug CDebug;

    std::vector <std::string> ChequeCaption;
    std::vector <_keys_info> Keys;

    std::vector<TOpInfo> OperatorsInfo;
    std::vector<_groups_info> GroupsInfo;

    void SetDefaultValues();
    bool GetOperatorsInfo();
    bool readMenu(const std::string& menuFileName);

    double GetComission(int _OpId, double Sum, TDateTime DT);                     //REWRITE
    std::string strToIntVector(std::string, TIntVector&, std::string parameter = "", bool isRepeatAllowed = false);

    AnsiString GetPath(AnsiString);
    AnsiString GetHost(AnsiString);

    bool IsOffline(int _OpNum, AnsiString& OfflinePeriodMessage);
    bool IsMoneyOffline();
    double GetPaymentMinSum(int _OpId, double MinSum,const TDateTime& DT);

    bool ProcessConfigFile(bool, bool, bool);
    bool processBarCodeTemplateFile(const char* FileName);

    void SetCardsInfo(int, AnsiString,TDateTime);
    AnsiString DeleteCardInfo(int _OpId, AnsiString _CardNameToDelete);

    TOpInfo Operator(int _OpId);
    _groups_info Group(int _GrId);
    
    TOpInfo OperatorByNum(int _OpNum);
    bool isOperatorExists(int _OpId);
    bool isXMLParserOK();
    void GetMenuTree(TTreeView* TV);
    int GetKeysNum(int _Id = 0);
    AnsiString GetStatServerHost();
    int GetStatServerPort();
    void ChangeStatServer();
    bool RestoreFile(AnsiString FileName);
    bool IfKeysExist(int _Id);
    void GetConfigTree(/*TVirtualStringTree *TV,*/ TStringList* RuleSL, bool FillRules);//Igor
    void ProcessCfgNode(/*TVirtualStringTree *TV, */_di_IXMLNode ProcessedNode, /*TVirtualNode* ParentNode, */bool ProcessAttributes, AnsiString NodeId, TStringList* RuleSL, bool FillRules);//Igor
    bool SaveCfgFile(/*TVirtualStringTree *TV*/);//Igor
    void SaveCfgNode(/*TVirtualStringTree *TV, */_di_IXMLNode ProcessedNode, /*TVirtualNode* ParentNode = NULL, */AnsiString Margin = "");//Igor
    void ClearConfigTree(/*TVirtualStringTree *TV*/);//Igor
    bool DeleteAllNodes(/*TVirtualStringTree *TV*/);//Igor
    void DeleteCfgNode(/*TVirtualStringTree *TV, TVirtualNode* ParentNode, */AnsiString Margin=" ");//Igor

    bool FillCountryCodes(AnsiString FileName);
    AnsiString GetCountryName(AnsiString CountryCode);
    int recepientMT;

    bool operatorExist(int opID);
};
//---------------------------------------------------------------------------
#endif
