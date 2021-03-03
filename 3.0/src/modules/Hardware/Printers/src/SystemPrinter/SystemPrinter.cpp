/* @file Системный принтер. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegExp>
#include <QtCore/QBuffer>
#include <QtGui/QMatrix>
#include <QtGui/QFontDatabase>
#include <Common/QtHeadersEnd.h>

// Modules
#include "SysUtils/ISysUtils.h"
#include "SysUtils/SystemPrinterStatusCodes.h"

// Project
#include "SystemPrinter.h"

namespace PrinterSettings = CHardware::Printer::Settings;

//--------------------------------------------------------------------------------
SystemPrinter::SystemPrinter()
{
	// данные устройства
	mDeviceName = "System";
	setConfigParameter(CHardware::Printer::NeedSeparating, false);
	mLineFeed = false;
	mSideMargin = 1.0;

	// теги
	mTagEngine = Tags::PEngine(new CSystemPrinter::TagEngine());
}

//--------------------------------------------------------------------------------
void SystemPrinter::initialize()
{
	mDeviceName = "System printer";

	TSystemPrinter::initialize();
}

//--------------------------------------------------------------------------------
bool SystemPrinter::isConnected()
{
	if (!mPrinter.isValid())
	{
		return false;
	}

	QVariantMap deviceData = ISysUtils::getPrinterData(mLog, mPrinter.printerName());
	QString idName = deviceData[CDeviceData::Name].toString();

	if (mIdName.isEmpty() || idName.isEmpty())
	{
		return true;
	}

	return idName.contains(mIdName, Qt::CaseInsensitive);
}

//--------------------------------------------------------------------------------
bool SystemPrinter::updateParameters()
{
	if (mPrinter.printerName().isEmpty())
	{
		return true;
	}

	mDeviceName = QString("System printer (%1)").arg(mPrinter.printerName());
	QVariantMap deviceData = ISysUtils::getPrinterData(mLog, mPrinter.printerName());

	for (auto it = deviceData.begin(); it != deviceData.end(); ++it)
	{
		setDeviceParameter(it.key(), it.value());
	}

	QString name = deviceData[CDeviceData::Name].toString();

	// в позициях шрифта Terminal
	if (name.contains("VKP80 II", Qt::CaseInsensitive))
	{
		setConfigParameter(CHardwareSDK::Printer::LineSize, 44);
		setConfigParameter(PrinterSettings::LeftMargin,  1);
		setConfigParameter(PrinterSettings::RightMargin, 1);
	}

	if (name.contains("TG2480-H", Qt::CaseInsensitive))
	{
		setConfigParameter(CHardwareSDK::Printer::LineSize, 40);
		setConfigParameter(PrinterSettings::LeftMargin,  0);
		setConfigParameter(PrinterSettings::RightMargin, 0);
	}

	QString errorMessage;

	if (!ISysUtils::setPrintingQueuedMode(mLog, mPrinter.printerName()))
	{
		toLog(LogLevel::Warning, "Failed to change printing queued mode, " + errorMessage);
	}

	return true;
}

//--------------------------------------------------------------------------------
bool SystemPrinter::printReceipt(const Tags::TLexemeReceipt & aLexemeReceipt)
{
	QStringList receipt;

	foreach (auto lexemeCollection, aLexemeReceipt)
	{
		foreach (auto lexemes, lexemeCollection)
		{
			QVariant line;

			foreach (auto lexeme, lexemes)
			{
				if (lexeme.tags.contains(Tags::Type::Image))
				{
					QByteArray data = QByteArray::fromBase64(lexeme.data.toLatin1());
					QImage image = QImage::fromData(data, "png");

					if (!image.isNull())
					{
						QMatrix matrix;
						matrix.scale(CSystemPrinter::ImageScalingFactor, CSystemPrinter::ImageScalingFactor);
						image = image.transformed(matrix);
						QBuffer buffer;

						if (!image.isNull() && image.save(&buffer, "png"))
						{
							lexeme.data = buffer.data().toBase64();
						}
					}
				}

				execTags(lexeme, line);
			}

			receipt << line.toString();
		}
	}

	for (int i = 0; i < receipt.size(); ++i)
	{
		int index = receipt[i].indexOf(QRegExp("[^ ]"));
		receipt[i] = receipt[i].replace(0, index, QString("&nbsp;").repeated(index)).replace(QRegExp(" +$"), "");
	}

	// Увеличиваем отступ от нижнего края листа, если требуется вывод номеров страниц
	qreal bottomMargin = getConfigParameter(PrinterSettings::PrintPageNumber).toBool() ? 12.7 : CSystemPrinter::DefaultMargin;
	qreal leftMargin   = getConfigParameter(PrinterSettings::LeftMargin, mSideMargin).toDouble();
	qreal rightMargin  = getConfigParameter(PrinterSettings::RightMargin, mSideMargin).toDouble();
	mPrinter.setPageMargins(leftMargin, CSystemPrinter::DefaultMargin, rightMargin, bottomMargin, QPrinter::Millimeter);

	QTextDocument document;	
	QString toPrint = receipt.join(CSystemPrinter::BRtag) + CSystemPrinter::BRtag;

	int lineSpacing = getConfigParameter(PrinterSettings::LineSpacing).toInt();
	int fontSize    = getConfigParameter(PrinterSettings::FontSize).toInt();

	QStringList textParameters;
	QStringList fontFamilies = QFontDatabase().families();

	if (fontFamilies.contains("Terminal"))
	{
		//textParameters << "font-family: Terminal";
	}

	//!! align работает начиная с Qt 5.2.1

	if (lineSpacing) textParameters << QString("line-height: %1%").arg(lineSpacing);
	if (fontSize)    textParameters << QString("font-size: %1px").arg(fontSize);

	toPrint = QString("<style>p {%1} </style><p>%2</p>").arg(textParameters.join("; ")).arg(toPrint);
	document.setHtml(toPrint);
	document.print(&mPrinter);

	return true;
}

//--------------------------------------------------------------------------------
bool SystemPrinter::getStatus(TStatusCodes & aStatusCodes)
{
	using namespace SDK::Driver;

	if (!isConnected())
	{
		return false;
	}

	if (mPrinter.printerState() == QPrinter::Error)
	{
		aStatusCodes.insert(DeviceStatusCode::Error::Unknown);
	}

	TStatusGroupNames groupNames;
	getPrinterStatus(mPrinter.printerName(), aStatusCodes, groupNames);

	if (mLastStatusesNames != groupNames)
	{
		mLastStatusesNames = groupNames;

		TStatusCollection statuses;

		foreach (int statusCode, aStatusCodes)
		{
			statuses[mStatusCodesSpecification->value(statusCode).warningLevel].insert(statusCode);
		}

		EWarningLevel::Enum warningLevel = getWarningLevel(statuses);
		LogLevel::Enum logLevel = (warningLevel == EWarningLevel::Error) ? LogLevel::Error :
			((warningLevel == EWarningLevel::Warning) ? LogLevel::Warning : LogLevel::Normal);

		QString log = "Device codes has changed:";

		for (auto it = mLastStatusesNames.begin(); it != mLastStatusesNames.end(); ++it)
		{
			QStringList statusNames = it->toList();
			qSort(statusNames);
			log += QString("\n%1 : %2").arg(it.key(), 12).arg(statusNames.join(", "));
		}

		toLog(logLevel, log);
	}

	return true;
}

//---------------------------------------------------------------------------
void SystemPrinter::getPrinterStatus(const QString & aPrinterName, TStatusCodes & aStatusCodes, TStatusGroupNames & aGroupNames) const
{
	TJobStatus jobs;
	ulong status;
	ulong attributes;
	QString errorMessage;

	if (!ISysUtils::getPrinterStatusData(mLog, aPrinterName, jobs, status, attributes))
	{
		aStatusCodes.insert(DeviceStatusCode::Warning::ThirdPartyDriver);

		return;
	}

	for (int i = 0; i < sizeof(ulong) * 8; ++i)
	{
		ulong key = 1 << i;

		if (WindowsPrinterStatuses.data().contains(key) && (status & key))
		{
			const SDeviceCodeSpecification & data = WindowsPrinterStatuses[key];
			aStatusCodes.insert(data.statusCode);
			aGroupNames["Statuses"].insert(data.description);
		}

		foreach (auto jobStatus, jobs)
		{
			if (WindowsPrinterJobStatuses.data().contains(key) && (jobStatus & key))
			{
				const SDeviceCodeSpecification & data = WindowsPrinterJobStatuses[key];
				aStatusCodes.insert(data.statusCode);
				aGroupNames["Job statuses"].insert(data.description);
			}
		}
	}

	if (!ISysUtils::setPrintingQueuedMode(mLog, aPrinterName))
	{
		aStatusCodes.insert(DeviceStatusCode::Warning::ThirdPartyDriver);
	}

	auto checkAttribute = [&] (ulong attribute) -> bool { return !mAttributesDisabled.contains(attribute) && (attributes & attribute); };

	if (checkAttribute(PRINTER_STATUS_NOT_AVAILABLE)   ||
		checkAttribute(PRINTER_ATTRIBUTE_WORK_OFFLINE) ||
		status & PRINTER_STATUS_PAUSED)
	{
		aStatusCodes.insert(DeviceStatusCode::Error::NotAvailable);
		aGroupNames["Attributes"].insert("work_offline");
	}
}

//--------------------------------------------------------------------------------
