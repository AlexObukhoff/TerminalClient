#ifndef CommandParametersH
#define CommandParametersH

#include <Classes.hpp>
#include <SyncObjs.hpp>

#define CPBufferSize 1024

class TCommandParameters
{
private:
protected:
  // ритическа€ секци€ дл€ доступа к классу полей, определ€ющему пол€ команды
  TCriticalSection* CommandParametersCriticalSection;

  void ClearBuffer(BYTE* Buffer);
  void move(BYTE* Dest, BYTE* Sour, unsigned int count);
public:
  TCommandParameters(TCriticalSection* CS);
  ~TCommandParameters();

  //буфер дл€ команды
  BYTE MainBuffer[CPBufferSize];
  //текущий размер отправл€емой команды
  int CommandSize;
  //код команды
  BYTE CommandCode;
  //код результата
  BYTE ResultCode;

  void SetParameters(BYTE* Buffer, int count, BYTE CommandCode, BYTE ResultCode = 0);
  void GetParameters(BYTE* Buffer, int& count, BYTE& CommandCode, BYTE& ResultCode);
};

#endif
