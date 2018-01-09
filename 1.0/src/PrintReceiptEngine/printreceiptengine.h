#ifndef __PrintReceiptTemplate_h
#define __PrintReceiptTemplate_h

#include <system.hpp>
#include <map.h>
#include <string.h>
#include <string>

/// Движок для печати чеков.
/*
Печатает чеки, шаблоны которях хранятся в XML файле, заменяя псевдопараметры
реальными значениями. Пример шаблона:

<?xml version="1.0" encoding="windows-1251" standalone="no"?>
<body>
		<String>-----------------------------------</String>
		<String>     Дата     Время     Терминал</String>
		<String>  %DATETIME%   %TERMNUMBER%</String>
		<String>Номер сессии : %SESSNUM%</String>
		<String>Сумма        : %AMOUNTALL% %INT_CURRENCY%.</String>
		<String>Комиссия     : %COMISSION% %INT_CURRENCY%.</String>
		<String>К зачислению : %AMOUNT% %INT_CURRENCY%.</String>
		<String>        СПАСИБО, СОХРАНЯЙТЕ ЧЕК</String>
		<String>      ПЛАТЕЖНАЯ СИСТЕМА КИБЕРПЛАТ</String>
</body>

В разное время и для разных чеков действителен разный набор параметров:

Действительны всегда:
%INT_DEALER_NAME%               - Имя дилера                          config.xml\dealer_info\dealer_name
%INT_DEALER_ADDRESS%            - Адрес дилера (Юридический)          config.xml\dealer_info\dealer_address
%INT_BUSINESS_DEALER_ADDRESS%   - Деловой(физический) адрес дилера    config.xml\dealer_info\business_dealer_address
%INT_DEALER_INN%                - ИНН дилера                          config.xml\dealer_info\dealer_inn
%INT_DEALER_PHONE%              - Телефон дилера                      config.xml\dealer_info\dealer_phone
%INT_POINT_ADDRESS%             - Адрес точки приёма платежей         config.xml\dealer_info\point_address
%TERMNUMBER%                    - Номер терминала                     config.xml\terminal\<number write_in_cheque="1">%TERMNUMBER%</number>
%INT_CURRENCY%                  - Валюта платежа                      config.xml\parameters\<currency currency_name="">
%INT_CONTRACT_NUMBER%           - Номер контракта                     config.xml\dealer_info\contract_number
%INT_BANK_NAME%                 - Наименование банка                  config.xml\dealer_info\bank_name
%INT_BANK_BIK%                  - БИК банка                           config.xml\dealer_info\bank_bik
%INT_BANK_PHONE%                - Телефон банка                       config.xml\dealer_info\bank_phone
%INT_BANK_ADDRESS%              - Адресс банка                        config.xml\dealer_info\bank_address
%INT_BANK_INN%                  - ИНН банка                           config.xml\dealer_info\bank_inn

Действительны при выдаче чека оплаты:
%SESSNUM%               - Номер сессии
%DATETIME%              - Текущие дата и время
%AMOUNTALL%             - Внесённая сумма
%COMISSION%             - Комиссия
%AMOUNT%                - Сумма на счёт плательщика
%OPNAME%                - Название оператора                            operators.xml\operator\name
%INT_RECIPIENT_NAME%    - Наименование организации, принявшей платёж    operators.xml\operator\name_for_cheque
%INT_RECIPIENT_INN%     - ИНН организации, принявшей платёж             operators.xml\operator\inn_for_cheque
Есть возможность задавать параметры в виде пар имя - значение.
Пока только для наименования и суммы платежа.
%RAW_PARAMETER_NAME%: %RAW_PARAMETER_DATA%

Имя файла - шаблона для чека задаётся в файле operators.xml
operator\cheque\filename без указания пути к файлу

Действительны при выдаче чека инкассации и печати баланса:
%INCASSRECEIPTCOUNTER%  - Порядковый номер чека текущей сессии инкассации (только во время инкассации)
%INCASSPERIODFROM%      - Начало диапазона дат, за который производится инкассация
%CURRENTDATETIME%       - Конец диапазона дат, за который производится инкассация
%KASETTENUMBER%         - Номер устройства, по которому производится инкассация
%BILLAMOUNT%            - Количество купюр
%TOTALSUMM%             - Общая сумма
%INCASSRECEIPTNUMBER%   - Номер чека
%ACCOUNTBALANCE%        - Баланс на счёте (не для всех чеков)
%<номинал>BILLNOMINAL%  - Имя номинала для купюры, где <номинал> - числовое значение номинала купюры
%<номинал>BILLAMOUNT%   - Количество купюры, где <номинал> - числовое значение номинала купюры
%<номинал>BILLSUMM%     - Сумма по значению купюры, где <номинал> - числовое значение номинала купюры
Имя файлов - incass.xml и balance.xml

При печати чека об ошибке
%DATETIME%              - Текущие дата и время
%AMOUNTALL%             - Внесённая сумма
%SESSNUM%               - Номер сессии
%OPNAME%                - Название оператора
Имя файла - шаблона для чека задаётся в файле operators.xml
operator\cheque\payment_error_filename без указания пути к файлу

Для системы денежных переводов <processor type="Cyberplat_MT"> введены дополнительные параметры
%TRANS_ID% -  номер транзакции
%MT_SYSTEM% -  система денежных переводов
%FROM_NAME% - Отправитель перевода
%TO_NAME% - Получатель перевода
%TO_BANK% - Адресс выдачи перевода
%CURRENT_CURRENCY% - Текущая валюта получателя перевода
%SYSTEM_COMISSION% - Комиссия взимаемая системой "Денежные переводы"
%COMISSION% - Остаток 
*/

typedef std::pair<std::string, std::string> TStandatdReceiptParametersPair;
typedef std::map<std::string, std::string> TStandatdReceiptParameters;
typedef std::pair<std::string, std::string*> TPermanetReceiptParametersPair;
typedef std::map<std::string, std::string*> TPermanetReceiptParameters;

class PrintReceiptTemplate
{
protected:
/*
m_RawParameters - параметры, подставляемые потоком. Строка ввода заранее неизвестна.
Состоит из пар имя параметра - значение параметра, обрамлённых в кавычки.
Пример:
m_RawParameters = "Параметр 1=1""Параметр 2=2"
В шаблоне чека:
%RAW_PARAMETER_NAME% равно %RAW_PARAMETER_DATA%
%RAW_PARAMETER_NAME% %RAW_PARAMETER_DATA%
Результат в чеке
Параметр 1 равно 1
Параметр 2 2
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