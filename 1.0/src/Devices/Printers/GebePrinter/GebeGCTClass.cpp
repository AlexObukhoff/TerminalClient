//---------------------------------------------------------------------------


#pragma hdrstop

#include "GebeGCTClass.h"
#include "GebeGCTThread.h"
#include "DeviceThread.h"
#include "boost/format.hpp"

//---------------------------------------------------------------------------

#pragma package(smart_init)

GebeGCT::GebeGCT(int ComPort,int BaudRate,TLogClass* _Log) : CPrinter(ComPort,_Log, "GebeGCT")
{
  DataLength = 1;
  delete Port;
  COMParameters->Parity = NOPARITY;
  COMParameters->BaudRate = 19200;
  if (BaudRate > 0)
      COMParameters->BaudRate = BaudRate;
  Port = NULL;
  Port = new TComPort(COMParameters,Log,true);
  Log->Write(COMParameters->PortNumberString.c_str());
  DeviceName = "GebeGCT";

  LoggingErrors = true;
  Init();
  SetInitialized();
  //GetState();
}

GebeGCT::~GebeGCT()
{
}

void GebeGCT::SendCommand()
{
  DeviceThread = new TDeviceThread(true,false);
  Start();
  delete DeviceThread;
  DeviceThread = NULL;
}


/* Процедура печати чека.
Отличается от нижеописанной тем, что ей передается  готовая очередь в виде списка
Описана для удобства.
Примечание: Принтер как-то странно обрабатывает поступающие команды.
Чтобы не было искажений данных при печати, команды нужно разделять задержками,
порядка 30-50 милисекунд.
Шапка и хвост вынесены в локальную процедуру печати, чтобы не загромождать код.
Нужно, чтобы длинна строк в массиве не превышла 28. Иначе текст будет искажен.
*/
void GebeGCT::PrintCheck(TStringList* Text)
{
  AnsiString subtext;
  Sleep(20);
  for(int i=0; i<Text->Count; i++)
  {// главный цикл печати
      subtext = Text->Strings[i];
      Sleep(30);     // обязательная задержка
      PrintLine(subtext);
  }
  Sleep(50);
  Feed();    //
  Sleep(30); //
  Feed();    //
  Sleep(30); //
  Feed();    //
  Sleep(30); //         Отступ, тобы не было отреза раньше времени
  Feed();    //
  Sleep(30); //
  Feed();    //
  Sleep(30); //
  Feed();    //
  Sleep(30); //        >
  Cut();// ОТРЕЗ
}

void GebeGCT::SendPacket(BYTE* command, int bytes_count,int datalen, BYTE* data, bool datafirst)
{
  if (!Port->PortInit)
    return;
  try
  {
    //здесь в буфер пишем команду для выполнения
    ClearCommand();
    ClearAnswer();

    if (!datafirst)
    {
      for(int i=0; i<bytes_count; i++)
        Command[i] = command[i];
        if (datalen != 0)
      {
        if (data!=NULL)
          memcpy(&Command[bytes_count],data,datalen);
      }
    }
    else
    {
      if (datalen != 0)
      {
        if (data!=NULL)
          memcpy(&Command[0],data,datalen);
      }
      for(int i=datalen; i<datalen+bytes_count; i++)
        Command[i] = command[i-datalen];
    }
     CommandSize = bytes_count+datalen;
   }
  __finally
  {
  }
}

// Печать строки текста
void GebeGCT::PrintLine(AnsiString text)
{
   BYTE command[2];
   //ClearCommand();   //очистка буферов
   //ClearAnswer();    //
   SendType = NotRecieveAnswer;    // не принимать ответы
   command[0] = 0x0A; //  < формируется команда печати строки
   command[1] = 0x0D; //  >  длинна выводимой строки не должна превышать 28
   int datalen = text.Length();
   Sleep(50);
   SendPacket(command,2,datalen,text.c_str());  // формирование пакета
   SendCommand();                               // посыл команды
}


void GebeGCT::Feed() // прогон строки
{             BYTE command[2];

   //ClearCommand();   //очистка буферов
   //ClearAnswer();    //
   command[0] = '\n'; //формирование команды сдвига на
   command[1] = '\r'; //             одну строку вниз
   SendType = NotRecieveAnswer;    // не принимать ответы
   Sleep(50);
   SendPacket(command,2,0,NULL,true); // формирование пакета
   SendCommand();                     // посыл команды
}



void GebeGCT::Init()// инициализация принтера
{
   BYTE command[2];
   //ClearCommand();   //очистка буферов
   //ClearAnswer();    //
   SendType = NotRecieveAnswer;    // не принимать ответы
   command[0] = 0x1B;    //формирование
   command[1] = 0x40;    // тела команды
   Sleep(50);
   SendPacket(command,2,0,NULL); // формирование пакета
   SendCommand();  // отправка команды
}

