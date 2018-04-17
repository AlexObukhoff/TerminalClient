/* @file POS-принтер. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QSet>
#include <Common/QtHeadersEnd.h>

// STL
#include <algorithm>

// Project
#include "POSPrinter.h"

using namespace SDK::Driver::IOPort::COM;
using namespace PrinterStatusCode;

POSPrinters::TModelIds POSPrinters::CModelData::mModelIds;

//--------------------------------------------------------------------------------
POSPrinter::POSPrinter() : mModelID(0), mPrintingStringTimeout(0)
{
	// данные устройства
	mDeviceName = CPOSPrinter::DefaultName;
	setConfigParameter(CHardware::Printer::FeedingAmount, 4);
	setConfigParameter(CHardware::Printer::Commands::Cutting, "\x1B\x69");

	setDefaultModelData();

	//TODO: реализовать буферный поиск
	TSerialDevicePortParameters & portParameters = mModelData.getDefault().parameters.portSettings->data();

	for (auto it = portParameters.begin(); it != portParameters.end(); ++it)
	{
		if (mPortParameters.contains(it.key()))
		{
			mPortParameters[it.key()] = it.value();
		}
	}

	mOverflow = false;
	mRussianCodePage = CPOSPrinter::RussianCodePage;
}

//--------------------------------------------------------------------------------
void POSPrinter::setDefaultModelData()
{
	using namespace SDK::Driver::IOPort::COM;

	// параметры порта
	POSPrinters::PSerialDevicePortParameters portParameters(new POSPrinters::CSerialDevicePortParameters());
	portParameters->data().insert(EParameters::BaudRate, POSPrinters::TSerialDevicePortParameter()
		<< EBaudRate::BR115200
		<< EBaudRate::BR19200
		<< EBaudRate::BR57600
		<< EBaudRate::BR38400
		<< EBaudRate::BR9600
		<< EBaudRate::BR4800);
	portParameters->data().insert(EParameters::Parity, POSPrinters::TSerialDevicePortParameter()
		<< EParity::No
		<< EParity::Even
		<< EParity::Odd);
	portParameters->data().insert(EParameters::ByteSize, POSPrinters::TSerialDevicePortParameter()
		<< 8);
	portParameters->data().insert(EParameters::RTS, POSPrinters::TSerialDevicePortParameter()
		<< ERTSControl::Toggle);
	portParameters->data().insert(EParameters::DTR, POSPrinters::TSerialDevicePortParameter()
		<< EDTRControl::Handshake);

	// теги по умолчанию
	Tags::PEngine tagEngine(new Tags::Engine());
	tagEngine->appendSingle(Tags::Type::Bold, "\x1B\x45", "\x01");
	tagEngine->appendSingle(Tags::Type::UnderLine, "\x1B\x2D", "\x01");
	tagEngine->set(Tags::Type::Image);
	tagEngine->set(Tags::Type::BarCode);

	// статусы TODO: корректировать при добавлении моделей
	POSPrinters::PAllErrors errors(new POSPrinters::CAllErrors());
	errors->data()[1][1].insert('\x08', DeviceStatusCode::Error::Unknown);
	errors->data()[2][1].insert('\x20', PrinterStatusCode::Error::PaperEnd);
	errors->data()[2][1].insert('\x40', DeviceStatusCode::Error::Unknown);
	errors->data()[3][1].insert('\x60', DeviceStatusCode::Error::Unknown);

	mModelData.setDefault(POSPrinters::SModelData(CPOSPrinter::DefaultName, false, POSPrinters::SParameters(portParameters, errors, tagEngine), "Default"));
	mPortParameters = portParameters->data();
}

//--------------------------------------------------------------------------------
bool POSPrinter::isConnected()
{
	if (!mIOPort->write(CPOSPrinter::Command::Initialize))
	{
		return false;
	}

	if (!waitReady(CPOSPrinter::AvailableWaiting))
	{
		return false;
	}

	/*
	bool errors = std::find_if(statusCodes.begin(), statusCodes.end(), [&] (int aStatusCode) -> bool
		{ return mStatusCodesSpecification->value(aStatusCode).warningLevel == SDK::Driver::EWarningLevel::Error; }) != statusCodes.end();

	//TODO: выяснить причины отсутствия ответа на данную команду для некоторых принтеров
	QByteArray answer;

	if (!errors && (!mIOPort->write(CPOSPrinter::Command::GetPaperStatus) || !getAnswer(answer, CPOSPrinter::Timeouts::Status) || answer.isEmpty()))
	{
		return false;
	}
	*/
	QByteArray answer;

	if (!getModelId(answer))
	{
		checkVerifying();

		return false;
	}

	bool onlyDefaultModels = isOnlyDefaultModels();
	mVerified = false;

	if (answer.isEmpty())
	{
		bool isLoading = !isAutoDetecting();
		bool result = onlyDefaultModels || isLoading;
		LogLevel::Enum logLevel = result ? LogLevel::Warning : LogLevel::Error;

		toLog(logLevel, QString("Unknown POS printer has detected, it is in error state (possibly), the plugin contains %1only default models and the type of searching is %2")
			.arg(onlyDefaultModels ? "" : "not ")
			.arg(isLoading ? "loading" : "autodetecting"));

		if (!onlyDefaultModels && isLoading)
		{
			mDeviceName = getConfigParameter(CHardwareSDK::ModelName).toString();
		}

		mVerified = result;

		if (result)
		{
			mTagEngine = mModelData.getDefault().parameters.tagEngine;
		}

		return result;
	}

	char modelId = parseModelId(answer);

	if (!mModelData.getModelIds().contains(modelId))
	{
		LogLevel::Enum logLevel = onlyDefaultModels ? LogLevel::Warning : LogLevel::Error;
		toLog(logLevel, QString("Unknown POS printer has detected, it model id = %1 is unknown and the plugin contains %2only default models")
			.arg(ProtocolUtils::toHexLog(modelId))
			.arg(onlyDefaultModels ? "" : "not "));

		if (onlyDefaultModels)
		{
			mTagEngine = mModelData.getDefault().parameters.tagEngine;
		}

		return onlyDefaultModels;
	}

	QString name = mModelData[modelId].name;
	QString description = mModelData[modelId].description;

	if (!description.isEmpty())
	{
		name += QString(" (%1)").arg(description);
	}

	if (!mModelData.data().keys().contains(modelId))
	{
		mModelCompatibility = false;
		toLog(LogLevel::Error, name + " is detected, but not supported by this pligin");

		mTagEngine = mModelData[mModelID].parameters.tagEngine;

		return true;
	}

	mModelID = modelId;
	mDeviceName = mModelData[mModelID].name;
	mTagEngine = mModelData[mModelID].parameters.tagEngine;

	processDeviceData();

	mVerified = mModelData[mModelID].verified;

	return true;
}

