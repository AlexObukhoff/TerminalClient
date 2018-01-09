#ifndef DF_ControllerClassH
#define DF_ControllerClassH

#include <Classes.hpp>
#include <SyncObjs.hpp>
#include <time.h>

#include "LogClass.h"
#include "DF_DescrClass.h"
#include "DF_FinderClass.h"

class DF_ControllerClass : public TThread
{
private:
//members
    TLogClass* Log;//лог класс
    TList* COMPorts;//список номеров найденных СОМ портов
    TList* Searchers;//список объектов ищущих оборудование
    TList* FoundDevices;//список найденных устройств

    clock_t BeginSearching;

    bool SearchComplete;//флаг окончания процесса поиска во всех потоках
    bool ExecuteComplete;//флаг завершения функции Execute
    int MaxComPortNumber;//максимальный номер СОМ порта
    DWORD SearchingTimeOut;//предельное время поиска оборудования в милисекундах

//properties

//functions
    void SearchCOMPorts();//определяет имеющиеся в системе СОМ порты
    void ClearObjects();//очистить списки объектов для начала нового поиска
    bool GetCompleteStatus();//смотрит список всех объектов и если все закончили работу выставляет SearchComplete в true
    void StopAllChildThreads();//останавливает все дочерние потоки осуществляющие поиск оборудования
    void StartAllChildThreads();//запускает все дочерние потоки на поиск оборудования
    void GetInformation(int index);//из списка объектов поиска по индексу выбираем объект и считываем информацию о найденном оборудовании
    bool WaitForTerminated();//в деструкторе объекта будет ждать завершения потока
    void WaitForCompleteStatus();//опрашивает все потоки на предмет завершения и ждёт окончания процесса поиска в теле основного потока
    void AddFoundDevice(AnsiString name, int port, int type);//добавляет найденное устройство в список

protected:
//members

//properties

//functions
    void __fastcall Execute();//функция основного потока, в которой будут определяться порты и запускаться параллельные потоки

public:
//members
    AnsiString ComPortsString;
    DF_DescrClass* DevicesInfo;//класс для информации о найденных устройствах

//properties
    __property bool CompleteStatus = {read = GetCompleteStatus};//определяет завершён ли процесс поиска

//constructor & destructor
    __fastcall DF_ControllerClass();
    virtual __fastcall ~DF_ControllerClass();

//functions
    void StartSearching();//запуск процесса поиска
    void StopSearching();//остановка процесса поиска
    bool WaitForSearchComplete();//ждёт окончания процесса поиска для внешнего приложения
    DF_device_info* GetFoundDevice();//возвращает последний указатель на класс описания найденного устройства из списка. необходимо вызвать delete после использования для объекта описания на верхнем уровне 
    bool GetSearchEnable(AnsiString DeviceName);//определяет, возможен ли поиск для данного устройства
};

#endif
