/* @file Протокол сторожевого таймера LDog. */

#pragma once

#include "Hardware/Common/ProtocolBase.h"

//--------------------------------------------------------------------------------
/// Класс протокола KKM.
class LDogWDProtocol: public ProtocolBase
{
public:
	/// Выполнить команду протокола.
	TResult processCommand(const QByteArray & aCommandData, QByteArray & aUnpackedData);

private:
	/// Подсчет контрольной суммы пакета данных.
	uchar calcCRC(const QByteArray & aData);

	/// Проверка ответа на валидность.
	bool checkAnswer(const QByteArray & aRequest, const QByteArray & aAnswer);

	/// Прочитать ответ.
	bool read(QByteArray & aAnswer);
};

//--------------------------------------------------------------------------------
