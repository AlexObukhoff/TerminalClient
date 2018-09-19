/* @file Протокол ccTalk. */

#pragma once

// Modules
#include "Hardware/Common/ProtocolBase.h"

// Project
#include "Hardware/Acceptors/CCTalkData.h"

//--------------------------------------------------------------------------------
namespace CCCTalk
{
	/// Неизвестный тип протокола.
	const char UnknownType[] = "unknown";
}

//--------------------------------------------------------------------------------
class CCTalkCAProtocol: public ProtocolBase
{
public:
	CCTalkCAProtocol();

	/// Установить тип CRC.
	void setType(const QString & aType);

	/// Установить адрес slave-устройства.
	void setAddress(uchar aAddress);

	/// Выполнить команду.
	TResult processCommand(const QByteArray & aCommandData, QByteArray & aAnswerData);

protected:
	/// Проверить валидность ответа.
	bool check(QByteArray & aAnswer);

	/// Получить пакет данных из порта.
	bool getAnswer(QByteArray & aAnswer, const QByteArray & aCommandData);

	/// Вычислить контрольную сумму пакета данных.
	uchar calcCRC8(const QByteArray & aData);
	ushort calcCRC16(const QByteArray & aData);

	/// Адрес устройства.
	char mAddress;

	/// тип CRC.
	QString mType;
};

//--------------------------------------------------------------------------------
