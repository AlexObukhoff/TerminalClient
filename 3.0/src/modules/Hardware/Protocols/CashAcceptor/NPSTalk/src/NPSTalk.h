/* @file Протокол NPSTalk. */

#pragma once

#include "Hardware/Common/ProtocolBase.h"

//--------------------------------------------------------------------------------
namespace NPSTalk
{
	const char Prefix  = '\x55';           /// Префикс.
	const char Postfix = '\x5A';           /// Постфикс.

	const int MinAnswerSize = 4;           /// Минимальный размер ответа монетника.
	const int MaxBadAnswerRepeats = 3;     /// Максимальное количество повторов со стороны монетника.
	const int Timeout = 300;               /// Таймаут ответа, [мс].
}

//--------------------------------------------------------------------------------
class NPSTalkProtocol: public ProtocolBase
{
public:
	/// Установить адрес slave-устройства.
	void setAddress(uchar aAddress);

	/// Выполнить команду.
	TResult processCommand(const QByteArray & aCommanddata, QByteArray & aAnswerData);

protected:
	/// Проверить валидность ответа.
	bool check(const QByteArray & aCommandData, const QByteArray & aAnswerData);

	/// Получить пакет данных из порта.
	bool getAnswer(QByteArray & aAnswer);

	/// Адрес устройства.
	uchar mDeviceAddress;
};

//--------------------------------------------------------------------------------
