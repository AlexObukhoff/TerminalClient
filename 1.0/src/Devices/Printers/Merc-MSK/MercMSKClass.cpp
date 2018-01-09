//---------------------------------------------------------------------------


#pragma hdrstop

#include "MercMSKClass.h"
#include "../StarTSP600/StarTSP600Thread.h"
#include "../StarTSP600/StarTSP600Defines.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

CMercMSK::CMercMSK(int comPort, int baudRate, TLogClass* logClass) : CPrinter(comPort, logClass, "MercMSK")
{
 DeviceName = "MercMSK";
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

CMercMSK::~CMercMSK()
{

}

////////////////////////////////////////////////////////////////////////////////
// public methods
int CMercMSK::Initialize()
{
 GetState();
 if(PrinterEnable)
  SetInitialized();

 return 0;
}

bool CMercMSK::IsPrinterEnable()
{
 GetState();
 return PrinterEnable;
}

void CMercMSK::PrintCheck(AnsiString text, std::string barcode)
{
 CharToOem(text.c_str(), text.c_str());
 std::string checkText = text.c_str();

 if(cmd_36(checkText) != CMDRES_SUCCESS)
  Log->Write("������ ������ ��������� � ������������ ������");

 cmd_52(5, true);
}

// ������ ����������� ����
void CMercMSK::PrintCheck(double Money, AnsiString Text)
{
 if(Fiscal == false)
 {
  Log->Write("������� ������ ����������� ���� �� ������������ ������������");
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
}

void CMercMSK::PrintXReport(AnsiString Text)
{
 if(Fiscal == false)
 {
  Log->Write("������� ������ X-������ �� ������������ ������������");
  return;
 }
 cmd_5F(0x31, 0x04);
}

void CMercMSK::PrintZReport(AnsiString Text)
{
 if(Fiscal == false)
 {
  Log->Write("������� ������ Z-������ �� ������������ ������������");
  return;
 }

 CharToOem(Text.c_str(), Text.c_str());
 std::string checkText = Text.c_str();
 if(checkText != "")
  cmd_36(checkText);

 Log->Write("PrintZReport");
 cmd_5F(0x30, 0x04);
}

void CMercMSK::GetState()
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
  Log->Write("������ ��������� ��������� ��������");
  DeviceState->StateCode = DSE_NOTMOUNT;
  DeviceState->OutStateCode = DSE_NOTMOUNT;
  PrinterEnable = false;
 }

 ChangeDeviceState();
}

bool CMercMSK::IsItYou()
{
 bool result = false;
 if(cmd_45() == CMDRES_SUCCESS);
  result = true;

 return result;
}

