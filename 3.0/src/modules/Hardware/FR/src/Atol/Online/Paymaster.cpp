/* @file Онлайн ФР Сенсис Казначей на протоколе АТОЛ. */

#pragma once

#include "Paymaster.h"

using namespace SDK::Driver;

//--------------------------------------------------------------------------------
Paymaster::Paymaster()
{
	mDeviceName = CAtolFR::Models::Paymaster;
	mSupportedModels = QStringList() << mDeviceName;
	mCanProcessZBuffer = true;
	mFRBuildUnifiedTaxes = 4799;

	using namespace SDK::Driver::IOPort::COM;

	// данные порта
	mPortParameters[EParameters::BaudRate].clear();
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR115200);
}

//--------------------------------------------------------------------------------
bool Paymaster::updateParameters()
{
	if (!TPaymaster::updateParameters())
	{
		return false;
	}

	QString automaticNumber = getConfigParameter(CHardware::FiscalFields::AutomaticNumber).toString().simplified();

	if (!automaticNumber.isEmpty())
	{
		QByteArray data;

		if (getTLV(FiscalFields::AutomaticNumber, data))
		{
			setDeviceParameter(CDeviceData::FR::AutomaticNumber, data.simplified().toULongLong());

			if (data != automaticNumber)
			{
				toLog(LogLevel::Warning, QString("%1: The automatic number in FR = %2 is different from AP number = %3").arg(mDeviceName).arg(data.data()).arg(automaticNumber));

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

	if (enterInnerMode(CAtolFR::InnerModes::Programming) && getFRParameter(CAtolOnlineFR::FRParameters::PrinterModel, data) && !data.isEmpty() && CPaymaster::PrinterModels.data().contains(data[0]))
	{
		setDeviceParameter(CDeviceData::FR::Printer, CPaymaster::PrinterModels[data[0]]);
	}

	enterInnerMode(mode);
}

//--------------------------------------------------------------------------------
bool Paymaster::enterExtendedMode()
{
	bool mode = mMode;

	if (!enterInnerMode(CAtolFR::InnerModes::Programming))
	{
		return false;
	}

	if (!setFRParameter(CAtolOnlineFR::FRParameters::SetAutoZReportTiming, CAtolOnlineFR::AutoZReportTimingEnable))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to enable auto Z-report timing");
		return false;
	}

	QByteArray documentPerforming;

	if (getFRParameter(CAtolOnlineFR::FRParameters::DocumentPerforming, documentPerforming) && !documentPerforming.isEmpty() && (documentPerforming[0] != CAtolOnlineFR::ZReportInBuffer))
	{
		bool result = true;

		if (!setFRParameter(CAtolOnlineFR::FRParameters::DocumentPerforming, CAtolOnlineFR::ZReportInBuffer))
		{
			toLog(LogLevel::Error, mDeviceName + "Failed to enter to Z-report mode");
			result = false;
		}

		if (!enterInnerMode(CAtolFR::InnerModes::Cancel))
		{
			return false;
		}

		if (!processCommand(CAtolOnlineFR::Commands::Reboot, QByteArray(1, ASCII::NUL)))
		{
			toLog(LogLevel::Error, mDeviceName + ": Failed to reboot FR properly");
			result = false;
		}

		SleepHelper::msleep(CAtolOnlineFR::RebootPause);

		if (!result)
		{
			return false;
		}
	}

	enterInnerMode(mode);

	return true;
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