//--------------------------------------------------------------------------------
void POSPrinter::processDeviceData()
{
	QByteArray answer;

	if (mIOPort->write(CPOSPrinter::Command::GetTypeId) && getAnswer(answer, CPOSPrinter::Timeouts::Info) && (answer.size() == 1))
	{
		setDeviceParameter(CDeviceData::Printers::Unicode,       ProtocolUtils::getBit(answer, 0));
		setDeviceParameter(CDeviceData::Printers::Cutter,        ProtocolUtils::getBit(answer, 1));
		setDeviceParameter(CDeviceData::Printers::LabelPrinting, ProtocolUtils::getBit(answer, 2));
	}

	if (mIOPort->write(CPOSPrinter::Command::GetROMVersion) && getAnswer(answer, CPOSPrinter::Timeouts::Info) && !answer.isEmpty())
	{
		QString firmware = (answer.size() > 1) ? answer : ProtocolUtils::toHexLog(char(answer[0]));
		setDeviceParameter(CDeviceData::Firmware, firmware);
	}
}

//--------------------------------------------------------------------------------
bool POSPrinter::getModelId(QByteArray & aAnswer) const
{
	return mIOPort->write(CPOSPrinter::Command::GetModelId) && getAnswer(aAnswer, CPOSPrinter::Timeouts::Info) && (aAnswer.size() <= 1);
}

