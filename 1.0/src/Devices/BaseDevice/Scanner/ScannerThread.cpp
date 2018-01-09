//---------------------------------------------------------------------------


#pragma hdrstop

#include "ScannerThread.h"
#include "boost\format.hpp"

//---------------------------------------------------------------------------
__fastcall CScannerThread::CScannerThread()
        : TDeviceThread(true)
{
 //Log->Write("CScanerThread ctor()");
}

__fastcall CScannerThread::~CScannerThread()
{
 Log->Write("CScanerThread ~ctor()");
}
//---------------------------------------------------------------------------

void __fastcall CScannerThread::ProcessLoopCommand()
{
 Log->Write("ProccesLoopCommand");

 Port->ReopenPort();

 memset(MainBuffer, 0, sizeof(MainBuffer));

 Port->ClearCOMPort();

  COMMTIMEOUTS cto;
  cto.ReadIntervalTimeout = 25;
  cto.ReadTotalTimeoutMultiplier = 0;
  cto.ReadTotalTimeoutConstant = 0;
  cto.WriteTotalTimeoutMultiplier = 0;
  cto.WriteTotalTimeoutConstant = 0;

  SetCommTimeouts(Port->Port, &cto);

  while(true)
  {
   DWORD totalBytesRead = 0;
   bool isSTX = false;
   bool isETX = false;
   bool isDataAccepted = false;

   while(!isDataAccepted)
   {
    DWORD bytesRead = 0;

    ReadFile(Port->Port, MainBuffer + totalBytesRead, 1, &bytesRead, NULL);

    if(isSTX && isETX && bytesRead == 1)
     isDataAccepted = true;

    if(isSTX && !isETX && MainBuffer[totalBytesRead] == 0x03)
     isETX = true;

    if(!isSTX && MainBuffer[totalBytesRead] == 0x02)
     isSTX = true;

    totalBytesRead += bytesRead;
   }

   BYTE ack = 0x06;
   DWORD bytesWrite = 0;
   WriteFile(Port->Port, &ack, 1, &bytesWrite, NULL);

   Log->WriteBuffer(MainBuffer, totalBytesRead);
   memset(TempBuffer, 0, sizeof(TempBuffer));
   memcpy(TempBuffer, MainBuffer + 2, totalBytesRead - 4);

   codeType = MainBuffer[1];
   codeValue = (char*)TempBuffer;
   unknownByte = MainBuffer[totalBytesRead - 1];

   Log->Write((boost::format("scanned value: %1%") % codeValue).str().c_str());

   // покажем что пришли данные
   DeviceState->scannerDataValue = codeValue;
   DeviceState->scannerDataType = codeType;
   DeviceState->OutStateCode = DSE_OK;
   ChangeDeviceState();

   //if(Terminate)
    //break;
  }
  //Port->ClearCOMPort();
}
/*
void __fastcall CScannerThread::ProcessLoopCommand()
{
 Log->Write("ProccesLoopCommand");

 Port->ReopenPort();
 BYTE portData[64];
 memset(portData, 0, sizeof(portData));

 Port->ClearCOMPort();

  COMMTIMEOUTS cto;
  cto.ReadIntervalTimeout = 25;
  cto.ReadTotalTimeoutMultiplier = 0;
  cto.ReadTotalTimeoutConstant = 0;
  cto.WriteTotalTimeoutMultiplier = 0;
  cto.WriteTotalTimeoutConstant = 0;

  SetCommTimeouts(Port->Port, &cto);

  while(true)
  {
   // ждем данные в порту
   DWORD bytesRead1 = 0;
   DWORD bytesRead2 = 0;
   ReadFile(Port->Port, portData, 32, &bytesRead1, NULL);

   // если считали ровно 32 байта, то проверим, может что-то осталось в буфере
   if(bytesRead1 == 32)
   {
    DWORD lastError;
    COMSTAT commStatus;
    ClearCommError(Port->Port, &lastError, &commStatus);
    if(commStatus.cbInQue > 0)
     ReadFile(Port->Port, portData + bytesRead1, commStatus.cbInQue, &bytesRead2, NULL);
   }

   bytesRead1 += bytesRead2;

   BYTE ack = 0x06;
   DWORD bytesWrite = 0;
   WriteFile(Port->Port, &ack, 1, &bytesWrite, NULL);

   Log->WriteBuffer(portData, bytesRead1);
   char tempBuf[64];
   memset(tempBuf, 0, sizeof(tempBuf));
   memcpy(tempBuf, portData + 2, bytesRead1 - 4);

   codeType = portData[1];
   codeValue = tempBuf;
   unknownByte = portData[bytesRead1 - 1];

   Log->Write((boost::format("scanned value: %1%") % codeValue).str().c_str());

   // покажем что пришли данные
   DeviceState->scannerDataValue = codeValue;
   DeviceState->scannerDataType = codeType;
   DeviceState->OutStateCode = DSE_OK;
   ChangeDeviceState();

   //if(Terminate)
    //break;
  }
  //Port->ClearCOMPort();
}
*/
#pragma package(smart_init)
