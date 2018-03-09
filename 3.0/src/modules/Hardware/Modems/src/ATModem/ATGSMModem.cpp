/* @file AT-совместимый модем. */

// STL
#include <algorithm>
#include <cmath>

// Thirdparty
#include <Common/QtHeadersBegin.h>
#include "smspdudecoder.h"
#include "smspduencoder.h"
#include <Common/QtHeadersEnd.h>

// Project
#include "Hardware/Modems/ModemStatusesDescriptions.h"
#include "ATGSMModem.h"

using namespace SDK::Driver;

//--------------------------------------------------------------------------------
ATGSMModem::ATGSMModem()
{
	mGsmDialect = AT::EModemDialect::DefaultAtGsm;
	mStatusCodesSpecification = DeviceStatusCode::PSpecifications(new ModemStatusCode::CSpecifications());

	mDeviceName = CATGSMModem::DefaultName;
}

//--------------------------------------------------------------------------------
bool ATGSMModem::getOperator(QString & aOperator)
{
	if (!checkConnectionAbility())
	{
		return false;
	}

	enableLocalEcho(false);
	processCommand(AT::Commands::CopsMode);

	QByteArray answer;
	QRegExp regExp("\".+\"");
	bool result = false;

	if (processCommand(AT::Commands::COPS, answer) && regExp.indexIn(answer) != -1)
	{
		// Парсим имя оператора.
		aOperator = regExp.cap(0).remove("\"");

		toLog(LogLevel::Normal, QString("Operator name: %1.").arg(aOperator));
		result = true;
	}

	mIOPort->close();

	return result;
}

//--------------------------------------------------------------------------------
void ATGSMModem::setDeviceConfiguration(const QVariantMap & aConfiguration)
{
	ATModemBase::setDeviceConfiguration(aConfiguration);

	if (aConfiguration.contains(CHardwareSDK::ModelName))
	{
		setDeviceName(aConfiguration.value(CHardwareSDK::ModelName).toString().toLatin1());
	}
}

//--------------------------------------------------------------------------------
void ATGSMModem::setDeviceName(const QByteArray & aFullName)
{
	ATModemBase::setDeviceName(aFullName);

	if (mDeviceName.contains("Cinterion", Qt::CaseInsensitive) || mDeviceName.contains("SIEMENS", Qt::CaseInsensitive))
	{
		mGsmDialect = AT::EModemDialect::Siemens;
	}
	else if (mDeviceName.contains("SIMCOM", Qt::CaseInsensitive))
	{
		mGsmDialect = AT::EModemDialect::SimCom;

		mDeviceName.remove(QRegExp("(\\s*Revision.*)"));
		mModemConfigTimeout = CATGSMModem::Timeouts::SimCom::Config;
	}
	else if (mDeviceName.contains("huawei", Qt::CaseInsensitive))
	{
		mGsmDialect = AT::EModemDialect::Huawei;

		QString value;

		if (parseFieldInternal(aFullName, "Manufacturer", value))
		{
			mDeviceName = value;
		}

		if (parseFieldInternal(aFullName, "Model", value))
		{
			mDeviceName += " " + value;
		}
	}
	else if (mDeviceName.contains(QRegExp("MF\\d{3}", Qt::CaseInsensitive)) || mDeviceName.contains("ZTE", Qt::CaseInsensitive))
	{
		mGsmDialect = AT::EModemDialect::ZTE;

		QString value;

		if (parseFieldInternal(aFullName, "Manufacturer", value))
		{
			mDeviceName = value;

			if (parseFieldInternal(aFullName, "Model", value))
			{
				mDeviceName += " " + value;
			}
		}
		else
		{
			mDeviceName = "ZTE " + mDeviceName.mid(QRegExp("MF\\d{3}", Qt::CaseInsensitive).indexIn(mDeviceName), 5);
		}
	}
}