//--------------------------------------------------------------------------------
char POSPrinter::parseModelId(QByteArray & aAnswer)
{
	return aAnswer[0];
}

//--------------------------------------------------------------------------------
void POSPrinter::checkVerifying()
{ 
	if (!isAutoDetecting() && containsConfigParameter(CHardwareSDK::ModelName))
	{
		QString modelName = getConfigParameter(CHardwareSDK::ModelName).toString();

		if (!modelName.isEmpty())
		{
			POSPrinters::TModelData::iterator modelDataIt = std::find_if(mModelData.data().begin(), mModelData.data().end(), [&] (const POSPrinters::SModelData & aModelData) -> bool
				{ return aModelData.name == modelName; });

			if (modelDataIt != mModelData.data().end())
			{
				mVerified = modelDataIt->verified;
			}
		}
	}
}

//--------------------------------------------------------------------------------
bool POSPrinter::updateParameters()
{
	//TODO: устанавливать размеры ячейки сетки для печати, с учетом выбранного режима.
	char lineSpacing = char(getConfigParameter(CHardware::Printer::Settings::LineSpacing, 0).toInt());
	QByteArray setLineSpacing;

	if (lineSpacing)
	{
		setLineSpacing = QByteArray(CPOSPrinter::Command::SetLineWidthMultiplier) + lineSpacing;
	}

	QByteArray command = QByteArray(CPOSPrinter::Command::Initialize) +
		CPOSPrinter::Command::SetEnabled + 
		setLineSpacing +
		CPOSPrinter::Command::SetCodePage + mRussianCodePage +
		CPOSPrinter::Command::SetCharacterSet + CPOSPrinter::USACharacters +
		CPOSPrinter::Command::SetStandartMode +
		CPOSPrinter::Command::AlignLeft;

	if (!mIOPort->write(command))
	{
		return false;
	}

	mVerified = mVerified && !isOnlyDefaultModels();
	SleepHelper::msleep(CPOSPrinter::InitializationPause);

	return true;
}

//--------------------------------------------------------------------------------
bool POSPrinter::isOnlyDefaultModels()
{
	return mModelData.data().keys().isEmpty();
}

