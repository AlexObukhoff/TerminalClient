/* @file Движок тегов. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegExp>
#include <Common/QtHeadersEnd.h>

// Project
#include "Tags.h"

//--------------------------------------------------------------------------------
void Tags::Engine::appendByGroup(bool aBitField, Type::Enum aType,
	const QByteArray & aPrefix,
	const QByteArray & aOpen,
	const QByteArray & aClose)
{
	append(aType, STagData(aBitField, aPrefix, aOpen, aClose));
	mPrefixData.insert(aType, aPrefix);
}

//--------------------------------------------------------------------------------
void Tags::Engine::appendSingle(Type::Enum aType,
	const QByteArray & aPrefix,
	const QByteArray & aOpen,
	const QByteArray & aClose)
{
	appendByGroup(false, aType, aPrefix, aOpen, aClose);
}

//--------------------------------------------------------------------------------
void Tags::Engine::appendCommon(Type::Enum aType,
	const QByteArray & aPrefix,
	const QByteArray & aOpen,
	const QByteArray & aClose)
{
	appendByGroup(true, aType, aPrefix, aOpen, aClose);
}

//--------------------------------------------------------------------------------
void Tags::Engine::set(Type::Enum aType)
{
	appendByGroup(false, aType, "", "", "");
}

//--------------------------------------------------------------------------------
QByteArray Tags::Engine::getTag(const TTypes & aTypes, const Direction::Enum aDirection) const
{
	if (aTypes.isEmpty())
	{
		return "";
	}

	STagData tagData = operator[](*aTypes.begin());
	bool isClose = aDirection == Direction::Close;

	// если не битовое поле или закрываем - склеиваем префикс и актив
	if ((aTypes.size() == 1) || isClose)
	{
		return tagData.prefix + (isClose ? tagData.close : tagData.open);
	}

	char tag = 0;

	foreach (Type::Enum type, aTypes)
	{
		if (mBuffer.contains(type))
		{
			tag |= operator[](type).open[0];
		}
	}

	return tagData.prefix + tag;
}

//--------------------------------------------------------------------------------
void Tags::Engine::splitForLexemes(const QString & aSource, TLexemesBuffer & aTagLexemes) const
{
	aTagLexemes.clear();
	QString source = aSource + Tags::None;

	QRegExp regExp(Tags::regExpData);
	regExp.setMinimal(true);

	int begin = 0;

	while (begin != -1)
	{
		begin = regExp.indexIn(source, begin);
		Tags::TTypes tags;
		QString lexeme;

		if (aTagLexemes.isEmpty())
		{
			//если 1-я лексема, то складываем в лист то, что до 1-го тега
			lexeme = (begin == -1) ? aSource : source.left(begin);
		}
		else if (begin != -1)
		{
			QStringList sourceParts = regExp.capturedTexts();

			Tags::Type::Enum type = Tags::Type::None;
			Tags::Direction::Enum direction;
			QString tagName = sourceParts[1];
			lexeme  = sourceParts[2];

			if (identifyTag(tagName, type, direction))
			{
				tags = aTagLexemes.last().tags;
				Tags::Type::Enum tagType = Tags::Types[tagName];

				if (direction == Tags::Direction::Open)
				{
					tags.insert(tagType);
				}
				else
				{
					//если закрываем тег и видим, что в пред. лексеме такого тега нет, значит, тег не был открыт;
					//тогда применяем этот тег ко всем предыдущим лексемам
					if (!aTagLexemes.isEmpty())
					{
						if (!aTagLexemes.last().tags.contains(tagType))
						{
							for (int i = 0; i < aTagLexemes.size(); ++i)
							{
								aTagLexemes[i].tags.insert(tagType);
							}
						}
					}

					tags.remove(tagType);
				}
			}
			else
			{
				QString nextTagName;
				bool isIdentify = identifyTag(sourceParts[3], type, direction);

				if (!isIdentify || (type != Tags::Type::None))
				{
					nextTagName = QString("[%1%2]")
						.arg(direction == Tags::Direction::Open ? "" : "/")
						.arg(sourceParts[3]);
				}

				//если тег не известен, то складываем его вместе с лексемой
				lexeme = QString("[%1]%2%3")
					.arg(tagName)
					.arg(lexeme)
					.arg(nextTagName);
			}

			begin += tagName.size() + lexeme.size();
		}

		aTagLexemes.push_back(Tags::SLexeme(lexeme, tags));
	}

	cleanLexemeBuffer(aTagLexemes);
}

//--------------------------------------------------------------------------------
void Tags::Engine::cleanLexemeBuffer(TLexemesBuffer & aTagLexemes) const
{
	for (int i = 0; i < aTagLexemes.size(); ++i)
	{
		if (i > 1)
		{
			if (aTagLexemes[i].tags == aTagLexemes[i - 1].tags)
			{
				aTagLexemes[i - 1].data += aTagLexemes[i].data;
				aTagLexemes.removeAt(i);
				i--;
			}
		}

		if (aTagLexemes[i].data.isEmpty())
		{
			aTagLexemes.removeAt(i);
			i--;
		}
	}
}

//--------------------------------------------------------------------------------
Tags::TGroupTypes Tags::Engine::groupsTypesByPrefix(const TTypes & aTypes) const
{
	TTypes types = aTypes;
	TGroupTypes result;

	foreach (Type::Enum type, aTypes)
	{
		if (mBuffer.contains(type))
		{
			if (!mBuffer[type].bitField)
			{
				TTypes singleTypes;
				singleTypes.insert(type);
				result.insert(singleTypes);
			}
			else
			{
				result.insert(mPrefixData.keys(mBuffer[type].prefix).toSet().intersect(types));
				types = aTypes;
			}
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
bool Tags::Engine::identifyTag(QString & aTag, Type::Enum & aType, Direction::Enum & aDirection) const
{
	aDirection = Direction::Open;

	if (aTag[0] == ASCII::ForwardSlash)
	{
		aDirection = Direction::Close;
		aTag = aTag.mid(1);
	}

	aType = Types[aTag];

	return aType != Type::None;
}

//--------------------------------------------------------------------------------
bool Tags::Engine::contains(Type::Enum aTag) const
{
	return mPrefixData.contains(aTag);
}

//--------------------------------------------------------------------------------
uint qHash(const Tags::TTypes & aTypes)
{
	uint result = 1;

	foreach (Tags::Type::Enum type, aTypes)
	{
		uint element = uint(type) + 2;

		switch (element)
		{
			case  6 : element = 17; break;
			case  8 : element = 19; break;
			case 10 : element = 23; break;
			case 12 : element = 27; break;
		}

		result *= element;
	}

	return result;
}

//--------------------------------------------------------------------------------
