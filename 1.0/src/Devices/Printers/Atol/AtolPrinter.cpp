//---------------------------------------------------------------------------

#include <string>
#include <list>
#include <algorithm>
#include <vector>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#pragma hdrstop

#include "AtolPrinter.h"
#include "AtolPrinterThread.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

// ������ ���
const BYTE cmdGetKKMState_ALL[] = {0x3F};
const BYTE cmdGetKKMState_Felix80K[] = {0x11};
// ����� ���
const BYTE cmdGetKKMMode_ALL[] = {0x45};
const BYTE cmdGetKKMMode_Felix80K[] = {0x17};
// ������ ������
const BYTE cmdPrintText_ALL[] = {0x4C};
const BYTE cmdPrintText_Felix80K[] = {0x1E};
// �����
const BYTE cmdCutFelixRK[] = "L=<CUT>=";
const BYTE cmdCutFelix3CK[] = {0x75, 0x00};
const BYTE cmdCutFelix80K[] = {0x47, 0x00};
// ���� � �����
const BYTE cmdEnterMode_ALL[] = {0x56};
const BYTE cmdEnterMode_Felix80K[] = {0x28};
//����� �� ������
const BYTE cmdExitMode_ALL[] = {0x48};
const BYTE cmdExitMode_Felix80K[] = {0x1A};
// ������� �����
const BYTE cmdOpenSession_ALL[] = {0x9A, 0x00};
const BYTE cmdOpenSession_Felix80K[] = {0x6C, 0x00};
// ������� ���
const BYTE cmdOpenCheque_ALL[] = {0x92, 0x00, 0x01};
const BYTE cmdOpenCheque_Felix80K[] = {0x64, 0x00, 0x01};
// ����������� ������
const BYTE cmdRegisterGoods_ALL[] = {0x52};
const BYTE cmdRegisterGoods_Felix80K[] = {0x24};
// ������� ���
const BYTE cmdCloseCheque_ALL[] = {0x4A};
const BYTE cmdCloseCheque_Felix80K[] = {0x1C};
// Z-�����
const BYTE cmdZReport_ALL[] = {0x5A};
const BYTE cmdZReport_Felix80K[] = {0x2C};

std::string printerNames[] = {"FelixRK", "Felix3CK", "Tornado", "MercuryMSK", "Felix80K"};

CAtolPrinter::CAtolPrinter(EAtolPrinter printerType, int comPort, int baudRate, TLogClass* logClass)
: CPrinter(comPort, logClass, printerNames[(int)printerType].c_str()), m_printerType(printerType)
{
    m_password = 0;
    m_adminPassword = 30;
    prepareCommandResults();

    DeviceName = printerNames[(int)m_printerType].c_str();
    LoggingErrors = true;
    DataLength = 1;

    COMParameters->Parity = NOPARITY;
    COMParameters->timeout = 600;
    if(baudRate > 0)
    {
        COMParameters->BaudRate = baudRate;
        if(Port)
            Port->ReopenPort();
    }

    initKKMCommands();
}

CAtolPrinter::~CAtolPrinter()
{

}
////////////////////////////////////////////////////////////////////////////////
// derived from CPrinter
int CAtolPrinter::Initialize()
{
/*
    GetState();
    if(PrinterEnable)
*/    
        SetInitialized();

    return 0;
}

void CAtolPrinter::GetState()
{
    if(execCmd(cmdGetKKMState, cmdGetKKMStateSize) == 0)
    {
        BYTE flag1 = Answer[9];
        getCurrentMode();
        logKKMState(flag1, Answer[2]);
        PrinterEnable = true;

        /*
        if(getCurrentMode() != 0xFF)
        {
            logKKMState(flag1, Answer[2]);
            PrinterEnable = true;
        }
        */
    }
    else
    {
        PrinterEnable = false;
    }
        
    if(!PrinterEnable)
    {
        Log->Write("Error getting printer status");
        DeviceState->StateCode = DSE_NOTMOUNT;
        DeviceState->OutStateCode = DSE_NOTMOUNT;
    }

    ChangeDeviceState();
}

