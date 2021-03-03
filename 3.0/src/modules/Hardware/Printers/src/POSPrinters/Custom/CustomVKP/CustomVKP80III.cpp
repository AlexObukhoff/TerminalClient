/* @file Принтер Custom VKP-80 III. */

#pragma once

#include "CustomVKP80III.h"

//--------------------------------------------------------------------------------
template class CustomVKP80III<TSerialPrinterBase>;

//--------------------------------------------------------------------------------
template<class T>
CustomVKP80III<T>::CustomVKP80III()
{
	mParameters = POSPrinters::CommonParameters;

	// параметры моделей
	setConfigParameter(CHardware::Printer::PresenterEnable, false);
	setConfigParameter(CHardware::Printer::RetractorEnable, false);

	// модели
	mDeviceName = "Custom VKP-80 III";
	mModelData.data().clear();
	mModelData.add("\x02\x05", true, mDeviceName);
}

//--------------------------------------------------------------------------------
template<class T>
bool CustomVKP80III<T>::updateParameters()
{
	setConfigParameter(CHardware::Printer::Settings::Loop, CHardwareSDK::Values::Use);

	return POSPrinter::updateParameters();
}

//--------------------------------------------------------------------------------
template<class T>
bool CustomVKP80III<T>::receiptProcessing()
{
	using namespace CHardware::Printer;
	using namespace CCustomVKP80III;

	int presentationLength = getConfigParameter(Settings::PresentationLength).toInt();
	QString ejectorActionParameter = getConfigParameter(Settings::PreviousAndNotTakenReceipts).toString();
	char ejectorAction = (ejectorActionParameter == Values::Retract) ? EjectorParameters::Retraction : EjectorParameters::Pushing;
	int leftReceiptTimeout =  0;

	if (ejectorActionParameter != CHardwareSDK::Values::Auto)
	{
		leftReceiptTimeout = getConfigParameter(Settings::LeftReceiptTimeout).toInt();
	}

	QByteArray command = Command::EjectorActivation;
	command.append(char(presentationLength));
	command.append(EjectorParameters::Blinking);
	command.append(ejectorAction);
	command.append(char(leftReceiptTimeout));

	return mIOPort->write(command);
}

//--------------------------------------------------------------------------------
