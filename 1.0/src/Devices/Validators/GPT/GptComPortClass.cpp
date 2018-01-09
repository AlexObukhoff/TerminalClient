//---------------------------------------------------------------------------
#pragma hdrstop
#include <vcl.h>
#include "GptComPortClass.h"
#include "LogClass.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

TGptComPortInitParameters::TGptComPortInitParameters()
{
  BaudRate = 9600;
  PortNumber = 1;
  StopBits = ONESTOPBIT;
  Parity = NOPARITY;
  ByteSize = 8;
  fParity = 0;
  DtrControl = DTR_CONTROL_HANDSHAKE;
  timeout = 200;
  Xon = 0;
  Xoff = 0;

}

TGptComPortInitParameters::~TGptComPortInitParameters()
{
}

AnsiString TGptComPortInitParameters::GetPortNumberString()
{
  PortNumberString.sprintf("Com%d",PortNumber);
  return PortNumberString;
}

void TGptComPortInitParameters::SetPortNumber(BYTE value)
{
/*
  PortNumber = value;
  char Port[5];
  itoa(PortNumber,(char*)Port,10);
  PortNumberString = "COM";
  PortNumberString += Port;
  */
  PortNumber = int(value);
  PortNumberString.sprintf("Com%d",value);

}

void TGptComPortInitParameters::SetParameters(BYTE PortNumber,DWORD BaudRate,BYTE StopBits,BYTE Parity,BYTE ByteSize,DWORD fParity,DWORD DtrControl,DWORD timeout)
{
  /*COMParametersCriticalSection->Acquire();
  try
  {
    SetPortNumber(PortNumber);
    this->BaudRate = BaudRate;
    this->StopBits = StopBits;
    this->Parity = Parity;
    this->ByteSize = ByteSize;
    this->fParity = fParity;
    this->DtrControl = DtrControl;
    this->timeout = timeout;
  }
  __finally
  {
    COMParametersCriticalSection->Release();
  }
    */
}

void TGptComPortInitParameters::GetParameters(BYTE& PortNumber,DWORD& BaudRate,BYTE& StopBits,BYTE& Parity,BYTE& ByteSize,DWORD& fParity,DWORD& DtrControl,DWORD& timeout)
{
      /*
  COMParametersCriticalSection->Acquire();
  try
  {
    PortNumber = this->PortNumber;
    BaudRate = this->BaudRate;
    StopBits = this->StopBits;
    Parity = this->Parity;
    ByteSize = this->ByteSize;
    fParity = this->fParity;
    DtrControl = this->DtrControl;
    timeout = this->timeout;
  }
  __finally
  {
    COMParametersCriticalSection->Release();
  }
        */
}







TGptComPort::TGptComPort(TComPortInitParameters* COMParameters, bool logging)
{
   this->COMParams = COMParameters;
//  str += COMParameters->GetPortNumberString();
  PortInit = false;
  Port = NULL;
  Log = new TLogClass("GPTComPortClass");
  //Init();

}

TGptComPort::~TGptComPort()
{
  Release();
}

bool TGptComPort::Init()
{
PortInit = false;
  LPVOID lpMsgBuf = NULL;
  //ShowMessage("bool TComPort::Init()"); Access Violation
   Log->Write("bool TGptComPort::Init() - init");

  try
  {
             Log->Write("bool TGptComPort::Init() - start try block");

          Port=NULL;
          Port = CreateFile(COMParams->GetPortNumberString().c_str(),
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            OPEN_EXISTING,
                            FILE_FLAG_OVERLAPPED,
                            NULL);

          /*
          Port = CreateFile("COM1",
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            OPEN_EXISTING,
                             //FILE_FLAG_OVERLAPPED,
                             FILE_ATTRIBUTE_NORMAL,
                            NULL);

            */
             Log->Write("bool TGptComPort::Init() - creating...");


          if(Port==NULL)
          {
            return( false);
          }


          Log->Write("bool TGptComPort::Init() - created");
 
          if (Port == INVALID_HANDLE_VALUE)
          {
            Port = NULL;
            FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                GetLastError(),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf,
                0,
                NULL
            );

            LocalFree(lpMsgBuf);
            Log->Write("bool TGptComPort::Init() - open state failure.");
            return false;
          }
          else Log->Write("bool TGptComPort::Init() - open state success.");

          Log->Write("bool TGptComPort::Init() - getting status");
          if ( !GetCommState(Port, &dcb) )
          {
            FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                GetLastError(),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf,
                0,
                NULL
            );
            Log->Write("bool TGptComPort::Init() - failure getting status.");
            return false;
          }
          else     Log->Write("bool TGptComPort::Init() - success getting status.");

          dcb.DCBlength = sizeof(DCB);
          dcb.fDummy2 = 0;

          DWORD BaudRate = COMParams->BaudRate;
          BYTE StopBits = COMParams->StopBits;
          BYTE Parity = COMParams->Parity;
          BYTE ByteSize = COMParams->ByteSize;
          DWORD fParity = COMParams->fParity;
          DWORD fDtrControl = COMParams->DtrControl;
          timeout = COMParams->timeout;

          dcb.BaudRate = BaudRate;
          dcb.StopBits = StopBits;
          dcb.Parity = Parity;
          dcb.ByteSize = ByteSize;
          dcb.fParity = fParity;
          dcb.fDtrControl = fDtrControl;

          Log->Write("bool TGptComPort::Init() - setting params...");

          if ( !SetCommState(Port, &dcb) )
          {

            FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                GetLastError(),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf,
                0,
                NULL
            );
            LocalFree(lpMsgBuf);
            Log->Write("bool TGptComPort::Init() - setting params failure...");
            return false;
          }
          else Log->Write("bool TGptComPort::Init() - setting params success.");



          ClearCOMPort();
          PortInit = true;
          Log->Write("bool TGptComPort::Init() - return.");
          return true;
  }
  __finally
  {
               Log->Write("bool TGptComPort::Init() - end ");
  }
  return false;
}


