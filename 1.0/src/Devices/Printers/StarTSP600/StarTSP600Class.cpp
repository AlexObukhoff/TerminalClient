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

// печать фискального чека
void CStarTSP600::PrintCheck(double Money, AnsiString Text)
{
 if(Fiscal == false)
 {
  Log->Write("Try to print fiscal receipt with nonfiscal configuration");
  return;
 }

 GetState();
 if(DeviceState->SubStateCode == ST_SESSION_CLOSED)
  cmd_31(""); // регистрируем кассира

 // печатаем обычный чек
 CharToOem(Text.c_str(), Text.c_str());
 std::string checkText = Text.c_str();
 cmd_36(checkText);

 // печатаем фискальный чек
 if(cmd_53_openDocument(Money) != CMDRES_SUCCESS)
 {
  cmd_53_cancelDocument();
  cmd_4F();
 }

 // обрез и прогон делать не надо, т.к. автоматом после закрытия документа
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

 // попытаемся получить статус
 if(cmd_45() == CMDRES_SUCCESS)
 {
  kkmStatus = translateToNumeric(Answer + 3, sizeof(WORD));

  if(cmd_5C("306") == CMDRES_SUCCESS) // расширенный статус принтера
  {
   extStatus1 = translateToNumeric(Answer + 20 + 0, sizeof(BYTE));
   extStatus2 = translateToNumeric(Answer + 20 + 2, sizeof(BYTE));
   extStatus3 = translateToNumeric(Answer + 20 + 4, sizeof(BYTE));
   extStatus4 = translateToNumeric(Answer + 20 + 6, sizeof(BYTE));
   PrinterEnable = true;
  }
 }

 // проверим статус
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
// команды к ККМ

// регистрация кассира
WORD CStarTSP600::cmd_31(std::string cashierName)
{
 prologueCmd(0x31);

 // номер кассира
 Command[CommandSize + 0] = '0';
 CommandSize += 3;
 CommandSize += 40;
 Command[CommandSize] = DIV;
 CommandSize++;
 epilogueCmd();

 return execCmd();
}

// cmd_36() - перевод принтера в нефискальный режим
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
  // добъем в конец символ один интересный
  Command[CommandSize] = ESC;
  CommandSize++;
  execCmd();

  // показываем что данных на печать больше не будет
  Command[0] = ESC;
  CommandSize = 1;

  result = execCmd();
 }
 return result;
}

// cmd_45() - получить версию прошивки принтера (подходит для инициализации)
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

// cmd_52() - управление прогоном и отрезом ленты
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

// cmd_53() - фискальный документ
WORD CStarTSP600::cmd_53_openDocument(double price)
{
 char priceValue[11];
 memset(priceValue, 0, 11);
 sprintf(priceValue, "%.2f", price);
 std::string strValue = priceValue;

 prologueCmd(0x53);
 addByteToCommand(0x30);
 addStringToCommand("04"); // закрытие документа

 // количество передаваемых реквизитов
 addStringToCommand("016");

 // передаем реквизиты
 std::string filler(40, '=');
 cmd_53_fiscalField(99, 20, filler, 0, 0);
 cmd_53_fiscalField(01, 20, "", 1, 0);
 cmd_53_fiscalField(02, 20, "", 2, 0);
 cmd_53_fiscalField(03, 20, "", 3, 0);
 cmd_53_fiscalField(04, 20, "", 4, 0);
 cmd_53_fiscalField(99, 20, filler, 5, 0);

 cmd_53_fiscalField(0, 20, "", 6, 0); // номер ККМ
 cmd_53_fiscalField(7, 20, "", 6, 32); // номер документа
 cmd_53_fiscalField(10, 20, "", 7, 0); // РНН, РНМ
 cmd_53_fiscalField(5, 20, "", 8, 0); // дата операции
 cmd_53_fiscalField(8, 20, "", 8, 32); // номер чека
 cmd_53_fiscalField(6, 20, "", 9); // кассир
 cmd_53_fiscalSale(strValue, 10); // цена услуги
 cmd_53_fiscalField(12, 20, "", 11); // итоговая сумма
 cmd_53_fiscalField(13, 20, strValue, 12); // уплаченная сумма
 cmd_53_fiscalField(14, 20, "", 13); // сдача

 epilogueCmd();

 return execCmd();
}

