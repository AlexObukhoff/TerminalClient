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

 // ������� ��������
 // ���������
 const BYTE* cmdGetKKMState;
 size_t cmdGetKKMStateSize;
 // �����
 const BYTE* cmdGetKKMMode;
 size_t cmdGetKKMModeSize;
 // ������ ������
 const BYTE* cmdPrintText;
 size_t cmdPrintTextSize;
 // �����
 const BYTE* cmdCut;
 size_t cmdCutSize;
 // ���� � �����
 const BYTE* cmdEnterMode;
 size_t cmdEnterModeSize;
 // ����� �� ������
 const BYTE* cmdExitMode;
 size_t cmdExitModeSize;
 // ������� �����
 const BYTE* cmdOpenSession;
 size_t cmdOpenSessionSize;
 // ������� ���
 const BYTE* cmdOpenCheque;
 size_t cmdOpenChequeSize;
 // ����������� ������
 const BYTE* cmdRegisterGoods;
 size_t cmdRegisterGoodsSize;
 // ������� ���
 const BYTE* cmdCloseCheque;
 size_t cmdCloseChequeSize;
 // Z-�����
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

  void PrintCheck(AnsiString text, std::string barcode = ""); // ������� ���
  void PrintCheck(double Money, AnsiString Text); // ���������� ���
  void PrintZReport(AnsiString Text = "");
};
#endif
