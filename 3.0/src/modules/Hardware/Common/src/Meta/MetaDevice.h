/* @file Базовый класс мета-устройства. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDateTime>
#include <QtCore/QReadWriteLock>
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
#include "Hardware/Common/MutexLocker.h"
#include "Hardware/Common/FunctionTypes.h"
#include "Hardware/Common/ASCII.h"
#include "Hardware/Protocols/Common/ProtocolUtils.h"

#pragma warning(disable : 4250) // warning 4250: 'class1' : inherits 'class2::member' via dominance
// Есть ветки наследования, интерфейсная и базовой реализации. Последняя содержит вызываемый функционал и
// сделана специально выше по уровню, чем соответствующий интерфейс, поэтому предупреждение подавлено и включается во все файлы.

//--------------------------------------------------------------------------------
/// Общие константы мета-устройств.
namespace CMetaDevice
{
	/// Имя устройства по умолчанию.
	const char DefaultName[] = "Meta device";
}

/// Обобщенные состояния выполнения запроса.
namespace ERequestStatus
{
	enum Enum
	{
		Success = 0,
		InProcess,
		Fail
	};
}

struct SLogData
{
	QString pluginConfig;
	QString requiedDevicePluginConfig;
	QString device;
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

/// Данные устройств.
typedef QMap<QString, QString> TDeviceData;

//--------------------------------------------------------------------------------
class MetaDevice : virtual private SDK::Driver::IDevice, public SDK::Driver::IDevice::IDetectingIterator
{
	SET_INTERACTION_TYPE(System)
	SET_SERIES("")
	SET_SUBSERIES("")

	typedef DefaultSeriesType TSeriesType;

public:
	MetaDevice();

	/// Логгировать.
	void toLog(LogLevel::Enum aLevel, const QString & aMessage) const;

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

	/// Возвращает конфигурацию устройства для сохранения.
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

	/// Установка параметра устройства.
	void setConfigParameter(const QString & aName, const QVariant & aValue);

	/// Получение параметра устройства.
	QVariant getConfigParameter(const QString & aName) const;
	QVariant getConfigParameter(const QString & aName, const QVariant & aDefault) const;

	/// Удаление параметра устройства.
	void removeConfigParameter(const QString & aName);

	/// Есть ли параметр у устройства?
	bool containsConfigParameter(const QString & aName) const;

	/// Установка данных устройства.
	void setDeviceParameter(const QString & aName, const QVariant & aValue, const QString & aExtensibleName = "");

	/// Получение параметров устройства.
	SLogData getDeviceData() const;
	QVariant getDeviceParameter(const QString & aName) const;

	/// Логгирование параметров устройства.
	void logDeviceData(const SLogData & aData) const;

	/// Получение заданной компоненты параметров устройства.
	QString getPartDeviceData(const TDeviceData & aData, bool aHideEmpty = true) const;

	/// Есть ли непустой параметр у устройства?
	bool containsDeviceParameter(const QString & aName) const;

	/// Из рабочего ли потока происходит вызов.
	bool isWorkingThread();

	/// Рабочий поток.
	QThread mThread;

	/// Название устройства.
	QString mDeviceName;

	/// Параметры устройства.
	QVariantMap mConfiguration;
	mutable QReadWriteLock mConfigurationGuard;

	/// Данные устройства.
	TDeviceData mDeviceData;

	/// Время последнего логгирования.
	QDate mLogDate;

	/// Итерация автопоиска.
	int mDetectingPosition;

	/// Устройство инициализировано.
	ERequestStatus::Enum mInitialized;

	/// Лог.
	ILog * mLog;

	/// Драйвера запускаются из под модуля платежей.
	bool mOperatorPresence;

	/// Таймаут ожидания потока при его завершении.
	unsigned long mExitTimeout;
};

//---------------------------------------------------------------------------
