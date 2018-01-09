/* @file Вспомогательные функции для протоколов. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/ASCII.h"

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
	QByteArray getBufferFromString(const QString & aData);

	/// Маскировать байт.
	char mask(char aData, const QString & aMask);

	/// Переворачивает QByteArray. Не имеет аналога в qt 4.6.1 и ниже.
	QByteArray revert(const QByteArray & aBuffer);

	/// Побайтно преобразовывает буфер из hex в BCD и заливает выбранным символом.
	QString hexToBCD(const QByteArray & aBuffer, char filler = ASCII::Zero);

	/// Делает из double инвертированный буфер.
	QByteArray getHexReverted(double aValue, int aSize, int aPrecision = 0);
}

//--------------------------------------------------------------------------------
