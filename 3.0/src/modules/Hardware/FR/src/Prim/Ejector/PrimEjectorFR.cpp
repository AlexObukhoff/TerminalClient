/* @file Базовый ФР ПРИМ c эжектором. */

// Modules
#include "Hardware/Printers/CustomVKP80.h"
#include "Hardware/Printers/POSPrinterData.h"
#include "PrimEjectorFRConstants.h"
#include "../PrimModelData.h"

// Project
#include "PrimEjectorFR.h"

//--------------------------------------------------------------------------------
template class PrimEjectorFR<PrimFRBase>;
template class PrimEjectorFR<PrimOnlineFRBase>;

//--------------------------------------------------------------------------------
template <class T>
PrimEjectorFR<T>::PrimEjectorFR()
{
	// данные устройства
	setConfigParameter(CHardware::Printer::RetractorEnable, true);

	mOldBuildNumber = false;
	mPrinter = PPrinter(new CustomVKP80());
	mDeviceName = CPrimFR::ModelData[CPrimFR::Models::PRIM_21K_03].name;
	mLPC22RetractorErrorCount = 0;
}

//--------------------------------------------------------------------------------
template <class T>
bool PrimEjectorFR<T>::updateParameters()
{
	return PrimPresenterFR<T>::updateParameters() && setPresentationMode() && checkPresentationLength();
}

//--------------------------------------------------------------------------------
template <class T>
bool PrimEjectorFR<T>::checkPresentationLength()
{
	if (!PrimFRBase::checkParameters())
	{
		return false;
	}

	CPrimFR::TData data;

	if (processCommand(CPrimFR::Commands::GetMoneyBoxSettings, &data) && (data.size() >= 8))
	{
		bool OK;
		int presentationLength = getConfigParameter(CHardware::Printer::Settings::PresentationLength).toInt();
		int FRPresentationLength = data[5].toInt(&OK, 16);

		if (!OK || (presentationLength == FRPresentationLength))
		{
			return OK;
		}

		data = data.mid(5);
		data[0] = int2ByteArray(presentationLength);

		return processCommand(CPrimFR::Commands::SetMoneyBoxSettings, data);
	}

	return false;
}

//---------------------------------------------------------------------------
template <class T>
void PrimEjectorFR<T>::cleanStatusCodes(TStatusCodes & aStatusCodes)
{
	if (mOldBuildNumber)
	{
		aStatusCodes.insert(DeviceStatusCode::Warning::Firmware);
	}

	if (aStatusCodes.contains(PrinterStatusCode::Warning::PaperEndVirtual))
	{
		aStatusCodes.remove(PrinterStatusCode::Warning::PaperEndVirtual);
		aStatusCodes.insert(PrinterStatusCode::Warning::PaperNearEnd);
	}

	TSerialFRBase::cleanStatusCodes(aStatusCodes);
}

//--------------------------------------------------------------------------------
template <class T>
bool PrimEjectorFR<T>::processAnswer(char aError)
{
	if (((aError == CPrimFR::Errors::IncorrigibleError) || (aError == CPrimFR::Errors::NotReadyForPrint)) && (mLPC22RetractorErrorCount < CPrimFR::MaxRepeat::RetractorError))
	{
		mProcessingErrors.push_back(aError);

		mLPC22RetractorErrorCount++;

		toLog(LogLevel::Normal, "Abnormal error, try to reset printer");

		return processEjectorAction(CPrimEjectorFRActions::Reset);
	}

	if (aError && (mLPC22RetractorErrorCount >= CPrimFR::MaxRepeat::RetractorError) &&
		(aError != CPrimFR::Errors::IncorrigibleError) &&
		(aError != CPrimFR::Errors::NotReadyForPrint))
	{
		toLog(LogLevel::Normal, "Reset Abnormal error count");
		mLPC22RetractorErrorCount = 0;
	}

	return PrimPresenterFR<T>::processAnswer(aError);
}

//--------------------------------------------------------------------------------
template <class T>
bool PrimEjectorFR<T>::push()
{
	if (mMode == EFRMode::Printer)
	{
		return TSerialFRBase::push();
	}

	return processEjectorAction(CPrimEjectorFRActions::Push);
}

//--------------------------------------------------------------------------------
template <class T>
bool PrimEjectorFR<T>::retract()
{
	if (mMode == EFRMode::Printer)
	{
		return TSerialFRBase::retract();
	}

	return processEjectorAction(CPrimEjectorFRActions::Retract);
}

//--------------------------------------------------------------------------------
template <class T>
bool PrimEjectorFR<T>::setPresentationMode()
{
	QString loop = getConfigParameter(CHardware::Printer::Settings::Loop).toString();

	if (loop == CHardwareSDK::Values::Auto)
	{
		return true;
	}

	bool loopEnable = loop == CHardwareSDK::Values::Use;

	if (mMode == EFRMode::Printer)
	{
		return mIOPort->write(loopEnable ? CPOSPrinter::Command::LoopEnable : CPOSPrinter::Command::LoopDisable);
	}

	return processEjectorAction(loopEnable ? CPrimEjectorFRActions::SetLoopEnabled : CPrimEjectorFRActions::SetLoopDisabled);
}

//--------------------------------------------------------------------------------
template <class T>
bool PrimEjectorFR<T>::present()
{
	return (mMode == EFRMode::Printer) && TSerialFRBase::present();
}

//--------------------------------------------------------------------------------
template <class T>
bool PrimEjectorFR<T>::processEjectorAction(const QString & aAction)
{
	CPrimFR::TData commandData;

	for (int i = 0; i < aAction.size(); ++i)
	{
		commandData << int2ByteArray(QString(aAction[i]).toInt(0, 16));
	}

	return processCommand(CPrimFR::Commands::SetEjectorAction, commandData);
}

//--------------------------------------------------------------------------------
