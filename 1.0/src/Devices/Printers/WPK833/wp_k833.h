#ifndef wp_k833H
#define wp_k833H

#include <vcl.h>
#include "CPrinter.h"
#include "LogClass.h";

class wp_k833 : public CPrinter
{
        private:
                HANDLE hCom;
                DCB oldDCB,newDCB;
                COMMTIMEOUTS oldCTO,newCTO;
                DWORD dWritten;

                TStringList *PrintData;         // контейнер для принтера

                AnsiString ComNumber;           // Номер ком порта, для старших и усб-ком должно быть с //.//в начале (типа того)
                int ComSpeed;            // Скорость порта (19200, 38400...)
                int FeedLine;                   // Количство строк отступа после тела чека до отреза
                bool AutoBuzzer;                // Пищать или нет при печати (def=true)
                bool AutoFeed;                  // Делать или нет отступ после чека (def=true)
                bool AutoCut;                   // Делать или нет отрез (def=true)

                void COMOpen();                 // открыть ком
                void COMClose();                // закрыть ком

                void Init();                    // Иницилизация принтера
                void Buzzer();                  // Пищалка принтера
                virtual void Feed(int count = 1);                    // Скрул бумаги
                void _Feed(int count = 1);                    // Скрул бумаги
                void Cut();                     // Отрез
                void Add(AnsiString);           // Добавить строку в тело чека
                void Clear();                   // Очистить тело чека
                void Print();                   // Печать чека
                void Font(unsigned char);       // задание шрифта (init=0x20)
                void SetCodeTable();
                void MinLineFeed();
                void CharacterSet();
                void SelectPrinter();

                void GetPrinterID();
        public:
                //wp_k833();                 // конструктор
                wp_k833(int ComPort = 1,int BaudRate = 0, TLogClass* _Log = NULL);
                virtual ~wp_k833();                // деструктор


                virtual bool IsPrinterEnable();
                virtual void PrintCheck(AnsiString text, std::string barcode = "");
                virtual void GetState();
                //virtual AnsiString GetStateDescription(BYTE code);

                virtual bool IsItYou();
                virtual int Initialize();
};

#endif
