/* @file Принтер Fujitsu на контроллере Trentino FTP-609. */

// Project
#include "FujitsuPrinters.h"
#include "FujitsuPrinterData.h"

using namespace SDK::Driver::IOPort::COM;
using namespace PrinterStatusCode;

//--------------------------------------------------------------------------------
FujitsuPrinter::FujitsuPrinter()
{
	// данные устройства
	mDeviceName = "Fujitsu FTP-609";

	// данные порта
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR115200);    // default
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR57600);
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR38400);
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR19200);
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);

	mPortParameters[EParameters::Parity].append(EParity::No);

	// теги
	mTagEngine = Tags::PEngine(new CFujitsu::TagEngine());

	// кодек
	mCodec = CodecByName[CHardware::Codepages::Win1251];

	// данные устройства
	setConfigParameter(CHardware::Printer::FeedingAmount, 4);
	setConfigParameter(CHardware::Printer::Commands::Cutting, "\x1B\x69");
}

//--------------------------------------------------------------------------------
bool FujitsuPrinter::isConnected()
{
	QByteArray answer;

	if (!processCommand(CFujitsu::Commands::Identification, &answer))
	{
		return false;
	}

	//TODO: переписка
	return (answer.size() >= 2);
}

//----------------------------------------------------------------------------
bool FujitsuPrinter::processCommand(const QByteArray & aCommand, QByteArray * aAnswer)
{
	if (!mIOPort->write(aCommand))
	{
		return false;
	}

	QByteArray data;
	QByteArray & answer = aAnswer ? *aAnswer : data;

	//TODO: переписка
	if (isNeedAnswer(aCommand))
	{
		SleepHelper::msleep(5);
		mIOPort->read(answer);

		return !answer.isEmpty();
	}

	return true;
}

//----------------------------------------------------------------------------
bool FujitsuPrinter::isNeedAnswer(const QByteArray & aCommand) const
{
	return (aCommand == CFujitsu::Commands::Identification) ||
	       (aCommand == CFujitsu::Commands::Status) ||
	       (aCommand == CFujitsu::Commands::Voltage);
}

//--------------------------------------------------------------------------------
bool FujitsuPrinter::updateParameters()
{
	return processCommand(CFujitsu::Commands::Initialize);
}

//--------------------------------------------------------------------------------
bool FujitsuPrinter::getStatus(TStatusCodes & aStatusCodes)
{
	QByteArray answer;

	if (!processCommand(CFujitsu::Commands::Status, &answer))
	{
		return false;
	}

	for (int i = 0; i < 8; ++i)
	{
		if ((answer[0] & (1 << i)) == (i != 7))
		{
			aStatusCodes.insert(CFujitsu::Statuses[i]);
		}
	}

	//TODO: buffer full

	//TODO: переписка - иногда не приходит ответ
	if (!processCommand(CFujitsu::Commands::Voltage, &answer))
	{
		return false;
	}

	double voltage = uchar(answer[0]) * 0.1;

	if (fabs(voltage - CFujitsu::Voltage::Nominal) > CFujitsu::Voltage::Delta)
	{
		aStatusCodes.insert(DeviceStatusCode::Error::PowerSupply);
	}

	return true;
}

//--------------------------------------------------------------------------------
