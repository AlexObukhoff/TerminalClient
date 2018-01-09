//---------------------------------------------------------------------------

#ifndef TGptComPortThreadH
#define TGptComPortThreadH
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "LogClass.h"
#include "GptComPortClass.h"
#include "ComPortParameters.h"
//---------------------------------------------------------------------------
typedef struct TGptState
{
  bool cb;
  unsigned int GlobalPay;    //��� ����� ��������� ��������
  unsigned int GlobalCount;  //��������� ����� �����
  char cval[4]; //��� ������
  // ������ ���� �������
  bool  BILL_STACKED_FLAG,    //     ������ �������� � �������� �����
        BILL_RETURNED_FLAG,   //     ������� ������, ������� � ������
        BILL_IN_ESCROW_FLAG;  //     ����. ���. ������
  // ������ ���� �������
  bool  CHEATED_FLAG,                 //   ���������
        POWER_ON_RESET_FLAG,          //   ������ ������������ ����������
        INVALID_ESCROW_REQUEST_FLAG,  //   ��������� ������
        REJECTING_FLAG,               //   ������� �������� ������
        STACKING_FLAG,                //   ������ ��������� ������ � ������
        RETURNING_FLAG,               //   ��������� ��������
        ACCEPTING_FLAG,               //   ��������� ������
        IDLE_FLAG;                    //   ����� �������� ������
   //������ ���� �������
   bool INHIBITED_FLAG,                  // ���� �������
         VALIDATOR_DISABLED,             // ����������������� ���������
         COMMUNICATION_TIME_OUT_FLAG,    // �� ������ ����� ���������
         POWER_UP_WITH_BILL_IN_CHANNEL,  // ��� ������� � ������� � ���������
         STACKER_FULL_FLAG,              // �������� ����������
         STACKER_OFF_OPEN_FLAG,          //��������� ����
         STACKER_JAM_FLAG,               // ������������ ���������
         BILL_JAM;                       // ������� � ���������
  // ��������� ���� ������� - ������� ������
  int Nominal;                   //������� ������ -1 ���� ������������
  bool ACCEPTED_FLAG;            // ������ �������
}TGptState;


class TGptComPortThread : public TThread
{
private:

protected:
        void __fastcall Execute();
        bool cicle_command; //���� ������� �����������, �� true ����� false
        TCriticalSection* CS;
public:
       TGptComPort* gptcomport;
       TComPortInitParameters* param;
          TLogClass* Log;
       TGptState* state;
       TLabel* Lab1;
       TLabel* Lab2;
       AnsiString AnsStr, Cnt;
       BYTE COMMAND[256]; // ����������� �������
       unsigned int     sizeof_command; // ������ ����������� �������
       BYTE ONE_COM[256]; // �������, ������� ����������� ��������
       unsigned int     sizeof_one_com; // ������ ����������� �������
       BYTE  ANSWER[256]; // ����� ���������
       unsigned int sizeof_answer;
       bool executeOne(); // ��������� 1 ��� ������� ONE_COM

        __fastcall TGptComPortThread(bool CreateSuspended);
       virtual void __fastcall pb();// ���� �����

       //�������������� �� ����������
       virtual bool __fastcall init_comport(const  TComPortInitParameters*);//������������� �����
       virtual bool __fastcall init_device();//������������� ����������

};

int GetNominal(AnsiString vname, int item); // �������� �����
TGptState GptByteToStructStatus(const BYTE* RAW_DATA,const  int zize_raw_data,const unsigned int gk,const unsigned int gp); // ����������� ���������
#endif
