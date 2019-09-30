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
		TAllErrors errors;
		Tags::Engine tagEngine;

		SParameters()
		{
			// теги по умолчанию
			tagEngine.appendSingle(Tags::Type::Bold, "\x1B\x45", "\x01");
			tagEngine.appendSingle(Tags::Type::UnderLine, "\x1B\x2D", "\x01");
			tagEngine.set(Tags::Type::Image);
			tagEngine.set(Tags::Type::BarCode);

			// статусы TODO: корректировать при добавлении моделей
			errors[1][1].insert('\x08', DeviceStatusCode::Error::Unknown);
			errors[2][1].insert('\x20', PrinterStatusCode::Error::PaperEnd);
			errors[2][1].insert('\x40', DeviceStatusCode::Error::Unknown);
			errors[3][1].insert('\x60', DeviceStatusCode::Error::Unknown);
		}
	};

	/// Параметры по умолчанию.
	const SParameters CommonParameters;

	struct SModelData
	{
		QString name;
		bool verified;
		QString description;

		SModelData();
		SModelData(const QString & aName, bool aVerified, const QString & aDescription);
	};

	typedef QMap<char, SModelData> TModelData;
	typedef QSet<char> TModelIds;

	class ModelData : public CSpecification<char, SModelData>
	{
	public:
		ModelData();
		void add(char aModelId, bool aVerified, const QString & aName, const QString & aDescription = "");
		const TModelIds & getModelIds();

	private:
		Q_DISABLE_COPY(ModelData)

		static TModelIds mModelIds;
		QMutex mMutex;
	};
}

//--------------------------------------------------------------------------------
