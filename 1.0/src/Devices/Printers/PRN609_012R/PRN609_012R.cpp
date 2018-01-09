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



/* ��������� ������ ����.
���������� �� ������������� ���, ��� �� ����������  ������� ������� � ���� ������
������� ��� ��������.
����������: ������� ���-�� ������� ������������ ����������� �������.
����� �� ���� ��������� ������ ��� ������, ������� ����� ��������� ����������,
������� 30-50 ����������.
����� � ����� �������� � ��������� ��������� ������, ����� �� ������������ ���.
�����, ����� ������ ����� � ������� �� �������� 28. ����� ����� ����� �������.
*/
void PRN609_012R::PrintCheck(TStringList* Text)
{
  AnsiString subtext;
  Sleep(1);
  PrintLine(" ------------------------- "); // ������ �����
  Sleep(10);
  for(int i=0; i<Text->Count; i++)
  {// ������� ���� ������
      subtext = Text->Strings[i];
      Sleep(1);     // ������������ ��������
      PrintLine(subtext);
  }
  PrintLine(" ------------------------- ");  //  <
  Sleep(30);                                  //  ������ ������
  PrintLine(" �������, ����������  ���  ");  //  >
  Sleep(30); //        <
  Feed();    //
  Feed();    //
  Feed();    //
  Sleep(30); //         ������, ���� �� ���� ������ ������ �������
  Feed();    //
  Feed();    //
  Feed();    //        >
  Cut();// �����
}

// ������ ������ ������
void PRN609_012R::PrintLine(AnsiString text)
{
  int datalen;
  BYTE command[2];

  SendType = NotRecieveAnswer;    // �� ��������� ������
  command[0] = 0x1B; //  < ����������� ������� ������ ������
  command[1] = 0x0A; //  >  ������ ��������� ������ �� ������ ��������� 28
  datalen = text.Length();
  sendComandToGptDevice_GetAnswer(command,2,NULL,0, COMParameters );
  sendComandToGptDevice_GetAnswer(text.c_str(),datalen,NULL,0, COMParameters );
}


void PRN609_012R::Feed() // ������ ������
{
  BYTE command[2];
  command[0] = 0x0A; //������������ ������� ������ ��
  sendComandToGptDevice_GetAnswer(command,1,NULL,0, COMParameters );
}


void PRN609_012R::Init()// ������������� ��������
{
   PrinterEnable =  false;
   BYTE command[5];
   BYTE font;

   command[0] = 0x1B;    // �������
   command[1] = 0x16;    //         �������������
   sendComandToGptDevice_GetAnswer(&command[1],1,NULL,0, COMParameters );
   command[1] = 0x00;    // ���� �������
   sendComandToGptDevice_GetAnswer(command,2,NULL,0, COMParameters );
   font =0x00;
   sendComandToGptDevice_GetAnswer(&font,1,NULL,0, COMParameters );
   Sleep(5);
  command[0] = 0x18; //������������ �������
  BYTE _State = (BYTE)State;
  sendComandToGptDevice_GetAnswer(&command[0],1,&_State,1, COMParameters );
  PrinterEnable = (State ==0x81);
}

void PRN609_012R::Cut()// ����� ����
{
  BYTE command;    // ���� �������
  command = 0x08;     //�������
  sendComandToGptDevice_GetAnswer(&command,1,NULL,0, COMParameters );
}



