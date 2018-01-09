#ifndef wp_t833H
#define wp_t833H

#include <vcl.h>
#include "CPrinter.h"
#include "LogClass.h";

class wp_t833 : public CPrinter
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
                virtual void Feed(int count = 1);                    // ����� ������
                void _Feed(int count = 1);                    // ����� ������
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
                //wp_t833();                 // �����������
                wp_t833(int ComPort = 1,int BaudRate = 0, TLogClass* _Log = NULL);
                virtual ~wp_t833();                // ����������


                virtual bool IsPrinterEnable();
                virtual void PrintCheck(AnsiString text, std::string barcode = "");
                virtual void GetState();
                virtual AnsiString GetStateDescription(BYTE code);

                virtual bool IsItYou();
                virtual int Initialize();
};

#endif
