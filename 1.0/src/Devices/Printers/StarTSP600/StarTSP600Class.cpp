//---------------------------------------------------------------------------


#pragma hdrstop

#include "StarTSP600Class.h"
#include "StarTSP600Thread.h"
#include "StarTSP600Defines.h"
#include "boost\format.hpp"

//---------------------------------------------------------------------------

#pragma package(smart_init)

CStarTSP600::CStarTSP600(const std::string& printerName,int comPort, int baudRate, TLogClass* logClass) : CPrinter(comPort, logClass, printerName.c_str())
{
 DeviceName = printerName.c_str();
 LoggingErrors = true;
 DataLength = 1;
 printerPassCode = "0000";

 COMParameters->Parity = NOPARITY;
 COMParameters->timeout = 600;
 if(baudRate > 0)
 {
  COMParameters->BaudRate = baudRate;
  if(Port)
   Port->ReopenPort();
 }

 prepareCmdResults();
}

CStarTSP600::~CStarTSP600()
{

}

////////////////////////////////////////////////////////////////////////////////
// public methods
int CStarTSP600::Initialize()
{
 GetState();
 if(PrinterEnable)
  SetInitialized();

 return 0;
}

bool CStarTSP600::IsPrinterEnable()
{
 GetState();
 return PrinterEnable;
}

void CStarTSP600::PrintCheck(AnsiString text, std::string barcode)
{
 CharToOem(text.c_str(), text.c_str());
 std::string checkText = text.c_str();

 if(cmd_36(checkText) != CMDRES_SUCCESS)
  Log->Write("Nonfiscal mode print error");

 cmd_52(5, true);
}

// ������ ����������� ����
void CStarTSP600::PrintCheck(double Money, AnsiString Text)
{
 if(Fiscal == false)
 {
  Log->Write("Try to print fiscal receipt with nonfiscal configuration");
  return;
 }

 GetState();
 if(DeviceState->SubStateCode == ST_SESSION_CLOSED)
  cmd_31(""); // ������������ �������

 // �������� ������� ���
 CharToOem(Text.c_str(), Text.c_str());
 std::string checkText = Text.c_str();
 cmd_36(checkText);

 // �������� ���������� ���
 if(cmd_53_openDocument(Money) != CMDRES_SUCCESS)
 {
  cmd_53_cancelDocument();
  cmd_4F();
 }

 // ����� � ������ ������ �� ����, �.�. ��������� ����� �������� ���������
// cmd_52(5, true);
}

void CStarTSP600::PrintXReport(AnsiString Text)
{
 if(Fiscal == false)
 {
  Log->Write("Try to print X-report with nonfiscal configuration");
  return;
 }
 cmd_5F(0x31, 0x04);
}

void CStarTSP600::PrintZReport(AnsiString Text)
{
 if(Fiscal == false)
 {
  Log->Write("Try to print X-report with nonfiscal configuration");
  return;
 }

 CharToOem(Text.c_str(), Text.c_str());
 std::string checkText = Text.c_str();
 if(checkText != "")
  cmd_36(checkText);

 Log->Write("PrintZReport");
 cmd_5F(0x30, 0x04);
}

void CStarTSP600::GetState()
{
 PrinterEnable = false;

 WORD kkmStatus = 0x0000;
 BYTE extStatus1 = 0x00;
 BYTE extStatus2 = 0x00;
 BYTE extStatus3 = 0x00;
 BYTE extStatus4 = 0x00;

 // ���������� �������� ������
 if(cmd_45() == CMDRES_SUCCESS)
 {
  kkmStatus = translateToNumeric(Answer + 3, sizeof(WORD));

  if(cmd_5C("306") == CMDRES_SUCCESS) // ����������� ������ ��������
  {
   extStatus1 = translateToNumeric(Answer + 20 + 0, sizeof(BYTE));
   extStatus2 = translateToNumeric(Answer + 20 + 2, sizeof(BYTE));
   extStatus3 = translateToNumeric(Answer + 20 + 4, sizeof(BYTE));
   extStatus4 = translateToNumeric(Answer + 20 + 6, sizeof(BYTE));
   PrinterEnable = true;
  }
 }

 // �������� ������
 if(PrinterEnable)
 {
  logStatus(kkmStatus, extStatus1, extStatus2, extStatus3, extStatus4);
 }
 else
 {
  Log->Write("Error getting printer status");
  DeviceState->StateCode = DSE_NOTMOUNT;
  DeviceState->OutStateCode = DSE_NOTMOUNT;
  PrinterEnable = false;
 }

 ChangeDeviceState();
}

