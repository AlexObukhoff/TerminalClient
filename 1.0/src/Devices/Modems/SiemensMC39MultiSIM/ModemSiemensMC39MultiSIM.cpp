//---------------------------------------------------------------------------
#pragma hdrstop

#include <string>

#include "ModemSiemensMC39MultiSIM.h"
#include "DeviceThread.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

#define BufferSize 500

using namespace std;

CModemSiemensMC39MultiSIM::CModemSiemensMC39MultiSIM(int ComPort,TLogClass* _Log) : CModemSiemensMC35i(ComPort, _Log, "SiemensMC39MultiSIM")
{
    DataLength = 1;
    COMParameters->Parity = NOPARITY;
    COMParameters->BaudRate = 115200;
    LoggingErrors = false;
    ThreadLifeTime = 3000;
    DeviceName = "SiemensMC39MultiSIM";
}

CModemSiemensMC39MultiSIM::~CModemSiemensMC39MultiSIM()
{
}

//Каждая команда согласно документации отправляется дважды на скорости 19200

bool CModemSiemensMC39MultiSIM::ResetModem()
{
    ReopenOn19200();
    AnsiString command = "@RESET";

    if (Log)
        Log->Write(">>>> ResetModem");
    if(!SendATCommand(command.c_str(),DATA_COMMAND))
    {
        if (Log)
            Log->Write("Выбран не тот COM порт!");
        return false;
    }
    if(!SendATCommand(command.c_str(),DATA_COMMAND))
    {
        if (Log)
            Log->Write("Выбран не тот COM порт!");
        return false;
    }

    ReopenOn115200();
    return true;
}

bool CModemSiemensMC39MultiSIM::ChangeSIM_No(int SIM_No)
{
    if ((SIM_No < 1)||(SIM_No > 4))
        return false;
    ReopenOn19200();

    AnsiString command = "@SIM_"+AnsiString(SIM_No);
    if (Log)
        Log->Write(">>>> ChangeSIM_No "+AnsiString(SIM_No));
    if(!SendATCommand(command.c_str(),DATA_COMMAND))
    {
        if (Log)
            Log->Write("Выбран не тот COM порт!");
        return false;
    }
    if(!SendATCommand(command.c_str(),DATA_COMMAND))
    {
        if (Log)
            Log->Write("Выбран не тот COM порт!");
        return false;
    }
    /*string str("");
    str = GetAnswerBuffer(20, "OK");
    AnsiString Answer = str.c_str();
    if (Log)
        Log->Write("Recieved answer: "+Answer);*/

    ReopenOn115200();
    return true;
}

bool CModemSiemensMC39MultiSIM::ChangeSIM()
{
    ReopenOn19200();
    AnsiString command = "@_SWAP";

    if (Log)
        Log->Write(">>>> ChangeSIM");
    if(!SendATCommand(command.c_str(),DATA_COMMAND))
    {
        if (Log)
            Log->Write("Выбран не тот COM порт!");
        return false;
    }
    if(!SendATCommand(command.c_str(),DATA_COMMAND))
    {
        if (Log)
            Log->Write("Выбран не тот COM порт!");
        return false;
    }

    ReopenOn115200();
    return true;
}

bool CModemSiemensMC39MultiSIM::IsItYou()
{
  try
  {
      AnsiString Type = GetModemType();
      if (Log)
          Log->Write("CModemSiemensMC39MultiSIM::IsItYou() found type: " + Type);
      std::string Temp(Type.c_str());
      if(Temp.find("MC39") != string::npos)
        return true;
      return false;
  }
  catch(Exception& ex)
  {
      if (Log)
          Log->Write("CModemSiemensMC39::IsItYou() Exception: " + AnsiString(ex.Message));
      return false;
  }
}

