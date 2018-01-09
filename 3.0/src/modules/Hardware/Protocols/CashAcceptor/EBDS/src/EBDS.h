/* @file Протокол EBDS. */

#pragma once

#include "Hardware/Common/ProtocolBase.h"

//--------------------------------------------------------------------------------
class EBDSProtocol : public ProtocolBase
{
public:
	EBDSProtocol();

	/// Выполнить команду протокола.
	TResult processCommand(const QByteArray & aCommandData, QByteArray & aAnswerData, bool aNeedAnswer);

private:
	/// Подсчет контрольной суммы пакета данных.
	const uchar calcCRC(const QByteArray & aData);

	/// Получить пакет данных из порта.
	const bool getAnswer(QByteArray & aAnswer);

	/// Проверка пришедших из порта данных.
	bool check(const QByteArray & aCommandPacket, const QByteArray & aAnswerPacket);

	/// Отличительный признак направления посылки.
	bool mACK;
};

//--------------------------------------------------------------------------------
