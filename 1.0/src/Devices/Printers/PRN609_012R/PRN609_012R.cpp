//---------------------------------------------------------------------------


#pragma hdrstop
#include "CPrinter.h"
#include "PRN609_012R.h"
#include "TGptComPortThread.h"
#include "DeviceThread.h"
#include "boost/format.hpp"

//---------------------------------------------------------------------------

#pragma package(smart_init)

#ifndef PRN609_012RH
#define PRN609_012RH

#endif




PRN609_012R::PRN609_012R(int ComPort,int BaudRate,TLogClass* _Log)
:CPrinter(ComPort,_Log, "PRN609_012R")
{
   AnsiString csname;
   if(Port!=NULL) delete Port;
   csname.sprintf("Com%d",ComPort);
   //if(_Log==NULL)
   Log = new TLogClass("PRN609_012R");
   COMParameters = new    TComPortInitParameters(NULL);
   COMParameters->PortNumberString=csname;
   COMParameters->BaudRate = 115200;
   if (BaudRate > 0)
      COMParameters->BaudRate = BaudRate;
   COMParameters->Parity   =NOPARITY;
   COMParameters->PortNumber = 1;
   COMParameters->StopBits = ONESTOPBIT;
   COMParameters->ByteSize = 8;
   COMParameters->timeout = 10;
   COMParameters->Xon = 0;
   COMParameters->Xoff = 0;
   Log->Write("init comport for PRN609_012R");
   LoggingErrors = true;
   SetInitialized();
   DeviceName = "PRN609_012R";
}

PRN609_012R::~PRN609_012R()
{
}



/* ѕроцедура печати чека.
ќтличаетс€ от нижеописанной тем, что ей передаетс€  готова€ очередь в виде списка
ќписана дл€ удобства.
ѕримечание: ѕринтер как-то странно обрабатывает поступающие команды.
„тобы не было искажений данных при печати, команды нужно раздел€ть задержками,
пор€дка 30-50 милисекунд.
Ўапка и хвост вынесены в локальную процедуру печати, чтобы не загромождать код.
Ќужно, чтобы длинна строк в массиве не превышла 28. »наче текст будет искажен.
*/
void PRN609_012R::PrintCheck(TStringList* Text)
{
  AnsiString subtext;
  Sleep(1);
  PrintLine(" ------------------------- "); // ѕечать шапки
  Sleep(10);
  for(int i=0; i<Text->Count; i++)
  {// главный цикл печати
      subtext = Text->Strings[i];
      Sleep(1);     // об€зательна€ задержка
      PrintLine(subtext);
  }
  PrintLine(" ------------------------- ");  //  <
  Sleep(30);                                  //  ѕечать хвоста
  PrintLine(" —ѕј—»Ѕќ, —ќ’–јЌя…“≈  „≈   ");  //  >
  Sleep(30); //        <
  Feed();    //
  Feed();    //
  Feed();    //
  Sleep(30); //         ќтступ, тобы не было отреза раньше времени
  Feed();    //
  Feed();    //
  Feed();    //        >
  Cut();// ќ“–≈«
}

// ѕечать строки текста
void PRN609_012R::PrintLine(AnsiString text)
{
  int datalen;
  BYTE command[2];

  SendType = NotRecieveAnswer;    // не принимать ответы
  command[0] = 0x1B; //  < формируетс€ команда печати строки
  command[1] = 0x0A; //  >  длинна выводимой строки не должна превышать 28
  datalen = text.Length();
  sendComandToGptDevice_GetAnswer(command,2,NULL,0, COMParameters );
  sendComandToGptDevice_GetAnswer(text.c_str(),datalen,NULL,0, COMParameters );
}


void PRN609_012R::Feed() // прогон строки
{
  BYTE command[2];
  command[0] = 0x0A; //формирование команды сдвига на
  sendComandToGptDevice_GetAnswer(command,1,NULL,0, COMParameters );
}


void PRN609_012R::Init()// инициализаци€ принтера
{
   PrinterEnable =  false;
   BYTE command[5];
   BYTE font;

   command[0] = 0x1B;    // команда
   command[1] = 0x16;    //         инициализации
   sendComandToGptDevice_GetAnswer(&command[1],1,NULL,0, COMParameters );
   command[1] = 0x00;    // тела команды
   sendComandToGptDevice_GetAnswer(command,2,NULL,0, COMParameters );
   font =0x00;
   sendComandToGptDevice_GetAnswer(&font,1,NULL,0, COMParameters );
   Sleep(5);
  command[0] = 0x18; //формирование команды
  BYTE _State = (BYTE)State;
  sendComandToGptDevice_GetAnswer(&command[0],1,&_State,1, COMParameters );
  PrinterEnable = (State ==0x81);
}

