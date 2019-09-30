/* @file Базовый класс мета-устройства. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDateTime>
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/IDevice.h>
#include <SDK/Drivers/InteractionTypes.h>

// Common
#include <Common/SleepHelper.h>

// Project
#include "Hardware/Common/HardwareConstants.h"
#include "Hardware/Common/DeviceDataConstants.h"
#include "Hardware/Common/DeviceLogicManager.h"
#include "Hardware/Common/MutexLocker.h"
#include "Hardware/Common/FunctionTypes.h"
#include "Hardware/Common/ASCII.h"
#include "Hardware/Protocols/Common/ProtocolUtils.h"
#include "Hardware/Common/DeviceUtils.h"

//--------------------------------------------------------------------------------
/// Общие константы мета-устройств.
namespace CMetaDevice
{
	/// Имя устройства по умолчанию.
	const char DefaultName[] = "Meta device";
}

/// Данные устройства для логгирования и мониторинга.
struct SLogData
{
	QString plugin;    /// ini плагина устройства.
	QString device;    /// данные устройства.
	QString config;    /// данные config.xml, относящиеся к работе устройства.

	QString requiedDevice;    /// ini плагина зависимого устройства.
};

//--------------------------------------------------------------------------------
/// Параметры путей плагинов.

/// Тип взаимодействия с устройством.
#define SET_INTERACTION_TYPE(aType) public: typedef SDK::Driver::CInteractionTypes::It##aType TIType; \
	static QString getInteractionType() { return SDK::Driver::CInteractionTypes::aType; }

/// Семейство (протокол etc).
#define SET_SERIES(aSeries) public: static QString getSeries() { return aSeries; }

/// Подсемейство (устройство/группа устройств в составе семейства, имеющие уникальную реализацию и/или настройки плагина).
#define SET_SUBSERIES(aSubSeries) public: static QString getSubSeries() { return aSubSeries; }

class DefaultSeriesType {};

//--------------------------------------------------------------------------------
template <class T>
class MetaDevice : public T, public SDK::Driver::IDevice::IDetectingIterator, public DeviceLogicManager
{
	SET_INTERACTION_TYPE(System)
	SET_SERIES("")
	SET_SUBSERIES("")

	typedef DefaultSeriesType TSeriesType;

public:
	MetaDevice();

#pragma region SDK::Driver::IDevice interface
	/// Возвращает название устройства.
	virtual QString getName() const;

	/// Переформировывает список параметров для автопоиска и устанавливает 1-й набор параметров из этого списка.
	virtual SDK::Driver::IDevice::IDetectingIterator * getDetectingIterator();

	/// Подключает и инициализует устройство. Обертка для вызова функционала в рабочем потоке.
	virtual void initialize();

	/// Освобождает ресурсы, связанные с устройством, возвращается в состояние до вызова initialize().
	virtual bool release();

	/// Соединяет сигнал данного интерфейса со слотом приёмника.
	virtual bool subscribe(const char * aSignal, QObject * aReceiver, const char * aSlot);

	/// Отсоединяет сигнал данного интерфейса от слота приёмника.
	virtual bool unsubscribe(const char * aSignal, QObject * aReceiver);

	/// Возвращает конфигурацию устройства.
	virtual QVariantMap getDeviceConfiguration() const;

	/// Устанавливает конфигурацию устройству.
	virtual void setDeviceConfiguration(const QVariantMap & aConfiguration);

	/// Обновить прошивку.
	virtual void updateFirmware(const QByteArray & aBuffer);

	/// Можно ли обновлять прошивку.
	virtual bool canUpdateFirmware();

	/// Установить лог.
	virtual void setLog(ILog * aLog);
#pragma endregion

#pragma region SDK::Driver::IDetectingIterator interface
	/// Переход к следующим параметрам устройства.
	virtual bool moveNext();

	/// Поиск устройства на текущих параметрах.
	virtual bool find();
#pragma endregion

protected:
	/// Автопоиск?
	bool isAutoDetecting() const;

	/// Получение параметров устройства.
	SLogData getDeviceData() const;

	/// Установка параметра устройства.
	void setDeviceParameter(const QString & aName, const QVariant & aValue, const QString & aExtensibleName = "", bool aUpdateExtensible = false);

	/// Получение параметра устройства.
	QVariant getDeviceParameter(const QString & aName) const;

	/// Есть ли непустой параметр у устройства?
	bool containsDeviceParameter(const QString & aName) const;

	/// Удалить параметр устройства.
	void removeDeviceParameter(const QString & aName);

	/// Логгирование параметров устройства.
	void logDeviceData(const SLogData & aData) const;

	/// Из рабочего ли потока происходит вызов.
	bool isWorkingThread();

	/// Рабочий поток.
	QThread mThread;

	/// Название устройства.
	QString mDeviceName;

	/// Данные устройства.
	TDeviceData mDeviceData;

	/// Время последнего логгирования.
	QDate mLogDate;

	/// Итерация автопоиска.
	int mDetectingPosition;

	/// Устройство инициализировано.
	ERequestStatus::Enum mInitialized;

	/// Драйвера запускаются из под модуля платежей.
	bool mOperatorPresence;

	/// Драйвера запускаются из под фискального сервера.
	bool mFiscalServerPresence;

	/// Таймаут ожидания потока при его завершении.
	unsigned long mExitTimeout;

	/// Ошибка инициализации.
	bool mInitializationError;
};

//---------------------------------------------------------------------------