/*  ��������� ��������� �������� GEBE
���� ��������� ����������� ����� �����������
���� ����:  Error,    State �   SubState.
�������� ��������� 3:
1) ���������
����� ��������� �������� ��������� �������
   Error    - ��� ����,
   State    - ��� �������,
   SubState - ��� �������.
2) ��� �������
����� ��������� �������� ��������� �������
   Error    - ��� ����,
   State    - ��� ����,
   SubState - ��� ����.
������� - ���� ������������� ����������, ���� ���������� ���������
���������� ��� ����������� ���������� �� ������.
3) ������ ����������
   Error    - �������  � getOneMessage(BYTE), ��. ����,
   State    - ��� ����,
   SubState - ��� �������.
����� ����� ��������� ����������� ���������������� ��������
��������� ���������� � ���������� ���� �����:
StateDescr;
SubStateDescr;
��� ������ ��� �������� ��������� ��������� �� ������ ����������.
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
  command = 0x18; //������������ �������
// ������ � ��������� ������
  BYTE _State = (BYTE)State;
  sendComandToGptDevice_GetAnswer(&command,1,&_State,1, COMParameters );


  StateDescr = "";
  buf=State&&0x80; // 7
  if(buf == 0x80)
  {
    StateDescr +="����������� ���������� (uncknown device)\n";
  }
  buf=State&&0x40;   //6
  if(buf == 0x40 )
  {
    StateDescr +="������������ ������ (Full buffer  16 bytes)\n";
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
    StateDescr +="������ ��������� (Cutter error)\n";
  }
  buf=State&&0x08;  //3
  if(buf == 0x08 )
  {
    StateDescr +="������� ������� (Head open)\n";
    Error=0x01;
  }
  buf=State&&0x04;  //2
  if(buf == 0x04 )
  {
    Error=0x40;
    StateDescr +="�������� ������� (Head too hot to print)\n";
  }
  buf=State&&0x02;  //1
  if(buf == 0x02 )
  {
    StateDescr +="����������� ������\n";
    Error=0x02;
  }
  buf=State&&0x01; //0
  if(buf != 0x01 )
  {
    StateDescr +="���������� ������� ������(Logic level is low)\n";
  }
  command = 0x19; //������������ �������
  buf = 0x00;
  sendComandToGptDevice_GetAnswer(&command,1,&buf,1, COMParameters );
  if((buf>230)||(buf<210))
  {
    Error=0x80;
    StateDescr +="������������ ���������� �������(Voltage error)\n";
  }
  AnswerStr = (boost::format("���������� ������� �������� - %d �. \n") % buf).str();
  StateDescr += AnswerStr;

  command = 0x1A; //������������ �������
  buf = 0x00;
  sendComandToGptDevice_GetAnswer(&command,1,&buf,1, COMParameters );
  AnswerStr = (boost::format("���������� ������� ������� - %d *�. \n") % buf).str();
  StateDescr += AnswerStr;
  if((State ==0x81)&&(Error == 0x00))
  {
    StateDescr +="���������� � ��������������� ���������!";
    State    = 0xFF;
    State    = 0x00;
    SubState = 0xFF;

    PrinterEnable = true;
  }


}


/*     �������� ����� ����
� ��������  �������� �������� ������ ������� ������, ����������� - "\r\n".
����������: ������� ���-�� ������� ������������ ����������� �������.
����� �� ���� ��������� ������ ��� ������, ������� ����� ��������� ����������,
������� 30-50 ����������.
����� � ����� �������� � ��������� ��������� ������, ����� �� ������������ ���.
�����, ����� ������ ����� � ������� �� �������� 28. ����� ����� ����� �������.
*/
void PRN609_012R::PrintCheck(AnsiString text, std::string barcode)
{// ������ ����

int pos;
char _subtext[1024];
AnsiString delim = "\r\n";
AnsiString subtext;

  PrintLine("\n ------------------------- "); // ������ �����

  while(true)
  {// ���� ���������� ������ � ���������������� ������
    pos = text.Pos(delim);                         //���������� ���������� �����������
    //if (pos == 0)  break;                          //���� ����� ������, �� ����������
      if (pos == 0)
      {
          AnsiString subtext = text;
          memset(_subtext,0,1024);
          CharToOem(subtext.c_str(), _subtext);
          PrintLine(AnsiString(_subtext));
          LinesCount++;
          break;
      }
    subtext = text.SubString(0,pos-1);             //��������� ������� �� ������
    text = text.SubString(pos+1,text.Length()-pos);//���������� �������
    if (!subtext.IsEmpty())          //�������������� �� ������ ������
    {
      Sleep(2);                     //������������ �������� ����� �������
      PrintLine(subtext);            //����
    }
  }
  PrintLine("\n ------------------------- ");  //  <
  Sleep(3);                                  //  ������ ������
  PrintLine("\n  �������, ����������  ���  ");  //  >
  Sleep(1); //        <
  Feed();    //
  Feed();    //
  Feed();    //
  Sleep(1); //         ������, ���� �� ���� ������ ������ �������
  Feed();    //
  Sleep(1); //        >
  Cut();// �����
}

