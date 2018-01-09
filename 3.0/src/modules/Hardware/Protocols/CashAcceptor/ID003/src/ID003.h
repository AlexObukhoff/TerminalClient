/* @file Протокол ID003. */

#pragma once

#include "Hardware/Common/ProtocolBase.h"

//--------------------------------------------------------------------------------
class ID003Protocol : public ProtocolBase
{
public:
	/// Выполнить команду протокола.
	TResult processCommand(const QByteArray & aCommandData, QByteArray & aAnswerData);

	/// Отсылка ACK.
	bool sendACK();

protected:
	/// Упаковка команды с данными.
	void pack(QByteArray & aCommandData);

	/// Подсчет контрольной суммы пакета данных.
	ushort calcCRC16(const QByteArray & aData);

	/// Получить пакет данных из порта.
	bool getAnswer(QByteArray & aAnswerData);

	/// Проверка пришедших из порта данных.
	bool check(const QByteArray & aPacket);
};

//--------------------------------------------------------------------------------
