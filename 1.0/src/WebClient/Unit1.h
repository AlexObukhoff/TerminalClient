//---------------------------------------------------------------------------

#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <AppEvnts.hpp>
#include "SHDocVw_OCX.h"
#include <ExtCtrls.hpp>
#include <OleCtrls.hpp>
#include <mmsystem.h>
#include <fstream>

//---------------------------------------------------------------------------

#include "TWConfig.h"
#include "TModemThread.h"
#include "TPayment.h"
#include "TPinPayment.h"
#include "THalfPinPayment.h"
#include "TMetroPayment.h"
#include "TAviaPayment.h"
#include "CCNETdevice.h"
#include "DeviceClass.h"
#include "CValidator.h"
#include "CKeyboard.h"
#include "IskraKeybDevice.h"
#include "ID003_1device.h"
#include "ID003_2device.h"
#include "JCMdevice.h"
#include "CPrinter.h"
#include "Citizen268Class.h"
#include "ShtrihPrinter.h"
#include "cbm1000t2.h"
#include "CWatchDog.h"
#include "WatchDog.h"
#include "WatchDogdevice.h"
#include "WatchDogOSMPdevice.h"
#include "WatchDogOSMP2device.h"
#include "TWConfig.h"
#include "LogClass.h"
#include "CMoneyCounter.h"
#include "PacketSender.h"
#include "CSPacketSender.h"
#include "SSPacketSender.h"
#include "SMSSender.h"
#include "TFileMap.h"
#include "StarTUP900Class.h"
#include "StarTSP700Class.h"
#include "CustomPrnClass.h"
#include "WinPrnClass.h"
#include "CitizenPPU231Class.h"
#include "CitizenPPU232Class.h"
#include "CitizenPPU700Class.h"
#include "Epson442Class.h"
#include "ICTDevice.h"
#include "PRN609_012R.h"
#include "SwecoinTTP2010Class.h"
#include "V2Edevice.h"
#include "SevenZipVCL.hpp"
#include "GebeGCTClass.h"
#include "TGetCardsInfoThread.h"
#include "Prim21kClass.h"
#include "Prim08TKClass.h"
#include "WatchDogAlarmDevice.h"
#include "CardReader.h"
#include "MetroCardDevice.h"
#include "NV9_CCNETdevice.h"
#include "WatchDogPlatixDevice.h"
#include "wp_t833.h"
#include "atolprinter.h"
#include "TPSound.h"
#include "KtekKeybDevice.h"
#include "SBK2Device.h"
#include "LDOGDevice.h"
#include "FairPayWDDevice.h"
#include "CitizenCPP8001Class.h"
#include "MEIDeviceClass.h"
#include "SIM2OSMPdevice.h"
#include "printreceiptengine.h"
#include "CCoinAcceptor.h"
#include "NRIdevice.h"

#include "StarTSP600Class.h"
#include "TTaxPayment.h"
#include "scanner.h"
#include "nullscanner.h"
#include "wp_k833.h"
#include "bd2class.h"
#include "notebook.h"
#include "BaseCardReader.h"
#include "CSankyoCardReader.h"

#include "boost/ptr_container/ptr_map.hpp"
#include "TFunctionThread.cpp"


typedef boost::ptr_map<int, CMoneyCounter> TMoneyCountersArray;
typedef boost::ptr_map<int, CMoneyCounter>::iterator TMoneyCountersArray_iterator;
typedef boost::ptr_map<int, TDeviceClass> TValidatorsArray;
typedef TFunctionThread<AnsiString> TPaymentInitThread;

int delayForm2 = 60;    //таймаут на Form2 - инкассация

