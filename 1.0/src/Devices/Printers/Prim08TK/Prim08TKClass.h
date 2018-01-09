//---------------------------------------------------------------------------

#ifndef Prim08TKClassH
#define Prim08TKClassH
//---------------------------------------------------------------------------
#endif

#include "CPrinter.h"
#include "Prim08TKThread.h"


#define FloatPrecision                            3
#define FirstDataByteAfterFrames                  21
#define FirstDataByteAfterFramesWithOutDateTime   9
#define CountByteOfFramesWithOutDateTime          14
#define CountByteOfFramesWithDateTime             27
#define FirstDataByteAfterFramesInAnswer          29
#define  CharsInString 40
#define  PassLength 4
#define  UserPassword "AERF\0"

#define CPC_BufferSize 2048

class CPrim08TKClass : public CPrinter
{
private:
//====================================================================================
    //BYTE Command[BufferSize];
    //BYTE Answer[BufferSize];
    //unsigned int CommandSize;
    //unsigned int AnswerSize;
    bool DSE_OK_Sensitive;

    BYTE Data[CPC_BufferSize];
    BYTE Code[2];
    char WinLetters[CharsInString+1];
    char DOSLetters[CharsInString+1];
    char Pass[PassLength+1];

    BYTE CommandNumber;
    int Position;
    BYTE ExecCode;
    int PasswordLength;
    tStringType StringType;
    BYTE CurrentCharsInString;
    int NumberOfLastCheque;
    float RestOfMoney;
    sDateTime DateTime;

    BYTE FiscalDocumentState;
    BYTE PrinterMode;
    BYTE ShiftState;

    void SetNumberOfLastCheque(int num);
    void SetRestOfMoney(float money);

    void SetBCC(BYTE *Buffer, unsigned int index);

    BYTE GetCommandNumber();
    int  GetUserPasswordLength();

    int BeginZReportNumber;
    int EndZReportNumber;

    void SetQuantityToBuffer(BYTE* Buffer, int index, float Number);
    void SetPriceToBuffer(BYTE* Buffer, int index, float Number);
    void SetMoneyToBuffer(BYTE* Buffer, int index, float Number);
    void SetDateToBuffer(BYTE *Buffer, int pos);
    void SetTimeToBuffer(BYTE *Buffer, int pos);
    void SetDateTimeToBuffer(BYTE *Buffer, int pos);
    void SetTextToBuffer(BYTE* Buffer, int index, char *Text);
    char *GetDateFromBuffer(BYTE *Buffer, int pos);
    char *GetTimeFromBuffer(BYTE *Buffer, int pos);
    WORD GetStatusWORD(BYTE *Buffer, int index);
    BYTE GetBYTE(BYTE *Buffer, int index);
    WORD GetWORD(BYTE *Buffer, int index);
    char* TrimString(char *string);
    char* AlignStringToCenter(char *str, int width);
    void SetByteToBuffer(BYTE *Buffer, char* Byte, int index);
    double GetCorrectResultR(double Number, int Offset);
    double round(double Number, int Precision);
    float GetCorrectMoney(float Price, float Quantity);
    void ConvertFloatToString(double Number, char *Buffer, int Precision);
    void PrepareFloat(float &Number, BYTE Precision);
    float GetMoneyFromBuffer(BYTE *Buffer, int &index);
    void ClearBuffers();
    int FindByteInBuffer(BYTE *Buffer, int size, BYTE Value, int Position = 0);
    int GetZReportFromBuffer(BYTE *Buffer, int &index);
    int GetFirstZReportFromBuffer(BYTE *Buffer, int &index);
    void SetZReportNumberIntoBuffer(BYTE* Buffer, int& ind, int ZReportNumber);
    AnsiString GetTextFromBuffer(BYTE *Buffer, int& index);

    BYTE CharToBYTE(BYTE ch);

    void StringToOem(char *Text);// result in DOSLetters
    void OemToString(char *Text);// result in WinLetters

    void AttachStandardFrames(BYTE*& Buffer, int ETXIndex, char *CommandCode, bool SetDateTime);

    BYTE GetError(int index);
    BYTE GetStatus(int index);

    void SetUserPassword(char *password);
    char *GetUserPassword();

    char* GetErrorExplanationString(char *DestPointer, BYTE Code);
    float GetRestOfMoney();

//==============================================================================

    int SendEvent();
    int StateExplanation(BYTE Value);

//=========================Commands=============================================
  bool GetStateBool();
  bool CommandOpenCheque(char* Type, AnsiString text= "");
  bool CommandPrintSale(char *Description, float Quantity, float Price, char* Unit, char* Section, AnsiString text = "");
  bool CommandPrintSummary();
  bool CommandPrintString(AnsiString text);
  bool CommandPrintTotalOfCheque(float Money, char* PaymentDescriptor, char* CardNumber);
  bool CommandCloseCheque();
  bool CommandSetDateTimeOne();
  bool CommandGetDateTime();
  bool CommandGetDateTime(char* Date, char* Time);
  bool CommandGetLastChequeNumber();
  bool CommandAddMoneyToCash(float Money);
  bool CommandTakeMoneyFromCash(float Money);
  bool CommandOpenSession();
  bool CommandOpenShift();
  bool CommandSetDocumentsParameters();
  bool CommandSetPrintersParameters();
  bool CommandGetXReportData();
  bool CommandXReport();
  bool CommandZReport();
  bool PrintSaleTicket(BYTE ucDocumentType, char *pcWareName, char *pcMeasure,float fPrice, BYTE ucPaymentMethod);

  //команды для работы с Z-отчётами в памяти СКЛ
  bool CommandZReportIntoBuffer();
  bool CommandZReport(int SessionNumber);
  bool CommandClearSKL();
  //номер первого нераспечатанного отчёта будем писать в одно из названий основных платежей
  bool CommandSaveFirstZReportNumber(int SessionNumber);
  int  CommandGetFirstZReportNumber();

  //нефискальные команды
  bool CommandOpenNFCheque();
  bool CommandPrintNFString(AnsiString text);
  bool CommandCloseNFCheque();

  bool CommandGetResources();
//==============================================================================

  virtual std::string GetStateDescription();
  virtual std::string GetStatusDescription(BYTE StatusCode);
  void SendCommand();

protected:
  virtual void SendPacket(BYTE*& command, char* CommandCode, BYTE* data = NULL, int datalen = 0, bool SetDateTime = true);
public:
  CPrim08TKClass(int ComPort, int BaudRate = 0, TLogClass* _Log = NULL);
  virtual ~CPrim08TKClass();

//=========================Commands=============================================
  void WriteErrorsToLog();
  virtual void GetState();
//==============================================================================
  virtual void PrintCheck(AnsiString text, std::string barcode = "");
  virtual void PrintNotFiscalCheck(AnsiString Text){PrintCheck(Text);};
  virtual void PrintCheck(double money, AnsiString text);
  bool         CommandCancelCheque();
  virtual void PrintXReport(AnsiString Text = "");
  virtual void PrintZReport(AnsiString Text = "");
  virtual void CashIncassation(double Money = 0);
  bool         PrintZReportsFromBuffer(int BeginSessionNumber = 0, int EndSessionNumber = 0);
  virtual bool IsItYou();
  virtual bool IsPrinterEnable();
  virtual int Initialize();
};
