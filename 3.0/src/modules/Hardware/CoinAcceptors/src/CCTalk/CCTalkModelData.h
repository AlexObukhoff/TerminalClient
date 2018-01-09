/* @file Данные моделей устройств на протоколе ccTalk. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <QtCore/QByteArray>
#include <QtCore/QSet>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/CashAcceptor/CurrencyList.h>

// Project
#include "Hardware/Common/Specifications.h"

//--------------------------------------------------------------------------------
namespace CCCTalk
{
	struct SVendorData
	{
		QString vendor;
		QString shortVendor;
		bool complexEnabling;

		SVendorData() : complexEnabling(false) {}
		SVendorData(const QString & aVendor, const QString & aShortVendor, bool aComplexEnabling = false) :
			vendor(aVendor), shortVendor(aShortVendor), complexEnabling(aComplexEnabling) {}
	};

	typedef QMap<QString, SVendorData>::iterator TVendorDataIt;

	class CVendorData : public CSpecification<QString, SVendorData>
	{
	public:
		CVendorData();
		QString getName(const QString & aId);

	private:
		void add(const QString & aVendorID, const QString & aVendor, const QString & aShortVendor = "", bool aComplexEnabling = false);
	};

	static CVendorData VendorData;

	//--------------------------------------------------------------------------------
	const char DefaultDeviceName[] = "Unknown coinacceptor based on ccTalk protocol";    /// Название устройства по умолчанию.
	const char DeviceNamePostfix[] = "coinacceptor, unknown model based on ccTalk protocol";    /// Оконание названия устройства известного производителя по умолчанию.

	/// Производители монетников, номиналы прошивки которых нуждаются в коррекции.
	const QStringList WrongFirmwareVendors = QStringList() << "NRI" << "Microcoin" << "ICT";

	//--------------------------------------------------------------------------------
	typedef QMap<int, double> TMinVersions;
	typedef QSet<uchar> TUnsupported;

	struct SModelData
	{
		TMinVersions minVersions;    /// минимальные версии в зависимости от валюты.
		TUnsupported unsupported;    /// неподдерживаемые команды.
		uchar fault;      /// неисправность при нажатиии на кнопку режекта.
		uchar error;      /// ошибка при нажатиии на кнопку режекта.
		QString model;    /// модель.
		bool verified;    /// модель протестирована.

		SModelData() : fault(0), error(0), verified(false) {}
		SModelData(uchar aFault, uchar aError, const QString & aModel, bool aVerified) : fault(aFault), error(aError), model(aModel), verified(aVerified) {}
	};

	//--------------------------------------------------------------------------------
	typedef QMap<QString, SModelData> TModelData;
	typedef QMap<QString, TModelData>::iterator TModelDataIt;

	class CModelData : public CSpecification<QString, TModelData>
	{
	public:
		CModelData();

	protected:
		void add(const QString & aVendorId, const QString & aModelId, uchar aFault = 0, uchar aError = 0, const QString & aModel = "", bool aVerified = true);
	};

	static CModelData ModelData;

	//--------------------------------------------------------------------------------
	QString getModelData(const QByteArray & aVendorID, const QByteArray & aModelID, SModelData & aModelData);
	QStringList getModels(bool aComplexEnabling);
}

//--------------------------------------------------------------------------------
