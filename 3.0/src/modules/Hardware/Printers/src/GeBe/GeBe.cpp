/* @file Принтер Gebe. */

#include "GeBe.h"
#include "GeBEData.h"

using namespace SDK::Driver::IOPort::COM;
using namespace PrinterStatusCode;

//--------------------------------------------------------------------------------
GeBe::GeBe()
{
	// данные устройства
	mDeviceName = "GeBE Compact";

	// данные порта
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR115200);  // preferable for work
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);    // default
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR57600);
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR38400);
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR19200);
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR4800);
	//TODO: добавить baudrate = 1200, 2400.

	mPortParameters[EParameters::Parity].append(EParity::No);    // default
	mPortParameters[EParameters::Parity].append(EParity::Space);
	mPortParameters[EParameters::Parity].append(EParity::Even);
	mPortParameters[EParameters::Parity].append(EParity::Odd);

	//TODO: добавить stopbit = 2.

	mLineSize  = CGeBE::LineSize;

	// теги
	mTagEngine = Tags::PEngine(new CGeBE::TagEngine());

	// кодек
	mCodec = CodecByName[CHardware::Codepages::Win1251];

	// данные устройства
	setConfigParameter(CHardware::Printer::FeedingAmount, 4);
	setConfigParameter(CHardware::Printer::Commands::Cutting, "\x1B\x43\x30");
}

//--------------------------------------------------------------------------------
bool GeBe::updateParameters()
{
	return mIOPort->write(QByteArray(CGeBE::Commands::Initilize) + CGeBE::Commands::SetFont);
}

//--------------------------------------------------------------------------------
bool GeBe::getStatus(TStatusCodes & aStatusCodes)
{
	QByteArray answer;

	if (!mIOPort->write(CGeBE::Commands::GetStatus) || !mIOPort->read(answer) || answer.isEmpty())
	{
		return false;
	}

	for (int i = 0; i < answer.size(); ++i)
	{
		aStatusCodes.insert(CGeBE::Statuses[answer[i]]);
	}

	return true;
}

//--------------------------------------------------------------------------------
