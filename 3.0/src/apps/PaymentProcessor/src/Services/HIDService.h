/* @file Cервис для работы с HID-устройствами. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QList>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/ILogable.h>

// SDK
#include <SDK/PaymentProcessor/Core/IHIDService.h>
#include <SDK/PaymentProcessor/Core/IDeviceService.h>
#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/Drivers/IHID.h>
#include <SDK/Drivers/ICardReader.h>

class IApplication;

//---------------------------------------------------------------------------
class HIDService : public SDK::PaymentProcessor::IHIDService, public SDK::PaymentProcessor::IService, private ILogable
{
	Q_OBJECT

public:
	/// Получение экземпляра FundsService.
	static HIDService * instance(IApplication * aApplication);

	HIDService(IApplication * aApplication);
	~HIDService();

	/// Инициализация сервиса.
	virtual bool initialize();

	/// IService: Закончена инициализация всех сервисов.
	virtual void finishInitialize();

	/// Возвращает false, если сервис не может быть остановлен в текущий момент.
	virtual bool canShutdown();

	/// Завершение работы сервиса. Возвращает false, если сервис не может быть остановлен в текущий момент.
	virtual bool shutdown();

	/// Получить имя сервиса.
	virtual QString getName() const;

	/// Получить параметры сервиса.
	virtual QVariantMap getParameters() const;

	/// Сброс служебной информации.
	virtual void resetParameters(const QSet<QString> & aParameters);

	/// Получение списка необходимых сервисов.
	virtual const QSet<QString> & getRequiredServices() const;

	/// Включить все устройства.
	virtual bool setEnable(bool aEnable, const QString & aDevice = QString());

	/// Конвертирует данные сканера в строку
	QString valueToString(const QVariant & aData);

private slots:
	/// Обновить список устройств.
	void updateHardwareConfiguration();

private slots:
	/// Получение сигнала - карта вставлена
	void onCardInserted(SDK::Driver::ECardType::Enum aCardType, const QVariantMap & aData);

	/// Получение сигнала - карта извлечена
	void onCardEjected();

	/// Получение состояния картридера
	void onStatusChanged(SDK::Driver::EWarningLevel::Enum aLevel, const QString & aTranslation, int aStatus);

private:
	IApplication * mApplication;
	SDK::PaymentProcessor::IDeviceService * mDeviceService;
	QList<SDK::Driver::IHID *> mHIDs;
	QList<SDK::Driver::ICardReader *> mCardReaders;
};

//---------------------------------------------------------------------------
