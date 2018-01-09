//---------------------------------------------------------------------------

#ifndef SwecoinTTP2010ClassH
#define SwecoinTTP2010ClassH
//---------------------------------------------------------------------------

#include "CPrinter.h"

class CSwecoinTTP2010 : public CPrinter
{
private:
    void Init();
    void Cut();
    void Feed(int count = 1);

    std::string GetStatusDescription(BYTE StatusCode);
    std::string GetStateDescription();

    void PrintString(AnsiString text);
    void Tab();

protected:
    void SendCommand();
    void SendPacket(BYTE* command, int bytes_count,int datalen, BYTE* data, bool datafirst = false);

public:
    CSwecoinTTP2010(int ComPort,int BaudRate = 0,TLogClass* _Log = NULL);
    virtual ~CSwecoinTTP2010();

    bool IsPrinterEnable();
    AnsiString GetID();
    void GetState();
    int Initialize();

    void ShriftOptionsEx(int option = 0);
    void PrintCheck(TStringList* Text);
    void PrintCheck(AnsiString text, std::string barcode = "");
    void PrintLine(AnsiString text);
    //void TestPrint();

    //new 18-06-2007
    void ClearPresenter();
    void EnforcedClearPresenter();

    bool IsItYou();
    //void LoadFont(const AnsiString File_SWF_Name);
};

 #endif