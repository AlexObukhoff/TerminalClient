/* @file Базовый принтер с портовой реализацией протокола. */

// Project
#include "PortPrinterBase.h"

//---------------------------------------------------------------------------
template class PortPrinterBase<PrinterBase<SerialDeviceBase<PortPollingDeviceBase<ProtoPrinter>>>>;
template class PortPrinterBase<PrinterBase<TCPDeviceBase<PortPollingDeviceBase<ProtoPrinter>>>>;

//---------------------------------------------------------------------------
template <class T>
PortPrinterBase<T>::PortPrinterBase()
{
	mIOMessageLogging = ELoggingType::ReadWrite;

	// кодек
	mCodec = CodecByName[CHardware::Codepages::CP866];
}

//--------------------------------------------------------------------------------
template <class T>
void PortPrinterBase<T>::finaliseInitialization()
{
	addPortData();

	T::finaliseInitialization();
}

//--------------------------------------------------------------------------------
template <class T>
bool PortPrinterBase<T>::releaseExternalResource()
{
	return !mOperatorPresence || !mIOPort || mIOPort->close();
}

//--------------------------------------------------------------------------------
template <class T>
bool PortPrinterBase<T>::getAnswer(QByteArray & aAnswer, int aTimeout) const
{
	setPortLoggingType(ELoggingType::Write);

	if (!mIOPort->read(aAnswer, aTimeout))
	{
		return false;
	}

	toLog(aAnswer.isEmpty() ? LogLevel::Warning : LogLevel::Normal, QString("%1: << {%2}").arg(mDeviceName).arg(aAnswer.toHex().data()));

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
void PortPrinterBase<T>::execTags(Tags::SLexeme & aTagLexeme, QVariant & aLine)
{
	QByteArray data = mCodec->fromUnicode(aTagLexeme.data);

	foreach(const Tags::TTypes types, mTagEngine->groupsTypesByPrefix(aTagLexeme.tags))
	{
		QByteArray openTag  = mTagEngine->getTag(types, Tags::Direction::Open);
		QByteArray closeTag = mTagEngine->getTag(types, Tags::Direction::Close);
		data = openTag + data + closeTag;
	}

	aLine = aLine.toByteArray() + data;
}

//--------------------------------------------------------------------------------
template <class T>
bool PortPrinterBase<T>::printReceipt(const Tags::TLexemeReceipt & aLexemeReceipt)
{
	mActualStringCount = 0;

	QList<int> commands = mUnnecessaryErrors.keys();
	bool isSimplePrinter = std::find_if(commands.begin(), commands.end(), [] (int command) -> bool { return command; }) == commands.end();

	if (isSimplePrinter && !updateParameters())
	{
		return false;
	}

	return T::printReceipt(aLexemeReceipt);
}

//--------------------------------------------------------------------------------
template <class T>
bool PortPrinterBase<T>::printOut(const SPrintingOutData & aPrintingOutData)
{
	setLog(aPrintingOutData.log);
	setDeviceConfiguration(aPrintingOutData.configuration);
	mConnected = true;
	mInitialized = ERequestStatus::Success;

	if (!checkConnectionAbility())
	{
		return false;
	}

	setPortLoggingType(ELoggingType::ReadWrite);
	int feeding = getConfigParameter(CHardware::Printer::FeedingAmount).toInt();

	bool result = updateParametersOut() && processReceipt(aPrintingOutData.receipt, aPrintingOutData.receiptProcessing);

	setConfigParameter(CHardware::Printer::FeedingAmount, feeding);
	setPortLoggingType(aPrintingOutData.IOMessageLogging);

	return result;
}

//---------------------------------------------------------------------------
template <class T>
bool PortPrinterBase<T>::printLine(const QVariant & aLine)
{
	QByteArray request = aLine.toByteArray();

	if (mLineFeed)
	{
		request += CPrinters::LineSpacer;
	}

	return printLine(request);
}

//---------------------------------------------------------------------------
template <class T>
bool PortPrinterBase<T>::printLine(const QByteArray & aLine)
{
	return mIOPort->write(aLine);
}

//--------------------------------------------------------------------------------
template <class T>
bool PortPrinterBase<T>::cut()
{
	if (!mIOPort->write(getConfigParameter(CHardware::Printer::Commands::Cutting).toByteArray()))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to cut paper");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool PortPrinterBase<T>::present()
{
	using namespace CHardware::Printer;

	QByteArray command = getConfigParameter(Commands::Presentation).toByteArray() + char(getConfigParameter(Settings::PresentationLength).toInt());

	if (!mIOPort->write(command))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to present paper");
		return false;
	}

	return true;
}

//---------------------------------------------------------------------------
template <class T>
bool PortPrinterBase<T>::push()
{
	QByteArray command = getConfigParameter(CHardware::Printer::Commands::Pushing).toByteArray();

	return mIOPort->write(command);
}

//---------------------------------------------------------------------------
template <class T>
bool PortPrinterBase<T>::retract()
{
	QByteArray command = getConfigParameter(CHardware::Printer::Commands::Retraction).toByteArray();

	return mIOPort->write(command);
}

//--------------------------------------------------------------------------------
template <class T>
bool PortPrinterBase<T>::waitAvailable()
{
	int timeout = CPortPrinter::PrintingStringTimeout * mActualStringCount;

	if (!timeout)
	{
		return true;
	}

	TStatusCodes statusCodes;
	auto condition = std::bind(&PortPrinterBase<T>::getStatus, this, std::ref(statusCodes));

	return PollingExpector().wait(condition, CPortPrinter::WaitingPollingInterval, timeout);
}

//--------------------------------------------------------------------------------
