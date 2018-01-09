#pragma hdrstop

#include "DF_DescrClass.h"

#pragma package(smart_init)

//==============================================================================
DF_device_info::DF_device_info()
{
    DeviceName = "";
    PortNumber = 0;
    Determinated = false;
    Read = false;
    Alert = false;
    Type = ht_Unknown;
}

DF_device_info::~DF_device_info()
{
}

//==============================================================================
DF_HardwareName::DF_HardwareName(AnsiString _Name, int _Type, bool _SearchEnable)
{
    Name = _Name;
    Type = _Type;
    SearchEnable = _SearchEnable;
}

DF_HardwareName::~DF_HardwareName()
{
}

//==============================================================================
DF_DescrClass::DF_DescrClass()
{
    _SearchType = ht_Unknown;
    _SearchDeviceName = "";

    for(int i=0; i<=2; i++)
    {
        Validator[i] = new DF_device_info();
        CoinAcceptor[i] = new DF_device_info();
    }

    Printer = new DF_device_info();

    WatchDog = new DF_device_info();

    CardReader = new DF_device_info();

    Modem = new DF_device_info();

    Keyboard = new DF_device_info();

    HardwareNames = new TList();
    InitHardwareNames();
}

DF_DescrClass::~DF_DescrClass()
{
    for(int i=0; i<=2; i++)
    {
        if (Validator[i])
          delete Validator[i];
        if (CoinAcceptor[i])
          delete CoinAcceptor[i];
    }

    if (Printer)
      delete Printer;

    if (WatchDog)
      delete WatchDog;

    if (CardReader)
      delete CardReader;

    if (Modem)
      delete Modem;

    if (Keyboard)
      delete Keyboard;

    //удаление всех объектов в списке имён оборудования
    for(int i=0; i<HardwareNames->Count; i++)
    {
        if (HardwareNames->Items[i] != NULL)
        {
            DF_HardwareName* name = (DF_HardwareName*)HardwareNames->Items[i];
            delete name;
        }
    }
    delete HardwareNames;
}

