#ifndef CommandParametersH
#define CommandParametersH

#include <Classes.hpp>
#include <SyncObjs.hpp>

#define CPBufferSize 1024

class TCommandParameters
{
private:
protected:
  //����������� ������ ��� ������� � ������ �����, ������������� ���� �������
  TCriticalSection* CommandParametersCriticalSection;

  void ClearBuffer(BYTE* Buffer);
  void move(BYTE* Dest, BYTE* Sour, unsigned int count);
public:
  TCommandParameters(TCriticalSection* CS);
  ~TCommandParameters();

  //����� ��� �������
  BYTE MainBuffer[CPBufferSize];
  //������� ������ ������������ �������
  int CommandSize;
  //��� �������
  BYTE CommandCode;
  //��� ����������
  BYTE ResultCode;

  void SetParameters(BYTE* Buffer, int count, BYTE CommandCode, BYTE ResultCode = 0);
  void GetParameters(BYTE* Buffer, int& count, BYTE& CommandCode, BYTE& ResultCode);
};

#endif
