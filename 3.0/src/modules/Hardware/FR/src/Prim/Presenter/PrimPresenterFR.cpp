/* @file ФР ПРИМ c презентером. */

// Modules
#include "Hardware/Printers/EpsonEUT400.h"
#include "../PrimModelData.h"

// Project
#include "PrimPresenterFR.h"

//--------------------------------------------------------------------------------
template class PrimPresenterFR<PrimFRBase>;
template class PrimPresenterFR<PrimOnlineFRBase>;

//--------------------------------------------------------------------------------
// Получить модели данной реализации.
namespace CPrimFR { inline TModels PresenterModels()
{
	return TModels()
		<< CPrimFR::Models::PRIM_21K_01
		<< CPrimFR::Models::PRIM_21K_02;
}}

//--------------------------------------------------------------------------------
template <class T>
PrimPresenterFR<T>::PrimPresenterFR()
{
	// данные устройства
	setConfigParameter(CHardware::Printer::PresenterEnable, true);
	setConfigParameter(CHardware::Printer::BCMaxSize, 17);
	mPrinter = PPrinter(new EpsonEUT400());
	mDeviceName = CPrimFR::ModelData[CPrimFR::Models::PRIM_21K_02].name;
	mModels = CPrimFR::PresenterModels();
}

//--------------------------------------------------------------------------------
template <class T>
QStringList PrimPresenterFR<T>::getModelList()
{
	return CPrimFR::getModelList(CPrimFR::PresenterModels());
}

//--------------------------------------------------------------------------------
template <class T>
bool PrimPresenterFR<T>::performReceipt(const QStringList & aReceipt, bool aProcessing)
{
	using namespace CHardware::Printer;

	QVariantMap config = getDeviceConfiguration();
	config.insert(OutCall, true);

	// иначе прошивка ФР может воспринять это как DLE-команду
	int presentationLength = config.value(Settings::PresentationLength).toInt();

	if (presentationLength == int(ASCII::DLE))
	{
		config.insert(Settings::PresentationLength, ++presentationLength);
	}

	SPrintingOutData printingOutData(mLog, aProcessing, mIOMessageLogging, config, aReceipt);

	return mPrinter->printOut(printingOutData);
}

//--------------------------------------------------------------------------------
