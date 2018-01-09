/* @file Структура, хранящая информацию о pin-карте. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

//---------------------------------------------------------------------------
struct SPinCard
{
	QString id;
	QString name;
	double amount;
	QStringList fields;

	SPinCard() : amount(0.0) {}
};

//---------------------------------------------------------------------------
