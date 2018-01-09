#ifndef DF_DescrClassH
#define DF_DescrClassH

#include <Classes.hpp>

//==============================================================================
//���� ������������
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
//����� �������� ��� ���������� � ��� ��� ��� ������ ������������� ����� ���������� � ��� ����
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
//���������� ����� ��������� ����������
class DF_device_info
{
public:
    AnsiString DeviceName;//��� ���������� ��� ������, ��� �� ��� ��� ���������� ����������
    int Type;
    int PortNumber;//����� ��� �����
    bool Determinated;//��������������� � true ���� ���������� � ������ ������ �������
    bool Read;//���� ����������� �� ��, ��� ������ ���� �������
    bool Alert;//������������ � true � ������ ������. � � false � ����� ����������� ���������� ������.
    //���� ��������� ������������ ����� �� ����� ������, ������ ������ ���������� �� ������ ����� ������ �� �����

    DF_device_info();
    virtual ~DF_device_info();
};

//==============================================================================
//����� ������� ���������� �� �����������
class DF_DescrClass
{
private:
//members
    int _SearchType;
    AnsiString _SearchDeviceName;

//properties

//functions
    void InitHardwareNames();//��������� ������ ������� ����� ���������� ������������
    int GetSearchType();
    void SetSearchType(int value);
    AnsiString GetSearchDeviceName();
    void SetSearchDeviceName(AnsiString value);

public:
//members
    TList* HardwareNames;//������ ��� ���� ��������� �� ������ ������ ���������

    DF_device_info* Validator[3];
    DF_device_info* CoinAcceptor[3];
    DF_device_info* Printer;
    DF_device_info* WatchDog;
    DF_device_info* CardReader;
    DF_device_info* Modem;
    DF_device_info* Keyboard;

//properties
    __property int SearchType = {read = GetSearchType, write = SetSearchType};//���� ht_Unknown, �� ���� ���������� ������ ����, ���� >=0 �� ���� ����������� ��� ����������
    __property AnsiString SearchDeviceName = {read = GetSearchDeviceName, write = SetSearchDeviceName};//���� �� "", �� ���� ���������� �� ������� �����

//functions
    DF_DescrClass();
    virtual ~DF_DescrClass();

    int GetHardwareType(AnsiString DeviceName);//���������� ��� ������� ���������� �� ��������� �����
    bool GetSearchEnable(AnsiString DeviceName);//����������, �������� �� ����� ��� ������� ����������
    AnsiString GetCorrectDeviceName(AnsiString Name);//������������ ����� ��������� � ������� � ������� �����
    AnsiString GetExternalDeviceName(AnsiString Name);//������������ ����� ��������� � ������� � ������� �����
};

#endif