void DF_DescrClass::InitHardwareNames()
{
    int Type = ht_Unknown;
    HardwareNames->Clear();
    //добавляем имена сторожевых таймеров
    Type = ht_WatchDog;
    HardwareNames->Add((void*)(new DF_HardwareName("st1.2",Type)));
    HardwareNames->Add((void*)(new DF_HardwareName("Alnico",Type),false));
    HardwareNames->Add((void*)(new DF_HardwareName("FairPay",Type)));
    HardwareNames->Add((void*)(new DF_HardwareName("LDOG",Type)));
    HardwareNames->Add((void*)(new DF_HardwareName("OSMP1",Type)));
    HardwareNames->Add((void*)(new DF_HardwareName("OSMP2",Type)));
    HardwareNames->Add((void*)(new DF_HardwareName("SBK2",Type)));
    HardwareNames->Add((void*)(new DF_HardwareName("Alarm",Type)));
    HardwareNames->Add((void*)(new DF_HardwareName("Platix",Type)));
    HardwareNames->Add((void*)(new DF_HardwareName("SIM2OSMP",Type)));

    //добавляем имена валидаторов
    Type = ht_Validator;
    HardwareNames->Add((void*)(new DF_HardwareName("CCNETValidator",Type)));
    HardwareNames->Add((void*)(new DF_HardwareName("JCMValidator",Type)));
    HardwareNames->Add((void*)(new DF_HardwareName("WBA003_1Validator",Type)));
    HardwareNames->Add((void*)(new DF_HardwareName("ICTValidator",Type)));
    HardwareNames->Add((void*)(new DF_HardwareName("MEIValidator",Type)));
    HardwareNames->Add((void*)(new DF_HardwareName("NV9_CCNETValidator",Type)));
    HardwareNames->Add((void*)(new DF_HardwareName("V2EValidator",Type)));

    //добавляем имена принтеров
    Type = ht_Printer;
    HardwareNames->Add((void*)(new DF_HardwareName("citizencbm100t2",Type)));
    HardwareNames->Add((void*)(new DF_HardwareName("citizen268",Type)));
    HardwareNames->Add((void*)(new DF_HardwareName("CitizenCPP8001",Type),false));
    HardwareNames->Add((void*)(new DF_HardwareName("CitizenPPU231",Type)));
    HardwareNames->Add((void*)(new DF_HardwareName("CitizenPPU232",Type)));
    HardwareNames->Add((void*)(new DF_HardwareName("CitizenPPU700",Type)));
    HardwareNames->Add((void*)(new DF_HardwareName("custom",Type)));
    HardwareNames->Add((void*)(new DF_HardwareName("Epson442",Type)));
    HardwareNames->Add((void*)(new DF_HardwareName("GebeGCT",Type),false));
    HardwareNames->Add((void*)(new DF_HardwareName("Prim21k",Type)));
    HardwareNames->Add((void*)(new DF_HardwareName("Prim08TK",Type),false));
    HardwareNames->Add((void*)(new DF_HardwareName("PRN609_012R",Type),false));
    HardwareNames->Add((void*)(new DF_HardwareName("ShtrihFR",Type)));
    HardwareNames->Add((void*)(new DF_HardwareName("StarTSP700",Type)));
    HardwareNames->Add((void*)(new DF_HardwareName("StarTUP900",Type)));
    HardwareNames->Add((void*)(new DF_HardwareName("SwecoinTTP2010",Type)));
    HardwareNames->Add((void*)(new DF_HardwareName("wp_t833",Type),false));
    HardwareNames->Add((void*)(new DF_HardwareName("WinPrinter",Type),false));

    //добавляем имена монетоприёмников
    Type = ht_CoinAcceptor;
    HardwareNames->Add((void*)(new DF_HardwareName("NRICoinAcceptor",Type)));

    //добавляем имена кард ридеров
    Type = ht_CardReader;
    HardwareNames->Add((void*)(new DF_HardwareName("MetroCardReader",Type)));

    //добавляем имена модемов
    Type = ht_Modem;
    HardwareNames->Add((void*)(new DF_HardwareName("ModemSiemensMC35i",Type)));
    HardwareNames->Add((void*)(new DF_HardwareName("SiemensMC39MultiSIM",Type)));

    //добавляем имена клавиатур
    Type = ht_Keyboard;
    HardwareNames->Add((void*)(new DF_HardwareName("IskraKeyboard",Type)));
    HardwareNames->Add((void*)(new DF_HardwareName("KtekKeyboard",Type),false));
}

int DF_DescrClass::GetHardwareType(AnsiString DeviceName)
{
    int result = ht_Unknown;

    for(int i=0; i<HardwareNames->Count; i++)
    {
        if (HardwareNames->Items[i] != NULL)
        {
            DF_HardwareName* name = (DF_HardwareName*)HardwareNames->Items[i];
            if (name->Name.LowerCase() == DeviceName.LowerCase())
            {
                result = name->Type;
                break;
            }
        }
    }
    return result;
}

bool DF_DescrClass::GetSearchEnable(AnsiString DeviceName)
{
    int result = false;

    for(int i=0; i<HardwareNames->Count; i++)
    {
        if (HardwareNames->Items[i] != NULL)
        {
            DF_HardwareName* name = (DF_HardwareName*)HardwareNames->Items[i];
            if (name->Name.LowerCase() == DeviceName.LowerCase())
            {
                result = name->SearchEnable;
                break;
            }
        }
    }
    return result;
}

int DF_DescrClass::GetSearchType()
{
    return _SearchType;
}

void DF_DescrClass::SetSearchType(int value)
{
    if (value >= ht_Unknown)
        _SearchType = value;
    else
        _SearchType = ht_Unknown;
}

AnsiString DF_DescrClass::GetSearchDeviceName()
{
    return _SearchDeviceName;
}

