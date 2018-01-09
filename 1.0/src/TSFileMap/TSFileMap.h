//---------------------------------------------------------------------------

#ifndef TSFileMapH
#define TSFileMapH
#include <time.h>
#include "LogClass.h"

//#pragma pack(push)
//#pragma pack(1)

struct TStrFileMap
{
    TDateTime ProgramDateTime;
    bool ClosedNormally;
    bool ProgramIsIdle;
};

//#pragma pack(pop)

class TSFileMap
{
    TLogClass* Log;
    bool InnerLog;
    void ShowError(AnsiString);
    TStrFileMap *Map;
    LPVOID lpFileMap;
    HANDLE hFileMapping;
    TDateTime ReadProgramTimeMark();
    void WriteProgramTimeMark(TDateTime);
    bool ReadClosedNormally();
    void WriteClosedNormally(bool);
    bool ReadProgramIsIdle();
    void WriteProgramIsIdle(bool);
public:
    bool Connected;
    TSFileMap(TLogClass*);
    ~TSFileMap();
    void Create(AnsiString);
    void Open(AnsiString);
    void Close();
    __property TDateTime ProgramTimeMark = { read=ReadProgramTimeMark, write=WriteProgramTimeMark };
    __property bool ClosedNormally = { read=ReadClosedNormally, write=WriteClosedNormally };
    __property bool ProgramIsIdle = { read=ReadProgramIsIdle, write=WriteProgramIsIdle };
    TDateTime GetCurrentTime();
};
//---------------------------------------------------------------------------
#endif
