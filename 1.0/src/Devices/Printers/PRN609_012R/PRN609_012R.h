//---------------------------------------------------------------------------

#ifndef PRN609_012RH
#define PRN609_012RH
#include "CPrinter.h"
#include "LogClass.h"
#include "ComPortParameters.h"


class PRN609_012R : public CPrinter
{

public:
  PRN609_012R(int ComPort,int BaudRate = 0,TLogClass* _Log = NULL);
  virtual ~PRN609_012R();

  virtual void Cut();  // ������ �����
  virtual void Feed(); // ����� �� ���� ������
  virtual void PrintLine(AnsiString text); // ������ ����� ������ (������ ������ 28 ��������)
  virtual void Init(); // �������������
  virtual void GetState();  // ��������� ���������   ****************(�� �������!!!)

  virtual void PrintCheck(TStringList* Text); //������ ���� (����������� �������)
  virtual void PrintCheck(AnsiString text, std::string barcode = "");   //������ ���� (����������� �������)
  virtual bool IsPrinterEnable(); //����������� �� �������
};

bool sendComandToGptDevice_GetAnswer(const BYTE* COMMAND, const int size_command, BYTE*& ANSWER , const int size_answer, const TComPortInitParameters* conf);

#endif
