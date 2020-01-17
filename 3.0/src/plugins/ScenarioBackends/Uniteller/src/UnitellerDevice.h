/* @file Виртуальный HID для Uniteller. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QSharedPointer>
#include <QtCore/QFutureWatcher>
#include <QtCore/QTimer>
#include <QtCore/QReadWriteLock>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/ICardReader.h>
#include <SDK/Drivers/Components.h>
#include <SDK/Drivers/InteractionTypes.h>

// Common
#include <Common/ILogable.h>

// Modules
#include "SingleDetectingIterator.h"

// Project
#include "API.h"

namespace Uniteller
{
	const char ModelName[] = "Uniteller";
}

//--------------------------------------------------------------------------------
class UnitellerDevice : public QObject, public SDK::Driver::ICardReader, private SDK::Driver::SingleDetectingIterator, public ILogable
{
	Q_OBJECT

public:
	UnitellerDevice();
	~UnitellerDevice();

	static QString getDeviceType() { return SDK::Driver::CComponents::CardReader; }
	static QString getInteractionType() { return SDK::Driver::CInteractionTypes::External; }
	static QString getSeries() { return Uniteller::ModelName; }
	static QString getSubSeries() { return ""; }

#pragma region SDK::Driver::IDevice interface

	/// Переформировывает список параметров для автопоиска и устанавливает 1-й набор параметров из этого списка.
	virtual SDK::Driver::IDevice::IDetectingIterator * getDetectingIterator();

	/// Подключает и инициализует устройство.
	virtual void initialize();

	/// Освобождает ресурсы, связанные с устройством, возвращается в состояние до вызова initialize().
	virtual bool release();

	/// Возвращает конфигурацию устройства для сохранения.
	virtual QVariantMap getDeviceConfiguration() const;

	/// Устанавливает конфигурацию устройству.
	virtual void setDeviceConfiguration(const QVariantMap & aConfiguration);

	/// Соединяет сигнал данного интерфейса со слотом приёмника.
	virtual bool subscribe(const char * aSignal, QObject * aReceiver, const char * aSlot);

	/// Отсоединяет сигнал данного интерфейса от слота приёмника.
	virtual bool unsubscribe(const char * aSignal, QObject * aReceiver);

	/// Возвращает название устройства.
	virtual QString getName() const;

	/// Задаёт лог.
	virtual void setLog(ILog * aLog);

	/// Обновить прошивку.
	virtual void updateFirmware(const QByteArray & aBuffer);

	/// Можно ли обновлять прошивку.
	virtual bool canUpdateFirmware();

#pragma endregion
#pragma region SDK::Driver::ICardReader interface

	/// Проверка доступности устройства и карты.
	virtual bool isDeviceReady() const;

	/// Выбросить карту (для моторизированных ридеров) или отключить электрически (для немоторизованных).
	virtual void eject();

#pragma endregion

public:
	void setCore(SDK::PaymentProcessor::ICore * aCore);

signals:
	/// Изменение состояния.
	void status(SDK::Driver::EWarningLevel::Enum, const QString &, int);
	void configurationChanged();

signals:
	/// Карта вставлена.
	void inserted(SDK::Driver::ECardType::Enum, const QVariantMap &);

	/// Карта извлечена.
	void ejected();

#pragma region IDevice::IDetectingIterator
private:
	/// Поиск устройства.
	virtual bool find();
#pragma endregion

private slots:
	void onReady();
	void onState(int aState, const QString & aDeviceName, bool aLast);
	void onDeviceEvent(Uniteller::DeviceEvent::Enum aEvent, Uniteller::KeyCode::Enum aKeyCode);
	void onEjected();

private:
	/// Параметры устройства.
	QVariantMap mConfiguration;
	mutable QReadWriteLock mConfigurationGuard;

	/// Установка параметра устройства.
	void setConfigParameter(const QString & aName, const QVariant & aValue);

	/// Получение параметра устройства.
	QVariant getConfigParameter(const QString & aName) const;

	/// Есть ли параметр у устройства?
	bool containsConfigParameter(const QString & aName) const;

private:
	QString mLastGeneralizedStatus;
	void sendStatus(SDK::Driver::EWarningLevel::Enum, const QString &, int);

private:
	SDK::PaymentProcessor::ICore * mCore;
	QList<QPair<int, QString>> mStates;
};

//--------------------------------------------------------------------------------