void GebeGCT::Cut()// отрез чека
{
   BYTE command[2];    // тело команды
   BYTE c;             // режим отеза, отрезать будем всегда по-полной.

   command[0] = 0x1B;     //команда
   command[1] = 0x43;     //     отрезать чек
   c=0;           // режим отреза: полный отрез

   //ClearCommand();        //очистка
   //ClearAnswer();         //       буферов
   Sleep(50);
   SendType = NotRecieveAnswer;    // не принимать ответ
   SendPacket(command,2,0,NULL); // формирование пакета
   SendCommand();  // отправка команды
   // команда послана, теперь отправка режима команды
   //(разделение обязательно, иначе не работает)
   SendType = NotRecieveAnswer;    // не принимать ответ
   SendPacket(&c,1,0,NULL);        // формирование пакета
   SendCommand();  // отправка  режима команды (отрезать полностью)
}

/*
Упаковывает коды всех ошибок в 1 байт.
Используется в  функции getState();
Номера соответствующих бит в байте Error:
76543210
ХХХХХХХХ
Расшифровка бдет следующей:
0) Код 48, Печатающая головка поднята
1) Код 50, Бумага закончилась (если только этот бит, то бумага зажевана)
2) Код 5A, >10%  бумаги  (при нехватке бумаги появляется с битом №1)
3) Код 47, ошибка сенсора ??? (Aux sensor)
4) Код 43, отрезатель заблокирован
5) Код 4B, Низкая температра.
6) Код 54, Перегрев головки печати.
7) Код 55 и  4D,  Низкое или высокое напряжение питание.
*/
std::string  GebeGCT::getOneMessage(BYTE data)
{
  std::string description;
  bool error;

  error = true; // для определения состояния устройства
  switch(data)
  {// Начало кодов ошибок и описаний
    case 0x43:
    {
      description ="Отрезатель чеков заблокирован (Cutter blocked)";
      Error=0x10;
      break;
    }
    case 0x47:
    {
      description ="Ошибка сенсора? (Aux sensor)";
      Error=0x08;
      break;
    }
    case 0x48:
    {
      description ="Печатающая головка поднята (Head lifted)";
      Error=0x01;
      break;
    }
    case 0x4B:
    {
      description ="Низкая темперура головки (Temp. low.)";
      Error=0x20;
      break;
    }
    case 0x4D:
    {
      description ="Слишком высокое напряжение питания (Vp too high)";
      Error=0x80;
      break;
    }
    case 0x50:
    {
      description ="Бумага закончилась (Paper end)";
      Error=0x02;
      break;
    }
    case 0x54:
    {
      description ="Перегрев головки (Temp. high)";
      Error=0x40;
      break;
    }
    case 0x55:
    {
      description ="Слишком низкое напряжение питания (Vp too low)";
      Error=Error || 0x80;
      break;
    }
    case 0x58:
    {
      description ="Все норально (Ok)";
      error = false;
      break;
    }
    case 0x5A:
    {
      description ="Осталось менее 10% бумаги (10% paper end)";
      Error= Error || 0x04;
      break;
    }
    default:
    {
      description ="Неизвестная ошибка печати (unknown printing error)";
    }
  }// Конец кодов ошибок и описаний
  if(error) State = 0x00; //если была ошибка, то запишем состояние как "ошибка"
  else       // инае запишем состояне как ОК
  {
    Error = 0x00;
    State = 0xFF;
  }
  return description;
}

