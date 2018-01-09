//---------------------------------------------------------------------------


#pragma hdrstop

#include "BD2Class.h"
#include "DeviceThread.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

bd2Class::bd2Class(int comPort,int baudRate, TLogClass* log)
: CPrinter(comPort, log, "bd2-286")
{
  COMParameters->Parity = NOPARITY;
  COMParameters->BaudRate = 19200;
  if (baudRate > 0)
      COMParameters->BaudRate = baudRate;
  if (Port)
      Port->ReopenPort();
  LoggingErrors = true;
  State = 0x00;
  OldState = 0x00;
  SubState = 0x00;
  OldSubState = 0x00;
  DeviceName = "bd2-286";
  DataLength = 1;
}

bd2Class::~bd2Class()
{

}

void bd2Class::PrintCheck(AnsiString text, std::string barcode)
{
  CharToOem(text.c_str(), text.c_str());
  std::string checkText = text.c_str();

  // отсылаем текст чека
  SendType = NotRecieveAnswer;
  sendCmd((const BYTE*)checkText.c_str(), checkText.length());

  // отсылаем каманду печати
  BYTE cmdPrint[] = {0x1B, 0x64, 0x05};
  //cmdPrint[2] = linesCount/2;
  SendType = NotRecieveAnswer;
  sendCmd(cmdPrint, sizeof(cmdPrint));

  cut();
}

void bd2Class::GetState()
{
  BYTE cmd[2] = {0x1B, 0x76};
  SendType = RecieveAnswer;
  sendCmd(cmd, sizeof(cmd));

  if(DeviceState->AnswerSize > 0)
  {
   if((Answer[0] & 0x04))
    DeviceState->OutStateCode = DSE_NOTPAPER;
   else
    DeviceState->OutStateCode = DSE_OK;
  }
  else
  {
   DeviceState->OutStateCode = DSE_HARDWARE_ERROR;
  }

  ChangeDeviceState();
}

int bd2Class::Initialize()
{
 BYTE cmd[] = {0x1B, 0x40};
 SendType = NotRecieveAnswer;
 sendCmd(cmd, sizeof(cmd));

 setPrintMode(FONTB);
 return 0;
}

void bd2Class::sendCmd(const BYTE* buffer, unsigned int bufferSize)
{
 ClearAnswer();
 ClearCommand();

 memcpy(Command, buffer, bufferSize);
 CommandSize = bufferSize;

 SendCommand();
}

void bd2Class::cut()
{
 BYTE cmd[] = {0x1B, 0x69};
 SendType = NotRecieveAnswer;
 sendCmd(cmd, sizeof(cmd));
}

void bd2Class::setPrintMode(BYTE flags)
{
 BYTE cmd[] = {0x1B, 0x21, 0x00};
 cmd[2] = flags;

 SendType = NotRecieveAnswer;
 sendCmd(cmd, sizeof(cmd));
}

