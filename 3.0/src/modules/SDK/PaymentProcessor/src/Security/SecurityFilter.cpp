/* @file Класс-фильтр содержимого полей ввода пользователя. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

// Project
#include "../Provider.h"
#include "SecurityFilter.h"

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
SecurityFilter::SecurityFilter(const SProvider & aProvider, SProviderField::SecuritySubsystem aSubsustem) :
	mProvider(aProvider),
	mSubsustem(aSubsustem)
{
}

//------------------------------------------------------------------------------
bool SecurityFilter::haveFilter(const QString & aParameterName) const
{
	QRegExp regExp = getMask(aParameterName);

	return !regExp.isEmpty() && regExp.isValid();
}

//------------------------------------------------------------------------------
QString SecurityFilter::apply(const QString & aParameterName, const QString & aValue) const
{
	QRegExp regExp = getMask(aParameterName);

	if (!regExp.isEmpty() && regExp.isValid())
	{
		if (regExp.indexIn(aValue) > -1)
		{
			QString value = aValue;

			for (int i = 1; i < regExp.capturedTexts().size(); i++)
			{
				int capSize = regExp.cap(i).size();
				value.replace(regExp.pos(i), capSize, QString("*").repeated(capSize));
			}

			return value;
		}
		else
		{
			qDebug() << QString("RegExp '%1' not found value in: %2").arg(regExp.pattern()).arg(aValue);
		}
	}

	return aValue;
}

//------------------------------------------------------------------------------
QRegExp SecurityFilter::getMask(const QString & aParameterName) const
{
	foreach(auto field, mProvider.fields)
	{
		if (aParameterName.contains(field.id, Qt::CaseInsensitive))
		{
			auto subsystem = field.security.contains(mSubsustem) ? mSubsustem : SProviderField::Default;
			QString regExp = field.security.value(subsystem, QString());

			if (!regExp.isEmpty())
			{
				QRegExp rx(regExp);
				//rx.setMinimal(true);

				if (rx.isValid())
				{
					return rx;
				}

				qDebug() << QString("RegExp '%1' not valid: %2.").arg(regExp).arg(rx.errorString());
			}
		}
	}

	return QRegExp();
}

//------------------------------------------------------------------------------
}} // PaymentProcessor::SDK