WORD CStarTSP600::cmd_53_cancelDocument()
{
 prologueCmd(0x53);
 addByteToCommand(0x30);
 addStringToCommand("08"); // аннулирование документа
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

 CommandSize += 5; // пропускаем неиспользуемые байты

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

 Command[CommandSize + 0] = 0x31; // отедел
 Command[CommandSize + 3] = 0x30; // код товара
 CommandSize += 10;

 CommandSize += 6; // процентная скидка / надбавка
 CommandSize += 12; // количество

 memcpy(Command + CommandSize, price.c_str(), price.length());

 CommandSize += 12;

 // единица измерения количества
 CommandSize += 6;
}

// cmd_5C() - получить параметры ККМ
WORD CStarTSP600::cmd_5C(std::string param)
{
 prologueCmd(0x5C);
 addStringToCommand(param);
 epilogueCmd();

 return execCmd();
}

// cmd_5F() - печать отчетов
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
// методы оформления команд принтера
// prologueCmd - создать пролог пакета
void CStarTSP600::prologueCmd(BYTE cmdNum)
{
 ClearCommand();
 Command[0] = STX;
 Command[1] = cmdNum;
 CommandSize += 2;
 addStringToCommand(printerPassCode);
}

// epilogueCmd - создать эпилог пакета
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
 // статус ККМ
 std::string totalStatus = "Статус принтера:\r\n";
 if(kkmStatus & 0x0001)
 {
  totalStatus += "Смена открыта\r\n";
  DeviceState->SubStateCode = ST_SESSION_OPENED;
 }
 else
 {
  totalStatus += "Смена закрыта\r\n";
  DeviceState->SubStateCode = ST_SESSION_CLOSED;
 }

 if(kkmStatus & 0x0010)
  totalStatus += "Аппарат фискализированн\r\n";
 else
  totalStatus += "Аппарат нефискализированн\r\n";

 if(kkmStatus & 0x0020)
  totalStatus += "Фискальная память подходит к концу\r\n";

 if(kkmStatus & 0x0040)
 {
  totalStatus += "Фискальная память исчерпана\r\n";
  DeviceState->StateCode = DSE_HARDWARE_ERROR;
 }

 ////////////////////////
 // расширенный статус 1
 if(status1 & 0x04)
  totalStatus += "Денежный ящик - разомкнут\r\n";
 else
  totalStatus += "Денежный ящик - замкнут\r\n";

 if(status1 & 0x08)
 {
  totalStatus += "Принтер занят, выключен или произошла ошибка\r\n";
  DeviceState->StateCode = DSE_HARDWARE_ERROR;
 }

 if(status1 & 0x10)
 {
  totalStatus += "Принтер готов к работе\r\n";
 }
 else
 {
  totalStatus += "Принтер не готов к работе\r\n";
  DeviceState->StateCode = DSE_NOTMOUNT;
 }

 ////////////////////////
 // расширенный статус 2
 if(status2 & 0x04)
 {
  totalStatus += "Крышка принтера открыта\r\n";
  // error code here
 }

 if((status2 & 0x40) == 0)
 {
  totalStatus += "Ошибка принтера\r\n";
  DeviceState->StateCode = DSE_HARDWARE_ERROR;
 }

 ////////////////////////
 // расширенный статус 3
 if(status3 & 0x04)
 {
  totalStatus += "Ошибка механики принтера\r\n";
  DeviceState->StateCode = DSE_HARDWARE_ERROR;
 }

 if(status3 & 0x08)
 {
  totalStatus += "Ошибка автонарезчика\r\n";
  DeviceState->StateCode = DSE_HARDWARE_ERROR;
 }

 if(status3 & 0x20)
 {
  totalStatus += "Критическая ошибка принтера\r\n";
  DeviceState->StateCode = DSE_HARDWARE_ERROR;
 }

 if(status3 & 0x40)
 {
  totalStatus += "Перегрев печатающей головки\r\n";
  DeviceState->StateCode = DSE_HARDWARE_ERROR;
 }

 ////////////////////////
 // расширенный статус 4
 if(status4 & 0x08)
 {
  totalStatus += "Лента близка к завершению\r\n";
  DeviceState->StateCode = DSE_NEARENDPAPER;
 }

 if(status4 & 0x40)
 {
  totalStatus += "Конец бумаги\r\n";
  DeviceState->StateCode = DSE_NOTPAPER;
 }

 Log->Write(totalStatus.c_str());

 DeviceState->OutStateCode = DeviceState->StateCode;
}

