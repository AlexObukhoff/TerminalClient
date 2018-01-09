/* @file Параметры POS-принтеров. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMutex>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/IOPort/COMParameters.h>

// Project
#include "Hardware/Common/Specifications.h"
#include "Hardware/Common/HardwareConstants.h"
#include "Hardware/Printers/PrinterStatusCodes.h"
#include "Hardware/Printers/Tags.h"

//--------------------------------------------------------------------------------
/// Типы данных POS-принтеров.
namespace POSPrinters
{
	/// Ошибки принтера.
	typedef QMap<char, int> TErrors;
	typedef QMap<char, TErrors> TCustomErrors;
	typedef CSpecification<char, TCustomErrors> CAllErrors;
	typedef QMap<char, TCustomErrors> TAllErrors;
	typedef QSharedPointer<CAllErrors> PAllErrors;

	typedef QList<int> TSerialDevicePortParameter;
	typedef QMap<int, TSerialDevicePortParameter> TSerialDevicePortParameters;
	typedef CSpecification<int, TSerialDevicePortParameter> CSerialDevicePortParameters;
	typedef QSharedPointer<CSerialDevicePortParameters> PSerialDevicePortParameters;

	struct SParameters
	{
		PSerialDevicePortParameters portSettings;
		PAllErrors errors;
		Tags::PEngine tagEngine;

		SParameters() {}
		SParameters(const PSerialDevicePortParameters & aPortSettings, const PAllErrors & aErrors, const Tags::PEngine & aTagEngine) :
			portSettings(aPortSettings), errors(aErrors), tagEngine(aTagEngine) {}
	};

	struct SModelData
	{
		QString name;
		bool verified;
		QString description;
		SParameters parameters;

		SModelData();
		SModelData(const QString & aName, bool aVerified, const SParameters & aParameters, const QString & aDescription);
	};

	typedef QMap<char, SModelData> TModelData;
	typedef QSet<char> TModelIds;

	class CModelData : public CSpecification<char, SModelData>
	{
	public:
		CModelData();
		void add(char aModelId, bool aVerified, const QString & aName, const SParameters & aParameters, const QString & aDescription = "");
		const TModelIds & getModelIds();

	private:
		Q_DISABLE_COPY(CModelData)

		static TModelIds mModelIds;
		QMutex mMutex;
	};
}

//--------------------------------------------------------------------------------
