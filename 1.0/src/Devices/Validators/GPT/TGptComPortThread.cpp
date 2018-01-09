//---------------------------------------------------------------------------
#pragma hdrstop
#include <time.h>
#include <math.h>
#include "TGptComPortThread.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

__fastcall TGptComPortThread::TGptComPortThread(bool CreateSuspended): TThread(CreateSuspended)
{
/*
   param = new    TComPortInitParameters(CS);
   param->PortNumberString="Com1";
   param->BaudRate = 9600;
   param->Parity   =EVENPARITY;
   param->PortNumber = 1;
   param->StopBits = ONESTOPBIT;
   param->ByteSize = 8;
   param->timeout = 200;
   param->Xon = 0;
   param->Xoff = 0;
   comport = new  TComPort(param , false);
   comport->timeout  = 250;
   cicle_command=true;
*/
  Log = new TLogClass("GPTComPortThread");
  //init_comport(NULL);
  //Sleep(10);
  param=NULL;
  cicle_command=true;
  gptcomport = NULL;
}
//инициализация порта
bool __fastcall TGptComPortThread::init_comport(const TComPortInitParameters* p)
{
   AnsiString buff;
 if(param!=NULL)
 {
   delete param;
   param = NULL;
   Log->Write("TGptComPortThread::init_comport(const TComPortInitParameters* p)   - deleting param");
 }

 CS = new   TCriticalSection();
 Log->Write("TGptComPortThread::init_comport(const TComPortInitParameters* p)   -new cs ");
 param = new    TComPortInitParameters(CS);
 Log->Write("TGptComPortThread::init_comport(const TComPortInitParameters* p)   -new param");

 if(gptcomport!=NULL)
 {
   delete(gptcomport);
   gptcomport = NULL;
 }

 if(p==NULL)
 {
   param->PortNumberString="Com1";
   param->BaudRate = 9600;
   param->Parity   =EVENPARITY;
   param->PortNumber = 1;
   param->StopBits = ONESTOPBIT;
   param->ByteSize = 8;
   param->timeout = 200;
   param->Xon = 0;
   param->Xoff = 0;
   gptcomport = new  TGptComPort(param , false);
   Log->Write("TGptComPortThread::init_comport(const TComPortInitParameters* p)   - new gptC");
   gptcomport->timeout  = 250;
   gptcomport->Init();
   return false;
 }
 else
 {
/*
   param->PortNumberString="Com1";
   param->BaudRate = 9600;
   param->Parity   =EVENPARITY;
   param->PortNumber = 1;
   param->StopBits = ONESTOPBIT;
   param->ByteSize = 8;
   param->timeout = 200;
   param->Xon = 0;
   param->Xoff = 0;
   gptcomport = new  TGptComPort(param , false);              // Log->Write("TGptComPortThread::init_comport(const TComPortInitParameters* p)   - new gptC");
   gptcomport->timeout  = 250;
   gptcomport->Init();
*/


   param->PortNumberString=p->PortNumberString;
   param->BaudRate   = p->BaudRate;
   param->Parity     = p->Parity;
   param->PortNumber = p->PortNumber;
   param->StopBits   = p->StopBits;
   param->ByteSize   = p->ByteSize;
   param->timeout    = p->timeout;
   param->Xon        = p->Xon;
   param->Xoff       = p->Xoff;

   buff.sprintf("%d, \n %d, \n %d, \n %d,\n %d,\n %d,\n %d,\n %d \n",
   param->BaudRate,
   param->Parity,
   param->PortNumber,
   param->StopBits,
   param->ByteSize,
   param->timeout,
   param->Xon,
   param->Xoff );
   Log->Write(buff.c_str());
/*
   Log->WriteBuer(p->BaudRate,1);
   Log->Write(p->Parity);
   Log->Write(p->PortNumber);
   Log->Write(p->StopBits);
   Log->Write(p->ByteSize);
   Log->Write(p->timeout);
   Log->Write(p->Xon);
   Log->Write(p->Xoff);
  */

   gptcomport = new  TGptComPort(param , false);
   Log->Write("TGptComPortThread::init_comport(const TComPortInitParameters* p)   -new gptG");
   gptcomport->timeout  = p->timeout;
   gptcomport->Init();
   Log->Write("TGptComPortThread::init_comport(const TComPortInitParameters* p)   - end");
 }

   Log->Write("TGptComPortThread::init_comport(const TComPortInitParameters* p)   - stop");
 return true;
}
//инициализация устройства
bool __fastcall TGptComPortThread::init_device()
{
 Log->Write("TGptComPortThread::init_device()   - 0");
 if(  gptcomport == NULL)  return false;
 if(!gptcomport->PortInit) return false;
 Log->Write("TGptComPortThread::init_device()   - true");

//разрешение всех купюр 5 типов
//#02#00#00#0F#41#1F#00#00#00#00#00#00#00#FF#8F  V
  sizeof_one_com = 15;
  ONE_COM[0] =0x02;
  ONE_COM[1] =0x00;
  ONE_COM[2] =0x00;
  ONE_COM[3] =0x0F;
  ONE_COM[4] =0x41;
  ONE_COM[5] =0x1F;
  ONE_COM[6] =0x00;
  ONE_COM[7] =0x00;
  ONE_COM[8] =0x00;
  ONE_COM[9] =0x00;
  ONE_COM[10]=0x00;
  ONE_COM[11]=0x00;
  ONE_COM[12]=0x00;
  ONE_COM[13]=0xFF;
  ONE_COM[14]=0x8F;
  Log->Write("TGptComPortThread::init_device()   - 000");
  gptcomport->ClearCOMPort();
  Sleep(200);
  gptcomport->WriteToPort(ONE_COM,sizeof_one_com);
  Log->Write("TGptComPortThread::init_device()   - 001");
  // место вылета
  Sleep(200);
  Log->Write("TGptComPortThread::init_device()   - 002");
  gptcomport->ClearCOMPort();
  Log->Write("TGptComPortThread::init_device()   - fly!!!!!");
// режима контроля инкассации
//#02#00#00#07#46#FF#B1
  ONE_COM[3]=0x07;
  ONE_COM[4]=0x46;
  ONE_COM[5]=0xFF;
  ONE_COM[6]=0xB1;
  sizeof_one_com = 7;
  Sleep(200);
  gptcomport->WriteToPort(ONE_COM,sizeof_one_com);
  gptcomport->ClearCOMPort();
  Sleep(50);
   Log->Write("TGptComPortThread::init_device()   - 2");
//T( 00006219 ms)CMD[ 02 00 00 07 48 FF AF ]
  ONE_COM[3]=0x07;
  ONE_COM[4]=0x48;
  ONE_COM[5]=0xFF;
  ONE_COM[6]=0xAF;
  sizeof_one_com = 7;
  gptcomport->WriteToPort(ONE_COM,sizeof_one_com);
  Sleep(50);
  gptcomport->ClearCOMPort();
   Log->Write("TGptComPortThread::init_device()   - 3");
//T( 00006938 ms)CMD[ 02 00 00 08 43 02 FF B1 ]
  ONE_COM[3]=0x08;
  ONE_COM[4]=0x43;
  ONE_COM[5]=0x02;
  ONE_COM[6]=0xFF;
  ONE_COM[7]=0xB1;
  sizeof_one_com = 8;
  gptcomport->WriteToPort(ONE_COM,sizeof_one_com);
  Sleep(50);
  gptcomport->ClearCOMPort();
   Log->Write("TGptComPortThread::init_device()   - 4");
//T( 00007297 ms)CMD[ 02 00 00 08 43 00 FF B3 ]
  ONE_COM[3]=0x08;
  ONE_COM[4]=0x43;
  ONE_COM[5]=0x02;
  ONE_COM[6]=0xFF;
  ONE_COM[7]=0xB3;
  sizeof_one_com = 8;
  gptcomport->WriteToPort(ONE_COM,sizeof_one_com);
  Sleep(50);
  gptcomport->ClearCOMPort();
   Log->Write("TGptComPortThread::init_device()   - 5");
//T( 00013766 ms)CMD[ 02 00 00 07 F1 FF 06 ]
  ONE_COM[3]=0x07;
  ONE_COM[4]=0xF1;
  ONE_COM[5]=0xFF;
  ONE_COM[6]=0x06;
  sizeof_one_com = 7;
  gptcomport->WriteToPort(ONE_COM,sizeof_one_com);
  Sleep(50);
  gptcomport->ClearCOMPort();
   Log->Write("TGptComPortThread::init_device()   - 6");
//T( 00014125 ms)CMD[ 02 00 00 08 40 01 FF B5 ]
  ONE_COM[3]=0x08;
  ONE_COM[4]=0x40;
  ONE_COM[5]=0x01;
  ONE_COM[6]=0xFF;
  ONE_COM[7]=0xB5;
  sizeof_one_com = 8;
  gptcomport->WriteToPort(ONE_COM,sizeof_one_com);
  Sleep(50);
  gptcomport->ClearCOMPort();
   Log->Write("TGptComPortThread::init_device()   - 7");
//02 00 00 08 43 00 FF B3
  ONE_COM[3]=0x08;
  ONE_COM[4]=0x43;
  ONE_COM[5]=0x00;
  ONE_COM[6]=0xFF;
  ONE_COM[7]=0xB3;
  sizeof_one_com = 8;
  gptcomport->WriteToPort(ONE_COM,sizeof_one_com);
  Sleep(50);
  gptcomport->ClearCOMPort();
   Log->Write("TGptComPortThread::init_device()   - 8");
  //02 00 00 08 40 02 FF B4
  ONE_COM[3]=0x08;
  ONE_COM[4]=0x40;
  ONE_COM[5]=0x02;
  ONE_COM[6]=0xFF;
  ONE_COM[7]=0xB4;
  sizeof_one_com = 8;
  gptcomport->WriteToPort(ONE_COM,sizeof_one_com);
  Sleep(50);
  gptcomport->ClearCOMPort();
   Log->Write("TGptComPortThread::init_device()   - 9");
//включение режима непрерывных опросов
  ONE_COM[3]=0x08;
  ONE_COM[4]=0x40;
  ONE_COM[5]=0x00;
  ONE_COM[6]=0xFF;
  ONE_COM[7]=0xB6;
  sizeof_one_com = 8;
  gptcomport->WriteToPort(ONE_COM,sizeof_one_com);
  Sleep(50);
  gptcomport->ClearCOMPort();
   Log->Write("TGptComPortThread::init_device()   - 10");
//02 00 00 07 CC FF 2B
  ONE_COM[3]=0x07;
  ONE_COM[4]=0xCC;
  ONE_COM[5]=0xFF;
  ONE_COM[6]=0x2B;
  sizeof_one_com = 7;
  gptcomport->WriteToPort(ONE_COM,sizeof_one_com);
  Sleep(1000);
  gptcomport->ClearCOMPort();
  sizeof_command=7;
  COMMAND[0]=0x02;
  COMMAND[1]=0x00;
  COMMAND[2]=0x00;
  COMMAND[3]=0x07;
  COMMAND[4]=0xCC;
  COMMAND[5]=0xFF;
  COMMAND[6]=0x2B;
   Log->Write("TGptComPortThread::init_device()   - end ");
  return true;
}