bool TGptComPort::Release()
{
  PortInit = false;
  if ((Port != NULL)&&(Port != INVALID_HANDLE_VALUE))
    CloseHandle(Port);
  return true;
}

bool TGptComPort::ClearCOMPort()
{


  Log->Write("bool TGptComPort::ClearCOMPort() - begin");
  if((Port == NULL)||(Port == INVALID_HANDLE_VALUE))
  {
    Log->Write("bool TGptComPort::ClearCOMPort() - port not initialize!");
    Log->Write("bool TGptComPort::ClearCOMPort() - end");
    return false;
  }
  if( PurgeComm(Port,PURGE_TXCLEAR) && PurgeComm(Port,PURGE_RXCLEAR))
  {
    Log->Write("bool TGptComPort::ClearCOMPort() - clearing Yes...");
    Log->Write("bool TGptComPort::ClearCOMPort() - end");
    return true;
  }
  else
  {
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0,
        NULL
    );
    LocalFree(lpMsgBuf);
    Log->Write("bool TGptComPort::ClearCOMPort() - clearing comporterror");
    Log->Write("bool TGptComPort::ClearCOMPort() - end");
    return false;
  }
  Log->Write("bool TGptComPort::ClearCOMPort() - emergency halt!");
}
bool TGptComPort::ReadFromPort(BYTE* data,unsigned int datesize)
{
  unsigned long ds;
  bool ret;

//  ClearCOMPort();
//Init();
 if(Port == INVALID_HANDLE_VALUE) return false;
 try
 {
  if(Port==NULL) return(false);
  ds = datesize;
  ret = ReadFile(
    Port, //HANDLE hFile	 handle of file to read
    data, // address of buffer that receives data
    ds,   // number of bytes to read
    &dWritten,//LPDWORD lpNumberOfBytesRead -  address of number of bytes read
    NULL//LPOVERLAPPED lpOverlapped 	-  address of structure for data
  );
  Sleep(200);
  ret=PurgeComm(Port, PURGE_RXABORT);
//Release();
  return ret;
  }
  __finally
  {
   return ret;
  }
}

bool TGptComPort::WriteToPort(BYTE* data,unsigned int datesize)
{
  DWORD numbytes, numbytes_ok, temp;
  COMSTAT ComState;
  OVERLAPPED Overlap;
  char buf_in[6] = "Hello!";
  numbytes = 6;

  unsigned long ds,wb;
  bool ret;
Log->Write("bool TGptComPort::WriteToPort(BYTE* data,unsigned int datesize) - begin");
Log->WriteBuffer(data, datesize);
 if(Port == INVALID_HANDLE_VALUE) return false;
 Log->Write("bool TGptComPort::WriteToPort(BYTE* data,unsigned int datesize) - 0");
 ds = datesize;
 ret= false;
 Log->Write("bool TGptComPort::WriteToPort(BYTE* data,unsigned int datesize) - try");
 try
 {
  ClearCOMPort();
  ClearCommError(Port, &temp, &ComState);
  if(temp!=0)     Log->Write("bool TGptComPort::ClearCOMPort() - ERROR com port");
  else            Log->Write("bool TGptComPort::ClearCOMPort() - com port status -ok");

  Log->Write("bool TGptComPort::WriteToPort(BYTE* data,unsigned int datesize) - writing...");
  ret = WriteFile(
    Port,	// handle to file to write to
    data,	// pointer to data to write to file
    ds,  	// number of bytes to write
    &dWritten,	// pointer to number of bytes written
    NULL 	// pointer to structure needed for overlapped I/O
   );
  Log->Write("bool TGptComPort::WriteToPort(BYTE* data,unsigned int datesize) - writing end");
  if(!ret)
  ret=PurgeComm(Port, PURGE_TXABORT);
  Log->Write("bool TGptComPort::WriteToPort(BYTE* data,unsigned int datesize) - stop");
  return ret;
}
  __finally
  {
  Log->Write("bool TGptComPort::WriteToPort(BYTE* data,unsigned int datesize) - end");
   return ret;
  }
}


