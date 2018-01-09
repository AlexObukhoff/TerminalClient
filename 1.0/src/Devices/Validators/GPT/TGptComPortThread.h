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
  unsigned int GlobalPay;    //Вся сумма введенная наличкой
  unsigned int GlobalCount;  //введенное число купюр
  char cval[4]; //код валюты
  // первый байт статуса
  bool  BILL_STACKED_FLAG,    //     купюра помещена в приемный лоток
        BILL_RETURNED_FLAG,   //     Возврат купюры, готовой к приему
        BILL_IN_ESCROW_FLAG;  //     сост. усл. приема
  // второй байт статуса
  bool  CHEATED_FLAG,                 //   Фальшивка
        POWER_ON_RESET_FLAG,          //   Прошла перезагрузка устройства
        INVALID_ESCROW_REQUEST_FLAG,  //   Выдернули купюру
        REJECTING_FLAG,               //   Процесс возврата купюры
        STACKING_FLAG,                //   Процес помещения купюры в стопку
        RETURNING_FLAG,               //   Состояние возврата
        ACCEPTING_FLAG,               //   состояние приема
        IDLE_FLAG;                    //   режим ожидания приема
   //третий байт статуса
   bool INHIBITED_FLAG,                  // флаг замятия
         VALIDATOR_DISABLED,             // неработоспособное состояние
         COMMUNICATION_TIME_OUT_FLAG,    // не полуен опрос состояния
         POWER_UP_WITH_BILL_IN_CHANNEL,  // был включен с купюрой в приемнике
         STACKER_FULL_FLAG,              // купюрник переполнен
         STACKER_OFF_OPEN_FLAG,          //контейнер снят
         STACKER_JAM_FLAG,               // заклинивание укладчика
         BILL_JAM;                       // замятие с возвратом
  // четвертый байт статуса - номинал купюры
  int Nominal;                   //номинал купюры -1 если неопределено
  bool ACCEPTED_FLAG;            // купюра принята
}TGptState;


class TGptComPortThread : public TThread
{
private:

protected:
        void __fastcall Execute();
        bool cicle_command; //если команда циклическая, то true иначе false
        TCriticalSection* CS;
public:
       TGptComPort* gptcomport;
       TComPortInitParameters* param;
          TLogClass* Log;
       TGptState* state;
       TLabel* Lab1;
       TLabel* Lab2;
       AnsiString AnsStr, Cnt;
       BYTE COMMAND[256]; // циклическая команда
       unsigned int     sizeof_command; // размер циклической команды
       BYTE ONE_COM[256]; // команда, которая выполняется единожды
       unsigned int     sizeof_one_com; // размер циклической команды
       BYTE  ANSWER[256]; // ответ усройства
       unsigned int sizeof_answer;
       bool executeOne(); // выполнить 1 раз команду ONE_COM

        __fastcall TGptComPortThread(bool CreateSuspended);
       virtual void __fastcall pb();// тело цикла

       //переписываются по усмотрению
       virtual bool __fastcall init_comport(const  TComPortInitParameters*);//инициализация порта
       virtual bool __fastcall init_device();//инициализация устройства

};

int GetNominal(AnsiString vname, int item); // номиналы валют
TGptState GptByteToStructStatus(const BYTE* RAW_DATA,const  int zize_raw_data,const unsigned int gk,const unsigned int gp); // расшифровка состояния
#endif
