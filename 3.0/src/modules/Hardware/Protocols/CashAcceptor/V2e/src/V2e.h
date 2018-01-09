/* @file Протокол V2e. */

#pragma once

#include "Hardware/Common/ProtocolBase.h"

//--------------------------------------------------------------------------------
class V2eProtocol: public ProtocolBase
{
public:
	/// Выполнить команду протокола.
	TResult processCommand(const QByteArray & aCommandData, QByteArray & aAnswerData);

protected:
	/// Подсчет контрольной суммы пакета данных.
	ushort calcCRC(const QByteArray & aData);

	/// Обработать ошибку в ответе купюроприемника.
	TResult handleError(const QByteArray & aRequest, QByteArray & aAnswer);

	/// Получить пакет данных из порта.
	bool getAnswer(QByteArray & aAnswer);

	/// Упаковка команды и данных в пакет.
	void pack(QByteArray & aCommandData);

	/// Проверка пришедших из порта данных.
	bool check(const QByteArray & aAnswer, const QByteArray & aRequest);
};

//--------------------------------------------------------------------------------
