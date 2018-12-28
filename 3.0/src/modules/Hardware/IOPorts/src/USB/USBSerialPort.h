/* @file Асинхронная Windows-реализация USB-порта. */

#pragma once

#include "Hardware/IOPorts/AsyncSerialPort.h"

//--------------------------------------------------------------------------------
/// Константы USB-порта.
namespace CUSBPort
{
	/// Пауза при открытии, [мс].
	const int OpeningPause = 500;

	/// Идентификационные теги устройств на USB-порту.
	namespace DeviceTags
	{
		const char ACPI[] = "ACPI";      /// ACPI-устрйоство.
		const char Mouse[] = "Mouse";     /// Mouse.
		const char USBPDO[] = "USBPDO";    /// Physical Device Object (PDO).
	}

	/// Размер буфера для чтения по умолчанию.
	const int DefaultMaxReadSize = 1024;

	/// Пустой буфер.
	const TReadingBuffer EmptyBuffer = TReadingBuffer(DefaultMaxReadSize, ASCII::NUL);

	/// GUIDы для автопоиска портов. Класс нужен для использования в static-фунцкии.
	class Uuids : public TUuids
	{
	public:
		Uuids()
		{
			append(GUID_DEVINTERFACE_COMPORT);
			append(GUIDs::USB1);
			append(GUIDs::USBHID);
		}
	};

	/// Системное свойство для формирования пути для открытия порта.
	const DWORD PathProperty = SPDRP_PHYSICAL_DEVICE_OBJECT_NAME;
}

//--------------------------------------------------------------------------------
class USBPort : public AsyncSerialPort
{
	SET_SERIES("USB")

public:
	USBPort();

	/// Очистка буферов порта.
	virtual bool clear();

	/// Проверка готовности порта.
	virtual bool checkReady();

	/// Получить системные свойства устройств.
	TWinDeviceProperties getDevicesProperties(bool aForce, bool aPDODetecting = false);

protected:
	/// Открыть порт.
	virtual bool performOpen();

	/// Чтение данных.
	virtual bool processReading(QByteArray & aData, int aTimeout);

	/// Мьютекс для защиты статических пропертей портов.
	static QMutex mSystemPropertyMutex;
};

//--------------------------------------------------------------------------------