class TForm1 : public TForm
{
__published:        // IDE-managed Components
    TCppWebBrowser *CppWebBrowser1;
    TTimer *CCNETStateChange;
    TTimer *CheckTimeTimer;
    TApplicationEvents *ApplicationEvents1;
    TTimer *CheckThreadsTimer;
    TTimer *PaymentTimeOutTimer;
    TTimer *StartAppTimer;
    TTimer *CheckPrinterStateTimer;
    TTimer *Timer1;
    TTimer *process;
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall CppWebBrowser1DocumentComplete(TObject *Sender, LPDISPATCH pDisp, Variant *URL);
    void __fastcall CCNETStateChangeTimer(TObject *Sender);
    void __fastcall CheckTimeTimerTimer(TObject *Sender);
    void __fastcall ApplicationEvents1Message(tagMSG &Msg, bool &Handled);
    void __fastcall CheckThreadsTimerTimer(TObject *Sender);
    void __fastcall PaymentTimeOutTimerTimer(TObject *Sender);
    void __fastcall StartAppTimerTimer(TObject *Sender);
    void __fastcall CheckPrinterStateTimerTimer(TObject *Sender);
    void __fastcall CppWebBrowser1BeforeNavigate2(TObject *Sender,
    LPDISPATCH pDisp, Variant *URL, Variant *Flags,
    Variant *TargetFrameName, Variant *PostData, Variant *Headers,
    VARIANT_BOOL *Cancel);
    void __fastcall Timer1Timer(TObject *Sender);
    void __fastcall FormCreate(TObject *Sender);
    void __fastcall processTimer(TObject *Sender);
    void __fastcall ApplicationEvents1Restore(TObject *Sender);
private:        // User declarations
    bool checkRecipient(unsigned int operatorId);
    int DeleteOldStatPackets(AnsiString a_work_directory);
    std::string parseScannerData(unsigned int operatorId, std::string data, BYTE type);
//    void ProcessGenKeys();
//    void GenKey();
    int GetKeyRequestsCount(TStringList* FileNames = NULL);
    void PrintAllUnprintedCheques();
    bool PrintRawData(std::string &Report, double Sum);
    int GetVolumeControlID();
    bool SetSoundVolume(DWORD dwVolume);
    void SendEMail(EMailTypes MessageType, AnsiString Header,AnsiString Body);
    void CheckTimeProc();
    void __fastcall StartApp();
    void EnterErrorMode();
    void QuitErrorMode();
    void BillEvent(double Nominal);
    void __fastcall RunConn();//start conn
    void RunExplorer();//start windows Explorer
    void __fastcall CloseConn();//close conn
    bool __fastcall TerminateConn();//terminate conn
    void DeviceStateChanged();
    void CheckThreads();
    void GoToMainMenu();
    void GoToPayment(int isButtonActive = 0);
    void GoToPrinterError();
    void PreparePaymentNoPrinter();
    std::string getFullPathURL(std::string);
    void GoToPaymentComplete(bool t_processing, bool t_Summ, bool t_msg = false, char* t_cmd = "");
    void GoToMessage(int, std::string = "");
    void GoToFullScreenMessage(AnsiString tParam, int ErrorCode = 0, std::string msg = "");
    std::string makePaymentURLParams(int);
    void PerformBalance();
    void askUserToIncassation();
    void PerformIncassation();
    void PerformPaperReload();
    bool AlreadyRunning(void);
    AnsiString MakeLine(int LeftMargin, AnsiString ParamName,int Margin, AnsiString ParamValue);
    void RenameTempFiles(AnsiString Dir);
    bool DeleteDir(AnsiString DirName);
    bool PrintZReport();
    void CheckForNumDBUpdate(void);

    int ValidatorPrevStatus;
    int ValidatorPrevMode;

    int CoinAcceptorPrevStatus;
    int CoinAcceptorPrevMode;

    int PrinterPrevMode;
    int PrinterPrevStatus;

    int CardReaderPrevMode;
    
    int WatchDogPrevMode;