bool CStarTSP600::IsItYou()
{
 bool result = false;
 if(cmd_45() == CMDRES_SUCCESS);
  result = true;

 return result;
}

////////////////////////////////////////////////////////////////////////////////
WORD CStarTSP600::execCmd()
{
 WORD result = CMDRES_FAILED;
 ClearAnswer();
 SendType = RecieveAnswer;

 Sleep(150);

 DeviceThread = new CStarTSP600Thread();
 Start();
 delete DeviceThread;

 if(DeviceState && DeviceState->ExecCode == EC_SUCCESS)
 {
  result = translateToNumeric(Answer + 8, sizeof(WORD));
  Log->Write(cmdResult[result].c_str());
 }

 return result;
}
////////////////////////////////////////////////////////////////////////////////
// ������� � ���

// ����������� �������
WORD CStarTSP600::cmd_31(std::string cashierName)
{
 prologueCmd(0x31);

 // ����� �������
 Command[CommandSize + 0] = '0';
 CommandSize += 3;
 CommandSize += 40;
 Command[CommandSize] = DIV;
 CommandSize++;
 epilogueCmd();

 return execCmd();
}

// cmd_36() - ������� �������� � ������������ �����
WORD CStarTSP600::cmd_36(std::string text)
{
 WORD result = CMDRES_FAILED;

 prologueCmd(0x36);
 epilogueCmd();

 result = execCmd();

 if(result == CMDRES_SUCCESS)
 {
  memcpy(Command, text.c_str(), text.length());
  CommandSize = text.length() + 1;
  // ������ � ����� ������ ���� ����������
  Command[CommandSize] = ESC;
  CommandSize++;
  execCmd();

  // ���������� ��� ������ �� ������ ������ �� �����
  Command[0] = ESC;
  CommandSize = 1;

  result = execCmd();
 }
 return result;
}

// cmd_45() - �������� ������ �������� �������� (�������� ��� �������������)
WORD CStarTSP600::cmd_45()
{
 prologueCmd(0x45);
 epilogueCmd();

 return execCmd();
}

WORD CStarTSP600::cmd_4F()
{
 prologueCmd(0x4F);
 epilogueCmd();

 return execCmd();
}

// cmd_52() - ���������� �������� � ������� �����
WORD CStarTSP600::cmd_52(BYTE linesFeed, bool cut)
{
 prologueCmd(0x52);

 addByteToCommandAsString(linesFeed);
 if(cut)
  addByteToCommandAsString(0x01);
 else
  addByteToCommandAsString(0x00);

 epilogueCmd();

 return execCmd();
}

// cmd_53() - ���������� ��������
WORD CStarTSP600::cmd_53_openDocument(double price)
{
 char priceValue[11];
 memset(priceValue, 0, 11);
 sprintf(priceValue, "%.2f", price);
 std::string strValue = priceValue;

 prologueCmd(0x53);
 addByteToCommand(0x30);
 addStringToCommand("04"); // �������� ���������

 // ���������� ������������ ����������
 addStringToCommand("016");

 // �������� ���������
 std::string filler(40, '=');
 cmd_53_fiscalField(99, 20, filler, 0, 0);
 cmd_53_fiscalField(01, 20, "", 1, 0);
 cmd_53_fiscalField(02, 20, "", 2, 0);
 cmd_53_fiscalField(03, 20, "", 3, 0);
 cmd_53_fiscalField(04, 20, "", 4, 0);
 cmd_53_fiscalField(99, 20, filler, 5, 0);

 cmd_53_fiscalField(0, 20, "", 6, 0); // ����� ���
 cmd_53_fiscalField(7, 20, "", 6, 32); // ����� ���������
 cmd_53_fiscalField(10, 20, "", 7, 0); // ���, ���
 cmd_53_fiscalField(5, 20, "", 8, 0); // ���� ��������
 cmd_53_fiscalField(8, 20, "", 8, 32); // ����� ����
 cmd_53_fiscalField(6, 20, "", 9); // ������
 cmd_53_fiscalSale(strValue, 10); // ���� ������
 cmd_53_fiscalField(12, 20, "", 11); // �������� �����
 cmd_53_fiscalField(13, 20, strValue, 12); // ���������� �����
 cmd_53_fiscalField(14, 20, "", 13); // �����

 epilogueCmd();

 return execCmd();
}

