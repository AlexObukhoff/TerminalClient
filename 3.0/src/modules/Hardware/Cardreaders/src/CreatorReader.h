/* @file Кардридер Creator. */
#pragma once

// Modules
#include "Hardware/Common/USBDeviceBase.h"
#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/CardReaders/ProtoMifareReader.h"
#include "Hardware/CardReaders/Creator.h"

// Project
#include "CreatorReaderDataTypes.h"

//------------------------------------------------------------------------------
class CreatorReader: public USBDeviceBase<PortPollingDeviceBase<ProtoMifareReader>>
{
	SET_SERIES("Creator")

public:
	CreatorReader();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

#pragma region SDK::Driver::ICardReader
	/// Проверка доступности устройства и карты.
	virtual bool isDeviceReady() const;

	/// Выбросить карту (для моторизированных ридеров) или отключить электрически (для немоторизованных).
	virtual void eject();
#pragma endregion

#pragma region SDK::Driver::IMifareReader
	/// Сброс карты по питанию.
	virtual bool reset(QByteArray & aAnswer);

	/// Произвести обмен данными с ридером или картой
	virtual bool communicate(const QByteArray & aSendMessage, QByteArray & aReceiveMessage);
#pragma endregion

protected:
	/// Получить статус;
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Выполнить команду.
	TResult processCommand(const QByteArray & aCommand, QByteArray * aAnswer = nullptr, bool aIOLogsDebugMode = false);
	TResult processCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer = nullptr, bool aIOLogsDebugMode = false);

	/// Проверить ответ.
	bool checkAnswer(const QByteArray & aCommand, const QByteArray & aAnswer);

	/// Прочитать данные магнитной полосы.
	bool readMSData(QVariantMap & aData);

	/// Протокол.
	Creator mProtocol;

	/// Положение карты по отношению к кардридеру.
	int mCardPosition;

	/// Тип IC карты.
	CCreatorReader::CardTypes::EICCPU::Enum mICCPUType;
};
