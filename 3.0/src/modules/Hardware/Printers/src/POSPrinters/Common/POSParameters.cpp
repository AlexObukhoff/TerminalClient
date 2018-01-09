/* @file Параметры POS-принтеров. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMutexLocker>
#include <Common/QtHeadersEnd.h>

// Project
#include "POSParameters.h"

//--------------------------------------------------------------------------------
namespace POSPrinters
{

SModelData::SModelData()
{
}

//--------------------------------------------------------------------------------
SModelData::SModelData(const QString & aName, bool aVerified, const SParameters & aParameters, const QString & aDescription) :
	name(aName), verified(aVerified), description(aDescription)
{
	parameters.portSettings = aParameters.portSettings;
	parameters.errors = aParameters.errors;
	parameters.tagEngine = aParameters.tagEngine;
}

//--------------------------------------------------------------------------------
CModelData::CModelData()
{
}

//--------------------------------------------------------------------------------
void CModelData::add(char aModelId, bool aVerified, const QString & aName, const SParameters & aParameters, const QString & aDescription)
{
	QMutexLocker locker(&mMutex);

	append(aModelId, SModelData(aName, aVerified, aParameters, aDescription));
	mModelIds.insert(aModelId);
}

//--------------------------------------------------------------------------------
const TModelIds & CModelData::getModelIds()
{
	QMutexLocker locker(&mMutex);

	return mModelIds;
}

} // POSPrinters

//--------------------------------------------------------------------------------