WORD CStarTSP600::cmd_53_cancelDocument()
{
 prologueCmd(0x53);
 addByteToCommand(0x30);
 addStringToCommand("08"); // ������������� ���������
 addStringToCommand("000");
 epilogueCmd();

 return execCmd();
}

WORD CStarTSP600::cmd_53_closeDocument()
{
 prologueCmd(0x53);
 addByteToCommand(0x30);
 addStringToCommand("04");
 addStringToCommand("000");
 epilogueCmd();

 return execCmd();
}

void CStarTSP600::cmd_53_fiscalField(unsigned int type, unsigned int flags, const std::string& data, unsigned int row, unsigned int col)
{
 addStringToCommand((boost::format("%02d") % type).str());
 addStringToCommand((boost::format("%04d") % flags).str());
 addStringToCommand((boost::format("%02d") % col).str());
 addStringToCommand((boost::format("%03d") % row).str());

 CommandSize += 5; // ���������� �������������� �����

 memcpy(Command + CommandSize, data.c_str(), data.length());
 CommandSize += 40;

 Command[CommandSize] = DIV;
 CommandSize++;
}

void CStarTSP600::cmd_53_fiscalSale(const std::string& price, unsigned int row, unsigned int col)
{
 addStringToCommand("11");
 addStringToCommand("0022");
 addStringToCommand((boost::format("%02d") % col).str());
 addStringToCommand((boost::format("%03d") % row).str());

 Command[CommandSize + 0] = 0x31; // ������
 Command[CommandSize + 3] = 0x30; // ��� ������
 CommandSize += 10;

 CommandSize += 6; // ���������� ������ / ��������
 CommandSize += 12; // ����������

 memcpy(Command + CommandSize, price.c_str(), price.length());

 CommandSize += 12;

 // ������� ��������� ����������
 CommandSize += 6;
}

// cmd_5C() - �������� ��������� ���
WORD CStarTSP600::cmd_5C(std::string param)
{
 prologueCmd(0x5C);
 addStringToCommand(param);
 epilogueCmd();

 return execCmd();
}

// cmd_5F() - ������ �������
WORD CStarTSP600::cmd_5F(BYTE type, BYTE flags)
{
 prologueCmd(0x5F);
 addByteToCommand(type);
 addByteToCommandAsString(flags);
 addStringToCommand("00");
 epilogueCmd();

 return execCmd();
}
////////////////////////////////////////////////////////////////////////////////
// ������ ���������� ������ ��������
// prologueCmd - ������� ������ ������
void CStarTSP600::prologueCmd(BYTE cmdNum)
{
 ClearCommand();
 Command[0] = STX;
 Command[1] = cmdNum;
 CommandSize += 2;
 addStringToCommand(printerPassCode);
}

// epilogueCmd - ������� ������ ������
void CStarTSP600::epilogueCmd()
{
 BYTE bcc = 0;
 for(int i = 1; i < CommandSize; i++)
  bcc += Command[i];

 char arbcc[3];
 sprintf(arbcc, "%02X", bcc);

 Command[CommandSize + 0] = arbcc[0];
 Command[CommandSize + 1] = arbcc[1];
 Command[CommandSize + 2] = ETX;
 CommandSize += 3;
}

void CStarTSP600::addStringToCommand(std::string s)
{
 memcpy(Command + CommandSize, s.c_str(), s.length());
 CommandSize += s.length();

 Command[CommandSize] = DIV;
 CommandSize++;
}

void CStarTSP600::addByteToCommandAsString(BYTE b)
{
 char value[3];
 sprintf(value, "%02X", b);
 Command[CommandSize + 0] = value[0];
 Command[CommandSize + 1] = value[1];
 Command[CommandSize + 2] = DIV;

 CommandSize += 3;
}

void CStarTSP600::addByteToCommand(BYTE b)
{
 Command[CommandSize + 0] = b;
 Command[CommandSize + 1] = DIV;

 CommandSize += 2;
}