void CStarTSP600::prepareCmdResults()
{
 for(int i = 0; i < 0xFF; i++)
  cmdResult[i] = "Неизвестный результат (изменилась прошивка?)";
/*
 cmdResult[0x00] = "Команда выполнена успешно";
 cmdResult[0x01] = "Ошибка в фискальных данных, аппарат блокирован.";
 cmdResult[0x02] = "Не закрыта смена";
 cmdResult[0x03] = "Исчерпан ресурс сменных записей в фискальную память.";
 cmdResult[0x04] = "Превышена длина поля команды.";
 cmdResult[0x05] = "Неверный формат поля команды.";
 cmdResult[0x06] = "Ошибка чтения таймера.";
 cmdResult[0x07] = "Неверная дата.";
 cmdResult[0x08] = "Неверное время.";
 cmdResult[0x09] = "Дата меньше последней даты, зарегистрированной в фискальной памяти.";
 cmdResult[0x0A] = "Операция прервана пользователем. Документ аннулирован.";
 cmdResult[0x0B] = "Запрещенная команда ПУ (см п. 3.3).";
 cmdResult[0x0C] = "Не открыта смена.";
 cmdResult[0x0D] = "Кассир не зарегистрирован.";
 cmdResult[0x0E] = "Переполнение приёмного буфера.";
 cmdResult[0x0F] = "Ошибка записи в фискальную память.";
 cmdResult[0x10] = "Ошибка установки таймера.";
 cmdResult[0x11] = "Неверный пароль налогового инспектора.";
 cmdResult[0x12] = "Неверный пароль на связь.";
 cmdResult[0x13] = "Исчерпан ресурс перерегистраций.";
 cmdResult[0x14] = "Аппарат не фискализирован.";
 cmdResult[0x15] = "Значение поля команды вне диапазона.";
 cmdResult[0x16] = "Ошибка чтения фискальной памяти.";
 cmdResult[0x17] = "Переполнение или отрицательный результат счётчика.";
 cmdResult[0x18] = "Обязательное поле команды имеет нулевую длину.";
 cmdResult[0x19] = "Неверный формат команды.";
 cmdResult[0x1A] = "Дата или время последнего документа в смене меньше предыдущего.";
 cmdResult[0x1B] = "Не используется.";
 cmdResult[0x1C] = "Ошибка в расположении реквизитов (пересечение или выход за область печати).";
 cmdResult[0x1D] = "Нет такой команды.";
 cmdResult[0x1E] = "Неверная контрольная сумма (BCC)";
 cmdResult[0x1F] = "Нет фискальных записей.";
 cmdResult[0x21] = "Оформление документа прервано по окончанию времени ожидания готовности принтера.";
 cmdResult[0x24] = "Буфер ответа пуст.";
 cmdResult[0x25] = "Услуга не введена";
 cmdResult[0x29] = "Дублирование обязательных реквизитов документа.";
 cmdResult[0x2A] = "Текущее состояние ККМ не позволяет выполнить операцию.";
 cmdResult[0x2B] = "Ошибка в данных энергонезависимой памяти. Аппарат блокирован.";
 cmdResult[0x2C] = "Невозможно выполнить инициализацию ФП. ФП уже инициализирована.";
 cmdResult[0x2D] = "Вывод прерван по окончанию времени ожидания готовности дисплея.";
 cmdResult[0x2E] = "Ошибка записи FLASH памяти.";
 cmdResult[0x2F] = "Ошибка. Нет записей.";
 cmdResult[0xA0] = "Переполнение ЭКЛ. Проведите распечатку ЭКЛ.";
*/
 cmdResult[0x00] = "Команда выполнена успешно";
 cmdResult[0x01] = "Ошибка в фискальных данных, аппарат блокирован.";
 cmdResult[0x02] = "Не закрыта смена";
 cmdResult[0x03] = "Исчерпан ресурс сменных записей в фискальную память.";
 cmdResult[0x04] = "Превышена длина поля команды.";
 cmdResult[0x05] = "Неверный формат поля команды.";
 cmdResult[0x06] = "Ошибка чтения таймера.";
 cmdResult[0x07] = "Неверная дата.";
 cmdResult[0x08] = "Неверное время.";
 cmdResult[0x09] = "Дата меньше последней даты, зарегистрированной в фискальной памяти.";
 cmdResult[0x0A] = "Операция прервана пользователем. Документ аннулирован.";
 cmdResult[0x0B] = "Запрещенная команда ПУ (см п. 3.3).";
 cmdResult[0x0C] = "Не открыта смена.";
 cmdResult[0x0D] = "Кассир не зарегистрирован.";
 cmdResult[0x0E] = "Переполнение приёмного буфера.";
 cmdResult[0x0F] = "Ошибка записи в фискальную память.";
 cmdResult[0x10] = "Ошибка установки таймера.";
 cmdResult[0x11] = "Неверный пароль налогового инспектора.";
 cmdResult[0x12] = "Неверный пароль на связь.";
 cmdResult[0x13] = "Исчерпан ресурс перерегистраций.";
 cmdResult[0x14] = "Аппарат не фискализирован.";
 cmdResult[0x15] = "Значение поля команды вне диапазона.";
 cmdResult[0x16] = "Ошибка чтения фискальной памяти.";
 cmdResult[0x17] = "Переполнение или отрицательный результат счётчика.";
 cmdResult[0x18] = "Обязательное поле команды имеет нулевую длину.";
 cmdResult[0x19] = "Неверный формат команды.";
 cmdResult[0x1A] = "Дата или время последнего документа в смене меньше предыдущего.";
 cmdResult[0x1B] = "Не используется.";
 cmdResult[0x1C] = "Ошибка в расположении реквизитов (пересечение или выход за область печати).";
 cmdResult[0x1D] = "Нет такой команды.";
 cmdResult[0x1E] = "Неверная контрольная сумма (BCC)";
 cmdResult[0x1F] = "Нет фискальных записей.";
 cmdResult[0x21] = "Оформление документа прервано по окончанию времени ожидания готовности принтера.";
 cmdResult[0x24] = "Буфер ответа пуст.";
 cmdResult[0x25] = "Услуга не введена";
 cmdResult[0x29] = "Дублирование обязательных реквизитов документа.";
 cmdResult[0x2A] = "Текущее состояние ККМ не позволяет выполнить операцию.";
 cmdResult[0x2B] = "Ошибка в данных энергонезависимой памяти. Аппарат блокирован.";
 cmdResult[0x2C] = "Невозможно выполнить инициализацию ФП. ФП уже инициализирована.";
 cmdResult[0x2D] = "Вывод прерван по окончанию времени ожидания готовности дисплея.";
 cmdResult[0x2E] = "Ошибка записи FLASH памяти.";
 cmdResult[0x2F] = "Ошибка. Нет записей.";
 cmdResult[0x30] = "Ошибка связи с ЭКЛЗ";
 cmdResult[0x31] = "Некорректный формат или параметр команды ЭКЛЗ";
 cmdResult[0x32] = "Некорректное состояние ЭКЛЗ";
 cmdResult[0x33] = "Авария ЭКЛЗ";
 cmdResult[0x34] = "Авария криптографического процессора ЭКЛЗ";
 cmdResult[0x35] = "Исчерпан временной ресурс использования ЭКЛЗ";
 cmdResult[0x36] = "ЭКЛЗ переполнена";
 cmdResult[0x37] = "Неверные дата или время в ЭКЛЗ";
 cmdResult[0x38] = "Нет запрошенных данных в ЭКЛЗ";
 cmdResult[0x39] = "Переполнение счётчиков ЭКЛЗ";
 cmdResult[0x42] = "Сбой криптопроцессора ЭКЛЗ";
 cmdResult[0x46] = "Ошибка протокола обмена ЭКЛЗ";
 cmdResult[0x47] = "Переполнение приёмного буфера ЭКЛЗ";
 cmdResult[0x48] = "Неверная контрольная сумма ЭКЛЗ";
 cmdResult[0x49] = "ЭКЛЗ активизирована в составе другой ККМ. Аппарат блокирован.";
 cmdResult[0x4A] = "ЭКЛЗ не активизирована.";
 cmdResult[0x4B] = "Неисправимая ошибка ЭКЛЗ";
 cmdResult[0x4C] = "Исчерпан ресурс активизаций ЭКЛЗ.";
 cmdResult[0x4D] = "ЭКЛЗ уже активизирована.";
 cmdResult[0x4E] = "Превышено количество секций в документе. (16 секций - ограничение ЭКЛЗ)";
 cmdResult[0x4F] = "Архив ЭКЛЗ закрыт или переполнение архива";
 cmdResult[0x50] = "Ошибка. Данные фискальной памяти и ЭКЛЗ различаются.";
 cmdResult[0x70] = "Переполнение счётчика количества в чеке, при сложении.";
 cmdResult[0x71] = "Переполнение счётчика количества в чеке, при вычитании.";
 cmdResult[0x72] = "Переполнение счётчика суммы аннулирований в чеке.";
 cmdResult[0x73] = "Переполнение сменного счётчика суммы аннулирований.";
 cmdResult[0x74] = "Переполнение сменного счётчика суммы продаж.";
 cmdResult[0x75] = "Переполнение сменного счётчика суммы наличных.";
 cmdResult[0x76] = "Переполнение сменного счётчика суммы оплаты в кредит.";
 cmdResult[0x77] = "Переполнение сменного счётчика суммы оплаты картой.";
 cmdResult[0x78] = "Переполнение сменного счётчика суммы наличных при вычитании.";
 cmdResult[0x79] = "Переполнение сменного счётчика суммы возвратов при оплате в кредит.";
 cmdResult[0x7A] = "Переполнение счётчика итоговой суммы в чеке при добавлении.";
 cmdResult[0x7B] = "Переполнение счётчика итоговой суммы в чеке при вычитании.";
 cmdResult[0x7C] = "Переполнение счётчика стоимости в чеке, при умножении количества на стоимость.";
 cmdResult[0x7D] = "Переполнение счётчика итоговой суммы в чеке при вычислении скидки.";
 cmdResult[0x7E] = "Переполнение счётчика итоговой суммы по секции в чеке при вычислении скидки.";
 cmdResult[0x7F] = "Переполнение счётчика итоговой суммы учёта тары в чеке при вычислении скидки.";
 cmdResult[0x80] = "Переполнение счётчика скидок в чеке.";
 cmdResult[0x81] = "Переполнение счётчика итоговой суммы налоговой группы в чеке при вычислении скидки.";
 cmdResult[0x82] = "Переполнение счётчика итоговой суммы безналоговой группы в чеке при вычислении скидки.";
 cmdResult[0x83] = "Переполнение счётчика итоговой суммы в чеке при вычислении надбавки.";
 cmdResult[0x84] = "Переполнение счётчика итоговой суммы по секции в чеке при вычислении надбавки.";
 cmdResult[0x85] = "Переполнение счётчика итоговой суммы учёта тары в чеке при вычислении надбавки.";
 cmdResult[0x86] = "Переполнение счётчика надбавок в чеке.";
 cmdResult[0x87] = "Переполнение счётчика итоговой суммы налоговой группы в чеке при вычислении надбавки.";
 cmdResult[0x88] = "Переполнение счётчика итоговой суммы безналоговой группы в чеке при вычислении надбавки.";
 cmdResult[0x89] = "Переполнение счётчика итоговой суммы по секции в чеке.";
 cmdResult[0x8A] = "Переполнение счётчика итоговой суммы учёта тары в чеке.";
 cmdResult[0x8B] = "Переполнение счётчика итоговой суммы налоговой группы в чеке.";
 cmdResult[0x8C] = "Переполнение счётчика итоговой суммы безналоговой группы в чеке.";
 cmdResult[0x8D] = "Переполнение счётчика итоговой суммы в чеке при вычислении аннулирования скидки.";
 cmdResult[0x8E] = "Переполнение счётчика итоговой суммы по секции в чеке при вычислении аннулирования скидки.";
 cmdResult[0x8F] = "Переполнение счётчика итоговой суммы учёта тары в чеке при вычислении аннулирования скидки.";
 cmdResult[0x90] = "Переполнение счётчика скидок в чеке при вычислении аннулирования скидки.";
 cmdResult[0x91] = "Переполнение счётчика итоговой суммы налоговой группы в чеке при вычислении аннулирования скидки.";
 cmdResult[0x92] = "Переполнение счётчика итоговой суммы безналоговой группы в чеке при вычислении аннулирования скидки.";
 cmdResult[0x93] = "Переполнение счётчика итоговой суммы в чеке при вычислении аннулирования надбавки.";
 cmdResult[0x94] = "Переполнение счётчика итоговой суммы по секции в чеке при вычислении аннулирования надбавки.";
 cmdResult[0x95] = "Переполнение счётчика итоговой суммы учёта тары в чеке при вычислении аннулирования надбавки.";
 cmdResult[0x96] = "Переполнение счётчика надбавок в чеке при вычислении аннулирования надбавки.";
 cmdResult[0x97] = "Переполнение счётчика итоговой суммы налоговой группы в чеке при вычислении аннулирования надбавки.";
 cmdResult[0x98] = "Переполнение счётчика итоговой суммы безналоговой группы в чеке при вычислении аннулирования надбавки.";
 cmdResult[0x99] = "Сумма оплаты меньше суммы чека.";
 cmdResult[0x9A] = "Переполнение счётчика итоговой суммы оплаты при комбинированной оплате.";
 cmdResult[0x9B] = "Сумма безналичной оплаты больше суммы чека.";
 cmdResult[0x9C] = "Блокировка выполнения команды. Расхождение текущей даты и даты последней записи в ФП больше запрограммированного значения. Для снятия блокировки необходимо выполнить команду программирования даты.";
 cmdResult[0x9D] = "Блокировка выполнения команды. Последний документ не напечатан. После восстановления работоспособности принтера (зарядить бумагу, опустить печатающую головку, закрыть крышку) принтер автоматически распечатывает документ и снимает блокировку.";
 cmdResult[0x9E] = "Блокировка выполнения команды. Выполняется тестирование оборудования ККМ.";
 cmdResult[0x9F] = "Блокировка ККМ. Необходимо выполнить гашение.";
 cmdResult[0xA1] = "Итоговая сумма чека возврата не равна итоговой возвращаемой сумме при комбинированном возврате";

}

