/* @file Типы данных моделей устройств. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

//--------------------------------------------------------------------------------
/// Данные устройства.
struct SModelDataBase
{
	QString model;
	bool verified;

	SModelDataBase(): verified(false) {}
	SModelDataBase(const QString & aModel, bool aVerified): model(aModel), verified(aVerified) {}
};

//--------------------------------------------------------------------------------
