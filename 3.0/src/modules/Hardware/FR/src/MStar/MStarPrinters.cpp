/* @file Принтеры MStar. */

#include "MStarPrinters.h"

using namespace PrinterStatusCode;

//--------------------------------------------------------------------------------
MStarPrinters::MStarPrinters() : mMode(EFRMode::Fiscal)
{
	// данные устройства
	mDeviceName = "Incotex protocol based FR";

	// протоколы
	mProtocols.append(PDeviceProtocol(new IncotexFR));

	mNextReceiptProcessing = false;
	setConfigParameter(CHardware::Printer::FeedingAmount, 4);
}

//--------------------------------------------------------------------------------
QStringList MStarPrinters::getModelList()
{
	return QStringList() << "Multisoft MStar-TK";
}

//--------------------------------------------------------------------------------
bool MStarPrinters::updateParameters()
{
	return true;
}

//--------------------------------------------------------------------------------
bool MStarPrinters::isConnected()
{
	IncotexFR * IncotexFRProtocol = mProtocol.dynamicCast<IncotexFR>().data();
	CIncotexFR::SUnpackedData unpackedData;

	if (!IncotexFRProtocol->processCommand(mIOPort, FRProtocolCommands::Identification, QVariantMap(), &unpackedData))
	{
		return false;
	}

	identify(unpackedData);

	mVerified = getModelList().contains(mDeviceName);

	return true;
}

//--------------------------------------------------------------------------------
bool MStarPrinters::cut()
{
	return printLine(CIncotexFR::Commands::Cut);
}

//--------------------------------------------------------------------------------
bool MStarPrinters::receiptProcessing()
{
	EFRMode::Enum mode = mMode;

	if (!setMode(EFRMode::Printer))
	{
		return false;
	}

	bool result = SerialFRBase::receiptProcessing();
	setMode(mode);

	return result;
}