void PRN609_012R::Cut()// отрез чека
{
  BYTE command;    // тело команды
  command = 0x08;     //команда
  sendComandToGptDevice_GetAnswer(&command,1,NULL,0, COMParameters );
}



/*  ѕолучение состо€ни€ принтера GEBE
 оды состо€ний описываютс€ трем€ переменными
типа байт:  Error,    State и   SubState.
ќсновных состо€ни€ 3:
1) Ќормально
Ѕайты состо€ни€ выгл€д€т следующим образом
   Error    - все нули,
   State    - все единицы,
   SubState - все единицы.
2) Ќет отклика
Ѕайты состо€ни€ выгл€д€т следующим образом
   Error    - все нули,
   State    - все нули,
   SubState - все нули.
ѕричина - либо ниесправность устройства, либо отсутствие питающего
напр€жени€ или отсутствует соединени€ по кабелю.
3) ќшибка устройства
   Error    - описано  в getOneMessage(BYTE), см. выше,
   State    - все нули,
   SubState - все единицы.
 роме кодов состо€ний формируетс€ непосредственное описание
состо€ни€ устройства в переменных типа строк:
StateDescr;
SubStateDescr;
ќни служат дл€ передачи аварийных сообщений на сервер статистики.
*/
void PRN609_012R::GetState()
{
/*
Bit Status 0 1
0 Near end Logic level is low Logic level is high
1 Paper Present Absent
2 Temperature Not too hot Head too hot to print
3 Head Closed Open
4 Cutter No error Error
5 Rx error No error Rx error
6 Buffer Not full. Full (less than 16 bytes)
7 Always 1.
*/
  BYTE command;
  BYTE buf;

  std::string AnswerStr;

  PrinterEnable =  false;
  command = 0x18; //формирование команды
// „тение и обработка ответа
  BYTE _State = (BYTE)State;
  sendComandToGptDevice_GetAnswer(&command,1,&_State,1, COMParameters );


  StateDescr = "";
  buf=State&&0x80; // 7
  if(buf == 0x80)
  {
    StateDescr +="Ќеизвестное устройство (uncknown device)\n";
  }
  buf=State&&0x40;   //6
  if(buf == 0x40 )
  {
    StateDescr +="ѕереполнение буфера (Full buffer  16 bytes)\n";
  }
  buf=State&&0x20;  //5
  if(buf == 0x20 )
  {
    StateDescr +="Rx error \n";
    Error=0x08;
  }
  buf=State&&0x10;  //4
  if(buf == 0x10 )
  {
    Error=0x10;
    StateDescr +="ќшибка отрезчика (Cutter error)\n";
  }
  buf=State&&0x08;  //3
  if(buf == 0x08 )
  {
    StateDescr +="ќткрыта головка (Head open)\n";
    Error=0x01;
  }
  buf=State&&0x04;  //2
  if(buf == 0x04 )
  {
    Error=0x40;
    StateDescr +="ѕерегрев головки (Head too hot to print)\n";
  }
  buf=State&&0x02;  //1
  if(buf == 0x02 )
  {
    StateDescr +="ќтсутствует бумага\n";
    Error=0x02;
  }
  buf=State&&0x01; //0
  if(buf != 0x01 )
  {
    StateDescr +="Ћогический уровень низкий(Logic level is low)\n";
  }
  command = 0x19; //формирование команды
  buf = 0x00;
  sendComandToGptDevice_GetAnswer(&command,1,&buf,1, COMParameters );
  if((buf>230)||(buf<210))
  {
    Error=0x80;
    StateDescr +="Ќеподход€щее напр€жение питани€(Voltage error)\n";
  }
  AnswerStr = (boost::format("Ќапр€жение питани€ принтера - %d ¬. \n") % buf).str();
  StateDescr += AnswerStr;

  command = 0x1A; //формирование команды
  buf = 0x00;
  sendComandToGptDevice_GetAnswer(&command,1,&buf,1, COMParameters );
  AnswerStr = (boost::format("“емператра головки головки - %d *—. \n") % buf).str();
  StateDescr += AnswerStr;
  if((State ==0x81)&&(Error == 0x00))
  {
    StateDescr +="”стройство в работоспособном состо€нии!";
    State    = 0xFF;
    State    = 0x00;
    SubState = 0xFF;

    PrinterEnable = true;
  }


}


