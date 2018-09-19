/* @file Драйвер PC/SC ридера. */
#pragma once

// Modules
#include "Hardware/Common/USBDeviceBase.h"
#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/CardReaders/ProtoMifareReader.h"

// Project
#include "PCSCReader.h"

//------------------------------------------------------------------------------
namespace CMifareReader
{
	/// Запрос получения версии карты.
	const QByteArray GetVersionRequest = QByteArray::fromRawData("\x80\x60\x00\x00\x00", 5);

	/// Заголовок ответа на запрос версии карты.
	const char SAM2Header[] = "\x04\x01\x01";
}

//------------------------------------------------------------------------------
typedef USBDeviceBase<PortPollingDeviceBase<ProtoMifareReader>> TMifareReader;

class MifareReader : public TMifareReader
{
	SET_INTERACTION_TYPE(System)
	SET_SERIES("PCSC")

public:
	MifareReader();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

#pragma region SDK::Driver::IDevice
	/// Освобождает ресурсы, связанные с устройством, возвращается в состояние до вызова initialize().
	virtual bool release();
#pragma endregion

#pragma region SDK::Driver::ICardReader
	/// Проверка доступности устройства и карты.
	virtual bool isDeviceReady() const;

	/// Выбросить карту (для моторизированных ридеров) или отключить электрически (для немоторизованных).
	virtual void eject();
#pragma endregion

#pragma region SDK::Driver::IMifareReader
	/// Сброс карты по питанию.
	virtual bool reset(QByteArray & aAnswer);

	/// Произвести обмен данными с картой
	virtual bool communicate(const QByteArray & aSendMessage, QByteArray & aReceiveMessage);
#pragma endregion

protected:
	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Список найденных в системе ридеров
	QStringList mDetectedReaders;

	/// Адаптер для работы с PS/CS API
	PCSCReader mReader;

	/// Готов ли к работе (инициализировался успешно, ошибок нет).
	bool mReady;
};

//------------------------------------------------------------------------------
