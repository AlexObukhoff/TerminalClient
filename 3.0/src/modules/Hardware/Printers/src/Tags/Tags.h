/* @file Движок тегов. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QByteArray>
#include <QtCore/QSet>
#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QSharedPointer>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/CodecDescriptions.h"
#include "Hardware/Common/Specifications.h"
#include "Hardware/Common/ASCII.h"

//--------------------------------------------------------------------------------
/// Теги.
namespace Tags
{
	/// BR.
	const char BR[] = "[br]";

	namespace Direction
	{
		enum Enum
		{
			Open,
			Close
		};
	}

	namespace Type
	{
		enum Enum
		{
			Bold,
			Italic,
			UnderLine,
			DoubleWidth,
			DoubleHeight,
			BarCode,
			Image,
			HR,
			Center,
			None
		};
	}

	typedef QSet<Type::Enum> TTypes;
	typedef QSet<TTypes> TGroupTypes;

	//--------------------------------------------------------------------------------
	class CTypes : public CSpecification<QString, Type::Enum>
	{
	public:
		CTypes()
		{
			append("b",    Type::Bold);
			append("i",    Type::Italic);
			append("u",    Type::UnderLine);
			append("dw",   Type::DoubleWidth);
			append("dh",   Type::DoubleHeight);
			append("bc",   Type::BarCode);
			append("img",  Type::Image);
			append("hr",   Type::HR);
			append("c",    Type::Center);
			append("none", Type::None);

			setDefault(Type::None);
		}
	};

	static CTypes Types;
	static QStringList Values(Types.data().keys());

	//--------------------------------------------------------------------------------
	const QString regExpData   = QString("%1(.*)%1").arg("\\[(/?[a-z]+)\\]");    /// Шаблон произвольного тега.
	const QString None = QString("[%1]").arg(Types.key(Type::None));          /// Открывающий тег none.

	struct SLexeme
	{
		QString data;
		TTypes tags;

		SLexeme() {}
		SLexeme(const QString & aData) : data(aData) {}
		SLexeme(const QString & aData, TTypes aTags) : tags(aTags), data(aData) {}
	};

	typedef QList<SLexeme> TLexemesBuffer;
	typedef QList<TLexemesBuffer> TLexemesCollection;
	typedef QList<TLexemesCollection> TLexemeReceipt;

	//--------------------------------------------------------------------------------
	struct STagData
	{
		QByteArray prefix;
		QByteArray open;
		QByteArray close;
		bool bitField;

		STagData() : bitField(false) {}
		STagData(bool aBitField, const QByteArray & aPrefix, const QByteArray & aOpen, const QByteArray & aClose) :
			bitField(aBitField), prefix(aPrefix), open(aOpen), close(aClose) {}
	};

	typedef QMap<Type::Enum, QByteArray> TPrefixData;
	typedef QMap<Type::Enum, STagData> TTagData;

	//--------------------------------------------------------------------------------
	class Engine: public CSpecification<Type::Enum, STagData>
	{
	public:
		/// Добавить в группу одиночный тег.
		void appendSingle(Type::Enum aType,
			const QByteArray & aPrefix,
			const QByteArray & aOpen,
			const QByteArray & aClose = QByteArray(1, ASCII::NUL));

		/// Получить в группу тег в виде битового поля.
		void appendCommon(Type::Enum aType,
			const QByteArray & aPrefix,
			const QByteArray & aOpen,
			const QByteArray & aClose = QByteArray(1, ASCII::NUL));

		/// Установить тег, который будет обработан особым образом.
		void set(Type::Enum aType);

		/// Получить массив байтов, соответствующий набору тегов и направлению тега (открывающий/закрывающий).
		QByteArray getTag(const TTypes & aTypes, const Direction::Enum aDirection) const;

		/// Ходим по буферу тегов, группируем по префиксам. Необходимо для тегов вида битовое поле.
		TGroupTypes groupsTypesByPrefix(const TTypes & aTypes) const;

		/// Ходим по исходной строке, ищем теги, и соответствующие им лексемы складываем в буфер.
		void splitForLexemes(const QString & aSource, TLexemesBuffer & aTagLexemes) const;

		/// Содержит ли экземпляр движка данный тег.
		bool contains(Type::Enum aTag) const;

	private:

		/// Добавить тег в группу.
		void appendByGroup(bool aBitField, Type::Enum aType,
			const QByteArray & aPrefix,
			const QByteArray & aOpen,
			const QByteArray & aClose);

		/// Чистим лист от 0-х лексем и одинаковых смежных тегов.
		void cleanLexemeBuffer(TLexemesBuffer & aTagLexemes) const;

		/// Идентифицируем тег по строке.
		bool identifyTag(QString & aTag, Type::Enum & aType, Direction::Enum & aDirection) const;

		/// Группы тегов (ранжированных по префиксам).
		TPrefixData mPrefixData;
	};

	typedef QSharedPointer<Engine> PEngine;
}

/// Вычисление хэша для сета енумов, принцип - перемножение простых чисел.
uint qHash(const Tags::TTypes & aTypes);

//--------------------------------------------------------------------------------
