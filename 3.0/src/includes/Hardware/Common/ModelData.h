/* @file Данные моделей устройств. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

// Proejct
#include "Hardware/Common/Specifications.h"

//--------------------------------------------------------------------------------
struct SBaseModelData
{
	QString name;
	bool verified;
	bool updatable;

	SBaseModelData(const QString & aName = "") : name(aName), verified(false), updatable(false) {}
	SBaseModelData(const QString & aName, bool aVerified, bool aUpdatable = false) : name(aName), verified(aVerified), updatable(aUpdatable) {}
};

//--------------------------------------------------------------------------------
template <class T>
class CBaseModelData : public CSpecification<T, SBaseModelData>
{
protected:
	void add(const T & aDeviceId, const QString & aModel, bool aVerified = false, bool aUpdatable = false)
	{
		append(aDeviceId, SBaseModelData(aModel, aVerified, aUpdatable));
	}
};

//--------------------------------------------------------------------------------
template<class T>
inline QStringList getModels()
{
	QSet<QString> result;

	foreach (SBaseModelData aData, T().data().values())
	{
		result << aData.modelName;
	}

	return result.toList();
}

//--------------------------------------------------------------------------------