/*     ѕ–ќ÷ƒ”–ј ѕ≈„“» „≈ ј
¬ качестве  парметра получает строку очереди печати, разделители - "\r\n".
ѕримечание: ѕринтер как-то странно обрабатывает поступающие команды.
„тобы не было искажений данных при печати, команды нужно раздел€ть задержками,
пор€дка 30-50 милисекунд.
Ўапка и хвост вынесены в локальную процедуру печати, чтобы не загромождать код.
Ќужно, чтобы длинна строк в массиве не превышла 28. »наче текст будет искажен.
*/
void PRN609_012R::PrintCheck(AnsiString text, std::string barcode)
{// печать чека

int pos;
char _subtext[1024];
AnsiString delim = "\r\n";
AnsiString subtext;

  PrintLine("\n ------------------------- "); // ѕечать шапки

  while(true)
  {// цикл разделени€ команд и последовательной печати
    pos = text.Pos(delim);                         //вычисление следующего разделител€
    //if (pos == 0)  break;                          //если конец строки, то прерватьс€
      if (pos == 0)
      {
          AnsiString subtext = text;
          memset(_subtext,0,1024);
          CharToOem(subtext.c_str(), _subtext);
          PrintLine(AnsiString(_subtext));
          LinesCount++;
          break;
      }
    subtext = text.SubString(0,pos-1);             //очередна€ команда на печать
    text = text.SubString(pos+1,text.Length()-pos);//оставша€с€ очередь
    if (!subtext.IsEmpty())          //предохранитель от пустых команд
    {
      Sleep(2);                     //ќЅя«ј“≈Ћ№Ќјя задержка перед печатью
      PrintLine(subtext);            //—јЅ∆
    }
  }
  PrintLine("\n ------------------------- ");  //  <
  Sleep(3);                                  //  ѕечать хвоста
  PrintLine("\n  —ѕј—»Ѕќ, —ќ’–јЌя…“≈  „≈   ");  //  >
  Sleep(1); //        <
  Feed();    //
  Feed();    //
  Feed();    //
  Sleep(1); //         ќтступ, тобы не было отреза раньше времени
  Feed();    //
  Sleep(1); //        >
  Cut();// ќ“–≈«
}

bool PRN609_012R::IsPrinterEnable()//  готовность к печати
{
//  AnsiString bug;
  bool ok, test;
  BYTE command[3];
  BYTE param;


  //Init();
  SendType = RecieveAnswer;
  command[0] = 0x1B;
  command[1] = 0x6B;
  param      = 0xFF;
  Sleep(50);      // задержка перед чтением состо€ни€

 //  bug.printf("размер  текущего ответа = %d, текущий ответ:\n\r (1_%d)(2_%d)(3_%d)(4_%d)(5_%d)(6_%d)(7_%d)(8_%d)(9_%d)(10_%d)",DeviceState->AnswerSize,Answer[0],Answer[1],
//   Answer[2],Answer[3],Answer[4],Answer[5],Answer[6],Answer[7],Answer[8],Answer[9]);
//   ShowMessage(bug);

  ok= false;
  test = (DeviceState->AnswerSize == 1);
  if(test) ok= ((Answer[0]==88)||(Answer[0]==90));
  return (test&&ok);
}

