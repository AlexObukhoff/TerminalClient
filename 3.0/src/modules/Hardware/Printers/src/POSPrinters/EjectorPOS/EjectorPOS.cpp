/* @file POS-принтеры  с эжектором. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtGlobal>
#include <Common/QtHeadersEnd.h>

// Project
#include "EjectorPOS.h"

using namespace SDK::Driver::IOPort::COM;
using namespace PrinterStatusCode;

//--------------------------------------------------------------------------------
template class EjectorPOS<TSerialPrinterBase>;
template class EjectorPOS<TLibUSBPrinterBase>;

//--------------------------------------------------------------------------------
template <class T>
EjectorPOS<T>::EjectorPOS()
{
	// данные устройства
	mDeviceName = "POS Printer with ejector";

	setConfigParameter(CHardware::Printer::PresenterEnable, true);
	setConfigParameter(CHardware::Printer::RetractorEnable, true);

	setConfigParameter(CHardware::Printer::Commands::Presentation, CPOSPrinter::Command::Present);
	setConfigParameter(CHardware::Printer::Commands::Pushing,      CPOSPrinter::Command::Push);
	setConfigParameter(CHardware::Printer::Commands::Retraction,   CPOSPrinter::Command::Retract);
}

//--------------------------------------------------------------------------------
template <class T>
void EjectorPOS<T>::setDeviceConfiguration(const QVariantMap & aConfiguration)
{
	POSPrinter::setDeviceConfiguration(aConfiguration);

	if (containsConfigParameter(CHardware::Printer::Settings::PresentationLength))
	{
		int presentationLength = qMax(CEjectorPOS::MinPresentationLength, getConfigParameter(CHardware::Printer::Settings::PresentationLength).toInt());
		setConfigParameter(CHardware::Printer::Settings::PresentationLength, presentationLength);
	}
}

//--------------------------------------------------------------------------------
template <class T>
bool EjectorPOS<T>::updateParameters()
{
	if (!POSPrinter::updateParameters())
	{
		return false;
	}

	QString loop = getConfigParameter(CHardware::Printer::Settings::Loop).toString();

	if (loop == CHardwareSDK::Values::Auto)
	{
		return true;
	}

	return mIOPort->write((loop == CHardwareSDK::Values::Use) ? CPOSPrinter::Command::LoopEnable : CPOSPrinter::Command::LoopDisable);
}

//--------------------------------------------------------------------------------