void CAtolPrinter::PrintCheck(AnsiString text, std::string barcode)
{
    CharToOem(text.c_str(), text.c_str());
    std::string check = text.c_str();

    printText(check);

    BYTE emptyLine[] = {0x00, 0x20};
    emptyLine[0] = *cmdPrintText;

    for(int i = 0; i < 5; i++)
        execCmd(emptyLine, sizeof(emptyLine));
        
    execCmd(cmdCut, cmdCutSize);
}

void CAtolPrinter::PrintCheck(double Money, AnsiString Text)
{
    if(!Fiscal)
    {
        Log->Write("Try to print fiscal receipt with nonfiscal configuration");
        return;
    }

    BYTE registerGoodsBuffer[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00};
    BYTE closeChequeBuffer[] = {0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};

    registerGoodsBuffer[0] = *cmdRegisterGoods;
    closeChequeBuffer[0] = *cmdCloseCheque;
    
    CharToOem(Text.c_str(), Text.c_str());
    std::string check = Text.c_str();

    WORD errorCode = enterMode(true, 0x01);

    if(errorCode == 0x88) // ����� ��������� 24 ����
    {
        // ���������� ������� Z-�����
        if(ZReportInBuffer)
            errorCode = printZReport();
    }

    // ������ ����� � �����, ��� ������� ����������� Z-����� � �����
    if(errorCode == 0)
    {
        if(execCmd(cmdOpenSession, cmdOpenSessionSize) == 0)
        {
            DWORD totalMoney = toBCD(Money * 100); // ����� � �������
            reverse(&totalMoney, sizeof(DWORD));
            memcpy(registerGoodsBuffer + 3, &totalMoney, sizeof(DWORD));
            memcpy(closeChequeBuffer + 4, &totalMoney, sizeof(DWORD));

            printText(check);

            execCmd(cmdOpenCheque, cmdOpenChequeSize);
            execCmd(registerGoodsBuffer, sizeof(registerGoodsBuffer));
            execCmd(closeChequeBuffer, sizeof(closeChequeBuffer));

            enterMode(false);
        }
    }
    else
    {
        Log->Write("Can't print receipt");
    }
}

void CAtolPrinter::PrintZReport(AnsiString Text)
{
    if(!Fiscal)
    {
        Log->Write("Try to print Z-report with nonfiscal configuration");
        return;
    }

    if(ZReportInBuffer && m_printerType != Felix80K)
    {
        BYTE cmdDelayedReportsOn[] = {0xB4};
        BYTE cmdDelayedReportsOff[] = {0xB5};

        enterMode(true, 0x03);
        execCmd(cmdDelayedReportsOn, sizeof(cmdDelayedReportsOn));
        enterMode(true, 0x03);
        execCmd(cmdDelayedReportsOff, sizeof(cmdDelayedReportsOff));
    }
    else if(ZReportInBuffer && m_printerType == Felix80K)
    {
        Log->Write("Felix80K does not support frk-buffer mode!");
    }
    else
    {
        printZReport();
    }
}

////////////////////////////////////////////////////////////////////////////////
// protected methods
WORD CAtolPrinter::printZReport()
{
    WORD result = enterMode(true, 0x03);
    if(result == 0)
    {
        //BYTE cmdZReport[] = {0x5A};
        result = execCmd(cmdZReport, cmdZReportSize);
        //������� ��������� �������
        BYTE currentMode = 0x00;
        do
        {
            currentMode = getCurrentMode();
            Sleep(500);
        }while(currentMode == 0x23);

        if(currentMode == 0x03)
        {
            Log->Write("Fiscal memory is over");
            GetState();
            return -1;
        }
        do
        {
            currentMode = getCurrentMode();
            Sleep(500);
        }while(currentMode == 0x17);

        enterMode(false);
    }

    return result;
}

