#ifndef WatchDogH
#define WatchDogH

#include <windows.h>
#include "CWatchDog.h"
#include "LptPort.h"
#include "LogClass.h"
#include "ModemSiemensMC35i.h"
#include "TWConfig.h"

#define WD_TYPE_ALNIKO 0
#define WD_TYPE_NEW_GT 1

DWORD WINAPI ThreadSendAlive();

class TWatchDog	: public CWatchDog
{
	private:
				int ModemPort;
			HANDLE		hThread;
        int 		WatchDogType;

        CModemSiemensMC35i* ModemSiemensMC35i;
        TWConfig* ConfigXML;
    public:
        //------------------------------------------
        TWatchDog(int LptPortNum, int WDType, TLogClass* Log = NULL, int ModemPort = 0);
        virtual ~TWatchDog();
        //------------------------------------------
        void StartTimer();
        void StopTimer();
        void ClearGSM();
        void ResetPC();
        bool IsItYou(){return false;}
        //------------------------------------------
        BOOL ReadLPTPort(int PinNumber);
        void WriteLPTPort(int PinNumber, BOOL PinLevel);
};
#endif
