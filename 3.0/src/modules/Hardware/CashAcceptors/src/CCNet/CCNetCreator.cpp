/* @file Купюроприемник Creator на протоколе CCNet. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegExp>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// Modules
#include "CCNetCreator.h"
#include "CCNetCreatorConstants.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//---------------------------------------------------------------------------
CCNetCreator::CCNetCreator()
{
	// данные устройства
	mDeviceName = CCCNet::Models::CreatorC100;
	mSupportedModels = QStringList() << mDeviceName;
	mNeedChangeBaudrate = true;
	setConfigParameter(CHardware::UpdatingFilenameExtension, "dat");

	setConfigParameter(CHardware::CashAcceptor::InitializeTimeout, CCCNetCreator::ExitInitializeTimeout);

	using namespace CCCNetCreator::Commands;

	mCommandData.add(GetInternalVersion, false);
	mCommandData.add(UpdatingFirmware::SetBaudRate, false, 500);
	mCommandData.add(UpdatingFirmware::WriteHead, false);
	mCommandData.add(UpdatingFirmware::WriteBlock, false);
	mCommandData.add("", false);
	mCommandData.add(UpdatingFirmware::Exit, false, 500);
}

//--------------------------------------------------------------------------------
QString CCNetCreator::parseDeviceData(const QByteArray & aData, const QString & aPattern)
{
	QByteArray data(aData);

	for (char ch = ASCII::NUL; ch < ASCII::Space; ++ch)
	{
		data.replace(ch, ASCII::STX);
	}

	QRegExp regExp(aPattern);

	if (regExp.indexIn(data.simplified().toLower()) != -1)
	{
		QStringList result = regExp.capturedTexts();

		if (result.size() > 1)
		{
			return result[1].toUpper();
		}
	}

	return "";
}

//--------------------------------------------------------------------------------
void CCNetCreator::processDeviceData(QByteArray & aAnswer)
{
	QString firmware = parseDeviceData(aAnswer.left(15), "[\\w]+([\\d]+)$");

	removeDeviceParameter(CDeviceData::Firmware);
	removeDeviceParameter(CDeviceData::InternalFirmware);

	if (firmware.size() == 6)
	{
		QString logDate = QDate::fromString(firmware.left(4).prepend("20"), CCCNetCreator::DateFormat).toString(CCCNetCreator::DateLogFormat);

		setDeviceParameter(CDeviceData::Date, logDate, CDeviceData::Firmware);
		setDeviceParameter(CDeviceData::Revision, firmware.right(2), CDeviceData::Firmware);
	}

	setDeviceParameter(CDeviceData::SerialNumber, aAnswer.mid(15, 12));

	if (!processCommand(CCCNetCreator::Commands::GetInternalVersion, &aAnswer))
	{
		mOldFirmware = true;

		return;
	}

	QString hardwareInternal = parseDeviceData(aAnswer.left(15), "[^\\d\\w]+([\\d\\w]+)$");
	setDeviceParameter(CDeviceData::InternalHardware, hardwareInternal);

	QString firmwareInternal = parseDeviceData(aAnswer.mid(15, 12), "ver([0-9\\,\\.]+)");
	setDeviceParameter(CDeviceData::Version, firmwareInternal, CDeviceData::InternalFirmware);

	QByteArray data = ProtocolUtils::clean(aAnswer.mid(27, 7));

	if (data.size() == 6)
	{
		QString logDate = QDate::fromString(data.left(4).prepend("20"), CCCNetCreator::DateFormat).toString(CCCNetCreator::DateLogFormat);

		setDeviceParameter(CDeviceData::Date, logDate, CDeviceData::InternalFirmware);
		setDeviceParameter(CDeviceData::Revision, data.right(2), CDeviceData::InternalFirmware);
	}
}

//--------------------------------------------------------------------------------
bool CCNetCreator::performUpdateFirmware(const QByteArray & aBuffer)
{
	if (!changeBaudRate(true) || !writeHead(aBuffer))
	{
		return false;
	}

	uint size = aBuffer.size() - CCCNetCreator::UpdatingFirmware::HeadSize;
	int blocks = qCeil(double(size) / CCCNetCreator::UpdatingFirmware::BlockSize);

	for (int i = 0; i < blocks; ++i)
	{
		if (!writeBlock(aBuffer, i, i == (blocks - 1)))
		{
			return false;
		}
	}

	if (mIOPort->getType() == EPortTypes::COM)
	{
		TPortParameters portParameters;
		mIOPort->getParameters(portParameters);
		portParameters[EParameters::BaudRate] = EBaudRate::BR9600;

		if (!mIOPort->setParameters(portParameters))
		{
			toLog(LogLevel::Error, mDeviceName + ": Failed to set port parameters");
			return false;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
bool CCNetCreator::performBaudRateChanging(const TPortParameters & aPortParameters)
{
	mProtocol.changePortParameters(aPortParameters);
	QByteArray answer;

	if (!processCommand(CCCNetCreator::Commands::UpdatingFirmware::SetBaudRate, &answer))
	{
		return false;
	}

	if (answer != CCCNetCreator::Commands::UpdatingFirmware::WriteHead)
	{
		toLog(LogLevel::Error, QString("%1: Wrong answer for set baud rate command = 0x%2, need 0x%3")
			.arg(mDeviceName).arg(answer.toHex().data()).arg(QByteArray(CCCNetCreator::Commands::UpdatingFirmware::WriteHead).toHex().data()));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool CCNetCreator::writeHead(const QByteArray & aBuffer)
{
	uint size = aBuffer.size() - CCCNetCreator::UpdatingFirmware::HeadSize;
	QByteArray commandData;

	for (int i = 0; i < 4; ++i)
	{
		commandData.append(uchar(size >> (i * 8)));
	}

	commandData.append(aBuffer.left(CCCNetCreator::UpdatingFirmware::HeadSize));
	QByteArray answer;

	if (!processCommand(CCCNetCreator::Commands::UpdatingFirmware::WriteHead, commandData, &answer))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to write firmware head");
		return false;
	}

	if (answer != CCCNetCreator::Commands::UpdatingFirmware::WriteBlock)
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to write firmware head due to error in answer");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool CCNetCreator::writeBlock(const QByteArray & aBuffer, int aIndex, bool aLast)
{
	int begin = CCCNetCreator::UpdatingFirmware::HeadSize + aIndex * CCCNetCreator::UpdatingFirmware::BlockSize;

	QByteArray commandData;
	commandData.append(uchar(aIndex));
	commandData.append(aBuffer.mid(begin, CCCNetCreator::UpdatingFirmware::BlockSize));

	QByteArray answer;

	if (!processCommand("", commandData, &answer))
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to write block %1").arg(aIndex));
		return false;
	}

	QByteArray exitData = CCCNetCreator::Commands::UpdatingFirmware::Exit;

	if (aLast && (answer == exitData))
	{
		return true;
	}

	if (answer.size() < 2)
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to write block %1 due to too small answer size = %2, need minimum 2").arg(aIndex).arg(answer.size()));
		return false;
	}

	if (answer[0] == CCCNetCreator::UpdatingFirmware::Answers::WritingBlockError)
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to write block %1 due to error in answer").arg(aIndex));
		return false;
	}

	uchar index = uchar(aIndex);
	uchar answerIndex = uchar(answer[1]);

	if (answerIndex != index)
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to write block %1 due to wrong block index in answer = %2, need %3").arg(aIndex).arg(answerIndex).arg(index));
		return false;
	}

	if (aLast && (!mProtocol.getAnswer(answer, mCommandData[exitData]) || (answer != exitData)))
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to read final packet after writing all updating blocks, need").arg(exitData.toHex().data()));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