bool sendComandToGptDevice_GetAnswer(const BYTE* COMMAND, const int size_command, BYTE*& ANSWER , const int size_answer, const TComPortInitParameters* conf)
{
HANDLE PortHandle;
DCB dcb;
DWORD temp;
COMMTIMEOUTS TMT;
COMSTAT ComState;
OVERLAPPED Overlap;
DWORD numbytes ,numbytes_ok;

//ќѕЌЅ≈ѕ…ё ќёѕёЋ≈–ѕЌЅ
if((COMMAND==NULL)||(conf==NULL))
return false;
if(size_command< 1)
return false ;

// ’ћ’∆’ё ’√ё∆’Џ: ёя’ћ”ѕЌћћЎ» ѕ≈‘’Ћ ќЌ –ѕ≈Ћ ќѕЌЅЌƒёЋ
// PortHandle=NULL;
numbytes_ok=0 ;
numbytes =0;

PortHandle = CreateFile( conf->GetPortNumberString().c_str(),
GENERIC_READ | GENERIC_WRITE,
0,
NULL,
OPEN_EXISTING,
FILE_ATTRIBUTE_NORMAL,
NULL);
if((PortHandle==NULL)||(PortHandle == INVALID_HANDLE_VALUE))
// ќѕЌЅ≈ѕ…ё яЌя–ЌЏћ’»
{// ≈я ’ ћ≈ —ƒё ЌяЁ Ќ–…ѕЎ–’≈
// ANSWER[0]='1';
return false ;
}
if ( !GetCommState(PortHandle, &dcb) )
{ // ≈я ’ ћ≈ —ƒё ЌяЁ ќЌ —¬’–Ё я–ё–—я
//ANSWER[0]='2';
return false;
}
//ќѕ’ЅЌƒ’Ћ ќёѕёЋ≈–ѕЎ … ћЌЅЌ» “ЌѕЋ≈
dcb.DCBlength = sizeof(DCB);
dcb.fDummy2 = 0;

//
DWORD BaudRate = conf->BaudRate;
BYTE StopBits = conf->StopBits;
BYTE Parity = conf->Parity;
BYTE ByteSize = conf->ByteSize;
DWORD fParity = conf->fParity;
DWORD fDtrControl = conf->DtrControl;
dcb.BaudRate = BaudRate;
dcb.StopBits = StopBits;
dcb.Parity = Parity;
dcb.ByteSize = ByteSize;
dcb.fParity = fParity;
dcb.fDtrControl = fDtrControl;
dcb.XonLim = conf->Xon;
dcb.XoffLim = conf->Xoff;
if(!SetCommState(PortHandle, &dcb))
{// ≈я ’ ћ≈ —ƒё ЌяЁ —я–ёћЌЅ’–Ё я–ё–—я
// ANSWER[0]='3';
return false;
}

//ѕёјЌ–ё я –ё»Ћё—–ёЋ’ ¬–≈ћ’Џ ’ √ёќ’я’
TMT.ReadIntervalTimeout= 10;
TMT.ReadTotalTimeoutMultiplier = 1; // √ћё¬≈ћ’» ў–’” –ё»Ћ ё—–ЌЅ ЅќЌ ћ≈ ”Ѕё–ё≈– ƒ Џ —Ѕ≈ѕ≈ћћЌ÷Ќ ќѕ’≈Ћё
TMT.ReadTotalTimeoutConstant = 100; // ƒё‘≈ ћё я…ЌѕЌя–’ 110 јЌƒ
TMT.WriteTotalTimeoutMultiplier = 20; // ’яќЌ Ё√—≈–яЏ Ѕ ƒёћћЌЋ я —¬ё≈ …ё… Ѕѕ≈ЋЏ Ќ‘’ƒёћ’Џ ќЌяЎ …’
TMT.WriteTotalTimeoutConstant = 5;
if(!SetCommTimeouts(PortHandle, &TMT))
{ // ≈я ’ ћ≈ —ƒё ЌяЁ —я–ёћЌЅ’–Ё –ё»Ћё—–Ў
// ANSWER[0]='4';
return false;
}

//бЎћ≈я≈Ћ Ћ—яЌѕ
if (!PurgeComm(PortHandle, PURGE_RXCLEAR))
{// ≈я ’ ћ≈ —ƒё ЌяЁ Ќ¬’я–’–Ё ј—“≈ѕ ќѕ’≈Ћё
// ANSWER[0]='5';
return false;
}
if(!PurgeComm(PortHandle, PURGE_TXCLEAR))
{ // ≈я ’ ћ≈ —ƒё ЌяЁ Ќ¬’я–’–Ё ј—“≈ѕ ќ≈ѕ≈ƒё¬’
// ANSWER[0]='6';
return false ;
}
ClearCommError(PortHandle, &temp, &ComState);
if(temp!= 0)
{// ≈я ’ ћ≈ —ƒё ЌяЁ Ќ¬’я–’–Ё ј—“≈ѕ Ќ№’јЌ…
// ANSWER[0]='7';
return false;
}
// ќЌяЎ …ё …ЌЋёћƒЎ
if((size_command > 28 )||(size_answer>5)) Sleep(100 );
if(!WriteFile(PortHandle, COMMAND ,size_command, &numbytes, NULL))
{ // ≈я ’ ћ≈ —ƒё ЌяЁ Ќ–ќѕёЅ’–Ё …ЌЋёћƒ—
// ANSWER[0]='8';
CloseHandle(PortHandle);
return false ;
}
// Ќ¬’я–…ё ј—“≈ѕё Ќ– Ќ№’јЌ…
temp=0 ;
ClearCommError(PortHandle, &temp, &ComState);
if(temp!= 0)
{// ≈я ’ ћ≈ —ƒё ЌяЁ Ќ¬’я–’–Ё ј—“≈ѕ Ќ№’јЌ…
// ANSWER[0]='9';
return false;
}
if(size_answer> 5) Sleep(200);
// ќѕ’≈Ћ Ќ–Ѕ≈–ё
if(!ReadFile(PortHandle, ANSWER,size_answer , &numbytes_ok, NULL))
{// ≈я ’ ћ≈ —ƒё ЌяЁ ќѕ’ћЏ–Ё Ќ–Ѕ≈–
// ANSWER[0]='A';
CloseHandle(PortHandle);
return false;
}
if((size_command > 28)||(size_answer>5)) Sleep( 100);
// √ё…ѕЎЅё≈Ћ ќЌѕ–
return CloseHandle(PortHandle);
}
//#endif