void DF_DescrClass::SetSearchDeviceName(AnsiString value)
{
    value = GetCorrectDeviceName(value);
    int Type = GetHardwareType(value);
    if (Type != ht_Unknown)
        _SearchDeviceName = value;
    else
        _SearchDeviceName = "";
}

AnsiString DF_DescrClass::GetCorrectDeviceName(AnsiString Name)
{
    if (Name.LowerCase() == "")
        return Name;

    if (Name.LowerCase() == "cashcode_ccnet")
        return "CCNETValidator";

    if ((Name.LowerCase() == "cashcode_wba003-2")||(Name.LowerCase() == "cashcode_wba003-1"))
        return "WBA003_1Validator";

    if ((Name.LowerCase() == "ict_ict004")||(Name.LowerCase() == "matrix_ict004"))
        return "ICTValidator";

    if (Name.LowerCase() == "jcm_id003")
        return "JCMValidator";

    if (Name.LowerCase() == "mei_ebds")
        return "MEIValidator";

    if (Name.LowerCase() == "nv9_ccnet")
        return "NV9_CCNETValidator";

    if (Name.LowerCase() == "gpt_v2e")
        return "V2EValidator";

    if (Name.LowerCase() == "nri_cctalk")
        return "NRICoinAcceptor";

    if (Name.LowerCase() == "citizencbm1000t2")
        return "citizencbm100t2";

    if (Name.LowerCase() == "av-268")
        return "citizen268";

    if (Name.LowerCase() == "citizen_cpp_8001")
        return "CitizenCPP8001";

    if (Name.LowerCase() == "citizen_ppu_231")
        return "CitizenPPU231";

    if (Name.LowerCase() == "citizen_ppu_232")
        return "CitizenPPU232";

    if (Name.LowerCase() == "citizen_ppu_700")
        return "CitizenPPU700";

    if (Name.LowerCase() == "custom_vkp_80")
        return "custom";

    if (Name.LowerCase() == "epson_442")
        return "Epson442";

    if (Name.LowerCase() == "gebegct")
        return "GebeGCT";

    if ((Name.LowerCase() == "prim08tk-frk-buffer")||
        (Name.LowerCase() == "prim08tk-frk")||
        (Name.LowerCase() == "prim08tk-kiosk"))
        return "Prim08TK";

    if ((Name.LowerCase() == "prim21k-frk-buffer")||
        (Name.LowerCase() == "prim21k-frk")||
        (Name.LowerCase() == "prim21k-kiosk"))
        return "Prim21k";

    if (Name.LowerCase() == "prn609_012r")
        return "PRN609_012R";

    if ((Name.LowerCase() == "shtrih-frk")||(Name.LowerCase() == "shtrih-kiosk")
        ||(Name.LowerCase() == "shtrih-frk-buffer"))
        return "ShtrihFR";

    if (Name.LowerCase() == "star_tsp_700")
        return "StarTSP700";

    if (Name.LowerCase() == "startup900")
        return "StarTUP900";

    if (Name.LowerCase() == "swecoin-ttp2010")
        return "SwecoinTTP2010";

    if (Name.LowerCase() == "wp_t833")
        return "wp_t833";

    if (Name.LowerCase() == "windows")
        return "WinPrinter";

    if (Name.LowerCase() == "st1.2")
        return "st1.2";

    if (Name.LowerCase() == "alnico")
        return "Alnico";

    if (Name.LowerCase() == "fairpay")
        return "FairPay";

    if (Name.LowerCase() == "ldog")
        return "LDOG";

    if (Name.LowerCase() == "osmp")
        return "OSMP1";

    if (Name.LowerCase() == "osmp2")
        return "OSMP2";

    if (Name.LowerCase() == "sbk2")
        return "SBK2";

    if (Name.LowerCase() == "alarm")
        return "Alarm";

    if (Name.LowerCase() == "platix")
        return "Platix";

    if (Name.LowerCase() == "sim2osmp")
        return "SIM2OSMP";

    if (Name.LowerCase() == "metro")
        return "MetroCardReader";

    if (Name.LowerCase() == "")
        return "ModemSiemensMC35i";

    if (Name.LowerCase() == "")
        return "SiemensMC39MultiSIM";

    if (Name.LowerCase() == "iskra")
        return "IskraKeyboard";

    if (Name.LowerCase() == "ktek")
        return "KtekKeyboard";

    return Name;
}

