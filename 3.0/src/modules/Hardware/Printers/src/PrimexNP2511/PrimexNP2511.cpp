/* @file Принтер PrimexNP2511. */

#include "PrimexNP2511.h"

using namespace SDK::Driver::IOPort::COM;
using namespace PrinterStatusCode;

//--------------------------------------------------------------------------------
PrimexNP2511::PrimexNP2511()
{
	// данные устройства
	mDeviceName = "Primex NP-2511";

	// данные порта
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR115200);   // default
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR38400);
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR19200);
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);

	mPortParameters[EParameters::Parity].append(EParity::No);    // default
	mPortParameters[EParameters::Parity].append(EParity::Even);
	mPortParameters[EParameters::Parity].append(EParity::Odd);

	// теги
	mTagEngine = Tags::PEngine(new CPrimexNP2511::TagEngine());

	// кодек
	mCodec = CodecByName[CHardware::Codepages::Win1251];

	// данные устройства
	mDeviceName = "Primex NP-2511";
	setConfigParameter(CHardware::Printer::FeedingAmount, 2);
	setConfigParameter(CHardware::Printer::Commands::Cutting, "\x1B\x69");
}

//--------------------------------------------------------------------------------
bool PrimexNP2511::isConnected()
{
	CPrimexNP2511::TDeviceParameters deviceParameters = CPrimexNP2511::CDeviceParameters().data();
	bool result = true;

	for (auto it = deviceParameters.begin(); it != deviceParameters.end(); ++it)
	{
		QString data;

		if (!processDeviceData(it, data))
		{
			result = false;
			break;
		}

		setDeviceParameter(it->description, data);
	}

	return result;
}

//--------------------------------------------------------------------------------
bool PrimexNP2511::backFeed(int aCount)
{
	return !aCount || mIOPort->write(QByteArray(CPrimexNP2511::Commands::BackFeed) + char(aCount));
}

//--------------------------------------------------------------------------------
bool PrimexNP2511::updateParameters()
{
	//TODO: вынести выбор кодовой страницы в настройки плагина
	return mIOPort->write(QByteArray(CPrimexNP2511::Commands::Initilize) + CPrimexNP2511::Commands::SetCyrillicPage);
}

//--------------------------------------------------------------------------------
bool PrimexNP2511::printReceipt(const Tags::TLexemeReceipt & aLexemeReceipt)
{
	//TODO: проверить возможности эжектора и вынести все соответствующие парамтры в настройки плагина
	return mIOPort->write(CPrimexNP2511::Commands::ClearDispenser) && backFeed(CPrimexNP2511::BackFeedCount) &&
	       mIOPort->write(CPrimexNP2511::Commands::AutoRetract) && SerialPrinterBase::printReceipt(aLexemeReceipt);
}

//--------------------------------------------------------------------------------
bool PrimexNP2511::printBarcode(const QString & aBarcode)
{
	QByteArray request = QByteArray(
		CPrimexNP2511::Commands::BarCode::SetHRIPosition) +
		CPrimexNP2511::Commands::BarCode::SetHeight +
		CPrimexNP2511::Commands::BarCode::SetFont + CPrimexNP2511::PrinterBarCodeFontSize +
		CPrimexNP2511::Commands::BarCode::SetWidth +
		CPrimexNP2511::Commands::BarCode::Print + mCodec->fromUnicode(aBarcode) + ASCII::NUL;

	return mIOPort->write(request);
}

//--------------------------------------------------------------------------------
bool PrimexNP2511::getStatus(TStatusCodes & aStatusCodes)
{
	QByteArray answer;
	
	if (!mIOPort->write(CPrimexNP2511::Commands::GetStatus) || !mIOPort->read(answer) || (answer.size() != 1))
	{
		return false;
	}

	for (int i = 0; i < 8; ++i)
	{
		if (answer[0] & (1 << i))
		{
			aStatusCodes.insert(CPrimexNP2511::Statuses[i]);
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
bool PrimexNP2511::processDeviceData(const CPrimexNP2511::TDeviceParametersIt & aIt, QString & aData)
{
	QByteArray answer;

	if (!mIOPort->write(CPrimexNP2511::Commands::PrinterInfo + aIt.key()) || !mIOPort->read(answer))
	{
		return false;
	}

	QString errorLog = QString("Failed to get %1 due to ").arg(aIt->description);

	if (answer.size() < aIt->size)
	{
		toLog(LogLevel::Error, errorLog + QString("length of the packet is too small, %1 < %2.")
			.arg(answer.size())
			.arg(aIt->size));
		return false;
	}

	if (answer[0] != ASCII::Full)
	{
		toLog(LogLevel::Error, errorLog + QString("first byte = %1 is wrong, need %2")
			.arg(ProtocolUtils::toHexLog(char(answer[0])))
			.arg(ProtocolUtils::toHexLog(ASCII::Full)));
		return false;
	}

	if (answer[1] != aIt.key())
	{
		toLog(LogLevel::Error, QString("second byte = %1 is wrong, need %2 = command")
			.arg(ProtocolUtils::toHexLog(char(answer[1])))
			.arg(ProtocolUtils::toHexLog(aIt.key())));
		return false;
	}

	aData = answer.mid(2);

	return true;
}

//--------------------------------------------------------------------------------