WORD CMercMSK::execCommand()
{
 WORD result = CMDRES_FAILED;
 ClearAnswer();
 //SendType = RecieveAnswer;

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
WORD CMercMSK::cmd_31(std::string cashierName)
{
 prologueCmd(0x31);

 // ����� �������
 Command[CommandSize + 0] = '0';
 CommandSize += 3;
 CommandSize += 40;
 Command[CommandSize] = DIV;
 CommandSize++;
 epilogueCmd();

 return execCommand();
}

// cmd_36() - ������� �������� � ������������ �����
WORD CMercMSK::cmd_36(std::string text)
{
 WORD result = CMDRES_FAILED;
 prologueCmd(0x36);
 epilogueCmd();

 result = execCommand();
 if(result == CMDRES_SUCCESS)
 {
  memcpy(Command, text.c_str(), text.length());
  CommandSize = text.length() + 1;
  // ������ � ����� ������ ���� ����������
  Command[CommandSize] = ESC;
  CommandSize++;
  execCommand();

  // ���������� ��� ������ �� ������ ������ �� �����
  Command[0] = ESC;
  CommandSize = 1;

  result = execCommand();
 }
 return result;
}

// cmd_45() - �������� ������ �������� �������� (�������� ��� �������������)
WORD CMercMSK::cmd_45()
{
 prologueCmd(0x45);
 addByteToCommand(0x00);
 epilogueCmd();

 return execCommand();
}

WORD CMercMSK::cmd_4F()
{
 prologueCmd(0x4F);
 epilogueCmd();

 return execCommand();
}

// cmd_52() - ���������� �������� � ������� �����
WORD CMercMSK::cmd_52(BYTE linesFeed, bool cut)
{
 prologueCmd(0x52);

 addByteToCommandAsString(linesFeed);
 if(cut)
  addByteToCommandAsString(0x01);
 else
  addByteToCommandAsString(0x00);

 epilogueCmd();

 return execCommand();
}

// cmd_53() - ���������� ��������
WORD CMercMSK::cmd_53_openDocument(double price)
{
 char priceValue[11];
 memset(priceValue, 0, 11);
 sprintf(priceValue, "%.2f", price);
 std::string strValue = priceValue;

 prologueCmd(0x53);
 addByteToCommand(0x30);
 addStringToCommand("04"); // �������� ���������

 // ���������� ������������ ����������
 addStringToCommand("010");
 // �������� ���������
 cmd_53_fiscalField("00", "0020", 0, ""); // ����� ���
 cmd_53_fiscalField("10", "0020", 1, ""); // ���, ���
 cmd_53_fiscalField("05", "0020", 2, ""); // ���� ��������
 cmd_53_fiscalField("07", "0020", 3, ""); // ����� ���������
 cmd_53_fiscalField("08", "0020", 4, ""); // ����� ����
 cmd_53_fiscalField("06", "0020", 5, ""); // ������
 cmd_53_fiscalSale(6, strValue); // ���� ������
 cmd_53_fiscalField("13", "0020", 7, strValue); // ���������� �����
 cmd_53_fiscalField("12", "0020", 8, ""); // �������� �����
 cmd_53_fiscalField("14", "0020", 9, ""); // �����

 epilogueCmd();

 return execCommand();
}

WORD CMercMSK::cmd_53_cancelDocument()
{
 prologueCmd(0x53);
 addByteToCommand(0x30);
 addStringToCommand("08"); // ������������� ���������
 addStringToCommand("000");
 epilogueCmd();

 return execCommand();
}

void CMercMSK::cmd_53_fiscalField(std::string type, std::string flags, unsigned int row, std::string data)
{
 addStringToCommand(type);
 addStringToCommand(flags);
 // �������� �� �����������
 Command[CommandSize + 0] = 0x30; // ������� ��������
 Command[CommandSize + 1] = 0x00;
 Command[CommandSize + 2] = 0x00; // CMD_SEPARATOR
 // �������� �� ���������
 Command[CommandSize + 3] = (BYTE)(row + '0');
 Command[CommandSize + 4] = 0x00;
 Command[CommandSize + 5] = 0x00;
 Command[CommandSize + 6] = 0x00; // CMD_SEPARATOR
 CommandSize += 7;

 CommandSize += 5; // ���������� �������������� �����


 if(data.length() > 40)
  data = data.substr(0, 39);
 memcpy(Command + CommandSize, data.c_str(), data.length());

 CommandSize += 40;

 Command[CommandSize] = DIV;
 CommandSize++;
}

void CMercMSK::cmd_53_fiscalSale(unsigned int row, std::string price)
{
 addStringToCommand("11");
 addStringToCommand("0022");
 // �������� �� �����������
 Command[CommandSize + 0] = 0x30; // ������� ��������
 Command[CommandSize + 1] = 0x00;
 Command[CommandSize + 2] = 0x00; // CMD_SEPARATOR
 // �������� �� ���������
 Command[CommandSize + 3] = (BYTE)(row + '0');
 Command[CommandSize + 4] = 0x00;
 Command[CommandSize + 5] = 0x00;
 Command[CommandSize + 6] = 0x00; // CMD_SEPARATOR
 CommandSize += 7;

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
WORD CMercMSK::cmd_5C(std::string param)
{
 prologueCmd(0x5C);
 addStringToCommand(param);
 epilogueCmd();

 return execCommand();
}

// cmd_5F() - ������ �������
WORD CMercMSK::cmd_5F(BYTE type, BYTE flags)
{
 prologueCmd(0x5F);
 addByteToCommand(type);
 addByteToCommandAsString(flags);
 addStringToCommand("00");
 epilogueCmd();

 return execCommand();
}
////////////////////////////////////////////////////////////////////////////////
// ������ ���������� ������ ��������
// prologueCmd - ������� ������ ������
void CMercMSK::prologueCmd(BYTE cmdNum)
{
 ClearCommand();
 Command[0] = STX;
 Command[1] = cmdNum;
 CommandSize += 2;
 addStringToCommand(printerPassCode);
}

// epilogueCmd - ������� ������ ������
void CMercMSK::epilogueCmd()
{
 BYTE bcc = 0;
 for(int i =1; i < CommandSize; i++)
  bcc += Command[i];

 char arbcc[3];
 sprintf(arbcc, "%02X", bcc);

 Command[CommandSize + 0] = arbcc[0];
 Command[CommandSize + 1] = arbcc[1];
 Command[CommandSize + 2] = ETX;
 CommandSize += 3;
}

void CMercMSK::addStringToCommand(std::string s)
{
 memcpy(Command + CommandSize, s.c_str(), s.length());
 CommandSize += s.length();

 Command[CommandSize] = DIV;
 CommandSize++;
}

void CMercMSK::addByteToCommandAsString(BYTE b)
{
 char value[3];
 sprintf(value, "%02X", b);
 Command[CommandSize + 0] = value[0];
 Command[CommandSize + 1] = value[1];
 Command[CommandSize + 2] = DIV;

 CommandSize += 3;
}

void CMercMSK::addByteToCommand(BYTE b)
{
 Command[CommandSize + 0] = b;
 Command[CommandSize + 1] = DIV;

 CommandSize += 2;
}

WORD CMercMSK::translateToNumeric(BYTE* buffer, unsigned int size)
{
 char* start_ptr = new char[size * 2+1];
 char* end_ptr;

 memcpy(start_ptr, buffer, size * 2);
 start_ptr[size * 2] = 0x00;

 WORD result = (WORD)strtol(start_ptr, &end_ptr, 16);

 delete[] start_ptr;

 return result;
}

void CMercMSK::logStatus(WORD kkmStatus, BYTE status1, BYTE status2, BYTE status3, BYTE status4)
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

void CMercMSK::prepareCmdResults()
{
 for(int i = 0; i < 0xFF; i++)
  cmdResult[i] = "����������� ���������";

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

