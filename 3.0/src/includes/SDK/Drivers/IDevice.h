/* @file Общий интерфейс устройства. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QByteArray>
#include <QtCore/QVariantMap>
#include <QtCore/QVariant>
#include <QtCore/QMetaType>
#include <Common/QtHeadersEnd.h>

class QObject;

// SDK
#include <SDK/Drivers/WarningLevel.h>
#include <SDK/Drivers/DeviceStatus.h>

// Common
#include <Common/ILog.h>

namespace DSDK = SDK::Driver;

namespace SDK {
namespace Driver {

//--------------------------------------------------------------------------------
class IDevice
{
public:
	/// Статус устройства изменен.
	static const char * StatusSignal; // = SIGNAL(status(SDK::Driver::EWarningLevel::Enum, const QString &, int));

	/// Инициализация окончена.
	static const char * InitializedSignal; // = SIGNAL(initialized());

	/// Прошивка устройства обновлена.
	static const char * UpdatedSignal; // SIGNAL(updated(bool aSuccess));

	/// Необходимо обновить конфигурацию.
	static const char * ConfigurationChangedSignal; // SIGNAL(configurationChanged());

	//--------------------------------------------------------------------------------
	/// Интерфейс итератора для поиска устройств на разных параметрах.
	class IDetectingIterator
	{
	public:
		/// Переход к следующим параметрам устройства.
		virtual bool moveNext() = 0;

		/// Поиск устройства на текущих параметрах.
		virtual bool find() = 0;

	protected:
		virtual ~IDetectingIterator() {}
	};

public:
	/// Возвращает название подключенного устройства. Обязан выдвать корректное значение после вызова initialize().
	virtual QString getName() const = 0;

	/// Переформировывает список параметров для автопоиска и устанавливает 1-й набор параметров из этого списка.
	virtual IDetectingIterator * getDetectingIterator() = 0;

	/// Подключает и инициализует устройство. Возвращает истину, если устройство готово к работе (установлен порт и т.д.).
	virtual void initialize() = 0;

	/// Освобождает ресурсы, связанные с устройством, возвращается в состояние до вызова initialize().
	virtual bool release() = 0;

	/// Соединяет сигнал данного интерфейса со слотом приёмника.
	virtual bool subscribe(const char * aSignal, QObject * aReceiver, const char * aSlot) = 0;

	/// Отсоединяет сигнал данного интерфейса от слота приёмника.
	virtual bool unsubscribe(const char * aSignal, QObject * aReceiver) = 0;

	/// Устанавливает конфигурацию устройству.
	virtual void setDeviceConfiguration(const QVariantMap & aConfiguration) = 0;

	/// Возвращает конфигурацию устройства для сохранения.
	virtual QVariantMap getDeviceConfiguration() const = 0;

	/// Обновить прошивку.
	virtual void updateFirmware(const QByteArray & aBuffer) = 0;

	/// Можно ли обновлять прошивку.
	virtual bool canUpdateFirmware() = 0;

	/// Установить лог.
	virtual void setLog(ILog * aLog) = 0;

protected:
	virtual ~IDevice() {}
};

}} // namespace SDK::Driver

Q_DECLARE_METATYPE(SDK::Driver::IDevice *);

//--------------------------------------------------------------------------------
