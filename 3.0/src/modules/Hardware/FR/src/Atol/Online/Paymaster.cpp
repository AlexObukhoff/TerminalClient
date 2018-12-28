/* @file Онлайн ФР Сенсис Казначей на протоколе АТОЛ. */

#pragma once

#include "Paymaster.h"

using namespace SDK::Driver;

//--------------------------------------------------------------------------------
Paymaster::Paymaster()
{
	mDeviceName = CAtolFR::Models::Paymaster;
	mSupportedModels = QStringList() << mDeviceName;
	mFRBuildUnifiedTaxes = 4799;

	setConfigParameter(CHardwareSDK::FR::CanWithoutPrinting, true);

	using namespace SDK::Driver::IOPort::COM;

	// данные порта
	mPortParameters[EParameters::BaudRate].clear();
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR115200);
}

//--------------------------------------------------------------------------------
void Paymaster::setDeviceConfiguration(const QVariantMap & aConfiguration)
{
	TPaymaster::setDeviceConfiguration(aConfiguration);

	bool notPrinting = getConfigParameter(CHardwareSDK::FR::WithoutPrinting) == CHardwareSDK::Values::Use;
	QString printerModel = getConfigParameter(CHardware::FR::PrinterModel, CAtolOnlinePrinters::Default).toString();

	if (aConfiguration.contains(CHardware::FR::PrinterModel) && (printerModel != CAtolOnlinePrinters::Default) && !notPrinting)
	{
		mPPTaskList.append([&] () { mNotPrintingError = !setNotPrintDocument(false); });
	}
}

//--------------------------------------------------------------------------------
char Paymaster::getPrinterId()
{
	QString printerModel = getConfigParameter(CHardware::FR::PrinterModel).toString();
	char result = CAtolOnlinePrinters::Models.data().key(printerModel, 0);

	if (!result)
	{
		toLog(LogLevel::Error, mDeviceName + ": Unknown printer model " + printerModel);
	}

	return result;
}

