#pragma hdrstop

#include "CommandParameters.h"

#pragma package(smart_init)

TCommandParameters::TCommandParameters(TCriticalSection* CS)
{
  CommandParametersCriticalSection = CS;
}

TCommandParameters::~TCommandParameters()
{
}

void TCommandParameters::ClearBuffer(BYTE* Buffer)
{
  memset(Buffer,0,sizeof(Buffer));
}

void TCommandParameters::move(BYTE *Dest, BYTE *Sour, unsigned int count)
{
  if ((Dest == NULL)||(Sour == NULL))
    return;
  for (unsigned int i=0; i<count; i++)
    Dest[i] = Sour[i];
  return;
}

void TCommandParameters::SetParameters(BYTE* Buffer, int count, BYTE CommandCode, BYTE ResultCode)
{
  CommandParametersCriticalSection->Acquire();
  try
  {
    move(MainBuffer,Buffer,count);
    this->CommandSize = count;
    this->CommandCode = CommandCode;
    this->ResultCode = ResultCode;
  }
  __finally
  {
    CommandParametersCriticalSection->Release();
  }
}

void TCommandParameters::GetParameters(BYTE* Buffer, int& count, BYTE& CommandCode, BYTE& ResultCode)
{
  if (CommandParametersCriticalSection == NULL) return;
  CommandParametersCriticalSection->Acquire();
  try
  {
    move(Buffer,MainBuffer,count);
    count = this->CommandSize;
    CommandCode = this->CommandCode;
    ResultCode = this->ResultCode;
  }
  __finally
  {
    CommandParametersCriticalSection->Release();
  }
}

