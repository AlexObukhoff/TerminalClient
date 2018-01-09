//---------------------------------------------------------------------------

#ifndef XMLInfoH
#define XMLInfoH

//#include "TWConfig.h"
#include <XMLDoc.hpp>
#include <common.h>
#include "LogClass.h"
#include "XMLPacket.h"

class TXMLInfo
{
    TLogClass* Log;
    bool InnerLog;
    bool Enabled;
    AnsiString FileName;
    xmlGuard <_di_IXMLDocument> dXML;
    xmlGuard <_di_IXMLNode> Root;
    _di_IXMLNode GetNode(AnsiString ObjectName, AnsiString KeyName);
    void InitFile();
    bool _FileSystemError;
    int ReadFileSystemError(){return _FileSystemError;};
    void WriteFileSystemError(int _Src) {FileSystemError=_Src;};
    TCriticalSection *CS;
    TFileStream *InfoFile;
    bool SaveFile(AnsiString _FileName, AnsiString SourceData);
public:
    //TXMLInfo(TWConfig*, TLogClass*);
    TXMLInfo(AnsiString, TLogClass*);
    ~TXMLInfo();
    double ReadDouble(AnsiString, AnsiString);
    AnsiString Read(AnsiString, AnsiString);
    TDateTime ReadDateTime(AnsiString, AnsiString);
    bool Write(AnsiString,AnsiString,AnsiString);
    void ReadNotes(AnsiString, TNotesVector&);
    void WriteNotes(AnsiString, TNotesVector&);
    void WriteNote(AnsiString, AnsiString, AnsiString);
    void WriteIncassation(AnsiString);
    TDateTime GetIncassationDT(AnsiString);
    void ReadPaymentsInfo(AnsiString, TStringList*);
    int GetPaymentsInfoCount(AnsiString);
    void ClearPaymentInfo(AnsiString ObjectName,AnsiString Info,AnsiString Sum);
    void reopen();
    void ClearPaymentsInfo(AnsiString ObjectName);
    void AddPaymentInfo(AnsiString ObjectName, double Sum, AnsiString Info);
    __property int FileSystemError = { read=ReadFileSystemError, write=WriteFileSystemError };
};

//---------------------------------------------------------------------------
#endif
