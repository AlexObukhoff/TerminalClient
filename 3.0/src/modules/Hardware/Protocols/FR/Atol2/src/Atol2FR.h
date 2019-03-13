/* @file Протокол ФР АТОЛ. */

#pragma once

#include "Hardware/Common/ProtocolBase.h"

//--------------------------------------------------------------------------------
/// Класс протокола KKM.
class Atol2FRProtocol: public ProtocolBase
{
public:
	/// Выполнить команду протокола.
	TResult processCommand(const QByteArray & aCommandData, QByteArray & aUnpackedAnswer, int aTimeout);

private:
	/// Вычислить CRC.
	char calcCRC(const QByteArray & aData);

	/// Исполнить команду.
	bool execCommand(QByteArray & aPacket);

	/// Распаковать данные.
	bool unpack(QByteArray & aAnswer);

	/// Получить пакет данных из порта.
	bool read(QByteArray & aData);

	/// Открыть сеанс связи для записи данных.
	bool openWriteSession();

	/// Открыть сеанс связи для чтения данных.
	bool openReadSession(int aTimeout);

	/// Закрыть сеанс связи для записи данных.
	bool closeWriteSession();

	/// Закрыть сеанс связи для чтения данных.
	bool closeReadSession();

	/// Отправить ACK.
	bool sendACK();
};

//--------------------------------------------------------------------------------
