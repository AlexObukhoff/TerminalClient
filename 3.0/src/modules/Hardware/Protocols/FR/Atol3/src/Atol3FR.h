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
	TResult processCommand(const QByteArray & aCommandData, QByteArray & aUnpackedAnswer, int aTimeout);

	/// Запросить результат предыдущей операции.
	TResult getResult(const QByteArray & aCommandData, QByteArray & aUnpackedAnswer);

//private:
	/// Упаковка данных (транспортный уровень).
	void pack(QByteArray & aData);

	/// Отменить задания.
	TResult execCommand(char aCommand, QByteArray & aAnswer, int aTimeout);
	TResult execCommand(char aCommand, const QByteArray & aCommandData, QByteArray & aAnswer, int aTimeout);

	/// Подсчет CRC.
	char calcCRC(const QByteArray & aData);

	/// Получить пакет данных из порта.
	bool read(QByteArray & aAnswer, int aTimeout);

	/// Проверка пришедших из порта данных.
	bool check(const QByteArray & aAnswer);

	/// Транспортный Id.
	uchar mId;
};

//--------------------------------------------------------------------------------
