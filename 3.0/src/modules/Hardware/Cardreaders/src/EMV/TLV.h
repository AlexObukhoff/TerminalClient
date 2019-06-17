/* @file Функционал Tag-Len-Value. 

         Based on code from https://github.com/lumag/emv-tools/ (c) lumag
*/

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <Common/QtHeadersEnd.h>

//--------------------------------------------------------------------------------
namespace EMV
{
namespace TLV
{
	namespace Tag
	{
		const quint16 ClassMask = 0xc0;
		const quint16 Complex   = 0x20;
		const quint16 ValueMask = 0x1f;
		const quint16 ValueCont = 0x1f;
		const quint16 Invalid   = 0;
	}

	namespace Len
	{
		const quint16 Long = 0x80;
		const quint16 Mask = 0x7f;
		const quint16 Invalid = 0;
	}

	struct SItem
	{
		quint16 tag;
		QByteArray body;

		SItem(): tag(Tag::Invalid) {}
		bool isEmpty() const { return tag == 0 || body.isEmpty(); }
		bool isComplex() const { return tag & Tag::Complex; }
	};

	/// Tag-Len-Value parser
	class TLVs
	{
	public:
		TLVs() {}
		TLVs(const QByteArray & aBuffer);

		/// Распарсить EMV TVL буфер
		bool parse(const QByteArray & aBuffer);

		/// Получить тег из списка
		SItem getTag(quint16 aTag);

	protected:
		/// Распарсить элемент EMV TVL буфера
		SItem parseItem(QByteArray & aBuffer);

		/// Парсинг тега данных
		quint16 parseTag(QByteArray & aBuffer);

		/// Парсинг длинны данных
		quint16 parseLen(QByteArray & aBuffer);

		/// Взять первый байт и вернуть его в виде quint16
		quint16 takeByte(QByteArray & aBuffer);

		QList<SItem> mItems;
	};
}}

//--------------------------------------------------------------------------------
