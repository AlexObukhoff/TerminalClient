/* @file Протокол AFP. */

#pragma once

#include "Hardware/Common/ProtocolBase.h"

//--------------------------------------------------------------------------------
class AFPFRProtocol: public ProtocolBase
{
public:
	AFPFRProtocol();

	/// Выполнить команду.
	TResult processCommand(const QByteArray & aCommandData, QByteArray & aAnswer, int aTimeout);

protected:
	/// Проверить валидность ответа.
	TResult check(const QByteArray & aRequest, const QByteArray & aAnswer);

	/// Получить пакет данных из порта.
	bool getAnswer(QByteArray & aAnswer, int aTimeout);

	/// Вычислить CRC.
	char calcCRC(const QByteArray & aData);

	/// Id пакета.
	uchar mId;
};

//--------------------------------------------------------------------------------
