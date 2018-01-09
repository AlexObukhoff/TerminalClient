/* @file Протокол сторожевого таймера OSMP 2.5. */

#pragma once

#include "Hardware/Common/ProtocolBase.h"

//--------------------------------------------------------------------------------
/// Класс протокола KKM.
class OSMPWDProtocol: public ProtocolBase
{
public:
	/// Выполнить команду протокола.
	TResult processCommand(const QByteArray & aCommandData, QByteArray & aUnpackedData, bool aNeedAnswer);

private:
	/// Подсчет контрольной суммы пакета данных.
	const uchar calcCRC(const QByteArray & aData);

	/// Распаковка пришедших из порта данных.
	bool checkAnswer(const QByteArray & aAnswerData);

	/// Получить пакет данных из порта.
	bool read(QByteArray & aData);
};

//--------------------------------------------------------------------------------