//--------------------------------------------------------------------------------
bool ATGSMModem::getInfo(QString & aInfo)
{
	toLog(LogLevel::Normal, "Retrieve modem info.");

	if (!checkConnectionAbility())
	{
		return false;
	}

	enableLocalEcho(false);

	getSIMData(AT::Commands::IMEI);
	getSIMData(AT::Commands::IMSI);
	getSIMData(AT::Commands::CNUM);

	QByteArray answer;
	QString value;

	if (processCommand(AT::Commands::Revision, answer))
	{
		QString data = parseFieldInternal(answer, "Revision", value) ? value : answer;
		setDeviceParameter(CDeviceData::Revision, data);
	}

	switch (mGsmDialect)
	{
		case AT::EModemDialect::Siemens:
		{
			getSIMData(AT::Commands::Siemens::SIMID);

			if (getSiemensCellList(value))
			{
				setDeviceParameter(CDeviceData::Modems::GSMCells, value);
			}

			break;
		}
		//--------------------------------------------------------------------------------
		case AT::EModemDialect::Huawei:
		{
			getSIMData(AT::Commands::Huawei::SIMID);

			break;
		}
		//--------------------------------------------------------------------------------
		case AT::EModemDialect::ZTE:
		{
			getSIMData(AT::Commands::ZTE::SIMID);

			break;
		}
		//--------------------------------------------------------------------------------
		case AT::EModemDialect::SimCom:
		{
			if (getSimCOMCellList(value))
			{
				setDeviceParameter(CDeviceData::Modems::GSMCells, value);
			}

			break;
		}
	}

	mIOPort->close();

	aInfo.clear();

	foreach (const QString & aKey, CATGSMModem::SIMRequestInfo.getDeviceDataKeys())
	{
		aInfo += QString("\n%1: %2").arg(aKey).arg(getDeviceParameter(aKey).toString());
	}

	return !aInfo.isEmpty();
}

//--------------------------------------------------------------------------------
void ATGSMModem::getSIMData(const QByteArray & aCommand)
{
	QByteArray answer;
	processCommand(aCommand, answer);

	CATGSMModem::SSIMRequestInfo SIMRequestInfo = CATGSMModem::SIMRequestInfo[aCommand];
	QRegExp regExp(SIMRequestInfo.regexpData);

	if (regExp.indexIn(answer) != -1)
	{
		QString result = regExp.cap(0);

		if (SIMRequestInfo.swapCharPair)
		{
			for (int i = 0; i < result.size(); i += 2)
			{
				QChar a = result[i];
				result[i] = result[i + 1];
				result[i + 1] = a;
			}
		}

		setDeviceParameter(SIMRequestInfo.name, result);
	}
}

