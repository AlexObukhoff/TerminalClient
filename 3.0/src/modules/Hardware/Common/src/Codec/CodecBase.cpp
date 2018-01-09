/* @file Базовый кодек для создания кастомных кодеков. */

#pragma once

#include "Hardware/Common/ASCII.h"
#include "CodecBase.h"

//---------------------------------------------------------------------------
CodecBase::CodecBase() : mMIB(0), mMinValueActive(0x0080)
{
}

//---------------------------------------------------------------------------
QByteArray CodecBase::name() const
{
	return mName;
}

//---------------------------------------------------------------------------
QList<QByteArray> CodecBase::aliases() const
{
	return QList<QByteArray>() << mName;
}

//---------------------------------------------------------------------------
int CodecBase::mibEnum() const
{
	return mMIB;
}

//---------------------------------------------------------------------------
QString CodecBase::convertToUnicode(const char * aBuffer, int aLength, ConverterState * /*aState*/) const
{
	QByteArray buffer = QByteArray::fromRawData(aBuffer, aLength);
	CharacterData & characterData = const_cast<CharacterData &>(mData);
	QByteArray defaultBuffer = characterData.getDefault().character.toLatin1();
	bool dataEmpty = characterData.data().isEmpty();

	auto replace = [&] (char ch) { if (uchar(ch) < uchar(mMinValueActive)) { dataEmpty ? buffer.replace(ch, defaultBuffer[0]) : buffer.replace(ch, ""); }};

	for (char ch = 0x00; ch < ASCII::Space; ++ch)
	{
		replace(ch);
	}

	replace(ASCII::DEL);

	QString result;

	for (int i = 0; i < buffer.size(); ++i)
	{
		if (!buffer[i] && mMinValueActive)
		{
			break;
		}

		if (uchar(buffer[i]) < uchar(mMinValueActive))
		{
			result += buffer[i];
		}
		else
		{
			result += mData[buffer[i]].character;
		}
	}

	return result;
}

//---------------------------------------------------------------------------
QByteArray CodecBase::convertFromUnicode(const QChar * aBuffer, int aLength, ConverterState * /*aState*/) const
{
	QByteArray result;
	CharacterData & characterData = const_cast<CharacterData &>(mData);
	QString data(aBuffer, aLength);

	for (int i = 0; i < aLength; ++i)
	{
		ushort unicode = aBuffer[i].unicode();
		char character = char(unicode);

		if (unicode > mMinValueActive)
		{
			QList<char> characters = characterData.data().keys(SCharData(data[i], true));

			if (characters.isEmpty())
			{
				characters = characterData.data().keys(SCharData(data[i], false));
			}

			character = characters.isEmpty() ? CCodec::DefaultCharacter : characters[0];
		}

		result.append(character);
	}

	return result;
}

//---------------------------------------------------------------------------
