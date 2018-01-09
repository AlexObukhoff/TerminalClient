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


/* ��������� ������ ����.
���������� �� ������������� ���, ��� �� ����������  ������� ������� � ���� ������
������� ��� ��������.
����������: ������� ���-�� ������� ������������ ����������� �������.
����� �� ���� ��������� ������ ��� ������, ������� ����� ��������� ����������,
������� 30-50 ����������.
����� � ����� �������� � ��������� ��������� ������, ����� �� ������������ ���.
�����, ����� ������ ����� � ������� �� �������� 28. ����� ����� ����� �������.
*/
void GebeGCT::PrintCheck(TStringList* Text)
{
  AnsiString subtext;
  Sleep(20);
  for(int i=0; i<Text->Count; i++)
  {// ������� ���� ������
      subtext = Text->Strings[i];
      Sleep(30);     // ������������ ��������
      PrintLine(subtext);
  }
  Sleep(50);
  Feed();    //
  Sleep(30); //
  Feed();    //
  Sleep(30); //
  Feed();    //
  Sleep(30); //         ������, ���� �� ���� ������ ������ �������
  Feed();    //
  Sleep(30); //
  Feed();    //
  Sleep(30); //
  Feed();    //
  Sleep(30); //        >
  Cut();// �����
}

void GebeGCT::SendPacket(BYTE* command, int bytes_count,int datalen, BYTE* data, bool datafirst)
{
  if (!Port->PortInit)
    return;
  try
  {
    //����� � ����� ����� ������� ��� ����������
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

// ������ ������ ������
void GebeGCT::PrintLine(AnsiString text)
{
   BYTE command[2];
   //ClearCommand();   //������� �������
   //ClearAnswer();    //
   SendType = NotRecieveAnswer;    // �� ��������� ������
   command[0] = 0x0A; //  < ����������� ������� ������ ������
   command[1] = 0x0D; //  >  ������ ��������� ������ �� ������ ��������� 28
   int datalen = text.Length();
   Sleep(50);
   SendPacket(command,2,datalen,text.c_str());  // ������������ ������
   SendCommand();                               // ����� �������
}


void GebeGCT::Feed() // ������ ������
{             BYTE command[2];

   //ClearCommand();   //������� �������
   //ClearAnswer();    //
   command[0] = '\n'; //������������ ������� ������ ��
   command[1] = '\r'; //             ���� ������ ����
   SendType = NotRecieveAnswer;    // �� ��������� ������
   Sleep(50);
   SendPacket(command,2,0,NULL,true); // ������������ ������
   SendCommand();                     // ����� �������
}



void GebeGCT::Init()// ������������� ��������
{
   BYTE command[2];
   //ClearCommand();   //������� �������
   //ClearAnswer();    //
   SendType = NotRecieveAnswer;    // �� ��������� ������
   command[0] = 0x1B;    //������������
   command[1] = 0x40;    // ���� �������
   Sleep(50);
   SendPacket(command,2,0,NULL); // ������������ ������
   SendCommand();  // �������� �������
}

void GebeGCT::Cut()// ����� ����
{
   BYTE command[2];    // ���� �������
   BYTE c;             // ����� �����, �������� ����� ������ ��-������.

   command[0] = 0x1B;     //�������
   command[1] = 0x43;     //     �������� ���
   c=0;           // ����� ������: ������ �����

   //ClearCommand();        //�������
   //ClearAnswer();         //       �������
   Sleep(50);
   SendType = NotRecieveAnswer;    // �� ��������� �����
   SendPacket(command,2,0,NULL); // ������������ ������
   SendCommand();  // �������� �������
   // ������� �������, ������ �������� ������ �������
   //(���������� �����������, ����� �� ��������)
   SendType = NotRecieveAnswer;    // �� ��������� �����
   SendPacket(&c,1,0,NULL);        // ������������ ������
   SendCommand();  // ��������  ������ ������� (�������� ���������)
}

/*
����������� ���� ���� ������ � 1 ����.
������������ �  ������� getState();
������ ��������������� ��� � ����� Error:
76543210
��������
����������� ���� ���������:
0) ��� 48, ���������� ������� �������
1) ��� 50, ������ ����������� (���� ������ ���� ���, �� ������ ��������)
2) ��� 5A, >10%  ������  (��� �������� ������ ���������� � ����� �1)
3) ��� 47, ������ ������� ??? (Aux sensor)
4) ��� 43, ���������� ������������
5) ��� 4B, ������ ����������.
6) ��� 54, �������� ������� ������.
7) ��� 55 �  4D,  ������ ��� ������� ���������� �������.
*/
std::string  GebeGCT::getOneMessage(BYTE data)
{
  std::string description;
  bool error;

  error = true; // ��� ����������� ��������� ����������
  switch(data)
  {// ������ ����� ������ � ��������
    case 0x43:
    {
      description ="���������� ����� ������������ (Cutter blocked)";
      Error=0x10;
      break;
    }
    case 0x47:
    {
      description ="������ �������? (Aux sensor)";
      Error=0x08;
      break;
    }
    case 0x48:
    {
      description ="���������� ������� ������� (Head lifted)";
      Error=0x01;
      break;
    }
    case 0x4B:
    {
      description ="������ ��������� ������� (Temp. low.)";
      Error=0x20;
      break;
    }
    case 0x4D:
    {
      description ="������� ������� ���������� ������� (Vp too high)";
      Error=0x80;
      break;
    }
    case 0x50:
    {
      description ="������ ����������� (Paper end)";
      Error=0x02;
      break;
    }
    case 0x54:
    {
      description ="�������� ������� (Temp. high)";
      Error=0x40;
      break;
    }
    case 0x55:
    {
      description ="������� ������ ���������� ������� (Vp too low)";
      Error=Error || 0x80;
      break;
    }
    case 0x58:
    {
      description ="��� �������� (Ok)";
      error = false;
      break;
    }
    case 0x5A:
    {
      description ="�������� ����� 10% ������ (10% paper end)";
      Error= Error || 0x04;
      break;
    }
    default:
    {
      description ="����������� ������ ������ (unknown printing error)";
    }
  }// ����� ����� ������ � ��������
  if(error) State = 0x00; //���� ���� ������, �� ������� ��������� ��� "������"
  else       // ���� ������� �������� ��� ��
  {
    Error = 0x00;
    State = 0xFF;
  }
  return description;
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
void GebeGCT::GetState()
{
  std::string bug;
//  bool ok, test;
  BYTE command[2];
  BYTE param;
  int i;

  Error    = 0x00;       //�������
  State    = 0x00;       //�������
  SubState = 0x00;
  ClearCommand();
  ClearAnswer();

  Init();                       //�������������
  SendType = RecieveAnswer;     //

  command[0] = 0x1B; //������������ �������
  command[1] = 0x6B; //
  param      = 0xFF; //

  Sleep(10);      // �������� ����� ������� ���������

  SendPacket(command,2,1,&param);// �������� �������
  SendCommand();

// ������ � ��������� ������
  if(DeviceState->AnswerSize == 0)
  { // ��� ����� � ���������
    Error    = 0x00;
    State    = 0x00;
    SubState = 0x00;
    StateDescr    = " �������   ���������������� ";
    SubStateDescr = " ������ �������� �� ������� ";
    DeviceState->StateCode = 0xFF;
    DeviceState->SubStateCode = 0x00;
    Error = 1;
    PrinterEnable = false;
    DeviceState->OutStateCode = DSE_NOTMOUNT;
    Log->Write("Printer has no answer.");
    DeviceState->StateDescription = "������� �� ���������";
    if (OldState != DeviceState->StateCode)
    {
      OldState = DeviceState->StateCode;
      DeviceState->StateChange = true;
      ChangeDeviceState();
    }
    return;
  }
  if((DeviceState->AnswerSize == 1)&&(Answer[0]==0x58))
  {// ��� ������
    Error    = 0x00;
    State    = 0xFF;
    SubState = 0xFF;
    StateDescr    = " ������� ��������������  ";
    SubStateDescr = " ������ �������� ������� ";
    return;
  }
  else bug = "������ �������� ";
  for(i=0;i<DeviceState->AnswerSize;i++) // ������������ ���������
    bug += (boost::format(",\n\r%1% ") % getOneMessage(Answer[i])).str();
  StateDescr = bug;

  //bug.printf("������  �������� ������ = %d, ������� �����:\n\r (1_%d)(2_%d)(3_%d)(4_%d)(5_%d)(6_%d)(7_%d)(8_%d)(9_%d)(10_%d)",DeviceState->AnswerSize,Answer[0],Answer[1],
  //Answer[2],Answer[3],Answer[4],Answer[5],Answer[6],Answer[7],Answer[8],Answer[9]);
  //ShowMessage(bug);
}


/*     �������� ����� ����
� ��������  �������� �������� ������ ������� ������, ����������� - "\r\n".
����������: ������� ���-�� ������� ������������ ����������� �������.
����� �� ���� ��������� ������ ��� ������, ������� ����� ��������� ����������,
������� 30-50 ����������.
����� � ����� �������� � ��������� ��������� ������, ����� �� ������������ ���.
�����, ����� ������ ����� � ������� �� �������� 28. ����� ����� ����� �������.
*/
void GebeGCT::PrintCheck(AnsiString text, std::string barcode)
{// ������ ����

int pos;
AnsiString delim = "\r\n";
AnsiString subtext;
char _subtext[1024];

//  Sleep(30);
  PrintLine(" -------------------------- "); // ������ �����
//  Sleep(30);
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
      Sleep(20);                     //������������ �������� ����� �������
      PrintLine(subtext);            //����
    }
  }
  Sleep(10); //        <
  Feed();    //
  Sleep(10); //
  Feed();    //
  Sleep(10); //
  Feed();    //
  Sleep(10); //         ������, ���� �� ���� ������ ������ �������
  Feed();    //
  Sleep(10); //
  Feed();    //
  Sleep(10); //
  Feed();    //
  Sleep(10); //        >
  Cut();// �����
}

bool GebeGCT::IsPrinterEnable()//  ���������� � ������
{
//  AnsiString bug;
  bool ok;//, test=true;
  BYTE command[3];
  BYTE param;

  ClearCommand();        //�������
  ClearAnswer();         //       �������
  //Init();
  SendType = RecieveAnswer;
  command[0] = 0x1B;
  command[1] = 0x6B;
  param      = 0xFF;
  Sleep(50);      // �������� ����� ������� ���������
  SendPacket(command,2,1,&param);
  SendCommand();
 //  bug.printf("������  �������� ������ = %d, ������� �����:\n\r (1_%d)(2_%d)(3_%d)(4_%d)(5_%d)(6_%d)(7_%d)(8_%d)(9_%d)(10_%d)",DeviceState->AnswerSize,Answer[0],Answer[1],
//   Answer[2],Answer[3],Answer[4],Answer[5],Answer[6],Answer[7],Answer[8],Answer[9]);
//   ShowMessage(bug);

  ok= false;
  //test = (DeviceState->AnswerSize == 1);
  //if(test)
    ok= ((Answer[0]==88)||(Answer[0]==90)||(Answer[0]==0x5a));
  return (ok);
}
