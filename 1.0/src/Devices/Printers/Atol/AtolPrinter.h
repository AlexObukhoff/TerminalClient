//---------------------------------------------------------------------------

#ifndef AtolPrinterH
#define AtolPrinterH
//---------------------------------------------------------------------------
#include <string>
#include "CPrinter.h"

typedef enum
{
 FelixRK = 0,
 Felix3CK,
 Tornado,
 MercuryMSK,
 Felix80K
}EAtolPrinter;


class CAtolPrinter : public CPrinter
{
protected:
 EAtolPrinter m_printerType;
 WORD m_password;
 DWORD m_adminPassword;
 std::string cmdResult[0xFF];

 // команда принтера
 // состояние
 const BYTE* cmdGetKKMState;
 size_t cmdGetKKMStateSize;
 // режим
 const BYTE* cmdGetKKMMode;
 size_t cmdGetKKMModeSize;
 // печать строки
 const BYTE* cmdPrintText;
 size_t cmdPrintTextSize;
 // отрез
 const BYTE* cmdCut;
 size_t cmdCutSize;
 // вход в режим
 const BYTE* cmdEnterMode;
 size_t cmdEnterModeSize;
 // выход из режима
 const BYTE* cmdExitMode;
 size_t cmdExitModeSize;
 // открыть смену
 const BYTE* cmdOpenSession;
 size_t cmdOpenSessionSize;
 // открыть чек
 const BYTE* cmdOpenCheque;
 size_t cmdOpenChequeSize;
 // регистрация товара
 const BYTE* cmdRegisterGoods;
 size_t cmdRegisterGoodsSize;
 // закрыть чек
 const BYTE* cmdCloseCheque;
 size_t cmdCloseChequeSize;
 // Z-отчет
 const BYTE* cmdZReport;
 size_t cmdZReportSize;
 
 DWORD toBCD(DWORD value);
 void reverse(void* buffer, size_t bufferSize);
 void logKKMState(BYTE flag1, BYTE flag2);
 void prepareCommandResults();
 void prepareCommand(const BYTE* cmd, size_t cmdSize);
 WORD execCmd(const BYTE* cmd, size_t cmdSize);
 WORD enterMode(bool isEnter, BYTE mode = 0);
 WORD printZReport();
 BYTE getCurrentMode();
 void printText(const std::string& text);
 void initKKMCommands();
public:
 CAtolPrinter(EAtolPrinter printerType, int comPort, int baudRate, TLogClass* logClass);
 ~CAtolPrinter();

 // derived from CPrinter
  int Initialize();
  void GetState();

  void PrintCheck(AnsiString text, std::string barcode = ""); // Обычный чек
  void PrintCheck(double Money, AnsiString Text); // фискальный чек
  void PrintZReport(AnsiString Text = "");
};
#endif
