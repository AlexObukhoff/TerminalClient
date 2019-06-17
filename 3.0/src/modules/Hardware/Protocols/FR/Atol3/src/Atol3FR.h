/* @file Протокол ФР АТОЛ3. */

#pragma once

#include "Hardware/Common/ProtocolBase.h"

//--------------------------------------------------------------------------------
/// Класс протокола KKM.
class Atol3FRProtocol: public ProtocolBase
{
public:
	Atol3FRProtocol();

	/// Выполнить команду протокола.
	TResult processCommand(uchar aTId, const QByteArray & aCommandData, QByteArray & aUnpackedAnswer, int aTimeout);

	/// Запросить результат предыдущей операции.
	TResult getResult(uchar aTId, QByteArray & aUnpackedAnswer);

	/// Подождать ответ.
	TResult waitForAnswer(QByteArray & aUnpackedAnswer);

	/// ACK.
	TResult sendACK(uchar aTId);

	/// Отмена всех заданий и очистка буфера.
	TResult cancel();

private:
	/// Выполнить задание.
	TResult execCommand(char aCommand, const QByteArray & aCommandData, QByteArray & aAnswer, int aTimeout);

	/// Подсчет CRC.
	char calcCRC(const QByteArray & aData);

	/// Получить пакет данных из порта.
	bool read(QByteArray & aAnswer, int aTimeout);

	/// Проверка пришедших из порта данных.
	bool check(const QByteArray & aAnswer);

	/// Заменить экранирующие символы.
	void replace(QByteArray & aData, int & aIndex, bool aDirection) const;

	/// Транспортный Id.
	uchar mId;
};

//--------------------------------------------------------------------------------