AnsiString DF_DescrClass::GetExternalDeviceName(AnsiString Name)
{
    if (Name.LowerCase() == "")
        return Name;

    if (Name.LowerCase() == "ccnetvalidator")
        return "cashcode_ccnet";

    if (Name.LowerCase() == "wba003_1validator")
        return "cashcode_wba003-1";

    if (Name.LowerCase() == "ictvalidator")
        return "ict_ict004";

    if (Name.LowerCase() == "JCMValidator")
        return "jcm_id003";

    if (Name.LowerCase() == "meivalidator")
        return "mei_ebds";

    if (Name.LowerCase() == "nv9_ccnetvalidator")
        return "nv9_ccnet";

    if (Name.LowerCase() == "v2evalidator")
        return "gpt_v2e";

    if (Name.LowerCase() == "nricoinacceptor")
        return "nri_cctalk";

    if (Name.LowerCase() == "citizencbm100t2")
        return "citizencbm1000t2";

    if (Name.LowerCase() == "citizen268")
        return "av-268";

    if (Name.LowerCase() == "citizencpp8001")
        return "citizen_cpp_8001";

    if (Name.LowerCase() == "citizenppu231")
        return "citizen_ppu_231";

    if (Name.LowerCase() == "citizenppu232")
        return "citizen_ppu_232";

    if (Name.LowerCase() == "citizenppu700")
        return "citizen_ppu_700";

    if (Name.LowerCase() == "custom")
        return "custom_vkp_80";

    if (Name.LowerCase() == "epson442")
        return "epson_442";

    if (Name.LowerCase() == "gebegct")
        return "gebegct";

    if (Name.LowerCase() == "prim08tk")
        return "prim08tk-frk";

    if (Name.LowerCase() == "prim21k")
        return "prim21k-frk";

    if (Name.LowerCase() == "prn609_012r")
        return "prn609_012r";

    if (Name.LowerCase() == "shtrihfr")
        return "shtrih-frk";

    if (Name.LowerCase() == "startsp700")
        return "star_tsp_700";

    if (Name.LowerCase() == "startup900")
        return "startup900";

    if (Name.LowerCase() == "swecointtp2010")
        return "swecoin-ttp2010";

    if (Name.LowerCase() == "wp_t833")
        return "wp_t833";

    if (Name.LowerCase() == "winprinter")
        return "windows";

    if (Name.LowerCase() == "st1.2")
        return "st1.2";

    if (Name.LowerCase() == "alnico")
        return "alnico";

    if (Name.LowerCase() == "fairpay")
        return "fairpay";

    if (Name.LowerCase() == "ldog")
        return "ldog";

    if (Name.LowerCase() == "osmp1")
        return "osmp";

    if (Name.LowerCase() == "osmp2")
        return "OSMP2";

    if (Name.LowerCase() == "sbk2")
        return "sbk2";

    if (Name.LowerCase() == "alarm")
        return "alarm";

    if (Name.LowerCase() == "platix")
        return "platix";

    if (Name.LowerCase() == "sim2osmp")
        return "sim2osmp";

    if (Name.LowerCase() == "metrocardreader")
        return "metro";

    if (Name.LowerCase() == "modemsiemensmc35i")
        return "mc35i";

    if (Name.LowerCase() == "siemensmc39multisim")
        return "mc39";

    if (Name.LowerCase() == "iskrakeyboard")
        return "iskra";

    if (Name.LowerCase() == "ktekkeyboard")
        return "ktek";

    return Name;
}

