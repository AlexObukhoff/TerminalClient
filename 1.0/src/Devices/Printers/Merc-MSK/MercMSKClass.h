//---------------------------------------------------------------------------

#ifndef MercMSKClassH
#define MercMSKClassH
//---------------------------------------------------------------------------
#endif

#include <string>
#include "CPrinter.h"

class CMercMSK : public CPrinter
{
 private:
  std::string printerPassCode;
  std::string cmdResult[0xFF];

  WORD translateToNumeric(BYTE* buffer, unsigned int size);
  void logStatus(WORD kkmStatus, BYTE status1, BYTE status2, BYTE status3, BYTE status4); // ����������� ������� ���
  void prepareCmdResults();

  // ������ ���������� ������ ��������
  void prologueCmd(BYTE cmdNum); // �������� ������ �������
  void epilogueCmd(); // �������� ������ �������
  void addStringToCommand(std::string s); // �������� ������ � ������� + ����������� �����
  void addByteToCommandAsString(BYTE b); // �������� ���� � ������� + ����������� �����
  void addByteToCommand(BYTE b); // �������� ���� � ������� + ����������� �����

  // ������� ���
  WORD cmd_31(std::string cashierName); // ����������� �������
  WORD cmd_36(std::string text); // ������ ������������� ���������
  WORD cmd_45(); // ������ ���. � ������ ���
  WORD cmd_52(BYTE linesFeed, bool cut); // ���������� �������� � ������� �����
  WORD cmd_53(double cash, std::string text); // ���������� ��������
  WORD cmd_5C(std::string param); // �������� ��������� ���
  WORD cmd_4F(); // ��������� �����
  WORD cmd_5F(BYTE type, BYTE flags); // ������ ������
  WORD cmd_53_openDocument(double price); // ������ ���������� ���������
  WORD cmd_53_cancelDocument(); // ������������ ��������

  void cmd_53_fiscalField(std::string type, std::string flags, unsigned int row, std::string data);
  void cmd_53_fiscalSale(unsigned int row, std::string price);

  WORD execCommand();            // ��������� �������
 public:
  CMercMSK(int comPort, int baudRate = 0, TLogClass* logClass = NULL);
  virtual ~CMercMSK();

  int Initialize();
  bool IsPrinterEnable();
  bool IsItYou();
  void GetState();

  void PrintCheck(AnsiString text, std::string barcode = ""); // ������� ���
  void PrintCheck(TStringList* text) {}
  void PrintCheck(double Money, AnsiString Text); // ���������� ���
  void PrintXReport(AnsiString Text = "");
  void PrintZReport(AnsiString Text = "");
};
