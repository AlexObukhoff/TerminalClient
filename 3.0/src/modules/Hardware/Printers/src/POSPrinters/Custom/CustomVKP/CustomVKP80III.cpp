/* @file Принтер Custom VKP-80 III. */

#pragma once

#include "CustomVKP80III.h"

//--------------------------------------------------------------------------------
CustomVKP80III::CustomVKP80III()
{
	// параметры моделей
	mDeviceName = "Custom VKP-80 III";
	mModelID = '\xFF';

	// модели
	POSPrinters::SParameters parameters(mModelData.getDefault().parameters);
	mModelData.data().clear();
	mModelData.add(mModelID, true, mDeviceName, parameters);

	setConfigParameter(CHardware::Printer::PresenterEnable, false);
	setConfigParameter(CHardware::Printer::RetractorEnable, false);
}

//--------------------------------------------------------------------------------
bool CustomVKP80III::getModelId(QByteArray & aAnswer) const
{
	return mIOPort->write(CCustomVKP80III::Command::GetModelId) && getAnswer(aAnswer, CPOSPrinter::Timeouts::Info) && (aAnswer.isEmpty() || (aAnswer.size() == 2));
}

//--------------------------------------------------------------------------------
bool CustomVKP80III::updateParameters()
{
	setConfigParameter(CHardware::Printer::Settings::Loop, CHardwareSDK::Values::Use);

	return POSPrinter::updateParameters();
}

//--------------------------------------------------------------------------------
char CustomVKP80III::parseModelId(QByteArray & aAnswer)
{
	if (aAnswer != CCustomVKP80III::ModelId)
	{
		toLog(LogLevel::Error, QString("%1: Wrong answer {%2}, need {%3}").arg(mDeviceName).arg(aAnswer.toHex().data()).arg(QByteArray(CCustomVKP80III::ModelId).toHex().data()));
		return 0;
	}

	return mModelID;
}

//--------------------------------------------------------------------------------
bool CustomVKP80III::receiptProcessing()
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