/*  Получение состояния принтера GEBE
Коды состояний описываются тремя переменными
типа байт:  Error,    State и   SubState.
Основных состояния 3:
1) Нормально
Байты состояния выглядят следующим образом
   Error    - все нули,
   State    - все единицы,
   SubState - все единицы.
2) Нет отклика
Байты состояния выглядят следующим образом
   Error    - все нули,
   State    - все нули,
   SubState - все нули.
Причина - либо ниесправность устройства, либо отсутствие питающего
напряжения или отсутствует соединения по кабелю.
3) Ошибка устройства
   Error    - описано  в getOneMessage(BYTE), см. выше,
   State    - все нули,
   SubState - все единицы.
Кроме кодов состояний формируется непосредственное описание
состояния устройства в переменных типа строк:
StateDescr;
SubStateDescr;
Они служат для передачи аварийных сообщений на сервер статистики.
*/
void GebeGCT::GetState()
{
  std::string bug;
//  bool ok, test;
  BYTE command[2];
  BYTE param;
  int i;

  Error    = 0x00;       //очистка
  State    = 0x00;       //буферов
  SubState = 0x00;
  ClearCommand();
  ClearAnswer();

  Init();                       //инициализация
  SendType = RecieveAnswer;     //

  command[0] = 0x1B; //формирование команды
  command[1] = 0x6B; //
  param      = 0xFF; //

  Sleep(10);      // задержка перед чтением состояния

  SendPacket(command,2,1,&param);// Отправка команды
  SendCommand();

// Чтение и обработка ответа
  if(DeviceState->AnswerSize == 0)
  { // нет связи с принтером
    Error    = 0x00;
    State    = 0x00;
    SubState = 0x00;
    StateDescr    = " Принтер   неработоспособен ";
    SubStateDescr = " Отклик принтера не получен ";
    DeviceState->StateCode = 0xFF;
    DeviceState->SubStateCode = 0x00;
    Error = 1;
    PrinterEnable = false;
    DeviceState->OutStateCode = DSE_NOTMOUNT;
    Log->Write("Printer has no answer.");
    DeviceState->StateDescription = "Принтер не подключен";
    if (OldState != DeviceState->StateCode)
    {
      OldState = DeviceState->StateCode;
      DeviceState->StateChange = true;
      ChangeDeviceState();
    }
    return;
  }
  if((DeviceState->AnswerSize == 1)&&(Answer[0]==0x58))
  {// все хорошо
    Error    = 0x00;
    State    = 0xFF;
    SubState = 0xFF;
    StateDescr    = " Принтер работоспособен  ";
    SubStateDescr = " Отклик принтера получен ";
    return;
  }
  else bug = "Ошибка принтера ";
  for(i=0;i<DeviceState->AnswerSize;i++) // формирование сообщения
    bug += (boost::format(",\n\r%1% ") % getOneMessage(Answer[i])).str();
  StateDescr = bug;

  //bug.printf("размер  текущего ответа = %d, текущий ответ:\n\r (1_%d)(2_%d)(3_%d)(4_%d)(5_%d)(6_%d)(7_%d)(8_%d)(9_%d)(10_%d)",DeviceState->AnswerSize,Answer[0],Answer[1],
  //Answer[2],Answer[3],Answer[4],Answer[5],Answer[6],Answer[7],Answer[8],Answer[9]);
  //ShowMessage(bug);
}


/*     ПРОЦДУРА ПЕЧТИ ЧЕКА
В качестве  парметра получает строку очереди печати, разделители - "\r\n".
Примечание: Принтер как-то странно обрабатывает поступающие команды.
Чтобы не было искажений данных при печати, команды нужно разделять задержками,
порядка 30-50 милисекунд.
Шапка и хвост вынесены в локальную процедуру печати, чтобы не загромождать код.
Нужно, чтобы длинна строк в массиве не превышла 28. Иначе текст будет искажен.
*/
void GebeGCT::PrintCheck(AnsiString text, std::string barcode)
{// печать чека

int pos;
AnsiString delim = "\r\n";
AnsiString subtext;
char _subtext[1024];

//  Sleep(30);
  PrintLine(" -------------------------- "); // Печать шапки
//  Sleep(30);
  while(true)
  {// цикл разделения команд и последовательной печати
    pos = text.Pos(delim);                         //вычисление следующего разделителя
    //if (pos == 0)  break;                          //если конец строки, то прерваться
      if (pos == 0)
      {
          AnsiString subtext = text;
          memset(_subtext,0,1024);
          CharToOem(subtext.c_str(), _subtext);
          PrintLine(AnsiString(_subtext));
          LinesCount++;
          break;
      }
    subtext = text.SubString(0,pos-1);             //очередная команда на печать
    text = text.SubString(pos+1,text.Length()-pos);//оставшаяся очередь
    if (!subtext.IsEmpty())          //предохранитель от пустых команд
    {
      Sleep(20);                     //ОБЯЗАТЕЛЬНАЯ задержка перед печатью
      PrintLine(subtext);            //САБЖ
    }
  }
  Sleep(10); //        <
  Feed();    //
  Sleep(10); //
  Feed();    //
  Sleep(10); //
  Feed();    //
  Sleep(10); //         Отступ, тобы не было отреза раньше времени
  Feed();    //
  Sleep(10); //
  Feed();    //
  Sleep(10); //
  Feed();    //
  Sleep(10); //        >
  Cut();// ОТРЕЗ
}

bool GebeGCT::IsPrinterEnable()//  готовность к печати
{
//  AnsiString bug;
  bool ok;//, test=true;
  BYTE command[3];
  BYTE param;

  ClearCommand();        //очистка
  ClearAnswer();         //       буферов
  //Init();
  SendType = RecieveAnswer;
  command[0] = 0x1B;
  command[1] = 0x6B;
  param      = 0xFF;
  Sleep(50);      // задержка перед чтением состояния
  SendPacket(command,2,1,&param);
  SendCommand();
 //  bug.printf("размер  текущего ответа = %d, текущий ответ:\n\r (1_%d)(2_%d)(3_%d)(4_%d)(5_%d)(6_%d)(7_%d)(8_%d)(9_%d)(10_%d)",DeviceState->AnswerSize,Answer[0],Answer[1],
//   Answer[2],Answer[3],Answer[4],Answer[5],Answer[6],Answer[7],Answer[8],Answer[9]);
//   ShowMessage(bug);

  ok= false;
  //test = (DeviceState->AnswerSize == 1);
  //if(test)
    ok= ((Answer[0]==88)||(Answer[0]==90)||(Answer[0]==0x5a));
  return (ok);
}