//--------------------------------------------------------------------------------
bool POSPrinter::printLine(const QByteArray & aString)
{
	QDateTime beginning = QDateTime::currentDateTime();

	if (!mIOPort->write(aString))
	{
		return false;
	}

	if (mPrintingStringTimeout)
	{
		int count = aString.contains(mTagEngine->value(Tags::Type::DoubleHeight).open) ? 2 : 1;
		int pause = count * mPrintingStringTimeout - int(beginning.msecsTo(QDateTime::currentDateTime()));

		if (pause > 0)
		{
			toLog(LogLevel::Debug, mDeviceName + QString(": Pause after printing line = %1 ms").arg(pause));
			SleepHelper::msleep(pause);
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
QByteArray POSPrinter::prepareBarcodePrinting()
{
	return QByteArray() +
		CPOSPrinter::Command::Barcode::Height + CPOSPrinter::Barcode::Height +
		CPOSPrinter::Command::Barcode::HRIPosition + CPOSPrinter::Barcode::HRIPosition +
		CPOSPrinter::Command::Barcode::FontSize + CPOSPrinter::Barcode::FontSize +
		CPOSPrinter::Command::Barcode::Width + CPOSPrinter::Barcode::Width;
}

//--------------------------------------------------------------------------------
bool POSPrinter::printBarcode(const QString & aBarcode)
{
	QByteArray data = mCodec->fromUnicode(aBarcode);
	QByteArray barcodePrinting = QByteArray() + CPOSPrinter::Command::Barcode::Print +	CPOSPrinter::Barcode::CodeSystem128 +
		char(data.size() + 2) + CPOSPrinter::Barcode::Code128Spec + data + '\x0A';

	return mIOPort->write(prepareBarcodePrinting() + barcodePrinting);
}

//--------------------------------------------------------------------------------
bool POSPrinter::getStatus(TStatusCodes & aStatusCodes)
{
	POSPrinters::TAllErrors errorData = mModelData[mModelID].parameters.errors->data();

	for (auto it = errorData.begin(); it != errorData.end(); ++it)
	{
		SleepHelper::msleep(CPOSPrinter::StatusPause);

		QList<char> statusCommands = it->keys();
		QByteArray answer;

		//TODO: попробовать сделать контроль префикса ответа для Custom VKP-80
		QList<char> bytes = it->keys();
		qSort(bytes);

		if (!mIOPort->write(QByteArray(CPOSPrinter::Command::GetStatus) + it.key()) || !readStatusAnswer(answer, CPOSPrinter::Timeouts::Status, int(bytes.last())) || answer.isEmpty())
		{
			return false;
		}

		mOverflow = answer.startsWith(ASCII::XOff);

		if (mOverflow && (mInitialized == ERequestStatus::Success) && !mStatusCollection.isEmpty() && !mStatusCollection.contains(DeviceStatusCode::Error::NotAvailable))
		{
			aStatusCodes = getStatusCodes(mStatusCollection);

			return true;
		}

		if (answer.size() != *(std::max_element(statusCommands.begin(), statusCommands.end())))
		{
			return false;
		}

		for (auto jt = it.value().begin(); jt != it.value().end(); ++jt)
		{
			for (auto kt = jt->begin(); kt != jt->end(); ++kt)
			{
				if (answer[jt.key() - 1] & kt.key())
				{
					aStatusCodes.insert(kt.value());
				}
			}
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
bool POSPrinter::readStatusAnswer(QByteArray & aAnswer, int aTimeout, int aBytesCount) const
{
	QVariantMap configuration;
	configuration.insert(CHardware::Port::IOLogging, QVariant().fromValue(ELoggingType::Write));
	mIOPort->setDeviceConfiguration(configuration);

	QTime timer;
	timer.start();

	do
	{
		QByteArray data;

		if (!mIOPort->read(data, 10))
		{
			return false;
		}

		aAnswer.append(data);
	}
	while ((timer.elapsed() < aTimeout) && (aAnswer.size() < aBytesCount));

	LogLevel::Enum logLevel = (aAnswer.size() < aBytesCount) ? LogLevel::Error : ((aAnswer.size() > aBytesCount) ? LogLevel::Warning : LogLevel::Normal);
	toLog(logLevel, QString("%1: << {%2}").arg(mDeviceName).arg(aAnswer.toHex().data()));

	return true;
}

//--------------------------------------------------------------------------------
bool POSPrinter::printImage(const QImage & aImage, const Tags::TTypes & aTags)
{
	int widthInBytes = aImage.width() / 8 + ((aImage.width() % 8) ? 1 : 0);
	bool doubleWidth  = aTags.contains(Tags::Type::DoubleWidth) || aTags.contains(Tags::Type::DoubleWidth);
	bool doubleHeight = aTags.contains(Tags::Type::DoubleHeight);
	char imageTagFactors = (doubleWidth  * CPOSPrinter::ImageFactors::DoubleWidth) |
	                       (doubleHeight * CPOSPrinter::ImageFactors::DoubleHeight);

	QByteArray request;
	request.append(CPOSPrinter::Command::PrintImage);
	request.append(imageTagFactors);
	request.append(uchar(widthInBytes));
	request.append(char(0));
	request.append(uchar(aImage.height() % 256));
	request.append(uchar(aImage.height() / 256));

	for (int i = 0; i < aImage.height(); ++i)
	{
		request.append((const char *)aImage.scanLine(i), widthInBytes);
	}

	if (!mIOPort->write(request))
	{
		return false;
	}

	//TODO: под рефакторинг.
	//Причины необходимости задержки ясны не до конца, т.к. задержка начинает работать после фактической печати картинки.
	//Задержка нужна тем большая, чем больше картинок печатается одновременно или почти одновременно, через какое-то количество строк текста.
	int pause = qMin(int(double(request.size()) / 2), 5000);
	toLog(LogLevel::Debug, mDeviceName + QString(": size = %1, pause = %2").arg(request.size()).arg(pause));
	SleepHelper::msleep(pause);

	return true;
}

//--------------------------------------------------------------------------------
