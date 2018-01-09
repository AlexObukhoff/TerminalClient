/* @file Протокол Puloon. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QList>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/ProtocolBase.h"

namespace CPuloon
{
	/// Размер данных о выдаваемом предмете.
	const int ItemDataSize = 3;
}

typedef QList<QByteArray> TAnswerList;

//--------------------------------------------------------------------------------
class Puloon: public ProtocolBase
{
public:
	/// Выполнить команду.
	TResult processCommand(const QByteArray & aCommandData, TAnswerList & aAnswerList, int aTimeout) const;

private:
	/// Подсчет CRC.
	uchar calcCRC(const QByteArray & aData) const;

	/// Исполнить команду.
	bool execCommand(const QByteArray & aCommandPacket, TAnswerList & aAnswerList, int aTimeout) const;

	/// Переполучить ответ.
	bool regetAnswer(TAnswerList & aAnswerList) const;

	/// Упаковать команду.
	void pack(const QByteArray & aCommandPacket, QByteArray & aPacket) const;

	/// Проверка ответа.
	bool check(const QByteArray & aAnswer, const QByteArray & aRequest) const;

	/// Прочитать ответ.
	bool getAnswer(TAnswerList & aAnswerList, int aTimeout) const;
};

//--------------------------------------------------------------------------------
