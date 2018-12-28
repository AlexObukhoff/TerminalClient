/* @file Движок фискальных тегов. */

#pragma once

// SDK
#include <SDK/Drivers/FR/FiscalDataTypes.h>

// Modules
#include "Hardware/Common/DeviceLogicManager.h"

// Project
#include "Hardware/FR/FiscalFieldDescriptions.h"
#include "Hardware/FR/FRBaseConstants.h"

/// Системы налогообложения (СНО).
typedef QList<char> TTaxSystems;

/// Признаки агента.
typedef QList<char> TAgentFlags;

/// Режимы работы.
typedef QList<char> TOperationModes;

//--------------------------------------------------------------------------------
class FFEngine: public DeviceConfigManager, public DeviceLogManager
{
public:
	FFEngine(ILog * aLog);

	/// Установка параметра.
	virtual void setConfigParameter(const QString & aName, const QVariant & aValue);

	/// Распарсить TLV-структуру.
	bool parseTLV(const QByteArray & aData, CFR::STLV & aTLV);

	/// Проверить данные TLV-структуры.
	bool checkTLVData(CFR::STLV & aTLV);

	/// Распарсить массив байтов STLV-структуры.
	CFR::TTLVList parseSTLV(const QByteArray & aData);

	/// Распарсить значение TLV-структуры.
	void parseTLVData(const CFR::STLV & aTLV, SDK::Driver::TFiscalPaymentData & aFPData);

	/// Распарсить значение STLV-структуры.
	void parseSTLVData(const CFR::STLV & aTLV, SDK::Driver::TComplexFiscalPaymentData & aPSData);

	/// Получить массив байтов TLV-структуры для установки тега.
	QByteArray getTLVData(int aField, const QVariant & aValue, QString * aLog = nullptr);
	QByteArray getTLVData(int aField, QString * aLog = nullptr);

	/// Получить массив байтов TLV-структуры для установки тега VLN-типа.
	QByteArray getDigitTLVData(qulonglong aValue);

	/// Установить данные платежа.
	void setFPData(SDK::Driver::TFiscalPaymentData & aFPData, int aField, const QVariant & aValue = QVariant());

	/// Проверить данные платежа.
	void checkFPData(SDK::Driver::TFiscalPaymentData & aFPData, int aField, const QVariant & aValue);
	void checkFPData(SDK::Driver::TFiscalPaymentData & aFPData, int aField);

	/// Проверить данные платежа, которые имеюттолько признак наличия названия поля.
	void checkSimpleFPData(SDK::Driver::TFiscalPaymentData & aFPData, int aField);

	/// Получить лог фискальных данных платежа.
	QString getFPDataLog(const SDK::Driver::TFiscalPaymentData & aFPData) const;

	/// Загрузить СНО.
	bool checkTaxSystems(char aData, TTaxSystems & aTaxSystems);

	/// Загрузить флаги агента.
	bool checkAgentFlags(char aData, TAgentFlags & aAgentFlags);

	/// Проверить режимы работы.
	bool checkOperationModes(char aData, TOperationModes & aOperationModes);

	/// Проверить возможность использования фискального реквизита.
	bool checkFiscalField(int aField, bool & aResult);

	/// Проверить корректность СНО дилера.
	bool checkDealerTaxSystem(ERequestStatus::Enum aInitialized, bool aCanLog);

	/// Проверить корректность флага агента дилера.
	bool checkDealerAgentFlag(ERequestStatus::Enum aInitialized, bool aCanLog);

	/// Проверить кассира.
	bool checkCashier(QString & aCashier);

	/// Проверить корректность СНО на платеже.
	bool checkTaxSystemOnPayment(SDK::Driver::SPaymentData & aPaymentData);

	/// Проверить корректность флага агента на платеже.
	bool checkAgentFlagOnPayment(SDK::Driver::SPaymentData & aPaymentData);

	/// Добавить/удалить/скорректировать фискальные теги, полученные после платежа.
	void filterAfterPayment(SDK::Driver::TFiscalPaymentData & aFPData, SDK::Driver::TComplexFiscalPaymentData & aPSData);

	/// Проверить ИНН.
	bool checkINN(const QString & aINN, int aType = CFR::INN::Person::Unknown) const;

	/// Привести строку с данными телефона к виду +{Ц}.
	QString filterPhone(const QString & aData) const;

	/// Добавить данные.
	void addData(const CFR::FiscalFields::TData & aData);

	/// Установить имя устройства.
	void setDeviceName(const QString & aDeviceName);

	/// Установить кодек.
	void setCodec(QTextCodec * aCodec);

protected:
	/// Данные фискальных реквизитов.
	CFR::FiscalFields::Data mFFData;

	/// Название устройства.
	QString mDeviceName;

	/// Драйвера запускаются из под модуля платежей.
	bool mOperatorPresence;

	/// Системы налогообложения (СНО).
	TTaxSystems mTaxSystems;

	/// Флаги агента.
	TAgentFlags mAgentFlags;

	/// Режимы работы.
	TOperationModes mOperationModes;
};

//--------------------------------------------------------------------------------