//---------------------------------------------------------------------------
void __fastcall TGptComPortThread::Execute()
{
  AnsiString ans,ss;
  int i; //счетчик циклов

  FreeOnTerminate = true; // освободить занятую потоком память по окончании его работы
  //обнуляем переменные число купюр и сумму
  state->GlobalCount =0;
  state->GlobalPay   =0;
  i=0;
  //первичная инициализация
 Log->Write("TGptComPortThread::Execute()   - 0");
  gptcomport->ClearCOMPort();
//  if(!init_device()) return;
 Log->Write("TGptComPortThread::Execute()   -1");
  Sleep(250);
  gptcomport->ClearCOMPort();
 Log->Write("TGptComPortThread::Execute()   - start while");
  while(true)
  {
     i++;
     ss.sprintf("%d",i);
     if(Terminated) break;  // прекратить извне поток
     pb();                 // тело цикла(можно менять)
     Sleep(50);
  }
}

// тело цикла(можно менять)
void __fastcall TGptComPortThread::pb()
{
        AnsiString ans,ss;
        gptcomport->ClearCOMPort();
        gptcomport->timeout = 250;
        Sleep(200);
        memset(ANSWER,0,256);
        if(!cicle_command)
        {
          gptcomport->WriteToPort(ONE_COM,    sizeof_one_com);
          Sleep(2*sizeof_one_com);
        }else
        {
          gptcomport->WriteToPort(COMMAND,sizeof_command);
          Sleep(2*sizeof_command);
        }
        cicle_command = true;
        gptcomport->ReadFromPort(ANSWER,   sizeof_answer);
        Sleep(50);
        if(Lab1!=NULL)
        {
          *state = GptByteToStructStatus(ANSWER,sizeof_answer, state->GlobalCount,state->GlobalPay);
          if((GptByteToStructStatus(ANSWER,sizeof_answer,0,0).Nominal!=-1)&&
             (GptByteToStructStatus(ANSWER,sizeof_answer,0,0).STACKING_FLAG))
          {
            state->GlobalCount = state->GlobalCount +1;
            state->GlobalPay = state->GlobalPay + state->Nominal;
            gptcomport->ClearCOMPort();
            Sleep(2000);
            return;
          }
          if(GptByteToStructStatus(ANSWER,sizeof_answer,0,0).Nominal<0)
          {
            Sleep(500);
          }
          if(
          (GptByteToStructStatus(ANSWER,sizeof_answer,0,0).BILL_RETURNED_FLAG)||
          (GptByteToStructStatus(ANSWER,sizeof_answer,0,0).REJECTING_FLAG)||
          (GptByteToStructStatus(ANSWER,sizeof_answer,0,0).STACKING_FLAG)||
          (GptByteToStructStatus(ANSWER,sizeof_answer,0,0).RETURNING_FLAG)||
          (GptByteToStructStatus(ANSWER,sizeof_answer,0,0).BILL_JAM)||
          (GptByteToStructStatus(ANSWER,sizeof_answer,0,0).CHEATED_FLAG)||
          (GptByteToStructStatus(ANSWER,sizeof_answer,0,0).INVALID_ESCROW_REQUEST_FLAG)||
          (GptByteToStructStatus(ANSWER,sizeof_answer,0,0).ACCEPTING_FLAG)
            )
          {
            gptcomport->ClearCOMPort();
            Sleep(1500);
            gptcomport->ClearCOMPort();
            return;
          }
        }
}

