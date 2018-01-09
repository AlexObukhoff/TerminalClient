/* @file Протокол ФР ШтрихПэй. */

#pragma once

#include "Hardware/Common/ProtocolBase.h"

//--------------------------------------------------------------------------------
/// Класс протокола ShtrihFR.
class ShtrihPayFRProtocol: public ProtocolBase
{
public:
	ShtrihPayFRProtocol();

	/// Выполнить команду.
	TResult processCommand(const QByteArray & aCommandData, QByteArray & aUnpackedAnswer, int aTimeout);

private:
	/// Подсчет CRC.
	ushort calcCRC(const QByteArray & aData);

	/// Исполнить команду.
	bool execCommand(const QByteArray & aCommandPacket, QByteArray & aAnswerData, int aTimeout);

	/// Переполучить ответ.
	bool regetAnswer(QByteArray & aAnswerData);

	/// Распаковка пришедших из порта данных.
	bool unpackData(const QByteArray & aPacket, QByteArray & aData);

	/// Прочитать ответ.
	bool getAnswer(QByteArray & aData, int aTimeout);

	/// Открыть сеанс связи.
	bool openSession();

	/// Отправить ACK.
	bool sendACK();

	/// Отправить NAK.
	bool sendNAK();

	/// Отправить ENQ.
	bool sendENQ();

	/// Номер пакета.
	ushort mPacketId;
};

//--------------------------------------------------------------------------------
