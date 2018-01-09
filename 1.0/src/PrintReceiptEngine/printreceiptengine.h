#ifndef __PrintReceiptTemplate_h
#define __PrintReceiptTemplate_h

#include <system.hpp>
#include <map.h>
#include <string.h>
#include <string>

/// ������ ��� ������ �����.
/*
�������� ����, ������� ������� �������� � XML �����, ������� ���������������
��������� ����������. ������ �������:

<?xml version="1.0" encoding="windows-1251" standalone="no"?>
<body>
		<String>-----------------------------------</String>
		<String>     ����     �����     ��������</String>
		<String>  %DATETIME%   %TERMNUMBER%</String>
		<String>����� ������ : %SESSNUM%</String>
		<String>�����        : %AMOUNTALL% %INT_CURRENCY%.</String>
		<String>��������     : %COMISSION% %INT_CURRENCY%.</String>
		<String>� ���������� : %AMOUNT% %INT_CURRENCY%.</String>
		<String>        �������, ���������� ���</String>
		<String>      ��������� ������� ���������</String>
</body>

� ������ ����� � ��� ������ ����� ������������ ������ ����� ����������:

������������� ������:
%INT_DEALER_NAME%               - ��� ������                          config.xml\dealer_info\dealer_name
%INT_DEALER_ADDRESS%            - ����� ������ (�����������)          config.xml\dealer_info\dealer_address
%INT_BUSINESS_DEALER_ADDRESS%   - �������(����������) ����� ������    config.xml\dealer_info\business_dealer_address
%INT_DEALER_INN%                - ��� ������                          config.xml\dealer_info\dealer_inn
%INT_DEALER_PHONE%              - ������� ������                      config.xml\dealer_info\dealer_phone
%INT_POINT_ADDRESS%             - ����� ����� ����� ��������         config.xml\dealer_info\point_address
%TERMNUMBER%                    - ����� ���������                     config.xml\terminal\<number write_in_cheque="1">%TERMNUMBER%</number>
%INT_CURRENCY%                  - ������ �������                      config.xml\parameters\<currency currency_name="">
%INT_CONTRACT_NUMBER%           - ����� ���������                     config.xml\dealer_info\contract_number
%INT_BANK_NAME%                 - ������������ �����                  config.xml\dealer_info\bank_name
%INT_BANK_BIK%                  - ��� �����                           config.xml\dealer_info\bank_bik
%INT_BANK_PHONE%                - ������� �����                       config.xml\dealer_info\bank_phone
%INT_BANK_ADDRESS%              - ������ �����                        config.xml\dealer_info\bank_address
%INT_BANK_INN%                  - ��� �����                           config.xml\dealer_info\bank_inn

������������� ��� ������ ���� ������:
%SESSNUM%               - ����� ������
%DATETIME%              - ������� ���� � �����
%AMOUNTALL%             - �������� �����
%COMISSION%             - ��������
%AMOUNT%                - ����� �� ���� �����������
%OPNAME%                - �������� ���������                            operators.xml\operator\name
%INT_RECIPIENT_NAME%    - ������������ �����������, ��������� �����    operators.xml\operator\name_for_cheque
%INT_RECIPIENT_INN%     - ��� �����������, ��������� �����             operators.xml\operator\inn_for_cheque
���� ����������� �������� ��������� � ���� ��� ��� - ��������.
���� ������ ��� ������������ � ����� �������.
%RAW_PARAMETER_NAME%: %RAW_PARAMETER_DATA%

��� ����� - ������� ��� ���� ������� � ����� operators.xml
operator\cheque\filename ��� �������� ���� � �����

������������� ��� ������ ���� ���������� � ������ �������:
%INCASSRECEIPTCOUNTER%  - ���������� ����� ���� ������� ������ ���������� (������ �� ����� ����������)
%INCASSPERIODFROM%      - ������ ��������� ���, �� ������� ������������ ����������
%CURRENTDATETIME%       - ����� ��������� ���, �� ������� ������������ ����������
%KASETTENUMBER%         - ����� ����������, �� �������� ������������ ����������
%BILLAMOUNT%            - ���������� �����
%TOTALSUMM%             - ����� �����
%INCASSRECEIPTNUMBER%   - ����� ����
%ACCOUNTBALANCE%        - ������ �� ����� (�� ��� ���� �����)
%<�������>BILLNOMINAL%  - ��� �������� ��� ������, ��� <�������> - �������� �������� �������� ������
%<�������>BILLAMOUNT%   - ���������� ������, ��� <�������> - �������� �������� �������� ������
%<�������>BILLSUMM%     - ����� �� �������� ������, ��� <�������> - �������� �������� �������� ������
��� ������ - incass.xml � balance.xml

��� ������ ���� �� ������
%DATETIME%              - ������� ���� � �����
%AMOUNTALL%             - �������� �����
%SESSNUM%               - ����� ������
%OPNAME%                - �������� ���������
��� ����� - ������� ��� ���� ������� � ����� operators.xml
operator\cheque\payment_error_filename ��� �������� ���� � �����

��� ������� �������� ��������� <processor type="Cyberplat_MT"> ������� �������������� ���������
%TRANS_ID% -  ����� ����������
%MT_SYSTEM% -  ������� �������� ���������
%FROM_NAME% - ����������� ��������
%TO_NAME% - ���������� ��������
%TO_BANK% - ������ ������ ��������
%CURRENT_CURRENCY% - ������� ������ ���������� ��������
%SYSTEM_COMISSION% - �������� ��������� �������� "�������� ��������"
%COMISSION% - ������� 
*/

typedef std::pair<std::string, std::string> TStandatdReceiptParametersPair;
typedef std::map<std::string, std::string> TStandatdReceiptParameters;
typedef std::pair<std::string, std::string*> TPermanetReceiptParametersPair;
typedef std::map<std::string, std::string*> TPermanetReceiptParameters;

class PrintReceiptTemplate
{
protected:
/*
m_RawParameters - ���������, ������������� �������. ������ ����� ������� ����������.
������� �� ��� ��� ��������� - �������� ���������, ���������� � �������.
������:
m_RawParameters = "�������� 1=1""�������� 2=2"
� ������� ����:
%RAW_PARAMETER_NAME% ����� %RAW_PARAMETER_DATA%
%RAW_PARAMETER_NAME% %RAW_PARAMETER_DATA%
��������� � ����
�������� 1 ����� 1
�������� 2 2
*/
    std::string m_RawParameters;
    TStandatdReceiptParameters m_ConstantReceiptParameters;
    TStandatdReceiptParameters m_StandatdReceiptParameters;
    TPermanetReceiptParameters m_PermanetReceiptParameters;
public:
    PrintReceiptTemplate();
    virtual ~PrintReceiptTemplate();
    std::string Print(const char *FileName);
    void SetStringParameter(const std::string &StringParameters, const char *Delimiter);
    void SetRawParameter(const std::string &RawParameters);
    const char *GetStandardParameter(const char *ParameterName);
    void SetParameter(const char *ParameterName, const char *ParameterValue);
    void SetParameter(const char *ParameterName, char ParameterValue);
    void SetParameter(const char *ParameterName, int ParameterValue);
    void SetParameter(const char *ParameterName, long ParameterValue);
    void SetParameter(const char *ParameterName, double ParameterValue);
    void SetParameter(const char *ParameterName, float ParameterValue);
    void SetParameter(const char *ParameterName, short ParameterValue);
};

#endif