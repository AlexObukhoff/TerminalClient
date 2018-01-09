#ifndef DF_FinderClassH
#define DF_FinderClassH

#include <Classes.hpp>
#include <SyncObjs.hpp>
#include <time.h>

#include "DF_DescrClass.h"
#include "LogClass.h"

//================================base==========================================
#include "CValidator.h"
#include "CCoinAcceptor.h"
#include "CPrinter.h"
#include "CWatchDog.h"
#include "CardReader.h"
#include "Modem.h"
#include "CKeyboard.h"
//===============================validators=====================================
#include "CCNETdevice.h"
#include "JCMdevice.h"
#include "ID003_2Device.h"
#include "ID003_1Device.h"
#include "ICTDevice.h"
#include "V2EDevice.h"
#include "MEIDevice.h"
#include "NV9_CCNETDevice.h"
//================================coinacceptors=================================
#include "NRIDevice.h"
//================================printers======================================
#include "Citizen268Class.h"
#include "ShtrihPrinter.h"
#include "cbm1000t2.h"
#include "CitizenPPU700Class.h"
#include "CitizenPPU231Class.h"
#include "CitizenPPU232Class.h"
#include "CustomPrnClass.h"
#include "Epson442Class.h"
#include "SwecoinTTP2010Class.h"
#include "StarTUP900Class.h"
#include "StarTSP700Class.h"
#include "ShtrihPrinter.h"
#include "WinPrnClass.h"
#include "wp_t833.h"
#include "Prim21KClass.h"
#include "Prim08TKClass.h"
#include "CitizenCPP8001Class.h"
#include "PRN609_012R.h"
//=================================watchdogs====================================
#include "WatchDogdevice.h"
#include "WatchDogOSMPdevice.h"
#include "WatchDogOSMP2device.h"
#include "WatchDogAlarmDevice.h"
#include "WatchDogPlatixDevice.h"
#include "LDOGDevice.h"
#include "FairPayWDDevice.h"
#include "SBK2Device.h"
#include "SIM2OSMPDevice.h"
//================================modems========================================
#include "ModemSiemensMC35i.h"
#include "ModemSiemensMC39MultiSIM.h"
//================================keyboards=====================================
#include "IskraKeybDevice.h"
#include "KtekKeybDevice.h"
//=================================cardreaders==================================
#include "MetroCardDevice.h"
//==============================================================================

class DF_FinderClass : public TThread
{
private:
//members
    TLogClass* Log;//лог класс
    bool SearchComplete;//флаг окончани€ процесса поиска оборудовани€
    bool ExecuteComplete;//флаг завершени€ функции Execute
    AnsiString CurrentDeviceName;
    AnsiString SubFolderName;//им€ каталога дл€ сохранени€ лог файлов

    clock_t BeginSearching;//врем€ начала поиска
    DWORD SearchingTimeOut;//врем€ отведЄнное на поиск оборудовани€ в данном потоке
    DWORD TerminateWaitTimeOut;//предельное врем€ ожидани€ завершени€ работы потока
//properties

//functions
    bool FindValidator(AnsiString Name);
    bool FindCoinAcceptor(AnsiString Name);
    bool FindPrinter(AnsiString Name);
    bool FindWatchDog(AnsiString Name);
    bool FindCardReader(AnsiString Name);
    bool FindModem(AnsiString Name);
    bool FindKeyboard(AnsiString Name);

    AnsiString GetSearchDeviceName();//если нужен поиск конкретного устройства, здесь просмотрев класс информации возвращаем его им€
    bool WaitForTerminated();//в деструкторе объекта будет ждать завершени€ потока
    bool FindDeviceByName(AnsiString DeviceName = "");//ищет на данном порту устройство с данным именем
    bool FindDevicesByType(int Type = -1);//ищет последовательно все устройства данного типа на данном порту
    bool FindDevicesByAllNames();//ищет последовательно все устройства на данном порту

protected:
//members

//properties

//functions
    void __fastcall Execute();//основной поток в котором будут определ€тьс€ порты и запускатьс€ параллельные потоки

public:
//members
    DF_DescrClass* DeviceInfo;//сласс дл€ информации о найденных устройствах
    int PortNumber;

//properties
    __property int CompleteStatus = {read = GetCompleteStatus};//определ€ет завершЄн ли процесс поиска

//functions
    __fastcall DF_FinderClass(int ComPort);
    virtual __fastcall ~DF_FinderClass();

    void StartSearching();//запуск процесса поиска
    void StopSearching();//остановка процесса поиска
    bool WaitForCompleteStatus();//ждЄт окончани€ процесса поиска, выходит при SearchComplete = true
    bool GetCompleteStatus();//смотрит список всех объектов и если все закончили работу выставл€ет SearchComplete в true
};


#endif
