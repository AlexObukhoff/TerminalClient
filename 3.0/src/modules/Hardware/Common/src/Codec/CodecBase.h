/* @file Базовый кодек для создания кастомных кодеков. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTextCodec>
#include <Common/QtHeadersEnd.h>

// Project
#include "Hardware/Common/Specifications.h"
#include "Hardware/Common/HardwareConstants.h"

//---------------------------------------------------------------------------
namespace CCodec
{
	/// Символ по умолчанию для неизвестных символов.
	const char DefaultCharacter = '?';
}

struct SCharData
{
	QString character;    /// UTF-8-cимвол по коду.
	bool main;    /// Признак основного символа при обратной перекодировке.

	SCharData() : character(QChar(CCodec::DefaultCharacter)), main(false) {}
	SCharData(const QString & aCharacter, bool aMain) : character(aCharacter), main(aMain) {}
	SCharData(const char * aCharacter, bool aMain) : character(QString::fromUtf8(aCharacter)), main(aMain) {}
	SCharData(const QChar & aCharacter, bool aMain) : character(aCharacter), main(aMain) {}

	bool operator == (const SCharData & aData) const
	{
		return (character == aData.character) && (main == aData.main);
	}
};

//---------------------------------------------------------------------------
class CharacterData: public CSpecification<char, SCharData>
{
public:
	CharacterData()
	{
		setDefault(SCharData());
	}

	void add(char aCode, const QString & aCharacter, bool aMain = true)
	{
		append(aCode, SCharData(aCharacter, aMain));
	}

	void add(char aCode, const char * aCharacter, bool aMain = true)
	{
		append(aCode, SCharData(aCharacter, aMain));
	}

	void add(char aCode)
	{
		append(aCode, mDefaultValue);
	}
};

//---------------------------------------------------------------------------
class CodecBase : public QTextCodec
{
public:
	CodecBase();

	/// Получить название.
	virtual QByteArray name() const;

	/// Получить список алиасов.
	virtual QList<QByteArray> aliases() const;

	/// Получить Id.
	virtual int mibEnum() const;

	/// Конвертировать массив байтов в юникодовую строку.
	virtual QString convertToUnicode(const char * aBuffer, int aLength, ConverterState * aState) const;

	/// Конвертировать юникодовую строку в массив байтов.
	virtual QByteArray convertFromUnicode(const QChar * aBuffer, int aLength, ConverterState * aState) const;

protected:
	/// Имя кодека.
	QByteArray mName;

	/// Id кодека >= 3000. http://www.iana.org/assignments/character-sets/character-sets.xhtml
	int mMIB;

	/// Массив данных для перекодировки.
	CharacterData mData;

	/// Минимальное значение unicode-символа для использования кодека для перекодировки.
	ushort mMinValueActive;
};

//---------------------------------------------------------------------------
