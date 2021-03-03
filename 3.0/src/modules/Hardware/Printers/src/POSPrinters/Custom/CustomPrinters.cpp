/* @file Принтеры Custom. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// Project
#include "CustomPrinters.h"

//--------------------------------------------------------------------------------
template class CustomPrinter<TSerialPrinterBase>;

//--------------------------------------------------------------------------------
template <class T>
CustomPrinter<T>::CustomPrinter()
{
	mParameters = POSPrinters::CommonParameters;

	// статусы ошибок
	mParameters.errors.clear();

	mParameters.errors[20][3].insert('\x01', PrinterStatusCode::Error::PaperEnd);
	mParameters.errors[20][3].insert('\x04', PrinterStatusCode::Warning::PaperNearEnd);
	mParameters.errors[20][3].insert('\x20', PrinterStatusCode::OK::PaperInPresenter);
	//mParameters.errors[20][3].insert('\x40', PrinterStatusCode::Warning::PaperEndVirtual);

	mParameters.errors[20][4].insert('\x01', PrinterStatusCode::Error::PrintingHead);
	mParameters.errors[20][4].insert('\x02', DeviceStatusCode::Error::CoverIsOpened);

	mParameters.errors[20][5].insert('\x01', PrinterStatusCode::Error::Temperature);
	mParameters.errors[20][5].insert('\x04', PrinterStatusCode::Error::Port);
	mParameters.errors[20][5].insert('\x08', DeviceStatusCode::Error::PowerSupply);
	mParameters.errors[20][5].insert('\x40', PrinterStatusCode::Error::PaperJam);

	mParameters.errors[20][6].insert('\x01', PrinterStatusCode::Error::Cutter);
	mParameters.errors[20][6].insert('\x4C', DeviceStatusCode::Error::MemoryStorage);

	// теги
	mParameters.tagEngine.appendSingle(Tags::Type::Italic, "\x1B\x34", "\x01");
	mParameters.tagEngine.appendCommon(Tags::Type::DoubleWidth,  "\x1B\x21", "\x20");
	mParameters.tagEngine.appendCommon(Tags::Type::DoubleHeight, "\x1B\x21", "\x10");

	// параметры моделей
	mDeviceName = "Custom Printer";
	setModelID('\x93');
	mPrintingStringTimeout = 50;

	// модели
	mModelData.data().clear();
	mModelData.add('\x93', false, CCustomPrinter::Models::TG2480);
	mModelData.add('\xA7', false, CCustomPrinter::Models::TG2460H);
	mModelData.add('\xAC', false, CCustomPrinter::Models::TL80);
	mModelData.add('\xAD', false, CCustomPrinter::Models::TL60);

	setConfigParameter(CHardware::Printer::FeedingAmount, 1);
}

//--------------------------------------------------------------------------------
template <class T>
QStringList CustomPrinter<T>::getModelList()
{
	return QStringList()
		<< CCustomPrinter::Models::TG2480
		<< CCustomPrinter::Models::TG2460H
		<< CCustomPrinter::Models::TL80
		<< CCustomPrinter::Models::TL60;
}

//--------------------------------------------------------------------------------
template<class T>
bool CustomPrinter<T>::updateParameters()
{
	if (!POSPrinter<T>::updateParameters())
	{
		return false;
	}

	return mIOPort->write(CPOSPrinter::Command::SetNarrowFont);
}

//--------------------------------------------------------------------------------
template <class T>
void CustomPrinter<T>::setDeviceConfiguration(const QVariantMap & aConfiguration)
{
	POSPrinter::setDeviceConfiguration(aConfiguration);

	int lineSpacing = getConfigParameter(CHardware::Printer::Settings::LineSpacing).toInt();
	int feeding = (lineSpacing >= 60) ? 0 : 1;

	setConfigParameter(CHardware::Printer::FeedingAmount, feeding);
}

//--------------------------------------------------------------------------------
template <class T>
bool CustomPrinter<T>::printImageDefault(const QImage & aImage, const Tags::TTypes & aTags)
{
	int width = aImage.width();
	int height = aImage.height();
	int lines = qCeil(height / double(CCustomPrinter::LineHeight));
	int widthInBytes = qCeil(width / 8.0);

	if (width > CCustomPrinter::MaxImageWidth)
	{
		toLog(LogLevel::Warning, mDeviceName + QString(": Image width > %1, so it cannot be printing properly").arg(CCustomPrinter::MaxImageWidth));
		return false;
	}

	if (!mIOPort->write(CPOSPrinter::Command::SetLineSpacing(0)))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to set null line spacing for printing the image");
		return false;
	}

	bool result = true;

	for (int i = 0; i < lines; ++i)
	{
		QList<QByteArray> lineData;

		for (int j = 0; j < CCustomPrinter::LineHeight; ++j)
		{
			int index = i * CCustomPrinter::LineHeight + j;

			if (index < height)
			{
				lineData << QByteArray::fromRawData((const char *)aImage.scanLine(index), widthInBytes);
			}
			else
			{
				lineData << QByteArray(widthInBytes, ASCII::NUL);
			}
		}

		QByteArray request(CCustomPrinter::Commands::PrintImage);
		request.append(CCustomPrinter::Image24BitMode);
		request.append(uchar(width % 256));
		request.append(uchar(width / 256));

		for (int j = 0; j < width; ++j)
		{
			QByteArray data(3, ASCII::NUL);
			char mask = 1 << (7 - (j % 8));

			for (int k = 0; k < CCustomPrinter::LineHeight; ++k)
			{
				if (lineData[k][j / 8] & mask)
				{
					data[k / 8] = data[k / 8] | (1 << (7 - (k % 8)));
				}
			}

			request.append(data);
		}

		if (!mIOPort->write(request + ASCII::LF))
		{
			result = false;

			break;
		}
	}

	int lineSpacing = getConfigParameter(CHardware::Printer::Settings::LineSpacing).toInt();

	if (!mIOPort->write(CPOSPrinter::Command::SetLineSpacing(lineSpacing)))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to set line spacing after printing the image");
	}

	return result;
}

//--------------------------------------------------------------------------------
template <class T>
bool CustomPrinter<T>::printImage(const QImage & aImage, const Tags::TTypes & aTags)
{
	int width = aImage.width();

	if (width > CCustomPrinter::GAM::MaxImageWidth)
	{
		toLog(LogLevel::Warning, mDeviceName + QString(": Image width > %1, so it cannot be printing properly").arg(CCustomPrinter::GAM::MaxImageWidth));
		return false;
	}

	QByteArray initializeGAM = QByteArray() +
		CCustomPrinter::GAM::Commands::SetPageLength +
		CCustomPrinter::GAM::Commands::SetResolution204 +
		CCustomPrinter::GAM::Commands::SetNoCompression;

	int leftMargin = qFloor((CCustomPrinter::GAM::MaxImageWidth - width) / 2.0);

	if (aTags.contains(Tags::Type::Center) && (leftMargin > 0))
	{
		initializeGAM += CCustomPrinter::GAM::Commands::SetLeftMargin;
		initializeGAM.insert(initializeGAM.size() - 1, QString::number(leftMargin));
	}

	int sizeTotal = initializeGAM.size();
	QDateTime startDT = QDateTime::currentDateTime();

	if (!mIOPort->write(initializeGAM))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to initialize GAM");
		return false;
	}

	int widthInBytes = qCeil(aImage.width() / 8.0);

	for (int i = 0; i < aImage.height(); ++i)
	{
		QByteArray data((const char *)aImage.scanLine(i), widthInBytes);
		QByteArray command = CCustomPrinter::GAM::Commands::SendData + data;
		command.insert(3, QString::number(data.size()));

		sizeTotal += command.size();

		if (!mIOPort->write(command))
		{
			toLog(LogLevel::Error, mDeviceName + ": Failed to send image data");
			return false;
		}
	}

	sizeTotal += QByteArray(CCustomPrinter::GAM::Commands::PrintImage).size();

	if (!mIOPort->write(CCustomPrinter::GAM::Commands::PrintImage))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to set resolution 204 dpi");
		return false;
	}

	SDK::Driver::TPortParameters portParameters;
	mIOPort->getParameters(portParameters);
	qint64 pause = CCustomPrinter::GAM::getImagePause(aImage, portParameters);

	qint64 elapsed = startDT.msecsTo(QDateTime::currentDateTime());
	toLog(LogLevel::Normal, mDeviceName + QString(": Pause after printing image = %1 - %2 = %3 (ms)").arg(pause).arg(elapsed).arg(pause - elapsed));
	pause -= elapsed;

	if (pause > 0)
	{
		SleepHelper::msleep(int(pause));
	}

	return true;
}

//--------------------------------------------------------------------------------
