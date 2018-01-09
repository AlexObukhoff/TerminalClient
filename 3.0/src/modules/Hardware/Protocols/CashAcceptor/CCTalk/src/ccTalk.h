/* @file Протокол ccTalk. */

#pragma once

#include "Hardware/Common/ProtocolBase.h"

//--------------------------------------------------------------------------------
namespace CCCTalk
{
	/// Дефолтный таймаут для ожидания ответа, [мс].
	const int ReadingTimeout = 500;

	namespace ECRCType
	{
		enum Enum
		{
			Simple,
			CRC8,
			CRC16
		};
	}
}

//--------------------------------------------------------------------------------
class CCTalkCAProtocol: public ProtocolBase
{
public:
	CCTalkCAProtocol();

	/// Установить тип CRC.
	void setCRCType(CCCTalk::ECRCType::Enum aType);

	/// Установить адрес slave-устройства.
	void setAddress(uchar aAddress);

	/// Выполнить команду.
	TResult processCommand(const QByteArray & aCommandData, QByteArray & aAnswerData);

protected:
	/// Проверить валидность ответа.
	bool check(QByteArray & aAnswer);

	/// Получить пакет данных из порта.
	bool getAnswer(QByteArray & aAnswer, const QByteArray & aCommandData);

	/// Адрес устройства.
	uchar mAddress;

	/// тип CRC.
	CCCTalk::ECRCType::Enum mCRCType;
};

//--------------------------------------------------------------------------------