//--------------------------------------------------------------------------------
bool ATGSMModem::parseFieldInternal(const QByteArray & aBuffer, const QString & aFieldName, QString & aValue)
{
	QRegExp rx(aFieldName + "[:\\s]+([^\\n\\r]+)", Qt::CaseInsensitive);

	if (rx.indexIn(QString::fromLatin1(aBuffer).trimmed()) != -1)
	{
		aValue = rx.cap(1).trimmed();

		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------
bool ATGSMModem::getSimCOMCellList(QString & aValue)
{
	QByteArray answer;
	QRegExp regExp("(\\d+),(\\d+),\"([\\d\\w]+)\",\"([\\d\\w]+)\"");
	QByteArray CGREG = AT::Commands::SimCom::CGREG;

	if (!processCommand(CGREG + "=2", CATGSMModem::Timeouts::CellInfo) ||
	    !processCommand(CGREG + "?", answer, CATGSMModem::Timeouts::CellInfo) || (regExp.indexIn(answer) == -1))
	{
		return false;
	}

	QStringList info = QStringList()
		<< "" << ""
		<< QString::number(regExp.cap(4).toInt(0, 16))
		<< QString::number(regExp.cap(3).toInt(0, 16))
		<< "" << "";

	aValue = info.join(",");

	return true;
}

//--------------------------------------------------------------------------------
bool ATGSMModem::getSiemensCellList(QString & aValue)
{
	QByteArray answer;

	if (!processCommand(AT::Commands::Siemens::GetCellList, answer, CATGSMModem::Timeouts::Siemens::CellInfo) || answer.contains("^SMONC:"))
	{
		return false;
	}

	QList<QString> cellList;
	QMap<QString, QStringList> cellsData;

	// MCC, MNC, LAC, cell, BSIC, chann, RSSI, C1, C2
	// Парсим
	QStringList items = QString::fromLatin1(answer).section(':', 1, 1).split(",");
	bool result = false;

	for (int i = 0; i < items.size(); i += 9)
	{
		//result: MCC,MNC,cell,LAC,RSSI,signal_strenght;

		if (items[i].trimmed().toInt() > 0 && (i+6) < items.size())
		{
			QStringList info;
			info << items[i].trimmed()
				<< items[i+1].trimmed()
				<< QString::number(items[i+3].toInt(0, 16)).trimmed()
				<< QString::number(items[i+2].toInt(0, 16)).trimmed()
				<< items[i+6].trimmed();
			
			cellList.push_back(items[i+5]);
			cellsData[items[i+5]] = info;

			result = true;
		}
	}

	QList<QByteArray> commands;
	commands
		<< AT::Commands::Siemens::ActiveCellInfo
		<< AT::Commands::Siemens::InactiveCellInfo;

	foreach (const QByteArray & command, commands)
	{
		if (processCommand(command, answer, CATGSMModem::Timeouts::CellInfo))
		{
			items = QString::fromLatin1(answer).trimmed().split("\n");

			foreach (const QString & line, items)
			{
				QStringList columns = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
				QString chann = columns[0].trimmed();

				if (chann.toInt() > 0)
				{
					cellsData[chann] << columns[2];

					result = true;
				}
			}
		}
	}

	QStringList value;

	foreach (const QString & chann, cellList)
	{
		value << cellsData[chann].join(",");
	}

	aValue = value.join(";");

	return result;
}

//--------------------------------------------------------------------------------
bool ATGSMModem::getSignalQuality(int & aQuality)
{
	/* Signal quality: +CSQ
		+CSQ: [rssi],[ber]

		where
		[rssi] – received signal strength indication
			0 – (-113) dBm or less
			1 – (-111) dBm
			2..30 – (-109)dBm..(-53)dBm / 2 dBm per step
			31 – (-51)dBm or greater
			99 – not known or not detectable		

		[ber] – bit error rate (in percent)

		http://gsmfordummies.com/air/radiolink.shtml
		http://www.freepatentsonline.com/6035183.html

	*/
	if (!checkConnectionAbility())
	{
		return false;
	}

	enableLocalEcho(false);

	QByteArray answer;
	processCommand(AT::Commands::CSQ, answer);

	// Парсим уровень сигнала.
	QRegExp regExp("(\\d+),(\\d+)");
	bool result = false;

	if (regExp.indexIn(answer) != -1)
	{
		result = true;

		aQuality = regExp.cap(1).toInt();
		int ber = regExp.cap(2).toInt();

		toLog(LogLevel::Normal, QString("Signal quality (rssi, ber):(%1, %2).").arg(aQuality).arg(ber));
	}

	mIOPort->close();

	return result;
}

//--------------------------------------------------------------------------------
bool ATGSMModem::reset()
{
	if (!checkConnectionAbility())
	{
		return false;
	}

	enableLocalEcho(false);

	// Сбрасываем модем.
	toLog(LogLevel::Normal, "Resetting modem to factory defaults...");

	switch(mGsmDialect)
	{
		case AT::EModemDialect::Siemens:
		{
			toLog(LogLevel::Normal, "Restart Siemens modem...");
			processCommand(AT::Commands::Siemens::Restart);

			break;
		}
		case AT::EModemDialect::ZTE:
		{
			toLog(LogLevel::Normal, "Power on ZTE modem...");
			processCommand(AT::Commands::ZTE::PowerOn);

			break;
		}
		case AT::EModemDialect::SimCom:
		{
			toLog(LogLevel::Normal, "Restart SimCOM modem...");
			processCommand(AT::Commands::SimCom::Restart);

			break;
		}
	}

	// Проверяем, откликается ли модем
	if (!checkAT(CATGSMModem::Timeouts::ResetConnection))
	{
		mIOPort->close();
		return false;
	}

	bool result = processCommand(AT::Commands::ATandF0);

	toLog(LogLevel::Normal, "Wait GSM network accessability...");

	if (waitNetworkAccessability(CATGSMModem::Timeouts::Connection))
	{
		toLog(LogLevel::Normal, "GSM network available.");
	}
	else
	{
		toLog(LogLevel::Warning, "GSM network still waiting...");
	}

	onPoll();

	mIOPort->close();

	return result;
}

//---------------------------------------------------------------------------------
bool ATGSMModem::getStatus(TStatusCodes & aStatuses)
{
	if (!checkConnectionAbility())
	{
		return false;
	}

	enableLocalEcho(false);

	if (!processCommand(AT::Commands::AT, CATGSMModem::Timeouts::ResetConnection))
	{
		return false;
	}

	QByteArray answer;

	if (!processCommand(AT::Commands::CPIN, answer))
	{
		aStatuses.insert(ModemStatusCode::Error::SIMError);
		toLog(LogLevel::Error, QString("SIM card error, modem answer '%1'.").arg(answer.data()));
	}
	else
	{
		ENetworkAccessability::Enum networkAccessability;

		if (!getNetworkAccessability(networkAccessability) ||
			(networkAccessability != ENetworkAccessability::RegisteredHomeNetwork && networkAccessability != ENetworkAccessability::SearchingOperator))
		{
			aStatuses.insert(ModemStatusCode::Error::NoNetwork);
			toLog(LogLevel::Error, QString("Network is not available, modem answer '%1'.").arg(answer.data()));
		}
	}

	// Закрываем порт. TODO: найти более подходящее место для этого.
	mIOPort->close();

	return true;
}

//-------------------------------------------------------------------------------
bool ATGSMModem::getNetworkAccessability(ENetworkAccessability::Enum & aNetworkAccessability)
{
	aNetworkAccessability = ENetworkAccessability::Unknown;

	QByteArray answer;
	processCommand(AT::Commands::CREG, answer);

	QRegExp regExp("\\+CREG:\\s+\\d,(\\d)");
	regExp.setMinimal(true);

	if (regExp.indexIn(answer) < 0)
	{
		return false;
	}

	aNetworkAccessability = static_cast<ENetworkAccessability::Enum>(regExp.cap(1).toInt());

	return true;
}

//-------------------------------------------------------------------------------
bool ATGSMModem::waitNetworkAccessability(int aTimeout)
{
	ENetworkAccessability::Enum networkAccessability;
	QTime t;

	t.start();

	do 
	{
		if (getNetworkAccessability(networkAccessability) && 
			(networkAccessability == ENetworkAccessability::RegisteredHomeNetwork || networkAccessability == ENetworkAccessability::RegisteredRoaming))
		{
			return true;
		}

	}
	while (t.elapsed() < aTimeout);

	return false;
}

//-------------------------------------------------------------------------------
bool ATGSMModem::getCUSDMessage(const QByteArray & aBuffer, QString & aMessage)
{
	QString str = QString::fromLatin1(aBuffer).simplified();

	if (mGsmDialect == AT::EModemDialect::SimCom)
	{
		str.clear();

		foreach (char c, aBuffer)
		{
			if (c)
			{
				str.append(c);
			}
		}
	}

	QRegExp regExp(".*\\+CUSD: ?(\\d)(\\,\"(.*)\"\\,(\\d+))?.*");

	if (!regExp.exactMatch(str) || (regExp.captureCount() < 4))
	{
		return false;
	}

	QString msg = regExp.cap(3);

	if (regExp.cap(4).toInt() == 72)
	{
		// Проверяем сообщение это HEX строка?
		QRegExp regExpHex("((\\d|[A-F]|[a-f])+)");

		if (regExpHex.exactMatch(msg))
		{
			QByteArray hex;

			// Декодируем сообщение.
			foreach(auto aChar, msg)
			{
				hex.append(aChar);

				if (hex.size() == 4)
				{
					aMessage.push_back(QChar(hex.toInt(0,16)));
					hex.clear();
				}
			}
		}
		else
		{
			char bigEndianBom[] = { '\xFE', '\xFF', '\x00' };
			msg.prepend(bigEndianBom);

			aMessage = QString::fromUtf16((const ushort *)msg.data(), msg.size() / 2);
		}
	}
	else
	{
		aMessage = msg;
	}

	return true;
}

//-------------------------------------------------------------------------------
bool ATGSMModem::processUSSD(const QString & aMessage, QString & aAnswer)
{
	if (!checkConnectionAbility())
	{
		return false;
	}

	enableLocalEcho(false);

	QByteArray command;
	int commandTimeout = CATGSMModem::Timeouts::Default;

	toLog(LogLevel::Normal, QString("ATGSMModem: sending USSD '%1'.").arg(aMessage));

	command.append(AT::Commands::CUSD);
	command.append(",\"");

	switch (mGsmDialect)
	{
		case AT::EModemDialect::Huawei:
		{
			command.append(SmsPduEncoder::encode(aMessage.toLatin1()).toHex());
			break;
		}
		case AT::EModemDialect::SimCom:
		{
			commandTimeout = CATGSMModem::Timeouts::SimCom::USSD;

			// Переключаем кодировку USSD
			processCommand(AT::Commands::SimCom::CSCS);
		}
		default:
		{
			command.append(aMessage);
		}
	}

	command.append("\",15");

	QByteArray answer;

	if (!processCommand(command, answer, commandTimeout))
	{
		if ((mGsmDialect != AT::EModemDialect::ZTE) || !answer.contains("Unexpected Data Value"))
		{
			mIOPort->close();
			return false;
		}

		// повторяем команду для ZTE модемов, т.к. они с первого раза USSD не понимают
		SleepHelper::msleep(CATGSMModem::Pauses::ZTE::USSDAttempt);

		toLog(LogLevel::Normal, "Retry send USSD for ZTE modems");

		if (!processCommand(command, answer, commandTimeout))
		{
			mIOPort->close();
			return false;
		}
	}

	for (int attempt = 0; attempt < CATGSMModem::BalanceAttempts; attempt++)
	{
		toLog(LogLevel::Normal, QString("Waiting for USSD answer, attempt %1.").arg(attempt + 1));

		SleepHelper::msleep(CATGSMModem::Pauses::BalanceAttempt);
		QByteArray data;

		// Делаем несколько попыток прочитать данные из порта.
		for (int i = 0; i < 4 && mIOPort->read(data); i++)
		{
			answer.append(data);

			toLog(LogLevel::Normal, QString("Total received string: %1").arg(QString::fromLatin1(answer)));

			// Вытаскиваем USSD часть из ответа.
			if (getCUSDMessage(answer, aAnswer))
			{
				toLog(LogLevel::Normal, QString("USSD answer: %1").arg(aAnswer));

				mIOPort->close();
				return true;
			}

			// Ответ от устройства содержит слово ERROR и ничего более.
			if (answer.indexOf("ERROR") >= 0)
			{
				toLog(LogLevel::Warning, QString("USSD request failed: %1").arg(QString::fromLatin1(answer)));

				mIOPort->close();
				return false;
			}

			SleepHelper::msleep(CATGSMModem::Pauses::BalanceData);
		}
	}

	mIOPort->close();

	return false;
}

//--------------------------------------------------------------------------------
bool ATGSMModem::sendMessage(const QString & aPhone, const QString & aMessage)
{
	// TODO: учитывать таймауты.
	if (!checkConnectionAbility())
	{
		return false;
	}

	enableLocalEcho(false);

	toLog(LogLevel::Normal, QString("ATGSMModem: sending SMS to number %1.").arg(aPhone));
	processCommand(AT::Commands::SetTextMode);

	QByteArray command = AT::Commands::SendSMS + aPhone.toLatin1();
	QByteArray answer;
	processCommand(command, answer);

	bool result = false;

	if (answer.indexOf(">") > -1)
	{
		command = aMessage.toLatin1() + AT::Commands::StrgZ;

		if (mIOPort->write(command))
		{
			// Ожидаем прихода уведомления об отправке сообщения.
			SleepHelper::msleep(CATGSMModem::Pauses::Message);

			result = mIOPort->read(answer) && (answer.indexOf("OK") > -1);
		}
	}

	mIOPort->close();

	return result;
}

//--------------------------------------------------------------------------------
bool ATGSMModem::takeMessages(TMessages & aMessages)
{
	if (!checkConnectionAbility())
	{
		return false;
	}

	enableLocalEcho(false);

	toLog(LogLevel::Normal, "Read SMS from the modem");
	processCommand(AT::Commands::SetPDUMode);

	QByteArray answer;
	processCommand(AT::Commands::ListSMS, answer, CATGSMModem::Timeouts::SMS);

	QString answerData = QString::fromLatin1(answer);

	QList<int> messageIds;
	QList<SmsPart> parts;

	for (int pos = 0; pos >= 0;)
	{
		QRegExp rx("\\+CMGL:\\s+(\\d+),.*[\\r\\n]+([0-9A-Fa-f]+)[\\r\\n]+");
		rx.setMinimal(true);

		pos = rx.indexIn(answerData, pos);

		if (pos >= 0)
		{
			toLog(LogLevel::Debug, QString("SMS %1: %2").arg(rx.cap(1).toInt()).arg(rx.cap(2)));

			messageIds << rx.cap(1).toInt();
			parts << Sms::decode(rx.cap(2));
			++pos;
		}
	}

	toLog(LogLevel::Normal, QString("Readed %1 SMS parts").arg(parts.size()));

	QList<Sms> messages = Sms::join(parts);
	toLog(LogLevel::Normal, QString("SMS parts joined to %1 SMS").arg(messages.size()));

	foreach (auto message, messages)
	{
		if (message.isValid())
		{
			GSM::SSMS sms;
		
			sms.date = message.getDateTime().toLocalTime();
			sms.from = message.getSenderAddress();
			sms.text = message.getText();

			aMessages << sms;
		}
		else
		{
			toLog(LogLevel::Warning, message.getErrorString());
		}
	}

	qStableSort(aMessages.begin(), aMessages.end(), [] (const SDK::Driver::GSM::SSMS & aSms1, const SDK::Driver::GSM::SSMS & aSms2) -> bool 
		{ return aSms1.date < aSms2.date; });

	// удаляем из модема все прочитанные сообщения
	foreach (auto partId, messageIds)
	{
		QByteArray command = AT::Commands::DeleteSMS + QByteArray::number(partId);
		processCommand(command);
	}

	return !aMessages.isEmpty();
}

//--------------------------------------------------------------------------------
