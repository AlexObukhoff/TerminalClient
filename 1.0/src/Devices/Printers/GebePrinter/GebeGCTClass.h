//---------------------------------------------------------------------------

#ifndef Citizen268ClassH
#define Citizen268ClassH
//---------------------------------------------------------------------------
#endif

#include "CPrinter.h"

class GebeGCT: public CPrinter
{
private:
  void SendCommand();
  std::string getOneMessage(BYTE);
public:
  GebeGCT(int ComPort,int BaudRate = 0,TLogClass* _Log = NULL);
  virtual ~GebeGCT();

  virtual void Cut();  // ������ �����
  virtual void Feed(); // ����� �� ���� ������
  virtual void PrintLine(AnsiString text); // ������ ����� ������ (������ ������ 28 ��������)
  virtual void Init(); // �������������
  virtual void GetState();  // ��������� ���������   ****************(�� �������!!!)

  virtual void PrintCheck(TStringList* Text); //������ ���� (����������� �������)
  virtual void PrintCheck(AnsiString text, std::string barcode = "");   //������ ���� (����������� �������)

  virtual bool IsPrinterEnable(); //����������� �� �������


protected:
  virtual void SendPacket(BYTE* command, int bytes_count,int datalen, BYTE* data, bool datafirst = false);

};


