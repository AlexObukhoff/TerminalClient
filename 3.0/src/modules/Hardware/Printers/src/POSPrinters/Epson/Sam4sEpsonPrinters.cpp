/* @file Принтеры Sam4s на эмуляции Epson. */

#pragma once

#include "Sam4sEpsonPrinters.h"
#include "Hardware/Printers/Sam4sModels.h"

//--------------------------------------------------------------------------------
template class Sam4sEpsonPrinter<TSerialPOSPrinter>;
template class Sam4sEpsonPrinter<TTCPPOSPrinter>;

//--------------------------------------------------------------------------------
template <class T>
Sam4sEpsonPrinter<T>::Sam4sEpsonPrinter()
{
	// параметры моделей
	mDeviceName = "Sam4s by Epson printer";
	setModelID('\x20');
	mDeviceDataTimeout = 50;
	setConfigParameter(CHardware::Printer::FeedingAmount, 3);

	// модели
	mModelData.data().clear();
	mModelData.add('\x20', true, CSam4s::Models::Ellix50);
}

//--------------------------------------------------------------------------------
template <class T>
QStringList Sam4sEpsonPrinter<T>::getModelList()
{
	return QStringList()
		<< CSam4s::Models::Ellix50;
}

//--------------------------------------------------------------------------------
template <class T>
void Sam4sEpsonPrinter<T>::setDeviceConfiguration(const QVariantMap & aConfiguration)
{
	EpsonPrinter<T>::setDeviceConfiguration(aConfiguration);

	int lineSpacing = getConfigParameter(CHardware::Printer::Settings::LineSpacing, 0).toInt();

	int feeding = 3;
	     if (lineSpacing >= 78) feeding = 1;
	else if (lineSpacing >= 52) feeding = 2;

	setConfigParameter(CHardware::Printer::FeedingAmount, feeding);
}

//--------------------------------------------------------------------------------
