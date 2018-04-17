/* @file Параметры плагинов для принтеров. */

// Modules
#include "Hardware/Common/HardwareConstants.h"

// Project
#include "PrinterPluginParameters.h"
#include "PrinterPluginParameterTranslations.h"

using namespace SDK::Plugin;

namespace PrinterSettings = CHardware::Printer::Settings;
namespace PrinterValues = CHardware::Printer::Values;
namespace Values = CHardware::Values;
namespace PPT = PluginParameterTranslations;

//------------------------------------------------------------------------------
SPluginParameter setPaginationDisabled()
{
	return SPluginParameter(CHardware::Printer::PrintPageNumber, SPluginParameter::Bool, false, PPT::PrintPageNumber, QString(), false, QVariantMap());
}

//------------------------------------------------------------------------------
SPluginParameter setRemoteSensor(bool aEnabled)
{
	return SPluginParameter(PrinterSettings::RemotePaperSensor, false, PPT::RemotePaperSensor, QString(), aEnabled ? Values::Use : Values::NotUse, QStringList() << Values::Use << Values::NotUse, false);
}

//------------------------------------------------------------------------------
SPluginParameter setJamSensorEnabled()
{
	return SPluginParameter(PrinterSettings::PaperJamSensor, false, PPT::PaperJamSensor, QString(), Values::Use, QStringList() << Values::Use << Values::NotUse, false);
}

//------------------------------------------------------------------------------
SPluginParameter setWeightSensorsEnabled()
{
	return SPluginParameter(PrinterSettings::PaperWeightSensors, false, PPT::PaperWeightSensors, QString(), Values::Use, QStringList() << Values::Use << Values::NotUse, false);
}

//------------------------------------------------------------------------------
SPluginParameter setFontSize(int aMin, int aMax, int aDefault, int aInterval)
{
	QStringList values;
	int steps = 2 + (aMax - aMin) / aInterval;

	for (int i = 0; i < steps; ++i)
	{
		int digit = qMin(aMin + i * aInterval, aMax);
		QString value = QString::number(digit);

		if (!i || (values[i - 1] != value))
		{
			values << value;
		}
	}

	return SPluginParameter(PrinterSettings::FontSize, false, PPT::FontSize, QString(), aDefault, values, false);
}

//------------------------------------------------------------------------------
SPluginParameter setLineSpacing(int aMin, int aMax, int aDefault, int aInterval, const QString & aOptionalTranslation)
{
	QStringList values(QString::number(aMin));

	if (aMin == 1)
	{
		aMin = 0;
	}

	for (int i = 0; i < (aMax - aMin) / aInterval; ++i)
	{
		int digit = qMin(aMin + (i + 1) * aInterval, aMax);
		values << QString::number(digit);
	}

	values.removeDuplicates();
	QString translation = PPT::LineSpacing;

	if (!aOptionalTranslation.isEmpty())
	{
		translation += " " + aOptionalTranslation;
	}

	return SPluginParameter(PrinterSettings::LineSpacing, false, translation, QString(), aDefault, values, false);
}

//------------------------------------------------------------------------------
SPluginParameter setFeedingFactor()
{
	return SPluginParameter(PrinterSettings::FeedingFactor, false, PPT::FeedingFactor, QString(), "2", QStringList() << "1" << "2", false);
}

//------------------------------------------------------------------------------
SPluginParameter setLoopEnabled(const QString & aOptionalTranslation, bool aNoChange)
{
	QStringList values;

	if (aNoChange)
	{
		values << Values::NoChange;
	}

	values << Values::Use << Values::NotUse;
	QString translation = PPT::Loop;

	if (!aOptionalTranslation.isEmpty())
	{
		translation += " " + aOptionalTranslation;
	}

	return SPluginParameter(PrinterSettings::Loop, false, translation, QString(), Values::Use, values, false);
}

//------------------------------------------------------------------------------
SPluginParameter setLeftReceiptTimeout(bool aZero)
{
	QStringList values = QStringList() << "10" << "20" << "30" << "40" << "50" << "60";

	if (aZero)
	{
		values.prepend("0");
	}

	return SPluginParameter(PrinterSettings::LeftReceiptTimeout, false, PPT::LeftReceiptTimeout, QString(), "30", values, false);
}

//------------------------------------------------------------------------------
SPluginParameter setLeftReceiptAction(const QString & aParameter, bool aRetract, bool aPush, const QString aDefault, bool aNoChange, const QString & aOptionalTranslation)
{
	QStringList values;

	if (aNoChange)
	{
		values << Values::NoChange;
	}

	if (aRetract)
	{
		values << PrinterValues::Retract;
	}

	if (aPush)
	{
		values << PrinterValues::Push;
	}

	QString title;

	     if (aParameter == PrinterSettings::PreviousAndNotTakenReceipts) title = PPT::PreviousAndNotTakenReceipts;
	else if (aParameter == PrinterSettings::PreviousReceipt) title = PPT::PreviousReceipt;
	else if (aParameter == PrinterSettings::NotTakenReceipt) title = PPT::NotTakenReceipt;

	if (!aOptionalTranslation.isEmpty())
	{
		title += " " + aOptionalTranslation;
	}

	return SPluginParameter(aParameter, false, title, QString(), aDefault, values, false);
}

//------------------------------------------------------------------------------
SPluginParameter setPresentationLength(const QString & aOptionalTranslation, int aMin, int aMax)
{
	QString translation = PPT::PresentLength;

	if (!aOptionalTranslation.isEmpty())
	{
		translation += " " + aOptionalTranslation;
	}

	QList<int> values = QList<int>() << 0 << 2 << 4 << 6 << 8 << 10 << 12 << 14 << 16;
	QStringList pluginValues;

	foreach(int value, values)
	{
		if (((aMin == -1) || (value >= aMin)) && ((aMax == -1) || (value <= aMax)))
		{
			pluginValues << QString::number(value);
		}
	}

	return SPluginParameter(PrinterSettings::PresentationLength, false, translation, QString(), "10", pluginValues, false);
}

//------------------------------------------------------------------------------
SPluginParameter setCustomCodepage()
{
	return SPluginParameter(CHardware::Codepage, false, PPT::Codepage, QString(), CHardware::Codepages::CP866, QStringList()
		<< CHardware::Codepages::CP866
		<< CHardware::Codepages::CustomKZT, false);
}

//------------------------------------------------------------------------------
SPluginParameter setBackFeed(const QString & aOptionalTranslation)
{
	QString translation = PPT::BackFeed;

	if (!aOptionalTranslation.isEmpty())
	{
		translation += " " + aOptionalTranslation;
	}

	return SPluginParameter(PrinterSettings::BackFeed, false, translation, QString(), Values::NotUse , QStringList() << Values::Use << Values::NotUse << Values::NoChange, false);
}

//------------------------------------------------------------------------------
