#ifndef DF_DescrClassH
#define DF_DescrClassH

#include <Classes.hpp>

//==============================================================================
//типы оборудования
enum DF_HardwareType
{
    ht_Unknown      = -1,
    ht_Validator    = 0,
    ht_CoinAcceptor = 1,
    ht_Printer      = 2,
    ht_WatchDog     = 3,
    ht_CardReader   = 4,
    ht_Modem        = 5,
    ht_Keyboard     = 6
};

//==============================================================================
//класс хранящий имя устройства и его тип для списка сопоставления имени устройства и его типа
class DF_HardwareName
{
public:
    AnsiString Name;
    int Type;
    bool SearchEnable;

    DF_HardwareName(AnsiString _Name, int _Type, bool _SearchEnable = true);
    virtual ~DF_HardwareName();
};

//==============================================================================
//простейший класс описатель устройства
class DF_device_info
{
public:
    AnsiString DeviceName;//имя устройства для поиска, или же имя уже найденного устройства
    int Type;
    int PortNumber;//номер СОМ порта
    bool Determinated;//устанавливается в true если устройство с данным именем найдено
    bool Read;//флаг указывающий на то, что данные были считаны
    bool Alert;//выставляется в true в начале поиска. и в false в конце корректного завершения поиска.
    //если произошла перезагрузка компа во время поиска, больше данное устройство на данном порту искать не стоит

    DF_device_info();
    virtual ~DF_device_info();
};

//==============================================================================
//Класс несущий информацию об устройствах
class DF_DescrClass
{
private:
//members
    int _SearchType;
    AnsiString _SearchDeviceName;

//properties

//functions
    void InitHardwareNames();//заполняет список именами всего доступного оборудования
    int GetSearchType();
    void SetSearchType(int value);
    AnsiString GetSearchDeviceName();
    void SetSearchDeviceName(AnsiString value);

public:
//members
    TList* HardwareNames;//список имён всех доступных на данный момент устройств

    DF_device_info* Validator[3];
    DF_device_info* CoinAcceptor[3];
    DF_device_info* Printer;
    DF_device_info* WatchDog;
    DF_device_info* CardReader;
    DF_device_info* Modem;
    DF_device_info* Keyboard;

//properties
    __property int SearchType = {read = GetSearchType, write = SetSearchType};//если ht_Unknown, то ищем устройство любого типа, если >=0 то ищем определённый тип устройства
    __property AnsiString SearchDeviceName = {read = GetSearchDeviceName, write = SetSearchDeviceName};//если не "", то ищем устройство по данному имени

//functions
    DF_DescrClass();
    virtual ~DF_DescrClass();

    int GetHardwareType(AnsiString DeviceName);//определяет тип данного устройства по заданному имени
    bool GetSearchEnable(AnsiString DeviceName);//определяет, возможен ли поиск для данного устройства
    AnsiString GetCorrectDeviceName(AnsiString Name);//сопоставляет имена устройств в классах и внешние имена
    AnsiString GetExternalDeviceName(AnsiString Name);//сопоставляет имена устройств в классах и внешние имена
};

#endif