bool TGptComPortThread::executeOne()
{
  //приостановить команду
  //перевести в режим одиночного исполнения
  //возобновить нить
  return false;
}
//-----
//Сопряженные функции НЕ ЧЛЕНЫ
int GetNominal(AnsiString vname, int item) // номиналы валют
{
  int ret;

  if(vname=="rur") //рубли
    switch(item)
    {
      case 1:
      {
        ret = 10;
        break;
      }
      case 2:
      {
        ret = 50;
        break;
      }
      case 3:
      {
        ret = 100;
        break;
      }
      case 4:
      {
        ret = 500;
        break;
       }
      case 5:
      {
        ret = 1000;
        break;
      }
      case 6:
      {
        ret = 5000;
        break;
      }
      default: ret = -1;
    }
  if(vname=="kzt") // тенге
    switch(item)
    {
      case 1:
      {
        ret = 200;
        break;
      }
      case 2:
      {
        ret = 500;
        break;
      }
      case 3:
      {
        ret = 1000;
        break;
      }
      case 4:
      {
        ret = 2000;
        break;
       }
      case 5:
      {
        ret = 5000;
        break;
      }
      case 6:
      {
        ret = 10000;
        break;
      }
      default: ret = -1;
    }
  return ret;
}



TGptState GptByteToStructStatus(const BYTE* RAW_DATA,const int zize_raw_data, const unsigned int gk, const unsigned int gp)
{
  TGptState ret;
  BYTE buf;
// проверка пакета на корректность
  if((zize_raw_data<12)||(RAW_DATA==NULL))
  {
    ret.cb = false;
    return ret;
  }
  if((RAW_DATA[0]!=0x02)||(RAW_DATA[1]!=0x00)||(RAW_DATA[2]!=0x00))
  {
    ret.cb = false;
    return ret;
  }
  if((RAW_DATA[3]<0x0D)||(RAW_DATA[4]!=0xCC))
  {
    ret.cb = false;
    return ret;
  }
  ret.cb =true;
  ret.GlobalCount=gk; // сохраняем количество купюр, введенных за сессию
  ret.GlobalPay  =gp; // сохраняем сумму налика за сессию
  //вычисление номинала купюры(для другой валюты нужно описать ее в функции)
  ret.Nominal = GetNominal("rur", int(RAW_DATA[8]));
  if(ret.Nominal<1) ret.cb = false;
  // Расшифровка первого  байта статуса
  buf = 0x80;
  ret.BILL_STACKED_FLAG   = ((buf&RAW_DATA[5])==buf);
  buf = 0x40;
  ret.BILL_RETURNED_FLAG  = ((buf&RAW_DATA[5])==buf);
  buf = 0x20;
  ret.BILL_IN_ESCROW_FLAG  = ((buf&RAW_DATA[5])==buf);
  // расшифровка второго байта статуса.
  buf = 0x80;
  ret.CHEATED_FLAG                   = ((buf&RAW_DATA[6])==buf);
  buf = 0x40;
  ret.POWER_ON_RESET_FLAG            = ((buf&RAW_DATA[6])==buf);
  buf = 0x20;
  ret.INVALID_ESCROW_REQUEST_FLAG    = ((buf&RAW_DATA[6])==buf);
  buf = 0x10;
  ret.REJECTING_FLAG                 = ((buf&RAW_DATA[6])==buf);
  buf = 0x08;
  ret.STACKING_FLAG                  = ((buf&RAW_DATA[6])==buf);
  buf = 0x04;
  ret.RETURNING_FLAG                 = ((buf&RAW_DATA[6])==buf);
  buf = 0x02;
  ret.ACCEPTING_FLAG                 = ((buf&RAW_DATA[6])==buf);
  buf = 0x01;
  ret.IDLE_FLAG                      = ((buf&RAW_DATA[6])==buf);
  // расшифровка третьего  байта статуса.
  buf = 0x80;
  ret.INHIBITED_FLAG                 = ((buf&RAW_DATA[7])==buf);
  buf = 0x40;
  ret.VALIDATOR_DISABLED             = ((buf&RAW_DATA[7])==buf);
  buf = 0x20;
  ret.COMMUNICATION_TIME_OUT_FLAG    = ((buf&RAW_DATA[7])==buf);
  buf = 0x10;
  ret.POWER_UP_WITH_BILL_IN_CHANNEL  = ((buf&RAW_DATA[7])==buf);
  buf = 0x08;
  ret.STACKER_FULL_FLAG              = ((buf&RAW_DATA[7])==buf);
  buf = 0x04;
  ret.STACKER_OFF_OPEN_FLAG          = ((buf&RAW_DATA[7])==buf);
  buf = 0x02;
  ret.STACKER_JAM_FLAG               = ((buf&RAW_DATA[7])==buf);
  buf = 0x01;
  ret.BILL_JAM                       = ((buf&RAW_DATA[7])==buf);
  // расшифровка пятого  байта статуса.
  buf = 0x01;
  ret.ACCEPTED_FLAG                  = ((buf&RAW_DATA[8])==buf);
  return ret;
}
