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
SModelData::SModelData(const QString & aName, bool aVerified, const QString & aDescription) : name(aName), verified(aVerified), description(aDescription)
{
}

//--------------------------------------------------------------------------------
ModelData::ModelData(): mMutex(QMutex::Recursive)
{
}

//--------------------------------------------------------------------------------
void ModelData::add(const QByteArray & aModelId, bool aVerified, const QString & aName, const QString & aDescription)
{
	QMutexLocker locker(&mMutex);

	append(aModelId, SModelData(aName, aVerified, aDescription));
}

//--------------------------------------------------------------------------------
void ModelData::add(char aModelId, bool aVerified, const QString & aName, const QString & aDescription)
{
	add(QByteArray(1, aModelId), aVerified, aName, aDescription);
}

//--------------------------------------------------------------------------------
int ModelData::getIdMaxSize()
{
	QMutexLocker locker(&mMutex);

	int result = 0;
	auto Ids = data().keys();

	foreach (auto id, Ids)
	{
		result = qMax(result, id.size());
	}

	return result;
}

} // POSPrinters

//--------------------------------------------------------------------------------
