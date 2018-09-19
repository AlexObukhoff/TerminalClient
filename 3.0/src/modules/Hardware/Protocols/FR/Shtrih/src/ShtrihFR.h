/* @file Протокол ФР Штрих. */

#pragma once

#include "Hardware/Common/ProtocolBase.h"

//--------------------------------------------------------------------------------
/// Класс протокола ShtrihFR.
class ShtrihFRProtocol: public ProtocolBase
{
public:
	ShtrihFRProtocol();

	/// Выполнить команду.
	TResult processCommand(const QByteArray & aCommandData, QByteArray & aUnpackedAnswer, int aTimeout);

	/// Установить таймаут технологических посылок.
	void setTransportTimeout(int aTimeout);

private:
	/// Подсчет CRC.
	uchar calcCRC(const QByteArray & aData);

	/// Исполнить команду.
	bool execCommand(const QByteArray & aCommand, QByteArray & aAnswer, int aTimeout);

	/// Переполучить ответ.
	bool regetAnswer(QByteArray & aAnswerData);

	/// Прочитать данные.
	bool readData(QByteArray & aData, int aTimeout = SDK::Driver::IIOPort::DefaultReadTimeout);

	/// Проверка пришедших из порта данных.
	bool check(const QByteArray & aAnswer);

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

	/// Таймаут технологических посылок.
	int mTransportTimeout;
};

//--------------------------------------------------------------------------------
