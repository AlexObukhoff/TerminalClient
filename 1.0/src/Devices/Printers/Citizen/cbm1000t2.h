#ifndef cbm1000t2H
#define cbm1000t2H

#include <vcl.h>
#include "CPrinter.h"
#include "LogClass.h";

class CBM1000Type2 : public CPrinter
{
        private:
                HANDLE hCom;
                DCB oldDCB,newDCB;
                COMMTIMEOUTS oldCTO,newCTO;
                DWORD dWritten;

                TStringList *PrintData;         // ��������� ��� ��������

                AnsiString ComNumber;           // ����� ��� �����, ��� ������� � ���-��� ������ ���� � //.//� ������ (���� ����)
                int ComSpeed;            // �������� ����� (19200, 38400...)
                int FeedLine;                   // ��������� ����� ������� ����� ���� ���� �� ������
                bool AutoBuzzer;                // ������ ��� ��� ��� ������ (def=true)
                bool AutoFeed;                  // ������ ��� ��� ������ ����� ���� (def=true)
                bool AutoCut;                   // ������ ��� ��� ����� (def=true)

                void COMOpen();                 // ������� ���
                void COMClose();                // ������� ���

                void Init();                    // ������������ ��������
                void Buzzer();                  // ������� ��������
                virtual void Feed(int count=1);                    // ����� ������
                void _Feed(int count=1);                    // ����� ������
                void Cut();                     // �����
                void Add(AnsiString);           // �������� ������ � ���� ����
                void Clear();                   // �������� ���� ����
                void Print();                   // ������ ����
                void Font(unsigned char);       // ������� ������ (init=0x20)
                void SetCodeTable();
                void MinLineFeed();
                void CharacterSet();
                void SelectPrinter();

                void GetPrinterID();
        public:
                //CBM1000Type2();                 // �����������
                CBM1000Type2(int ComPort = 1, int BaudRate = 0, TLogClass* _Log = NULL);
                virtual ~CBM1000Type2();                // ����������


                virtual bool IsPrinterEnable();
                virtual void PrintCheck(AnsiString text, std::string barcode = "");
                virtual void GetState();
                virtual std::string GetStateDescription(BYTE code);

                virtual bool IsItYou();
                virtual int Initialize();
};

#endif
