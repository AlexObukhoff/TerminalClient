/* @file Принтер Citizen PPU-231. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// Project
#include "CitizenPPU231.h"
#include "CitizenPPU231Constants.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//--------------------------------------------------------------------------------
CitizenPPU231::CitizenPPU231()
{
	// данные порта
	mPortParameters = mModelData.getDefault().parameters.portSettings->data();
	mModelData.data().clear();

	mPortParameters[EParameters::BaudRate].clear();
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR19200);
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR4800);

	// теги
	mTagEngine = Tags::PEngine(new CCitizenPPU231::TagEngine());

	// данные устройства
	mDeviceName = "Citizen PPU-231";
	mLineSize = CCitizenPPU231::LineSize;
	mAutoDetectable = false;
	mPrintingStringTimeout = 100;
	mRussianCodePage = '\x07';
}

//--------------------------------------------------------------------------------
bool CitizenPPU231::isConnected()
{
	return TSerialPrinterBase::isConnected();
}

//--------------------------------------------------------------------------------
bool CitizenPPU231::updateParameters()
{
	if (!POSPrinter::updateParameters())
	{
		return false;
	}

	int lineSpacing = getConfigParameter(CHardware::Printer::Settings::LineSpacing).toInt();
	int feedingFactor = getConfigParameter(CHardware::Printer::Settings::FeedingFactor).toInt();
	lineSpacing = feedingFactor * qCeil(360 / (lineSpacing * 4.0));
	setConfigParameter(CHardware::Printer::FeedingAmount, lineSpacing);

	mVerified = true;

	return true;
}

//--------------------------------------------------------------------------------
bool CitizenPPU231::printReceipt(const Tags::TLexemeReceipt & aLexemeReceipt)
{
	waitAvailable();

	return SerialPrinterBase::printReceipt(aLexemeReceipt);
}

//--------------------------------------------------------------------------------
bool CitizenPPU231::getStatus(TStatusCodes & aStatusCodes)
{
	QByteArray answer;
	PollingExpector expector;
	auto poll = [&] () -> bool { return mIOPort->write(CPOSPrinter::Command::GetPaperStatus) && getAnswer(answer, 50); };

	if (!expector.wait<bool>(poll, [&answer]() -> bool { return !answer.isEmpty(); }, CCitizenPPU231::PollIntervals::Status, CCitizenPPU231::Timeouts::Status, true))
	{
		return false;
	}

	for (int i = 0; i < 8; ++i)
	{
		if (answer[0] & (1 << i))
		{
			aStatusCodes.insert(CCitizenPPU231::Statuses[i]);
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
bool CitizenPPU231::printBarcode(const QString & aBarcode)
{
	QByteArray barcodePrinting = CPOSPrinter::Command::Barcode::Print + CCitizenPPU231::Barcode::CodeSystem128 +
		CCitizenPPU231::Barcode::Code128Spec + mCodec->fromUnicode(aBarcode) + CCitizenPPU231::Barcode::Postfix;

	return mIOPort->write(prepareBarcodePrinting() + barcodePrinting);
}

//--------------------------------------------------------------------------------