BYTE CAtolPrinter::getCurrentMode()
{
    execCmd(cmdGetKKMMode, cmdGetKKMModeSize);
    return Answer[1];
    /*
    BYTE result = 0xFF;

    if(execCmd(cmdGetKKMMode, cmdGetKKMModeSize) == 0)
        result = Answer[1];

    return result;
    */
}

DWORD CAtolPrinter::toBCD(DWORD value)
{
        if(value > 99999999)
                return -1;

        DWORD result = 0;
        DWORD delimiter = 10000000;
        for(int i = sizeof(DWORD) * 2 - 1; i >= 0; i--)
        {
                DWORD remainder = value % delimiter;
                result += ((value - remainder) / delimiter) << (i * 4);

                value -= (value - remainder);
                delimiter /= 10;
        }

        return result;
}

void CAtolPrinter::reverse(void* buffer, size_t bufferSize)
{
    BYTE* pBuf = (BYTE*)buffer;
    for(size_t i = 0; i < bufferSize / 2; i++)
    {
        BYTE tmp = pBuf[i];
        pBuf[i] = pBuf[bufferSize - 1 - i];
        pBuf[bufferSize - 1 - i] = tmp;
    }
}

WORD CAtolPrinter::enterMode(bool isEnter, BYTE mode)
{
    BYTE enterModeBuffer[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//    BYTE cmdExitMode[] = {0x48};

    if(isEnter)
    {
        enterModeBuffer[0] = *cmdEnterMode;
        enterModeBuffer[1] = mode;
        DWORD adminPassword = toBCD(m_adminPassword);
        reverse(&adminPassword, sizeof(adminPassword));
        memcpy(enterModeBuffer + 2, &adminPassword, sizeof(DWORD));
        return execCmd(enterModeBuffer, sizeof(enterModeBuffer));
    }
    else
    {
        return execCmd(cmdExitMode, sizeof(cmdExitMode));
    }
}

WORD CAtolPrinter::execCmd(const BYTE* cmd, size_t cmdSize)
{
    WORD result = -1;
    ClearAnswer();
    AnswerSize = sizeof(Answer);    // ����� Answer - �� ����� (� ������� �� DeviceThread, ��� �� ����������� ����������)

    Log->Write("Sending command:");
    prepareCommand(cmd, cmdSize);
    Log->WriteBuffer(Command, CommandSize);

    DeviceThread = new CAtolPrinterThread();
    Start();
    AnswerSize = DeviceThread->AnswerSize;
    
    delete DeviceThread;
    DeviceThread = 0;

    if(DeviceState && DeviceState->ExecCode == CMDRES_OK)
    {
        Log->Write("Receive answer");
        if(AnswerSize != -1)
            Log->WriteBuffer(Answer, AnswerSize);
        // ��������� �� ��� ������
        if(Answer[0] == 'U')
        {
            // ������.. ��� ��� :)
            result = Answer[1];
            Log->Write(cmdResult[result].c_str());
        }
        else
        {
            result = 0;
        }
    }

    return result;
}

void CAtolPrinter::prepareCommand(const BYTE* cmd, size_t cmdSize)
{
    // ������������� ���p����� ����
    CommandSize = 0;
    Command[CommandSize++] = STX;

    // ������������� ������
    WORD password = (WORD)toBCD(m_password);
    reverse(&password, sizeof(WORD));
    memcpy(Command + CommandSize, &password, sizeof(WORD));
    CommandSize += sizeof(WORD);

    // ��������� DLE � ETX
    for(size_t i = 0; i < cmdSize; i++)
    {
        if(cmd[i] == DLE || cmd[i] == ETX)
            Command[CommandSize++] = DLE;
        Command[CommandSize++] = cmd[i];
    }

    // ��������� ETX
    Command[CommandSize++] = ETX;

    // ������������ crc (� ������� �����, ��� ��� STX �� ����������� ��� ��������)
    BYTE crc = 0x00;
    for(size_t i = 1; i < (size_t)CommandSize; i++)
        crc ^= Command[i];

    Command[CommandSize++] = crc;
}

void CAtolPrinter::logKKMState(BYTE flag1, BYTE flag2)
{
 DeviceState->StateCode = DSE_OK;
 // flag1 - +9 �� ������ 0x3F
 // flag2 - +2 �� ������ 0x45
 std::string printerStatus = "printer status:\r\n";
 std::string yes = "yes\r\n";
 std::string no = "no\r\n";

 printerStatus += "fiscal: ";
 if(flag1 & 0x01)
  printerStatus += yes;
 else
  printerStatus += no;

 printerStatus += "shift is open: ";
 if(flag1 & 0x02)
  printerStatus += yes;
 else
  printerStatus += no;

 printerStatus += "moneyBox is open: ";
 if(flag1 & 0x04)
  printerStatus += no;
 else
  printerStatus += yes;

 printerStatus += "cover is open: ";
 if(flag1 & 0x20)
  printerStatus += yes;
 else
  printerStatus += no;

 printerStatus += "battery low: ";
 if(flag1 & 0x80)
  printerStatus += yes;
 else
  printerStatus += no;

 if(flag2 & 0x01)
 {
  printerStatus += "no paper1\r\n";
  DeviceState->StateCode = DSE_NOTPAPER;
 }

 if(flag2 & 0x02)
 {
  printerStatus += "link failed\r\n";
  DeviceState->StateCode = DSE_HARDWARE_ERROR;
 }

 if(m_printerType == Felix80K)
 {
   if(flag2 & 0x04)
   {
    printerStatus += "open cover\r\n";
    DeviceState->StateCode = DSE_PRN_HEADUP;
   }
   if(flag2 & 0x08)
   {
    printerStatus += "no paper2\r\n";
    DeviceState->StateCode = DSE_NOTPAPER;
   }
 }
 else
 {
   if(flag2 & 0x04)
   {
    printerStatus += "printing device hardware error\r\n";
    DeviceState->StateCode = DSE_HARDWARE_ERROR;
   }
 }
 Log->Write(printerStatus.c_str());

 DeviceState->OutStateCode = DeviceState->StateCode;
}

void CAtolPrinter::prepareCommandResults()
{
	for(size_t i = 0; i < sizeof(cmdResult)/sizeof(std::string); i++)
		cmdResult[i] = (boost::format("unknown error code: %1%") % i).str();

	cmdResult[0x00] = "no errors";
	cmdResult[0x01] = "����������� ����� ���������� ��� ������";
	cmdResult[0x08] = "�������� ���� (�����)";
	cmdResult[0x0A] = "�������� ����������";
	cmdResult[0x0B] = "������������ �������� ����������";
	cmdResult[0x0C] = "���������� ������ ��������� ��������";
	cmdResult[0x0D] = "������ �� ���� ���������� (� ���� ���������������� ������� ���������� ������� � ��������� �����)";
	cmdResult[0x0E] = "���������� ������ ��������� ��������";
	cmdResult[0x0F] = "��������� ������ �� �������� ����������";
	cmdResult[0x10] = "������/�������� �� ���������� �������� ����������";
	cmdResult[0x11] = "�������� ��� ������";
	cmdResult[0x12] = "�������� �����-��� ������";
	cmdResult[0x13] = "�������� ������";
	cmdResult[0x14] = "�������� �����";
	cmdResult[0x15] = "��� ������������� � ������ ����� ����";
	cmdResult[0x16] = "��������� ������������� ����� ����";
	cmdResult[0x18] = "��� ������ ������ ��� �������� �� ���";
	cmdResult[0x19] = "��� ������������� ��� ������ �������";
	cmdResult[0x1A] = "����� � �������� �������. ���� � ����� ����������.";
	cmdResult[0x1B] = "���������� �������� ���������� ���������� (�� ��������� ����������� ���� ������).";
	cmdResult[0x1E] = "���� � ����� ������������";
	cmdResult[0x1F] = "��������� ���� � �����";
	cmdResult[0x20] = "���� � ����� � ��� ������ ��� � ����";
	cmdResult[0x21] = "���������� ������� �����";
	cmdResult[0x3D] = "����� �� ������";
	cmdResult[0x3E] = "������� �����-��� � ����������� <>1.000";
	cmdResult[0x3F] = "������������ ������ ����";
	cmdResult[0x40] = "������������� ���������� ������";
	cmdResult[0x41] = "������������ ���������� ������ ����������";
	cmdResult[0x42] = "��������������� ����� �� ������ � ������ ����";
	cmdResult[0x43] = "������ ����� �� ���������� � ����, ������ ����������";
	cmdResult[0x44] = "Memo Plus(tm) 3(tm) ������������� � ��";
	cmdResult[0x45] = "������ ����������� ����� ������� �������� Memo Plus(tm) 3(tm)";
	cmdResult[0x46] = "�������� ������� �� ���";
	cmdResult[0x66] = "������� �� ����������� � ������ ������ ���";
	cmdResult[0x67] = "��� ������";
	cmdResult[0x68] = "��� ����� � ��������� �����";
	cmdResult[0x69] = "������������ ������ ����������� ����������";
	cmdResult[0x6A] = "�������� ��� ����";
	cmdResult[0x6B] = "��� ������ ����� ��������";
	cmdResult[0x6C] = "�������� ����� ��������";
	cmdResult[0x6D] = "������������ ������� ����������";
	cmdResult[0x6E] = "��� ����� � ������� ��������";
	cmdResult[0x6F] = "�������� ����� �������� / �������� �����������";
	cmdResult[0x70] = "����� ������ ������, ��� ���� �������� ������ ����� ������";
	cmdResult[0x71] = "����� �� �������� �������� ��������� ����� ����";
	cmdResult[0x72] = "����� �������� ������ ����� ����";
	cmdResult[0x73] = "���������� ������ ����� �������� ��� �������������";
	cmdResult[0x75] = "������������ ����� ��������";
	cmdResult[0x76] = "(���������������)";
	cmdResult[0x7A] = "������ ������ ��� �� ����� ��������� �������";
	cmdResult[0x7B] = "�������� �������� ������ / ��������";
	cmdResult[0x7C] = "�������� ����� ������ / �������� ����������";
	cmdResult[0x7D] = "�������� ������";
	cmdResult[0x7E] = "�������� ��� ������";
	cmdResult[0x7F] = "������������ ��� ���������";
	cmdResult[0x80] = "�������� ��������� � ������� ��������";
	cmdResult[0x81] = "������������ ����� ����";
	cmdResult[0x82] = "������ ��� ������������� � �������� ����������";
	cmdResult[0x84] = "������������ ������ ����������� �����";
	cmdResult[0x86] = "�������� �������� ����� ������ ����� ����";
	cmdResult[0x87] = "������ ��� �������� � �������� ����������";
	cmdResult[0x88] = "����� ��������� 24 ����";
	cmdResult[0x89] = "������ ��� ������� � �������� ����������";
	cmdResult[0x8A] = "������������ ��";
	cmdResult[0x8C] = "�������� ������";
	cmdResult[0x8D] = "����� ����������� ����� �� ����������";
	cmdResult[0x8E] = "���� ��������� ����������� �����";
	cmdResult[0x8F] = "���������� ����� (��������� ������� ����������)";
	cmdResult[0x91] = "�������� ����� �������";
	cmdResult[0x92] = "�������� ����� ����";
	cmdResult[0x93] = "�������� ����� ����";
	cmdResult[0x94] = "�������� ����";
	cmdResult[0x95] = "�������� �����";
	cmdResult[0x96] = "����� ���� �� ������ ������ ����� ������";
	cmdResult[0x97] = "������� ����� ����� ����������";
	cmdResult[0x98] = "� ��� ��� ����� ��� �������";
	cmdResult[0x9A] = "��� ������ � �������� ����������";
	cmdResult[0x9B] = "��� ������ � �������� ����������";
	cmdResult[0x9C] = "����� �������, �������� ����������";
	cmdResult[0x9D] = "��� �������������, ���� ����� ������ ������� � ��";
	cmdResult[0x9E] = "��������� ����� ��� �����";
	cmdResult[0x9F] = "���������� ��������������� �� ����� ���� ����� 4";
	cmdResult[0xA0] = "������ �.�.";
	cmdResult[0xA2] = "�������� �����";
	cmdResult[0xA3] = "�������� ��� ������";
	cmdResult[0xA4] = "������������ ������";
	cmdResult[0xA5] = "������������ ��������� ����� ���";
	cmdResult[0xA6] = "������������ ���";
	cmdResult[0xA7] = "������������ ���";
	cmdResult[0xA8] = "��� �� ���������������";
	cmdResult[0xA9] = "�� ����� ��������� �����";
	cmdResult[0xAA] = "��� �������";
	cmdResult[0xAB] = "����� �� �������������";
	cmdResult[0xAC] = "��� ���������� ���� � ��";
	cmdResult[0xAD] = "��� ������ ������� ��";
	cmdResult[0xAE] = "������������ ��� ��� ����� ���� ������ ���";
	cmdResult[0xB0] = "��������� ���������� ������ �������";
	cmdResult[0xB1] = "������� �� ��������� ���������� ������ ������ ���";
	cmdResult[0xB2] = "���������� ������ ������/��������";
	cmdResult[0xB3] = "���������� ������� ��� ������ ����� ������ (� ���� ������������ �������� ��� �������� ��������)";
	cmdResult[0xBA] = "������ ������ � ���������� �������";
	cmdResult[0xBE] = "���������� �������� ���������������� ������";
	cmdResult[0xC8] = "��� ����������, ��������������� ������ �������";
	cmdResult[0xC9] = "��� ����� � ������� �����������";
	cmdResult[0xCA] = "��������� ��������� ���";
	cmdResult[0xCB] = "������ ����� ����������� � ����";
	cmdResult[0xCC] = "��������� ����� ���";
	cmdResult[0xCD] = "�������� ��������";
	cmdResult[0xCF] = "� ��� ����������� 20 �����������";
	cmdResult[0xD0] = "����������� ������ ���� � ������� ������ ��� ����������";
	cmdResult[0xD1] = "�������� ������� ��������";
	cmdResult[0xD2] = "������ ������ � ���� �� ������ ���������� I2C";
	cmdResult[0xD3] = "������ ������� �������� ����";
	cmdResult[0xD4] = "�������� ��������� ����";
	cmdResult[0xD5] = "������������ ������ ����";
	cmdResult[0xD6] = "������ ������-���������� ����";
	cmdResult[0xD7] = "�������� ��������� ������ ����";
	cmdResult[0xD8] = "���� �����������";
	cmdResult[0xD9] = "� ���� �������� �������� ���� ��� �����";
	cmdResult[0xDA] = "� ���� ��� ����������� ������";
	cmdResult[0xDB] = "������������ ���� (���� ����)";
}

void CAtolPrinter::printText(const std::string& text)
{
    BYTE cmd[50];
    std::string t = text;
#ifdef BUILDER_RULEZ
    boost::algorithm::replace_all(t, "\r", "\n");
    std::vector<std::string> lines;
    
    boost::split(lines, t, boost::is_any_of("\n"));
    BOOST_FOREACH(std::string cl, lines)
    {
        if(cl == "")
            continue;
        if(cl.length() > 48)
        {
            Log->Write("print line too long. trim");
            cl = cl.substr(48);
        }

        cmd[0] = *cmdPrintText;
        memcpy(cmd + 1, printLine.c_str(), printLine.length());
        execCmd(cmd, printLine.length() + 1);
    }
#else
    std::string::size_type pos = t.find("\r");
    while(pos != std::string::npos)
    {
        t[pos] = '\n';
        pos = t.find("\r");
    }

    std::string::size_type posStart = 0;
    std::string::size_type posEnd;
    while(posStart < t.length())
    {
        std::string cl = "";

        posEnd = t.find("\n", posStart);
        if(posEnd == std::string::npos)
            cl = t.substr(posStart);
        else
            cl = t.substr(posStart, posEnd - posStart);

        if(cl.length() > 48)
        {
            Log->Write("print line too long. trim");
            cl = cl.substr(48);
        }

        if(cl != "")
        {
            cmd[0] = *cmdPrintText;
            memcpy(cmd + 1, cl.c_str(), cl.length());
            execCmd(cmd, cl.length() + 1);
        }

        posStart = posEnd + 1;
    };

#endif
}

void CAtolPrinter::initKKMCommands()
{
    cmdGetKKMState = cmdGetKKMState_ALL;
    cmdGetKKMStateSize = sizeof(cmdGetKKMState_ALL);

    cmdGetKKMMode = cmdGetKKMMode_ALL;
    cmdGetKKMModeSize = sizeof(cmdGetKKMMode_ALL);

    cmdPrintText = cmdPrintText_ALL;
    cmdPrintTextSize = sizeof(cmdPrintText_ALL);

    cmdCut = cmdCutFelixRK;
    cmdCutSize = sizeof(cmdCutFelixRK);

    cmdEnterMode = cmdEnterMode_ALL;
    cmdEnterModeSize = sizeof(cmdEnterMode_ALL);

    cmdExitMode = cmdExitMode_ALL;
    cmdExitModeSize = sizeof(cmdExitMode_ALL);

    cmdOpenSession = cmdOpenSession_ALL;
    cmdOpenSessionSize = sizeof(cmdOpenSession_ALL);

    cmdOpenCheque = cmdOpenCheque_ALL;
    cmdOpenChequeSize = sizeof(cmdOpenCheque_ALL);

    cmdRegisterGoods = cmdRegisterGoods_ALL;
    cmdRegisterGoodsSize = sizeof(cmdRegisterGoods_ALL);

    cmdCloseCheque = cmdCloseCheque_ALL;
    cmdCloseChequeSize = sizeof(cmdCloseCheque_ALL);

    cmdZReport = cmdZReport_ALL;
    cmdZReportSize = sizeof(cmdZReport_ALL);

    if(m_printerType == Felix3CK)
    {
        cmdCut = cmdCutFelix3CK;
        cmdCutSize = sizeof(cmdCutFelix3CK);
    }
    if(m_printerType == Felix80K)
    {
        cmdGetKKMState = cmdGetKKMState_Felix80K;
        cmdGetKKMStateSize = sizeof(cmdGetKKMState_Felix80K);

        cmdGetKKMMode = cmdGetKKMMode_Felix80K;
        cmdGetKKMModeSize = sizeof(cmdGetKKMMode_Felix80K);

        cmdPrintText = cmdPrintText_Felix80K;
        cmdPrintTextSize = sizeof(cmdPrintText_Felix80K);

        cmdCut = cmdCutFelix80K;
        cmdCutSize = sizeof(cmdCutFelix80K);

        cmdEnterMode = cmdEnterMode_Felix80K;
        cmdEnterModeSize = sizeof(cmdEnterMode_Felix80K);

        cmdExitMode = cmdExitMode_Felix80K;
        cmdExitModeSize = sizeof(cmdExitMode_Felix80K);

        cmdOpenSession = cmdOpenSession_Felix80K;
        cmdOpenSessionSize = sizeof(cmdOpenSession_Felix80K);

        cmdOpenCheque = cmdOpenCheque_Felix80K;
        cmdOpenChequeSize = sizeof(cmdOpenCheque_Felix80K);

        cmdRegisterGoods = cmdRegisterGoods_Felix80K;
        cmdRegisterGoodsSize = sizeof(cmdRegisterGoods_Felix80K);

        cmdCloseCheque = cmdCloseCheque_Felix80K;
        cmdCloseChequeSize = sizeof(cmdCloseCheque_Felix80K);

        cmdZReport = cmdZReport_Felix80K;
        cmdZReportSize = sizeof(cmdZReport_Felix80K);
    }
}

