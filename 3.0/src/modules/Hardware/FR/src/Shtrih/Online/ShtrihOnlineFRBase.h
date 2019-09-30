/* @file Онлайн ФР семейства Штрих. */

#pragma once

// Modules
#include "Hardware/Common/TCPDeviceBase.h"
#include "Hardware/Printers/PortPrintersBase.h"

// Project
#include "../Base/ShtrihFRBase.h"
#include "ShtrihFROnlineConstants.h"

//--------------------------------------------------------------------------------
template<class T>
class ShtrihOnlineFRBase : public ShtrihFRBase<T>
{
	SET_SERIES("ShtrihOnline")

public:
	ShtrihOnlineFRBase();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

protected:
	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Получить номер смены.
	virtual int getSessionNumber();

	/// Запросить и вывести в лог критичные параметры ФР.
	virtual void processDeviceData();

	/// Установить значение налога.
	virtual bool setTaxValue(SDK::Driver::TVAT aVAT, int aGroup);

	/// Снять Z-отчет.
	virtual bool execZReport(bool aAuto);

	/// Установить TLV-параметр.
	virtual bool setTLV(int aField, bool aForSale = false);

	/// Проверить название продажи.
	virtual void checkSalesName(QString & aName);

	/// Печать фискального чека.
	virtual bool performFiscal(const QStringList & aReceipt, const SDK::Driver::SPaymentData & aPaymentData, quint32 * aFDNumber = nullptr);

	/// Получить фискальные теги по номеру документа.
	virtual bool getFiscalFields(quint32 aFDNumber, SDK::Driver::TFiscalPaymentData & aFPData, SDK::Driver::TComplexFiscalPaymentData & aPSData);

	/// Продажа.
	virtual bool sale(const SDK::Driver::SUnitData & aUnitData, SDK::Driver::EPayOffTypes::Enum aPayOffType);

	/// Закрыть чек.
	virtual bool closeDocument(double aSum, SDK::Driver::EPayTypes::Enum aPayType);

	/// Возможно ли принудительное включение буфера статусов после выполнения печатной операции.
	virtual bool canForceStatusBufferEnable();

	/// Обработка ответа на предыдущей команды. Автоисправление некоторых ошибок.
	virtual bool processAnswer(const QByteArray & aCommand, char aError);

	/// Установить флаги по ошибке в ответе.
	virtual void setErrorFlags();

	/// Открыть смену.
	virtual bool openSession();

	/// Включить автообновление прошивок.
	bool enableFirmwareUpdating();

	/// Проверить параметр автообновления прошивок.
	bool checkFirmwareUpdatingData(const CShtrihFR::FRParameters::SData & aData, int aValue, const QString & aLogData, bool & aNeedReboot);

	/// Включить/выключить режим непечати документов.
	virtual bool setNotPrintDocument(bool aEnabled, bool aZReport = false);

	/// Софтварная перезагрузка.
	bool reboot();

	/// Установка кассира.
	bool setCashier();

	/// Поддерживается команда запроса статуса принтера.
	bool mPrinterStatusEnabled;

	/// Невозможно включить автообновление прошивки.
	bool mNotEnableFirmwareUpdating;
};

typedef ShtrihOnlineFRBase<ShtrihTCPFRBase> ShtrihOnlineTCPFR;
typedef ShtrihOnlineFRBase<ShtrihSerialFRBase> ShtrihOnlineSerialFR;

//--------------------------------------------------------------------------------
