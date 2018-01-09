/* @file Протокол ФР Касби. */

#pragma once

#include "Hardware/Common/ProtocolBase.h"

//--------------------------------------------------------------------------------
/// Класс протокола KKM.
class KasbiFRProtocol: public ProtocolBase
{
public:
	/// Выполнить команду протокола.
	TResult processCommand(const QByteArray & aCommandData, QByteArray & aUnpackedAnswer, int aTimeout);

private:
	/// Подсчет контрольной суммы пакета данных.
	ushort calcCRC(const QByteArray & aData);

	/// Проверка пришедших из порта данных.
	bool check(const QByteArray & aAnswer);

	/// Получить пакет данных из порта.
	bool read(QByteArray & aAnswer, int aTimeout);
};

//--------------------------------------------------------------------------------
