/* @file Принтер Custom VKP-80 III. */

#pragma once

#include "CustomVKP80III.h"

//--------------------------------------------------------------------------------
template class CustomVKP80III<TSerialPrinterBase>;

//--------------------------------------------------------------------------------
template<class T>
CustomVKP80III<T>::CustomVKP80III()
{
	// параметры моделей
	mDeviceName = "Custom VKP-80 III";
	mModelID = '\xFF';

	// модели
	mParameters = POSPrinters::CommonParameters;
	mModelData.data().clear();
	mModelData.add(mModelID, true, mDeviceName);

	setConfigParameter(CHardware::Printer::PresenterEnable, false);
	setConfigParameter(CHardware::Printer::RetractorEnable, false);
}

//--------------------------------------------------------------------------------
template<class T>
bool CustomVKP80III<T>::getModelId(QByteArray & aAnswer) const
{
	return mIOPort->write(CCustomVKP80III::Command::GetModelId) && getAnswer(aAnswer, CPOSPrinter::Timeouts::Info) && (aAnswer.isEmpty() || (aAnswer.size() == 2));
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
char CustomVKP80III<T>::parseModelId(QByteArray & aAnswer)
{
	if (aAnswer != CCustomVKP80III::ModelId)
	{
		toLog(LogLevel::Error, QString("%1: Wrong answer {%2}, need {%3}").arg(mDeviceName).arg(aAnswer.toHex().data()).arg(QByteArray(CCustomVKP80III::ModelId).toHex().data()));
		return 0;
	}

	return mModelID;
}

//--------------------------------------------------------------------------------
template<class T>
bool CustomVKP80III<T>::receiptProcessing()
{
	int presentationLength = getConfigParameter(CHardware::Printer::Settings::PresentationLength).toInt();
	QString ejectorActionParameter = getConfigParameter(CHardware::Printer::Settings::PreviousAndNotTakenReceipts).toString();
	char ejectorAction = (ejectorActionParameter == CHardware::Printer::Values::Retract) ? CCustomVKP80III::Retraction : CCustomVKP80III::Pushing;
	int leftReceiptTimeout =  0;

	if (ejectorActionParameter != CHardwareSDK::Values::Auto)
	{
		leftReceiptTimeout = getConfigParameter(CHardware::Printer::Settings::LeftReceiptTimeout).toInt();
	}

	QByteArray command = CCustomVKP80III::Command::EjectorActivation;
	command.append(char(presentationLength));
	command.append(CCustomVKP80III::Blinking);
	command.append(ejectorAction);
	command.append(char(leftReceiptTimeout));

	return mIOPort->write(command);
}

//--------------------------------------------------------------------------------