bool PRN609_012R::IsPrinterEnable()//  ���������� � ������
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
  Sleep(50);      // �������� ����� ������� ���������

 //  bug.printf("������  �������� ������ = %d, ������� �����:\n\r (1_%d)(2_%d)(3_%d)(4_%d)(5_%d)(6_%d)(7_%d)(8_%d)(9_%d)(10_%d)",DeviceState->AnswerSize,Answer[0],Answer[1],
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

//�������� ����������
if((COMMAND==NULL)||(conf==NULL))
return false;
if(size_command< 1)
return false ;

// �������������: ����������� ����� �� ���� ��������
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
// �������� ���������
{// ���� �� ������� ��������
// ANSWER[0]='1';
return false ;
}
if ( !GetCommState(PortHandle, &dcb) )
{ // ���� �� ������� �������� ������
//ANSWER[0]='2';
return false;
}
//�������� ��������� � ����� �����
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
{// ���� �� ������� ���������� ������
// ANSWER[0]='3';
return false;
}

//������ � ���������� ������ � ������
TMT.ReadIntervalTimeout= 10;
TMT.ReadTotalTimeoutMultiplier = 1; // �������� ���� ���� ����� ������ ������� ��� ���������� ������
TMT.ReadTotalTimeoutConstant = 100; // ���� �� �������� 110 ���
TMT.WriteTotalTimeoutMultiplier = 20; // ������������ � ������ ������ ��� ����� �������� �������
TMT.WriteTotalTimeoutConstant = 5;
if(!SetCommTimeouts(PortHandle, &TMT))
{ // ���� �� ������� ���������� ��������
// ANSWER[0]='4';
return false;
}

//������� �����
if (!PurgeComm(PortHandle, PURGE_RXCLEAR))
{// ���� �� ������� �������� ����� ������
// ANSWER[0]='5';
return false;
}
if(!PurgeComm(PortHandle, PURGE_TXCLEAR))
{ // ���� �� ������� �������� ����� ��������
// ANSWER[0]='6';
return false ;
}
ClearCommError(PortHandle, &temp, &ComState);
if(temp!= 0)
{// ���� �� ������� �������� ����� ������
// ANSWER[0]='7';
return false;
}
// ������� �������
if((size_command > 28 )||(size_answer>5)) Sleep(100 );
if(!WriteFile(PortHandle, COMMAND ,size_command, &numbytes, NULL))
{ // ���� �� ������� ��������� �������
// ANSWER[0]='8';
CloseHandle(PortHandle);
return false ;
}
// ������� ������ �� ������
temp=0 ;
ClearCommError(PortHandle, &temp, &ComState);
if(temp!= 0)
{// ���� �� ������� �������� ����� ������
// ANSWER[0]='9';
return false;
}
if(size_answer> 5) Sleep(200);
// ����� ������
if(!ReadFile(PortHandle, ANSWER,size_answer , &numbytes_ok, NULL))
{// ���� �� ������� ������� �����
// ANSWER[0]='A';
CloseHandle(PortHandle);
return false;
}
if((size_command > 28)||(size_answer>5)) Sleep( 100);
// ��������� ����
return CloseHandle(PortHandle);
}
//#endif