//--------------------------------------------------------------------------------
bool MStarPrinters::printLine(const QByteArray & aString)
{
	QVariantMap commandData;
	commandData.insert(CHardware::Printer::ByteString, aString);

	if (!mProtocol->processCommand(mIOPort, FRProtocolCommands::PrintString, commandData))
	{
		toLog(LogLevel::Error, "MStarPrinters: Failed to process line printing");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool MStarPrinters::processReceipt(const QStringList & aReceipt, bool aProcessing)
{
	if (!setMode(EFRMode::Printer))
	{
		return false;
	}

	bool result = SerialFRBase::processReceipt(aReceipt, aProcessing);
	setMode(EFRMode::Fiscal);

	return result;
}

//--------------------------------------------------------------------------------
bool MStarPrinters::getStatus(TStatusCodes & aStatusCodes)
{
	IncotexFR * IncotexFRProtocol = mProtocol.dynamicCast<IncotexFR>().data();
	CIncotexFR::SUnpackedData unpackedData;
	QByteArray commandData;
	QByteArray answerData;

	if (!IncotexFRProtocol->getStatus(mIOPort, unpackedData))
	{
		toLog(LogLevel::Error, "MStarPrinters: Failed to process command 'GetCodeStatus'");
		return false;
	}

	standartCodeError(unpackedData, aStatusCodes);

	return true;
}

//--------------------------------------------------------------------------------
void MStarPrinters::identify(const CIncotexFR::SUnpackedData & aUnpackedData)
{
	QString deviceData = QString("\nfiscal_mode  : %1").arg(isFiscal() ? CHardware::Values::Use : CHardware::Values::NotUse);

	if ((aUnpackedData.vendorName == CIncotexFR::Answer::Identification::MSoftVendor) &&
	    (aUnpackedData.modelName  == CIncotexFR::Answer::Identification::MStarTKModel))
	{
		mDeviceName = "Multisoft MStar-TK";
		deviceData += QString("\nfirmware : %1").arg(aUnpackedData.softVersion.data());

		setConfigParameter(CHardware::DeviceData, deviceData);
	}
	else
	{
		toLog(LogLevel::Error, QString("MStarPrinters: Unknown vendor = %1 or model = %2, failed to identify model and firm")
			.arg(QString(aUnpackedData.vendorName.toHex()))
			.arg(QString(aUnpackedData.modelName.toHex())));
	}
}

//--------------------------------------------------------------------------------
bool MStarPrinters::performFiscal(const QStringList & aReceipt, double aAmount)
{
	toLog(LogLevel::Normal, "Printing fiscal document, receipt:\n" + aReceipt.join("\n"));

	bool result = processReceipt(aReceipt, false);

	QVariantMap commandData;
	commandData.insert(CHardware::FiscalPrinter::Amount, aAmount);

	toLog(LogLevel::Normal, "MStarPrinters: Begin processing command 'sale'");

	if (!mProtocol->processCommand(mIOPort, FRProtocolCommands::Sale, commandData))
	{
		toLog(LogLevel::Error, "MStarPrinters: Failed to process command 'sale', processing receipt and exit!");
		receiptProcessing();

		return false;
	}

	return result;
}

//--------------------------------------------------------------------------------
bool MStarPrinters::performEncashment(const QStringList & aReceipt)
{
	toLog(LogLevel::Normal, "Printing encashment, receipt:\n" + aReceipt.join("\n"));

	// запрашиваем сумму в кассе
	CIncotexFR::SUnpackedData unpackedData;
	QVariantMap commandData;
	commandData.insert(CHardware::Register, CIncotexFR::Registers::SumInCash);

	if (!mProtocol->processCommand(mIOPort, FRProtocolCommands::GetFRRegisters, commandData, &unpackedData))
	{
		toLog(LogLevel::Error, "MStarPrinters: Failed to get the register 'sum in cash'");
		return false;
	}

	// делаем выплату
	TReceiptBuffer receiptBuffer;
	makeReceipt(aReceipt, receiptBuffer);
	commandData.insert(CHardware::Printer::Receipt, QVariant().fromValue(&receiptBuffer));
	commandData.insert(CHardware::FiscalPrinter::Amount, unpackedData.Register);

	return mProtocol->processCommand(mIOPort, FRProtocolCommands::Encashment, commandData);
}

//--------------------------------------------------------------------------------
bool MStarPrinters::processXReport()
{
	return mProtocol->processCommand(mIOPort, FRProtocolCommands::XReport, QVariantMap());
}
//--------------------------------------------------------------------------------
bool MStarPrinters::processPayout()
{
	return true;
}

//--------------------------------------------------------------------------------
bool MStarPrinters::performZReport(bool /*aPrintDeferredReports*/)
{
	toLog(LogLevel::Normal, "MStarPrinters: Begin processing command 'print Z report'");

	if (!mProtocol->processCommand(mIOPort, FRProtocolCommands::ZReport, QVariantMap()))
	{
		toLog(LogLevel::Error, "MStarPrinters: Failed to process command 'print Z report'");
		return false;
	}

	toLog(LogLevel::Warning, "MStarPrinters: command 'ZReport' is successfully processed");

	return true;
}

//--------------------------------------------------------------------------------
void MStarPrinters::standartCodeError(CIncotexFR::SUnpackedData & aUnpacketAnswer, TStatusCodes & aStatusCodes)
{
	aStatusCodes.clear();

	if (aUnpacketAnswer.OfflineError || aUnpacketAnswer.PrinterError || aUnpacketAnswer.TechnoMode)
	{
		aStatusCodes.insert(Error::PrinterFR);
	}

	if (aUnpacketAnswer.PaperEnd)
	{
		aStatusCodes.insert(Error::PaperEnd);
	}

	if (aUnpacketAnswer.CutterOff)
	{
		aStatusCodes.insert(Error::Cutter);
	}

	if (aUnpacketAnswer.EKLZError)
	{
		aStatusCodes.insert(FRStatusCode::Error::EKLZ);
	}

	if (aUnpacketAnswer.EKLZNearEnd)
	{
		aStatusCodes.insert(FRStatusCode::Warning::EKLZNearEnd);
	}

	if (aUnpacketAnswer.FiscalMemoryEnd)
	{
		aStatusCodes.insert(FRStatusCode::Error::FiscalMemory);
	}

	if (aUnpacketAnswer.FiscalMemoryNearEnd)
	{
		aStatusCodes.insert(FRStatusCode::Warning::FiscalMemoryNearEnd);
	}
}

//--------------------------------------------------------------------------------
bool MStarPrinters::setMode(EFRMode::Enum aMode)
{
	if (mMode == aMode)
	{
		return true;
	}

	mMode = aMode;

	bool isPrinterMode = aMode == EFRMode::Printer;
	toLog(LogLevel::Normal, QString("MStarPrinters: Begin processing command 'set to %1 mode'").arg(isPrinterMode ? "printer" : "fiscal"));

	if (!mProtocol->processCommand(mIOPort, isPrinterMode ? FRProtocolCommands::SetPrinterMode : FRProtocolCommands::SetFiscalMode, QVariantMap()))
	{
		toLog(LogLevel::Error, QString("MStarPrinters: Failed to process command 'set to %1 mode'")
			.arg(isPrinterMode ? "printer" : "fiscal"));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool MStarPrinters::isSessionOpened()
{
	return mProtocol.dynamicCast<IFRProtocol>()->isSessionOpened();
}

//--------------------------------------------------------------------------------
