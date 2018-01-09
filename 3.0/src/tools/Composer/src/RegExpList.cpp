/* @file RegExp list class. */

#include "RegExpList.h" 

//---------------------------------------------------------------------------
void RegExpList::add(const QRegExp & aRegExp)
{
	mList.push_back(aRegExp);
}
//---------------------------------------------------------------------------
void RegExpList::add(const QString & aRegExp, QRegExp::PatternSyntax aSyntax /*= QRegExp::Wildcard*/)
{
	mList.push_back(QRegExp(aRegExp, Qt::CaseInsensitive, aSyntax));
}

//---------------------------------------------------------------------------
bool RegExpList::contains(const QString & aValue) const
{
	foreach (auto exp, mList)
	{
		if (exp.isValid() && exp.exactMatch(aValue))
		{
			return true;
		}
	}

	return false;
}
