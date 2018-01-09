/* @file Базовый ФР АТОЛ c эжектором. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// Project
#include "AtolEjectorFR.h"
#include "AtolEjectorFRConstants.h"

//--------------------------------------------------------------------------------
template class AtolEjectorFR<AtolSerialFR>;
template class AtolEjectorFR<AtolOnlineFRBase>;

//--------------------------------------------------------------------------------
template <class T>
AtolEjectorFR<T>::AtolEjectorFR()
{
	//количество строк шапки - изменено, т.к. шапка печаталась сверху и снизу фискального чека
	mDocumentCapStrings = 0;

	// теги
	mTagEngine = Tags::PEngine(new CAtolEjectorFR::TagEngine());
}

//--------------------------------------------------------------------------------
template <class T>
bool AtolEjectorFR<T>::setEjectorMode(char aEjectorMode)
{
	char mode = mMode;

	if (!enterInnerMode(CAtolFR::InnerModes::Programming))
	{
		toLog(LogLevel::Error, "AtolEjectorFR: Failed to set document processing parameters for receipts");
		return false;
	}

	bool success = setFRParameter(CAtolFR::FRParameters::EjectorParameters, aEjectorMode);

	enterInnerMode(mode);

	return success;
}

//--------------------------------------------------------------------------------
template <class T>
bool AtolEjectorFR<T>::clearImageCounter()
{
	if (!processCommand(CAtolFR::Commands::PrinterAccess, CAtolEjectorFR::ImageProcessing::ClearCounterData))
	{
		toLog(LogLevel::Error, "AtolEjectorFR: Failed to load data image block");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool AtolEjectorFR<T>::printImage(const QImage & aImage, const Tags::TTypes & /*aTags*/)
{
	if (!clearImageCounter())
	{
		return false;
	}

	int width = aImage.width();
	int height = aImage.height();

	if (width > CAtolEjectorFR::ImageProcessing::MaxWidth)
	{
		toLog(LogLevel::Warning, QString("AtolEjectorFR: Image width > %1, the excess part will be truncated").arg(CAtolEjectorFR::ImageProcessing::MaxWidth));
	}

	int widthInBytes = qCeil(width / 8.0);
	int widthInPixels = widthInBytes * 8;

	int lines = qCeil(height / double(CAtolEjectorFR::ImageProcessing::LineHeight));
	int pixelsInLine = widthInPixels * CAtolEjectorFR::ImageProcessing::LineHeight;

	int linesInPart  = qMin(lines, int((CAtolEjectorFR::ImageProcessing::MaxPartSize  * 8.0) / pixelsInLine));
	int blocksInPart = qCeil(linesInPart * widthInPixels / double(CAtolEjectorFR::ImageProcessing::MaxBlockSize));
	int parts = qCeil(double(lines) / linesInPart);

	for (int i = 0; i < parts; ++i)
	{
		int partheightInBytes = qCeil(linesInPart * CAtolEjectorFR::ImageProcessing::LineHeight / 8.0);
		QByteArray initCommandData = QByteArray(CAtolEjectorFR::ImageProcessing::InitData).append(uchar(widthInBytes)).append(uchar(partheightInBytes));

		if (!processCommand(CAtolFR::Commands::PrinterAccess, initCommandData))
		{
			toLog(LogLevel::Error, "AtolEjectorFR: Failed to initialize image processing for part " + QString::number(i));
			return false;
		}

		QList<QByteArray> lineData;
		int stringsInPart = linesInPart * CAtolEjectorFR::ImageProcessing::LineHeight;

		for (int j = 0; j < stringsInPart; ++j)
		{
			int index = stringsInPart * i + j;

			if (index < height)
			{
				lineData << QByteArray::fromRawData((const char *)aImage.scanLine(index), widthInBytes);
			}
			else
			{
				lineData << QByteArray(widthInBytes, ASCII::NUL);
			}
		}

		QByteArray buffer;

		for (int m = 0; m < widthInPixels; ++m)
		{
			char mask = 1 << (7 - (m % 8));

			for (int n = 0; n < linesInPart; ++n)
			{
				char data = ASCII::NUL;

				for (int p = 0; p < CAtolEjectorFR::ImageProcessing::LineHeight; ++p)
				{
					int index = n * CAtolEjectorFR::ImageProcessing::LineHeight + p;

					if (lineData[index][m / 8] & mask)
					{
						data |= 1 << (7 - p);
					}
				}

				buffer.append(data);
			}
		}

		for (int j = 0; j < blocksInPart; ++j)
		{
			QByteArray commandData = buffer.mid(j * CAtolEjectorFR::ImageProcessing::MaxBlockSize, CAtolEjectorFR::ImageProcessing::MaxBlockSize);

			if (!processCommand(CAtolFR::Commands::PrinterAccess, CAtolFR::Commands::PrinterAccess + commandData))
			{
				toLog(LogLevel::Error, "AtolEjectorFR: Failed to load data image block");
				clearImageCounter();

				return false;
			}
		}

		if (!processCommand(CAtolFR::Commands::PrinterAccess, CAtolEjectorFR::ImageProcessing::WriteData))
		{
			toLog(LogLevel::Error, "AtolEjectorFR: Failed to write data image");
			clearImageCounter();

			return false;
		}

		if (!clearImageCounter() && (i != (parts - 1)))
		{
			toLog(LogLevel::Error, "AtolEjectorFR: Cannot continue printing image parts");
			return false;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
