/* @file Спецификация данных моделей USB-устройств для автопоиска. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QSharedPointer>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/Specifications.h"
#include "Hardware/Common/USBDeviceModelDataTypes.h"
#include "Hardware/Common/USBDeviceVendors.h"

//--------------------------------------------------------------------------------
namespace CUSBDevice
{
	/// данные моделей по PID-ам.
	template <class T>
	class ProductDataBase : public CSpecification<quint16, T>
	{
	public:
		QStringList getModelList(const QString & aVendor);
		void setDefaultModel(const QString & aModel);
		TProductData getProductData();

	protected:
		TProductData mProductData;
	};

	//--------------------------------------------------------------------------------
	class ProductData: public ProductDataBase<SProductData>
	{
	public:
		void add(quint16 aPID, const QString & aModel, bool aVerified = false)
		{
			append(aPID, SProductData(aModel, aVerified));
		}
	};

	//--------------------------------------------------------------------------------
	/// Данные моделей по VID-ам.
	class DetectingData : public CSpecification<quint16, ProductData>
	{
	public:
		void set(const QString & aVendor, quint16 aPID, const QString & aModel, bool aVerified = false);
		void set(const QString & aVendor, const QString & aDeviceName, quint16 aPID);
		void set(const SDetectingData & aDetectingData);
		void set(const QString & aVendor, const TProductData & aProductData);

	protected:
		static CUSBVendors::Data mVendorData;
	};

	typedef QSharedPointer<DetectingData> PDetectingData;
}

//--------------------------------------------------------------------------------
