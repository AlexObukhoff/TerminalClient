/* @file RegExp list class. */
#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QList>
#include <QtCore/QRegExp>
#include <Common/QtHeadersEnd.h>

//---------------------------------------------------------------------------
class RegExpList
{
	QList<QRegExp> mList;
public:

	/// Добавить новое выражение
	void add(const QRegExp & aRegExp);

	/// Добавить новое выражение
	void add(const QString & aRegExp, QRegExp::PatternSyntax aSyntax = QRegExp::Wildcard);

	/// проверить вхождение в список
	bool contains(const QString & aValue) const;
};