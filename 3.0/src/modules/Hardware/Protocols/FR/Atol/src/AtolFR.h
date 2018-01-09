/* @file Протокол ФР АТОЛ. */

#pragma once

#include "Hardware/Common/ProtocolBase.h"

//--------------------------------------------------------------------------------
/// Класс протокола KKM.
class AtolFRProtocol: public ProtocolBase
{
public:
	/// Выполнить команду протокола.
	TResult processCommand(const QByteArray & aCommandData, QByteArray & aUnpackedAnswer, int aTimeout);

private:
	/// Подсчет контрольной суммы пакета данных.
	const ushort calcCRC(const QByteArray & aData);

	/// Исполнить команду.
	bool execCommand(QByteArray & aPacket);

	/// Упаковка команды и данных в пакет.
	void packData(const QByteArray & aCommandPacket, QByteArray & aPacket);

	/// Распаковка пришедших из порта данных.
	bool unpackData(const QByteArray & aPacket, QByteArray & aData);

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
