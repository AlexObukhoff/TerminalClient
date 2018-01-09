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

// статус ккм
const BYTE cmdGetKKMState_ALL[] = {0x3F};
const BYTE cmdGetKKMState_Felix80K[] = {0x11};
// режим ккм
const BYTE cmdGetKKMMode_ALL[] = {0x45};
const BYTE cmdGetKKMMode_Felix80K[] = {0x17};
// печать строки
const BYTE cmdPrintText_ALL[] = {0x4C};
const BYTE cmdPrintText_Felix80K[] = {0x1E};
// отрез
const BYTE cmdCutFelixRK[] = "L=<CUT>=";
const BYTE cmdCutFelix3CK[] = {0x75, 0x00};
const BYTE cmdCutFelix80K[] = {0x47, 0x00};
// вход в режим
const BYTE cmdEnterMode_ALL[] = {0x56};
const BYTE cmdEnterMode_Felix80K[] = {0x28};
//выход из режима
const BYTE cmdExitMode_ALL[] = {0x48};
const BYTE cmdExitMode_Felix80K[] = {0x1A};
// открыть смену
const BYTE cmdOpenSession_ALL[] = {0x9A, 0x00};
const BYTE cmdOpenSession_Felix80K[] = {0x6C, 0x00};
// открыть чек
const BYTE cmdOpenCheque_ALL[] = {0x92, 0x00, 0x01};
const BYTE cmdOpenCheque_Felix80K[] = {0x64, 0x00, 0x01};
// регистрация товара
const BYTE cmdRegisterGoods_ALL[] = {0x52};
const BYTE cmdRegisterGoods_Felix80K[] = {0x24};
// закрыть чек
const BYTE cmdCloseCheque_ALL[] = {0x4A};
const BYTE cmdCloseCheque_Felix80K[] = {0x1C};
// Z-отчет
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

    if(errorCode == 0x88) // смена превысила 24 часа
    {
        // необходимо сделать Z-отчет
        if(ZReportInBuffer)
            errorCode = printZReport();
    }

    // смогли войти в режим, или успешно распечатали Z-отчет в буфер
    if(errorCode == 0)
    {
        if(execCmd(cmdOpenSession, cmdOpenSessionSize) == 0)
        {
            DWORD totalMoney = toBCD(Money * 100); // сумму в копейки
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
        //ожидаем окончания гашения
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
    AnswerSize = sizeof(Answer);    // здесь Answer - на стеке (в отличие от DeviceThread, где он передеается указателем)

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
        // посмотрим на код ответа
        if(Answer[0] == 'U')
        {
            // ошибка.. или нет :)
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
    // устанавливаем стаpтовый байт
    CommandSize = 0;
    Command[CommandSize++] = STX;

    // устанавливаем пароль
    WORD password = (WORD)toBCD(m_password);
    reverse(&password, sizeof(WORD));
    memcpy(Command + CommandSize, &password, sizeof(WORD));
    CommandSize += sizeof(WORD);

    // маскируем DLE и ETX
    for(size_t i = 0; i < cmdSize; i++)
    {
        if(cmd[i] == DLE || cmd[i] == ETX)
            Command[CommandSize++] = DLE;
        Command[CommandSize++] = cmd[i];
    }

    // добавляем ETX
    Command[CommandSize++] = ETX;

    // подсчитываем crc (с первого байта, так как STX не учитывается при подсчете)
    BYTE crc = 0x00;
    for(size_t i = 1; i < (size_t)CommandSize; i++)
        crc ^= Command[i];

    Command[CommandSize++] = crc;
}

void CAtolPrinter::logKKMState(BYTE flag1, BYTE flag2)
{
 DeviceState->StateCode = DSE_OK;
 // flag1 - +9 от ответа 0x3F
 // flag2 - +2 от ответа 0x45
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
	cmdResult[0x01] = "Контрольная лента обработана без ошибок";
	cmdResult[0x08] = "Неверная цена (сумма)";
	cmdResult[0x0A] = "Неверное количество";
	cmdResult[0x0B] = "Переполнение счетчика наличности";
	cmdResult[0x0C] = "Невозможно сторно последней операции";
	cmdResult[0x0D] = "Сторно по коду невозможно (в чеке зарегистрировано меньшее количество товаров с указанным кодом)";
	cmdResult[0x0E] = "Невозможен повтор последней операции";
	cmdResult[0x0F] = "Повторная скидка на операцию невозможна";
	cmdResult[0x10] = "Скидка/надбавка на предыдущую операцию невозможна";
	cmdResult[0x11] = "Неверный код товара";
	cmdResult[0x12] = "Неверный штрих-код товара";
	cmdResult[0x13] = "Неверный формат";
	cmdResult[0x14] = "Неверная длина";
	cmdResult[0x15] = "ККМ заблокирована в режиме ввода даты";
	cmdResult[0x16] = "Требуется подтверждение ввода даты";
	cmdResult[0x18] = "Нет больше данных для передачи ПО ККМ";
	cmdResult[0x19] = "Нет подтверждения или отмены продажи";
	cmdResult[0x1A] = "Отчет с гашением прерван. Вход в режим невозможен.";
	cmdResult[0x1B] = "Отключение контроля наличности невозможно (не настроены необходимые типы оплаты).";
	cmdResult[0x1E] = "Вход в режим заблокирован";
	cmdResult[0x1F] = "Проверьте дату и время";
	cmdResult[0x20] = "Дата и время в ККМ меньше чем в ЭКЛЗ";
	cmdResult[0x21] = "Невозможно закрыть архив";
	cmdResult[0x3D] = "Товар не найден";
	cmdResult[0x3E] = "Весовой штрих-код с количеством <>1.000";
	cmdResult[0x3F] = "Переполнение буфера чека";
	cmdResult[0x40] = "Недостаточное количество товара";
	cmdResult[0x41] = "Сторнируемое количество больше проданного";
	cmdResult[0x42] = "Заблокированный товар не найден в буфере чека";
	cmdResult[0x43] = "Данный товар не продавался в чеке, сторно невозможно";
	cmdResult[0x44] = "Memo Plus(tm) 3(tm) заблокировано с ПК";
	cmdResult[0x45] = "Ошибка контрольной суммы таблицы настроек Memo Plus(tm) 3(tm)";
	cmdResult[0x46] = "Неверная команда от ККМ";
	cmdResult[0x66] = "Команда не реализуется в данном режиме ККМ";
	cmdResult[0x67] = "Нет бумаги";
	cmdResult[0x68] = "Нет связи с принтером чеков";
	cmdResult[0x69] = "Механическая ошибка печатающего устройства";
	cmdResult[0x6A] = "Неверный тип чека";
	cmdResult[0x6B] = "Нет больше строк картинки";
	cmdResult[0x6C] = "Неверный номер регистра";
	cmdResult[0x6D] = "Недопустимое целевое устройство";
	cmdResult[0x6E] = "Нет места в массиве картинок";
	cmdResult[0x6F] = "Неверный номер картинки / картинка отсутствует";
	cmdResult[0x70] = "Сумма сторно больше, чем было получено данным типом оплаты";
	cmdResult[0x71] = "Сумма не наличных платежей превышает сумму чека";
	cmdResult[0x72] = "Сумма платежей меньше суммы чека";
	cmdResult[0x73] = "Накопление меньше суммы возврата или аннулирования";
	cmdResult[0x75] = "Переполнение суммы платежей";
	cmdResult[0x76] = "(зарезервировано)";
	cmdResult[0x7A] = "Данная модель ККМ не может выполнить команду";
	cmdResult[0x7B] = "Неверная величина скидки / надбавки";
	cmdResult[0x7C] = "Операция после скидки / надбавки невозможна";
	cmdResult[0x7D] = "Неверная секция";
	cmdResult[0x7E] = "Неверный вид оплаты";
	cmdResult[0x7F] = "Переполнение при умножении";
	cmdResult[0x80] = "Операция запрещена в таблице настроек";
	cmdResult[0x81] = "Переполнение итога чека";
	cmdResult[0x82] = "Открыт чек аннулирования – операция невозможна";
	cmdResult[0x84] = "Переполнение буфера контрольной ленты";
	cmdResult[0x86] = "Вносимая клиентом сумма меньше суммы чека";
	cmdResult[0x87] = "Открыт чек возврата – операция невозможна";
	cmdResult[0x88] = "Смена превысила 24 часа";
	cmdResult[0x89] = "Открыт чек продажи – операция невозможна";
	cmdResult[0x8A] = "Переполнение ФП";
	cmdResult[0x8C] = "Неверный пароль";
	cmdResult[0x8D] = "Буфер контрольной ленты не переполнен";
	cmdResult[0x8E] = "Идет обработка контрольной ленты";
	cmdResult[0x8F] = "Обнуленная касса (повторное гашение невозможно)";
	cmdResult[0x91] = "Неверный номер таблицы";
	cmdResult[0x92] = "Неверный номер ряда";
	cmdResult[0x93] = "Неверный номер поля";
	cmdResult[0x94] = "Неверная дата";
	cmdResult[0x95] = "Неверное время";
	cmdResult[0x96] = "Сумма чека по секции меньше суммы сторно";
	cmdResult[0x97] = "Подсчет суммы сдачи невозможен";
	cmdResult[0x98] = "В ККМ нет денег для выплаты";
	cmdResult[0x9A] = "Чек закрыт – операция невозможна";
	cmdResult[0x9B] = "Чек открыт – операция невозможна";
	cmdResult[0x9C] = "Смена открыта, операция невозможна";
	cmdResult[0x9D] = "ККМ заблокирована, ждет ввода пароля доступа к ФП";
	cmdResult[0x9E] = "Заводской номер уже задан";
	cmdResult[0x9F] = "Количество перерегистраций не может быть более 4";
	cmdResult[0xA0] = "Ошибка Ф.П.";
	cmdResult[0xA2] = "Неверная смена";
	cmdResult[0xA3] = "Неверный тип отчета";
	cmdResult[0xA4] = "Недопустимый пароль";
	cmdResult[0xA5] = "Недопустимый заводской номер ККМ";
	cmdResult[0xA6] = "Недопустимый РНМ";
	cmdResult[0xA7] = "Недопустимый ИНН";
	cmdResult[0xA8] = "ККМ не фискализирована";
	cmdResult[0xA9] = "Не задан заводской номер";
	cmdResult[0xAA] = "Нет отчетов";
	cmdResult[0xAB] = "Режим не активизирован";
	cmdResult[0xAC] = "Нет указанного чека в КЛ";
	cmdResult[0xAD] = "Нет больше записей КЛ";
	cmdResult[0xAE] = "Некорректный код или номер кода защиты ККМ";
	cmdResult[0xB0] = "Требуется выполнение общего гашения";
	cmdResult[0xB1] = "Команда не разрешена введенными кодами защиты ККМ";
	cmdResult[0xB2] = "Невозможна отмена скидки/надбавки";
	cmdResult[0xB3] = "Невозможно закрыть чек данным типом оплаты (в чеке присутствуют операции без контроля наличных)";
	cmdResult[0xBA] = "Ошибка обмена с фискальным модулем";
	cmdResult[0xBE] = "Необходимо провести профилактические работы";
	cmdResult[0xC8] = "Нет устройства, обрабатывающего данную команду";
	cmdResult[0xC9] = "Нет связи с внешним устройством";
	cmdResult[0xCA] = "Ошибочное состояние ТРК";
	cmdResult[0xCB] = "Больше одной регистрации в чеке";
	cmdResult[0xCC] = "Ошибочный номер ТРК";
	cmdResult[0xCD] = "Неверный делитель";
	cmdResult[0xCF] = "В ККМ произведено 20 активизаций";
	cmdResult[0xD0] = "Активизация данной ЭКЛЗ в составе данной ККМ невозможна";
	cmdResult[0xD1] = "Перегрев головки принтера";
	cmdResult[0xD2] = "Ошибка обмена с ЭКЛЗ на уровне интерфейса I2C";
	cmdResult[0xD3] = "Ошибка формата передачи ЭКЛЗ";
	cmdResult[0xD4] = "Неверное состояние ЭКЛЗ";
	cmdResult[0xD5] = "Неисправимая ошибка ЭКЛЗ";
	cmdResult[0xD6] = "Авария крипто-процессора ЭКЛЗ";
	cmdResult[0xD7] = "Исчерпан временной ресурс ЭКЛЗ";
	cmdResult[0xD8] = "ЭКЛЗ переполнена";
	cmdResult[0xD9] = "В ЭКЛЗ переданы неверная дата или время";
	cmdResult[0xDA] = "В ЭКЛЗ нет запрошенных данных";
	cmdResult[0xDB] = "Переполнение ЭКЛЗ (итог чека)";
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

