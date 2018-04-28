/* @file ФР семейства Штрих с ограниченным управлением эжектором. */

#include "ShtrihRetractorFRLite.h"

//--------------------------------------------------------------------------------
template class ShtrihRetractorFRLite<ShtrihOnlineFRBase<ShtrihTCPFRBase>>;
template class ShtrihRetractorFRLite<ShtrihOnlineFRBase<ShtrihSerialFRBase>>;
template class ShtrihRetractorFRLite<ShtrihSerialFR>;

//--------------------------------------------------------------------------------
template <class T>
bool ShtrihRetractorFRLite<T>::updateParameters()
{
	if (!T::updateParameters())
	{
		return false;
	}

	if (!CShtrihFR::FRParameters::Fields.data().contains(mModel))
	{
		toLog(LogLevel::Normal, QString("ShtrihFR: Cannot set any fields for the device with model Id %1 as no data of system tables").arg(mModel));
		return true;
	}

	using namespace CHardware::Printer;

	//17. Авторетракция чеков - нет
	QByteArray data;

	if ((!getFRParameter(mParameters.autoRetractingCheques, data) || data.isEmpty() || bool(data[0])) &&
	      setFRParameter(mParameters.autoRetractingCheques, false))
	{
		mNeedReboot = true;
	}

	//18. Авторетракция отчетов - нет
	setFRParameter(mParameters.autoRetractingReports, false);

	//19. Длина презентации
	if (containsConfigParameter(Settings::PresentationLength))
	{
		int presentationLength = getConfigParameter(Settings::PresentationLength).toInt();
		setFRParameter(mParameters.presentationLength, char(presentationLength));
	}

	//20. Таймаут забытого чека
	if (containsConfigParameter(Settings::LeftReceiptTimeout))
	{
		int timeout = getConfigParameter(Settings::LeftReceiptTimeout).toInt();
		setFRParameter(mParameters.leftReceiptTimeout, uchar(timeout));
	}

	//21. Петля
	if (containsConfigParameter(Settings::Loop))
	{
		char loop = char(getConfigParameter(Settings::Loop).toString() == CHardwareSDK::Values::Use);

		if ((!getFRParameter(mParameters.loop, data) || data.isEmpty() || (data[0] != loop)) &&
		      setFRParameter(mParameters.loop, loop))
		{
			mNeedReboot = true;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool ShtrihRetractorFRLite<T>::cut()
{
	QByteArray commandData(1, CShtrihFR::PartialCutting);

	if (!processCommand(CShtrihFR::Commands::Cut, commandData))
	{
		toLog(LogLevel::Error, "ShtrihEjectorFR: Failed to cut");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
void ShtrihRetractorFRLite<T>::appendStatusCodes(ushort aFlags, TStatusCodes & aStatusCodes)
{
	T::appendStatusCodes(aFlags, aStatusCodes);

	// бумага в презентере
	bool paperInPresenter = (~aFlags & CShtrihFR::Statuses::PaperInPresenter);

	if (paperInPresenter)
	{
		aStatusCodes.insert(PrinterStatusCode::OK::PaperInPresenter);
	}

	// косвенные ошибки замятия бумаги и открытой крышки (только если есть эжектор)
	if ((~aFlags & CShtrihFR::Statuses::PaperLeverNotDropped) && isPaperLeverExist())
	{
		aStatusCodes.insert(paperInPresenter ? PrinterStatusCode::Error::PaperJam : DeviceStatusCode::Error::CoverIsOpened);
		toLog(LogLevel::Error, QString("ShtrihEjectorFR: Paper lever is lifted, %1 indirect error")
			.arg(paperInPresenter ? "paper jam" : "opened cover"));
	}
}

//--------------------------------------------------------------------------------
