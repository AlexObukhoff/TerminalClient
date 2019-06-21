/* @file Вспомогательные функции для протоколов. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <QtCore/QByteArray>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/ASCII.h"

namespace CProtocolUtils
{
	/// Регэксп для парсинга дампов в логах;
	const char LogRexExp[] = "\\{([0-9a-fA-F]+)\\}";
}

//--------------------------------------------------------------------------------
namespace ProtocolUtils
{
	/// Конвертировать число в hex-строку.
	template<class T>
	QString toHexLog(T aData);

	/// Очистить строку.
	template<class T>
	T clean(const T & aData);

	/// Получить булево значение бита в буфере байтов по номеру бита.
	bool getBit(const QByteArray & aBuffer, int aShift, bool invert = false);

	/// Получить массив байтов по строке лога.
	QByteArray getBufferFromString(QString aData);

	/// Получить список из массивов байтов по массиву строк.
	typedef QList<QByteArray> TBufferList;
	TBufferList getBufferListFromStrings(QStringList aDataList);

	/// Получить список из массивов байтов по строке логов. Логи должны быть разделены \t.
	TBufferList getBufferListFromLog(const QString & aData);

	/// Получить список из массивов байтов из файла.
	TBufferList getBufferListFromFile(const QString & aFileName);

	/// Маскировать байт.
	char mask(char aData, const QString & aMask);

	/// Переворачивает QByteArray. Не имеет аналога в qt 4.6.1 и ниже.
	template <class T>
	T revert(const T & aBuffer);

	/// Побайтно преобразовывает буфер из hex в BCD и заливает выбранным символом.
	QString hexToBCD(const QByteArray & aBuffer, char filler = ASCII::Zero);

	/// Делает из double инвертированный буфер.
	QByteArray getHexReverted(double aValue, int aSize, int aPrecision = 0);
}

//--------------------------------------------------------------------------------
