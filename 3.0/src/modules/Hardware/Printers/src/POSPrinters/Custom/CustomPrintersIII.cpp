/* @file Принтеры Custom модификации III. */

#pragma once

#include "CustomPrintersIII.h"

//--------------------------------------------------------------------------------
template class CustomPrinterIII<TSerialPrinterBase>;

//--------------------------------------------------------------------------------
template<class T>
CustomPrinterIII<T>::CustomPrinterIII()
{
	// параметры моделей
	mPrintingStringTimeout = 0;
	setConfigParameter(CHardware::Printer::FeedingAmount, 0);

	// модели
	mDeviceName = "Custom printer III";

	mModelData.data().clear();
	mModelData.add("\x02\x45", false, CCustomPrinterIII::Models::TG2460HIII);
	mModelData.add("\x02\x53", false, CCustomPrinterIII::Models::TG2460HIIIEJC);
	mModelData.add("\x02\x55", false, CCustomPrinterIII::Models::TG1260HIII);
	mModelData.add("\x02\x51", false, CCustomPrinterIII::Models::TL80III);
	mModelData.add("\x02\x50", false, CCustomPrinterIII::Models::TL60III);
}

//--------------------------------------------------------------------------------
template<class T>
bool CustomPrinterIII<T>::updateParameters()
{
	if (!CustomPrinter<T>::updateParameters())
	{
		return false;
	}

	if (!mIOPort->write(CCustomPrinterIII::Command::SetTGHEmulation))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to set TGH emulation");
		return false;
	}

	// команды баланса скорость/качество печати. Нет разницы ни по скорости, ни по качеству
	//mIOPort->write(QByteArray::fromRawData("\x1B\x78\x01", 3));
	//mIOPort->write(QByteArray::fromRawData("\x1D\xF0\x00", 3));

	return mIOPort->write(CCustomPrinterIII::Command::EnableJammingSensor);
}

//--------------------------------------------------------------------------------
template<class T>
void CustomPrinterIII<T>::processDeviceData()
{
	CustomPrinter<T>::processDeviceData();

	QByteArray answer;

	if (mIOPort->write(CCustomPrinterIII::Command::GetSerialNumber) && getAnswer(answer, CPOSPrinter::Timeouts::Info))
	{
		setDeviceParameter(CDeviceData::SerialNumber, answer);
	}
}

//--------------------------------------------------------------------------------
template<class T>
bool CustomPrinterIII<T>::receiptProcessing()
{
	bool result = CustomPrinter<T>::receiptProcessing();

	//непонятно, насколько нужно делать это после каждой печати
	//mIOPort->write(CCustomPrinterIII::Command::PerformAntiJammingAction);

	return result;
}

//--------------------------------------------------------------------------------