    TPaymentInitThread* PaymentInitThread;
    AnsiString GetHWErrorDescription(int status);
    void EnterServiceMenu();
    void ShutDown();
    void CheckTerminalState();
//    bool GenKeys(int KeysId, AnsiString Login, AnsiString Password, AnsiString URL, AnsiString& Message);
    void ShowSetupForm();
    void SetTopWindow(HWND TopWindowHandle);
    void saveDataToFile(const char* Data);
    void replace_with(std::string & src, const std::string & what, const std::string & with);
// Varible members
    bool CheckMonSrvrConnectRQ;
    AnsiString CfgFileName;
    AnsiString OperCfgFileName;
    const std::string m_menuFileName;
    MMRESULT mmr;
    HMIXEROBJ ghmx;
    short PrevTerminalState;
    TPSound *Sound;
    bool SendingSMSAllowed;
    int ValidatorID;
    int BillAcceptorFatalErrorsCount;
    bool BillAcceptorFatalErrorSent;
    TDateTime RegisterCommandDT;
    TDateTime ProgramStartDT;
    bool BillAcceptorFatalError;
    ULONG USB1_OVERCURRENT_MSG;
    ULONG USB2_OVERCURRENT_MSG;
    ULONG WDT_KEYUP_MSG;
    ULONG WDT_KEYDOWN_MSG;
    ULONG DEVICE_STATE_CHANGED;
    bool FinishPaymentInitiated;
    AnsiString PacketFileName;
    bool FirstValidatorErrorSent;
    bool FirstPrinterErrorSent;
    bool FirstWatchDogErrorSent;
    bool FirstCardReaderErrorSent;
    bool LogWriteErrorFound;
    bool LogErrorDone;
    TDateTime NextCheckDBUpdateTime;
    HWND ActiveWindow;
    TMoneyCountersArray MoneyCounters;
    TValidatorsArray Validators;
    void logicMoneyTransfer(const AnsiString& LocationString,const TLocationParser& Location,const AnsiString& recepient,std::string* newLocation);
    bool isNavigateLogicMoneyTransfer(const TLocationParser& Location);
    std::string savedLocation;
    void saveEmpty_iface_details();
    double __fastcall ValidateCoinAcceptorValue(unsigned int Value, unsigned int Precession);
    const int cSendTokenMinute;
public:                // User declarations
    virtual __fastcall ~TForm1();
    __fastcall TForm1(TComponent* Owner);
    void Reboot();
    void SendNotification(EMailTypes MessageType, AnsiString TypeDescr,BYTE Code, AnsiString CodeDescr);
    AnsiString SendSMSNotification(AnsiString DMessage = "", bool bDoNoStore = false);
    void Payment_Init(AnsiString);
    void Navigate(AnsiString);
    void HardwareInit();
    void FinishPayment(bool bCancel, int iCommand = 0);
    void CheckConnectionToCyberplat();
    void CheckConnectionToMonitoringServer();
    HANDLE StartProgram(AnsiString ApplicationName);
    bool IsProcessRunning(HANDLE PrHandle);
    void __fastcall LogError();
// Variable members
   CScanner *scannerDevice;
   CBaseCardReader* cardReaderDevice;

    HANDLE ConnHandle;
    TLogClass* Log;
    std::string Locale;
    TWConfig* Cfg;
    TFileMap *FileMap;
    CValidator*    Validator;
    CCoinAcceptor* CoinAcceptor;
    CPrinter*      Printer;
    CWatchDog*     WatchDog;
    CKeyboard*     Keyboard;
    CCardReader*	 CardReader;
    CMoneyCounter* BillCounter;
    TModemThread*  ModemThread;
    TGetCardsInfoThread *GetCardsInfoThread;
    TPayment*      Payment;
    TXMLInfo *InfoFile;
    TPaymentBook* PB;
    bool InitFinished;
    std::string getBarCodeString(int _validator);
protected:
    std::string GetLocale(std::auto_ptr <TLocationParser> &Location);
    void PrepareBalance(int CassetteID, PrintReceiptTemplate &PrintTemplate);
    bool IncassTerminal(int CassetteID);
    bool PrintBalance(int CassetteID);
    bool PrintCheque(std::string &FieldsData, double Sum, double Comission, bool PaymentResult,TPayment* Payment/*, char* technologProc = ""*/);
    bool PrintPaymentErrror(const PrintReceiptTemplate& prt,double Sum,TPayment* Payment);
    void WaitForForm2Action();
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//----------------------------------------------------------------------------
class paymentParameters
{
public:
  double m_sum;
  double m_paysum;
  double m_minsum;
  double m_fixsum;
  double m_maxsum;
  double m_cms;
  static double m_minshowsum;
  double m_rest;
  bool m_is_payment_will_pass;

  std::string m_s_sum;
  std::string m_s_paysum;
  std::string m_s_minsum;
  std::string m_s_fixsum;
  std::string m_s_maxsum;
  std::string m_s_cms;
  std::string m_s_minshowsum;
  std::string m_s_rest;
  std::string m_pstate;

  paymentParameters(bool nominal = true);
  void swap(const paymentParameters& rhs);
  paymentParameters(const paymentParameters& ob);
  paymentParameters& operator=(const paymentParameters& ob);
  double getMinLocaleRemainder(double, std::vector<int>&, int);
};
//---------------------------------------------------------------------------
#endif