WORD CStarTSP600::translateToNumeric(BYTE* buffer, unsigned int size)
{
 char* start_ptr = new char[size * 2+1];
 char* end_ptr;

 memcpy(start_ptr, buffer, size * 2);
 start_ptr[size * 2] = 0x00;

 WORD result = (WORD)strtol(start_ptr, &end_ptr, 16);

 delete[] start_ptr;

 return result;
}

void CStarTSP600::logStatus(WORD kkmStatus, BYTE status1, BYTE status2, BYTE status3, BYTE status4)
{
 DeviceState->StateCode = DSE_OK;
 ////////////////////////
 // ������ ���
 std::string totalStatus = "������ ��������:\r\n";
 if(kkmStatus & 0x0001)
 {
  totalStatus += "����� �������\r\n";
  DeviceState->SubStateCode = ST_SESSION_OPENED;
 }
 else
 {
  totalStatus += "����� �������\r\n";
  DeviceState->SubStateCode = ST_SESSION_CLOSED;
 }

 if(kkmStatus & 0x0010)
  totalStatus += "������� ���������������\r\n";
 else
  totalStatus += "������� �����������������\r\n";

 if(kkmStatus & 0x0020)
  totalStatus += "���������� ������ �������� � �����\r\n";

 if(kkmStatus & 0x0040)
 {
  totalStatus += "���������� ������ ���������\r\n";
  DeviceState->StateCode = DSE_HARDWARE_ERROR;
 }

 ////////////////////////
 // ����������� ������ 1
 if(status1 & 0x04)
  totalStatus += "�������� ���� - ���������\r\n";
 else
  totalStatus += "�������� ���� - �������\r\n";

 if(status1 & 0x08)
 {
  totalStatus += "������� �����, �������� ��� ��������� ������\r\n";
  DeviceState->StateCode = DSE_HARDWARE_ERROR;
 }

 if(status1 & 0x10)
 {
  totalStatus += "������� ����� � ������\r\n";
 }
 else
 {
  totalStatus += "������� �� ����� � ������\r\n";
  DeviceState->StateCode = DSE_NOTMOUNT;
 }

 ////////////////////////
 // ����������� ������ 2
 if(status2 & 0x04)
 {
  totalStatus += "������ �������� �������\r\n";
  // error code here
 }

 if((status2 & 0x40) == 0)
 {
  totalStatus += "������ ��������\r\n";
  DeviceState->StateCode = DSE_HARDWARE_ERROR;
 }

 ////////////////////////
 // ����������� ������ 3
 if(status3 & 0x04)
 {
  totalStatus += "������ �������� ��������\r\n";
  DeviceState->StateCode = DSE_HARDWARE_ERROR;
 }

 if(status3 & 0x08)
 {
  totalStatus += "������ �������������\r\n";
  DeviceState->StateCode = DSE_HARDWARE_ERROR;
 }

 if(status3 & 0x20)
 {
  totalStatus += "����������� ������ ��������\r\n";
  DeviceState->StateCode = DSE_HARDWARE_ERROR;
 }

 if(status3 & 0x40)
 {
  totalStatus += "�������� ���������� �������\r\n";
  DeviceState->StateCode = DSE_HARDWARE_ERROR;
 }

 ////////////////////////
 // ����������� ������ 4
 if(status4 & 0x08)
 {
  totalStatus += "����� ������ � ����������\r\n";
  DeviceState->StateCode = DSE_NEARENDPAPER;
 }

 if(status4 & 0x40)
 {
  totalStatus += "����� ������\r\n";
  DeviceState->StateCode = DSE_NOTPAPER;
 }

 Log->Write(totalStatus.c_str());

 DeviceState->OutStateCode = DeviceState->StateCode;
}