//--------------------------------------------------------------------------------
bool Paymaster::updateParameters()
{
	if (!TPaymaster::updateParameters())
	{
		return false;
	}

	mOldFirmware = mOldFirmware || (mFRBuild && (mFRBuild < CPaymaster::MinFRBuild));

	QByteArray data;

	#define SET_LCONFIG_FISCAL_FIELD(aName) if (getTLV(CFR::FiscalFields::aName, data)) { mFFEngine.setLConfigParameter(CFiscalSDK::aName, data); \
		QString value = mFFEngine.getConfigParameter(CFiscalSDK::aName, data).toString(); toLog(LogLevel::Normal, mDeviceName + \
			QString(": Add %1 = \"%2\" to config data").arg(mFFData.getTextLog(CFR::FiscalFields::aName)).arg(value)); }

	SET_LCONFIG_FISCAL_FIELD(OFDURL);

	QString automaticNumber = getConfigParameter(CFiscalSDK::AutomaticNumber).toString().simplified();

	if (!automaticNumber.isEmpty())
	{
		if (getTLV(CFR::FiscalFields::AutomaticNumber, data))
		{
			setDeviceParameter(CDeviceData::FR::AutomaticNumber, data.simplified().toULongLong());

			if (data != automaticNumber)
			{
				toLog(LogLevel::Warning, QString("%1: The automatic number in FR = %2 is different from AP number = %3").arg(mDeviceName).arg(data.data()).arg(automaticNumber));

				//TODO: если будет возвращаться результат - то заходить сюда только в фискальном режиме
				//return setTLV(FiscalFields::AutomaticNumber);
			}
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
void Paymaster::processDeviceData()
{
	TPaymaster::processDeviceData();

	QByteArray data;
	char mode = mMode;

	if (enterInnerMode(CAtolFR::InnerModes::Programming) && getFRParameter(CAtolOnlineFR::FRParameters::PrinterModel, data) && !data.isEmpty())
	{
		if (!CAtolOnlinePrinters::Models.data().contains(data[0]))
		{
			toLog(LogLevel::Error, QString("%1: Unknown printer model Id %2").arg(mDeviceName).arg(uchar(data[0])));
		}
		else
		{
			QString printerModel = CAtolOnlinePrinters::Models[data[0]];
			QString configModel = getConfigParameter(CHardware::FR::PrinterModel).toString();

			if ((configModel.isEmpty() || (configModel == CAtolOnlinePrinters::Default)) && (printerModel != configModel))
			{
				setConfigParameter(CHardware::FR::PrinterModel, printerModel);

				emit configurationChanged();
			}
		}
	}

	enterInnerMode(mode);
}

//--------------------------------------------------------------------------------
bool Paymaster::enterExtendedMode()
{
	char mode = mMode;

	if (!enterInnerMode(CAtolFR::InnerModes::Programming))
	{
		return false;
	}

	if (!setFRParameter(CAtolOnlineFR::FRParameters::SetAutoZReportTiming, CAtolOnlineFR::AutoZReportTimingEnable))
	{
		enterInnerMode(mode);

		toLog(LogLevel::Error, mDeviceName + ": Failed to enable auto Z-report timing");
		return false;
	}

	QByteArray data;
	bool result = getFRParameter(CAtolOnlineFR::FRParameters::DocumentPerforming, data) && !data.isEmpty() && (data[0] == CAtolOnlineFR::ZReportInBuffer);

	if (!result)
	{
		result = setFRParameter(CAtolOnlineFR::FRParameters::DocumentPerforming, CAtolOnlineFR::ZReportInBuffer);

		if (!result)
		{
			toLog(LogLevel::Error, mDeviceName + "Failed to enter to Z-report mode");
		}
		else
		{
			result = reboot();
		}
	}

	enterInnerMode(mode);

	return result;
}

//--------------------------------------------------------------------------------
bool Paymaster::setNotPrintDocument(bool aEnabled, bool /*aZReport*/)
{
	char printingValue = aEnabled ? CAtolOnlinePrinters::Memory : getPrinterId();

	if (!printingValue)
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to set printer model due to printer model Id is invalid");
		return false;
	}

	char mode = mMode;

	if (!enterInnerMode(CAtolFR::InnerModes::Programming))
	{
		return false;
	}

	QByteArray data;
	bool result = getFRParameter(CAtolOnlineFR::FRParameters::PrinterModel, data) && !data.isEmpty() && (data[0] == printingValue);

	if (!result)
	{
		result = setFRParameter(CAtolOnlineFR::FRParameters::PrinterModel, printingValue) && reboot();
	}

	enterInnerMode(mode);

	return result;
}

//--------------------------------------------------------------------------------
bool Paymaster::printDeferredZReports()
{
	if (!TPaymaster::printDeferredZReports())
	{
		return false;
	}

	if (!processCommand(CAtolOnlineFR::Commands::ClearZBuffer))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to clear Z-buffer");
	}

	return true;
}

//--------------------------------------------------------------------------------
bool Paymaster::performZReport(bool /*aPrintDeferredReports*/)
{
	toLog(LogLevel::Normal, "AtolFR: Processing Z-report");

	// если ККМ работает в расширенном режиме - печатаем отложенные Z-отчеты
	bool printDeferredZReportSuccess = printDeferredZReports();
	bool ZReportSuccess = execZReport(true) && printDeferredZReports();
	bool result = printDeferredZReportSuccess || ZReportSuccess;

	// ошибки наполнения и переполнения буфера Z-Отчетов можно будет сбросить, если будет доработан функционал запроса свободного места в Z-буфере
	//checkZBufferState();

	if (result)
	{
		mZBufferOverflow = false;

		if (printDeferredZReportSuccess)
		{
			mZBufferFull = false;
		}
	}

	exitInnerMode();

	return result;
}

//--------------------------------------------------------------------------------
bool Paymaster::setFRParameters()
{
	if (!TPaymaster::setFRParameters())
	{
		return false;
	}

	return setFRParameter(CAtolOnlineFR::FRParameters::SetReportsProcessing, CAtolOnlineFR::PresentReports);
}

//--------------------------------------------------------------------------------
void Paymaster::parseShortStatusFlags(char aFlags, TStatusCodes & aStatusCodes)
{
	TPaymaster::parseShortStatusFlags(aFlags, aStatusCodes);

	QString printerModel = getConfigParameter(CHardware::FR::PrinterModel).toString();

	if (aFlags & CPaymaster::ShortStatusError::PaperJam)
	{
		aStatusCodes.insert(PrinterStatusCode::Error::PaperJam);
	}

	if ((aFlags & CPaymaster::ShortStatusError::Presenter) && (printerModel == CAtolOnlinePrinters::CitizenPPU700))
	{
		aStatusCodes.insert(PrinterStatusCode::Error::Presenter);
	}
}

//--------------------------------------------------------------------------------
