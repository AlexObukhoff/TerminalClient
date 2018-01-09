/* @file Спецификация данных моделей USB-устройств для автопоиска. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QSharedPointer>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/Specifications.h"

//--------------------------------------------------------------------------------
namespace CUSBDevice
{
	/// Данные устройства.
	struct SData
	{
		QString model;
		bool verified;

		SData(): verified(false) {}
		SData(const QString & aModel, bool aVerified): model(aModel), verified(aVerified) {}
	};

	/// данные моделей по PID-ам.
	class CData : public CSpecification<quint32, SData>
	{
	public:
		void add(quint32 aPID, const QString & aModel, bool aVerified = false)
		{
			append(aPID, SData(aModel, aVerified));
		}

		void setDefaultModel(const QString & aModel)
		{
			setDefault(SData(aModel, false));
		}
	};

	/// Данные моделей по VID-ам.
	class CDetectingData : public CSpecification<quint32, CData>
	{
	public:
		void add(quint32 aVID, quint32 aPID, const QString & aModel, bool aVerified = false)
		{
			data()[aVID].append(aPID, SData(aModel, aVerified));
		}
	};

	typedef QSharedPointer<CDetectingData> PDetectingData;
}

//--------------------------------------------------------------------------------