void CStarTSP600::prepareCmdResults()
{
 for(int i = 0; i < 0xFF; i++)
  cmdResult[i] = "����������� ��������� (���������� ��������?)";
/*
 cmdResult[0x00] = "������� ��������� �������";
 cmdResult[0x01] = "������ � ���������� ������, ������� ����������.";
 cmdResult[0x02] = "�� ������� �����";
 cmdResult[0x03] = "�������� ������ ������� ������� � ���������� ������.";
 cmdResult[0x04] = "��������� ����� ���� �������.";
 cmdResult[0x05] = "�������� ������ ���� �������.";
 cmdResult[0x06] = "������ ������ �������.";
 cmdResult[0x07] = "�������� ����.";
 cmdResult[0x08] = "�������� �����.";
 cmdResult[0x09] = "���� ������ ��������� ����, ������������������ � ���������� ������.";
 cmdResult[0x0A] = "�������� �������� �������������. �������� �����������.";
 cmdResult[0x0B] = "����������� ������� �� (�� �. 3.3).";
 cmdResult[0x0C] = "�� ������� �����.";
 cmdResult[0x0D] = "������ �� ���������������.";
 cmdResult[0x0E] = "������������ �������� ������.";
 cmdResult[0x0F] = "������ ������ � ���������� ������.";
 cmdResult[0x10] = "������ ��������� �������.";
 cmdResult[0x11] = "�������� ������ ���������� ����������.";
 cmdResult[0x12] = "�������� ������ �� �����.";
 cmdResult[0x13] = "�������� ������ ���������������.";
 cmdResult[0x14] = "������� �� ��������������.";
 cmdResult[0x15] = "�������� ���� ������� ��� ���������.";
 cmdResult[0x16] = "������ ������ ���������� ������.";
 cmdResult[0x17] = "������������ ��� ������������� ��������� ��������.";
 cmdResult[0x18] = "������������ ���� ������� ����� ������� �����.";
 cmdResult[0x19] = "�������� ������ �������.";
 cmdResult[0x1A] = "���� ��� ����� ���������� ��������� � ����� ������ �����������.";
 cmdResult[0x1B] = "�� ������������.";
 cmdResult[0x1C] = "������ � ������������ ���������� (����������� ��� ����� �� ������� ������).";
 cmdResult[0x1D] = "��� ����� �������.";
 cmdResult[0x1E] = "�������� ����������� ����� (BCC)";
 cmdResult[0x1F] = "��� ���������� �������.";
 cmdResult[0x21] = "���������� ��������� �������� �� ��������� ������� �������� ���������� ��������.";
 cmdResult[0x24] = "����� ������ ����.";
 cmdResult[0x25] = "������ �� �������";
 cmdResult[0x29] = "������������ ������������ ���������� ���������.";
 cmdResult[0x2A] = "������� ��������� ��� �� ��������� ��������� ��������.";
 cmdResult[0x2B] = "������ � ������ ����������������� ������. ������� ����������.";
 cmdResult[0x2C] = "���������� ��������� ������������� ��. �� ��� ����������������.";
 cmdResult[0x2D] = "����� ������� �� ��������� ������� �������� ���������� �������.";
 cmdResult[0x2E] = "������ ������ FLASH ������.";
 cmdResult[0x2F] = "������. ��� �������.";
 cmdResult[0xA0] = "������������ ���. ��������� ���������� ���.";
*/
 cmdResult[0x00] = "������� ��������� �������";
 cmdResult[0x01] = "������ � ���������� ������, ������� ����������.";
 cmdResult[0x02] = "�� ������� �����";
 cmdResult[0x03] = "�������� ������ ������� ������� � ���������� ������.";
 cmdResult[0x04] = "��������� ����� ���� �������.";
 cmdResult[0x05] = "�������� ������ ���� �������.";
 cmdResult[0x06] = "������ ������ �������.";
 cmdResult[0x07] = "�������� ����.";
 cmdResult[0x08] = "�������� �����.";
 cmdResult[0x09] = "���� ������ ��������� ����, ������������������ � ���������� ������.";
 cmdResult[0x0A] = "�������� �������� �������������. �������� �����������.";
 cmdResult[0x0B] = "����������� ������� �� (�� �. 3.3).";
 cmdResult[0x0C] = "�� ������� �����.";
 cmdResult[0x0D] = "������ �� ���������������.";
 cmdResult[0x0E] = "������������ �������� ������.";
 cmdResult[0x0F] = "������ ������ � ���������� ������.";
 cmdResult[0x10] = "������ ��������� �������.";
 cmdResult[0x11] = "�������� ������ ���������� ����������.";
 cmdResult[0x12] = "�������� ������ �� �����.";
 cmdResult[0x13] = "�������� ������ ���������������.";
 cmdResult[0x14] = "������� �� ��������������.";
 cmdResult[0x15] = "�������� ���� ������� ��� ���������.";
 cmdResult[0x16] = "������ ������ ���������� ������.";
 cmdResult[0x17] = "������������ ��� ������������� ��������� ��������.";
 cmdResult[0x18] = "������������ ���� ������� ����� ������� �����.";
 cmdResult[0x19] = "�������� ������ �������.";
 cmdResult[0x1A] = "���� ��� ����� ���������� ��������� � ����� ������ �����������.";
 cmdResult[0x1B] = "�� ������������.";
 cmdResult[0x1C] = "������ � ������������ ���������� (����������� ��� ����� �� ������� ������).";
 cmdResult[0x1D] = "��� ����� �������.";
 cmdResult[0x1E] = "�������� ����������� ����� (BCC)";
 cmdResult[0x1F] = "��� ���������� �������.";
 cmdResult[0x21] = "���������� ��������� �������� �� ��������� ������� �������� ���������� ��������.";
 cmdResult[0x24] = "����� ������ ����.";
 cmdResult[0x25] = "������ �� �������";
 cmdResult[0x29] = "������������ ������������ ���������� ���������.";
 cmdResult[0x2A] = "������� ��������� ��� �� ��������� ��������� ��������.";
 cmdResult[0x2B] = "������ � ������ ����������������� ������. ������� ����������.";
 cmdResult[0x2C] = "���������� ��������� ������������� ��. �� ��� ����������������.";
 cmdResult[0x2D] = "����� ������� �� ��������� ������� �������� ���������� �������.";
 cmdResult[0x2E] = "������ ������ FLASH ������.";
 cmdResult[0x2F] = "������. ��� �������.";
 cmdResult[0x30] = "������ ����� � ����";
 cmdResult[0x31] = "������������ ������ ��� �������� ������� ����";
 cmdResult[0x32] = "������������ ��������� ����";
 cmdResult[0x33] = "������ ����";
 cmdResult[0x34] = "������ ������������������ ���������� ����";
 cmdResult[0x35] = "�������� ��������� ������ ������������� ����";
 cmdResult[0x36] = "���� �����������";
 cmdResult[0x37] = "�������� ���� ��� ����� � ����";
 cmdResult[0x38] = "��� ����������� ������ � ����";
 cmdResult[0x39] = "������������ ��������� ����";
 cmdResult[0x42] = "���� ���������������� ����";
 cmdResult[0x46] = "������ ��������� ������ ����";
 cmdResult[0x47] = "������������ �������� ������ ����";
 cmdResult[0x48] = "�������� ����������� ����� ����";
 cmdResult[0x49] = "���� �������������� � ������� ������ ���. ������� ����������.";
 cmdResult[0x4A] = "���� �� ��������������.";
 cmdResult[0x4B] = "������������ ������ ����";
 cmdResult[0x4C] = "�������� ������ ����������� ����.";
 cmdResult[0x4D] = "���� ��� ��������������.";
 cmdResult[0x4E] = "��������� ���������� ������ � ���������. (16 ������ - ����������� ����)";
 cmdResult[0x4F] = "����� ���� ������ ��� ������������ ������";
 cmdResult[0x50] = "������. ������ ���������� ������ � ���� �����������.";
 cmdResult[0x70] = "������������ �������� ���������� � ����, ��� ��������.";
 cmdResult[0x71] = "������������ �������� ���������� � ����, ��� ���������.";
 cmdResult[0x72] = "������������ �������� ����� ������������� � ����.";
 cmdResult[0x73] = "������������ �������� �������� ����� �������������.";
 cmdResult[0x74] = "������������ �������� �������� ����� ������.";
 cmdResult[0x75] = "������������ �������� �������� ����� ��������.";
 cmdResult[0x76] = "������������ �������� �������� ����� ������ � ������.";
 cmdResult[0x77] = "������������ �������� �������� ����� ������ ������.";
 cmdResult[0x78] = "������������ �������� �������� ����� �������� ��� ���������.";
 cmdResult[0x79] = "������������ �������� �������� ����� ��������� ��� ������ � ������.";
 cmdResult[0x7A] = "������������ �������� �������� ����� � ���� ��� ����������.";
 cmdResult[0x7B] = "������������ �������� �������� ����� � ���� ��� ���������.";
 cmdResult[0x7C] = "������������ �������� ��������� � ����, ��� ��������� ���������� �� ���������.";
 cmdResult[0x7D] = "������������ �������� �������� ����� � ���� ��� ���������� ������.";
 cmdResult[0x7E] = "������������ �������� �������� ����� �� ������ � ���� ��� ���������� ������.";
 cmdResult[0x7F] = "������������ �������� �������� ����� ����� ���� � ���� ��� ���������� ������.";
 cmdResult[0x80] = "������������ �������� ������ � ����.";
 cmdResult[0x81] = "������������ �������� �������� ����� ��������� ������ � ���� ��� ���������� ������.";
 cmdResult[0x82] = "������������ �������� �������� ����� ������������ ������ � ���� ��� ���������� ������.";
 cmdResult[0x83] = "������������ �������� �������� ����� � ���� ��� ���������� ��������.";
 cmdResult[0x84] = "������������ �������� �������� ����� �� ������ � ���� ��� ���������� ��������.";
 cmdResult[0x85] = "������������ �������� �������� ����� ����� ���� � ���� ��� ���������� ��������.";
 cmdResult[0x86] = "������������ �������� �������� � ����.";
 cmdResult[0x87] = "������������ �������� �������� ����� ��������� ������ � ���� ��� ���������� ��������.";
 cmdResult[0x88] = "������������ �������� �������� ����� ������������ ������ � ���� ��� ���������� ��������.";
 cmdResult[0x89] = "������������ �������� �������� ����� �� ������ � ����.";
 cmdResult[0x8A] = "������������ �������� �������� ����� ����� ���� � ����.";
 cmdResult[0x8B] = "������������ �������� �������� ����� ��������� ������ � ����.";
 cmdResult[0x8C] = "������������ �������� �������� ����� ������������ ������ � ����.";
 cmdResult[0x8D] = "������������ �������� �������� ����� � ���� ��� ���������� ������������� ������.";
 cmdResult[0x8E] = "������������ �������� �������� ����� �� ������ � ���� ��� ���������� ������������� ������.";
 cmdResult[0x8F] = "������������ �������� �������� ����� ����� ���� � ���� ��� ���������� ������������� ������.";
 cmdResult[0x90] = "������������ �������� ������ � ���� ��� ���������� ������������� ������.";
 cmdResult[0x91] = "������������ �������� �������� ����� ��������� ������ � ���� ��� ���������� ������������� ������.";
 cmdResult[0x92] = "������������ �������� �������� ����� ������������ ������ � ���� ��� ���������� ������������� ������.";
 cmdResult[0x93] = "������������ �������� �������� ����� � ���� ��� ���������� ������������� ��������.";
 cmdResult[0x94] = "������������ �������� �������� ����� �� ������ � ���� ��� ���������� ������������� ��������.";
 cmdResult[0x95] = "������������ �������� �������� ����� ����� ���� � ���� ��� ���������� ������������� ��������.";
 cmdResult[0x96] = "������������ �������� �������� � ���� ��� ���������� ������������� ��������.";
 cmdResult[0x97] = "������������ �������� �������� ����� ��������� ������ � ���� ��� ���������� ������������� ��������.";
 cmdResult[0x98] = "������������ �������� �������� ����� ������������ ������ � ���� ��� ���������� ������������� ��������.";
 cmdResult[0x99] = "����� ������ ������ ����� ����.";
 cmdResult[0x9A] = "������������ �������� �������� ����� ������ ��� ��������������� ������.";
 cmdResult[0x9B] = "����� ����������� ������ ������ ����� ����.";
 cmdResult[0x9C] = "���������� ���������� �������. ����������� ������� ���� � ���� ��������� ������ � �� ������ �������������������� ��������. ��� ������ ���������� ���������� ��������� ������� ���������������� ����.";
 cmdResult[0x9D] = "���������� ���������� �������. ��������� �������� �� ���������. ����� �������������� ����������������� �������� (�������� ������, �������� ���������� �������, ������� ������) ������� ������������� ������������� �������� � ������� ����������.";
 cmdResult[0x9E] = "���������� ���������� �������. ����������� ������������ ������������ ���.";
 cmdResult[0x9F] = "���������� ���. ���������� ��������� �������.";
 cmdResult[0xA1] = "�������� ����� ���� �������� �� ����� �������� ������������ ����� ��� ��������������� ��������";

}

