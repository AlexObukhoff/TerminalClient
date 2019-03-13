/* @file  Настройки терминала (данной инсталляции ПО). */

// STL
#include <array>

// Boost
#include <boost/property_tree/ptree.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

// SDK
#include <SDK/Drivers/Components.h>

// Project
#include "TerminalSettings.h"

namespace SDK {
namespace PaymentProcessor {

//---------------------------------------------------------------------------
namespace CDefaults
{
	const char DefaultDatabaseName[] = "data.db";
	const int DefaultDatabasePort = 0;
}

//---------------------------------------------------------------------------
QString TerminalSettings::getAdapterName()
{
	return CAdapterNames::TerminalAdapter;
}

//---------------------------------------------------------------------------
TerminalSettings::TerminalSettings(TPtree & aProperties)
	: mProperties(aProperties.get_child(CAdapterNames::TerminalAdapter, aProperties))
{
}

//---------------------------------------------------------------------------
TerminalSettings::~TerminalSettings()
{
}

//---------------------------------------------------------------------------
void TerminalSettings::initialize()
{
	foreach (auto error, mProperties.get("config.terminal.critical_errors", QString()).split(",", QString::SkipEmptyParts))
	{
		bool ok = false;
		int errorId = error.toInt(&ok);

		if (ok)
		{
			mCriticalErrors << errorId;
		}
	}
}

//---------------------------------------------------------------------------
SConnection TerminalSettings::getConnection() const
{
	SConnection connection;

	try
	{
		connection.name = mProperties.get<QString>("terminal.connection.name");

		QString type = mProperties.get<QString>("terminal.connection.type");
		connection.type = type.compare("modem", Qt::CaseInsensitive) ? EConnectionTypes::Unmanaged : EConnectionTypes::Dialup;
		connection.checkInterval = mProperties.get("config.terminal.connection_check_interval.<xmlattr>.value", CConnection::DefaultCheckInterval);
		connection.checkInterval = connection.checkInterval < 1 ? 1 : connection.checkInterval;

		QNetworkProxy proxy;

		auto proxyType = mProperties.get<QString>("terminal.proxy.type", QString("none"));

		if (proxyType == "http")
		{
			proxy.setType(QNetworkProxy::HttpProxy);
		}
		else if (proxyType == "http_caching")
		{
			proxy.setType(QNetworkProxy::HttpCachingProxy);
		}
		else if (proxyType == "socks5")
		{
			proxy.setType(QNetworkProxy::Socks5Proxy);
		}
		else
		{
			proxy.setType(QNetworkProxy::NoProxy);
		}

		if (proxy.type() != QNetworkProxy::NoProxy)
		{
			proxy.setHostName(mProperties.get<QString>("terminal.proxy.host"));
			proxy.setPort(mProperties.get<QString>("terminal.proxy.port").toUShort());
			proxy.setPassword(mProperties.get("terminal.proxy.password", QString()));
			proxy.setUser(mProperties.get("terminal.proxy.login", QString()));
		}

		connection.proxy = proxy;
	}
	catch (std::exception & e)
	{
		toLog(LogLevel::Error, QString("Connection settings error: %1. Using defaults.").arg(e.what()));
	}

	return connection;
}

//---------------------------------------------------------------------------
void TerminalSettings::setConnection(const SConnection & aConnection)
{
	mProperties.put("terminal.connection.name", aConnection.name);
	mProperties.put("terminal.connection.type", (aConnection.type == EConnectionTypes::Dialup) ? "modem" : "unmanaged");

	if (aConnection.proxy.type() != QNetworkProxy::NoProxy)
	{
		mProperties.put("terminal.proxy.host", aConnection.proxy.hostName());
		mProperties.put("terminal.proxy.password", aConnection.proxy.password());
		mProperties.put("terminal.proxy.login", aConnection.proxy.user());
		mProperties.put("terminal.proxy.port", QString("%1").arg(aConnection.proxy.port()));

		QString proxyType;

		switch (aConnection.proxy.type())
		{
			case QNetworkProxy::HttpCachingProxy: proxyType = "http_caching"; break;
			case QNetworkProxy::HttpProxy: proxyType = "http"; break;
			case QNetworkProxy::Socks5Proxy: proxyType = "socks5"; break;
			default: proxyType = "http";
		}

		mProperties.put("terminal.proxy.type", proxyType);
	}
	else
	{
		try
		{
			mProperties.get_child("terminal.proxy").clear();
		}
		catch (std::exception & )
		{
			// Ветвь не была найдена.
		}
	}
}

//---------------------------------------------------------------------------
QList<IConnection::CheckUrl> TerminalSettings::getCheckHosts() const
{
	QList<IConnection::CheckUrl> hosts;

	for (int i = 1; ; i++)
	{
		try
		{
			QUrl url = mProperties.get(QString("system.check_hosts.host%1").arg(i).toStdString(), QString());
			QString response = mProperties.get(QString("system.check_hosts.response%1").arg(i).toStdString(), QString());

			if (url.isEmpty())
				break;

			hosts << IConnection::CheckUrl(url, response);
		}
		catch (std::exception & e)
		{
			toLog(LogLevel::Error, QString("Failed to parse check host. Error: %1.").arg(e.what()));

			break;
		}
	}

	return hosts;
}

//---------------------------------------------------------------------------
SDatabaseSettings TerminalSettings::getDatabaseSettings() const
{
	SDatabaseSettings databaseSettings;

	databaseSettings.name = mProperties.get("system.database.name", QString(CDefaults::DefaultDatabaseName));
	databaseSettings.host = mProperties.get("system.database.host", QString());
	databaseSettings.port = mProperties.get("system.database.port", CDefaults::DefaultDatabasePort);
	databaseSettings.user = mProperties.get("system.database.user", QString());
	databaseSettings.password = mProperties.get("system.database.password", QString());

	return databaseSettings;
}

//---------------------------------------------------------------------------
QStringList TerminalSettings::getDeviceList() const
{
	QStringList deviceList;

	TPtree empty;
	BOOST_FOREACH (const TPtree::value_type & value, mProperties.get_child("terminal.hardware", empty))
	{
		deviceList.append(value.second.get_value<QString>());
	}

	return deviceList;
}

//---------------------------------------------------------------------------
void TerminalSettings::setDeviceList(const QStringList & aHardware)
{
	TPtree branch;

	int id = 0;
	foreach (QString configName, aHardware)
	{
		branch.put(std::string("device") + boost::lexical_cast<std::string>(id), configName);
		id++;
	}

	mProperties.put_child("terminal.hardware", branch);
}

//---------------------------------------------------------------------------
SMonitoringSettings TerminalSettings::getMonitoringSettings() const
{
	SMonitoringSettings settings;

	settings.url = QUrl(mProperties.get<QString>("system.monitoring.url", QString()));
	settings.restUrl = QUrl(mProperties.get<QString>("system.monitoring.rest_url", QString()));
	settings.heartbeatTimeout = mProperties.get<int>("system.monitoring.heartbeat", settings.heartbeatTimeout);
	settings.restCheckTimeout = mProperties.get<int>("system.monitoring.rest_check_timeout", settings.restCheckTimeout);	
	settings.restLimit = mProperties.get<int>("config.terminal.block_by_rest", 0);

	settings.cleanupItems = mProperties.get<QString>("system.user_cleanup.remove", "").split(",", QString::SkipEmptyParts);
	settings.cleanupItems.replaceInStrings(QRegExp("^\\s+|\\s+$"), QString());
	settings.cleanupItems.removeAll("");

	settings.cleanupExclude = mProperties.get<QString>("system.user_cleanup.exclude", "").split(",", QString::SkipEmptyParts);
	settings.cleanupExclude.replaceInStrings(QRegExp("^\\s+|\\s+$"), QString());
	settings.cleanupExclude.removeAll("");

	return settings;
}

//---------------------------------------------------------------------------
QMap<int, SKeySettings> TerminalSettings::getKeys() const
{
	QMap<int, SKeySettings> keys;

	TPtree empty;
	BOOST_FOREACH (const TPtree::value_type & value, mProperties.get_child("keys", empty))
	{
		try
		{
			if (value.first == "<xmlattr>")
			{
				continue;
			}

			SKeySettings key;

			key.id = value.second.get<int>("<xmlattr>.id");
			key.engine = value.second.get<int>("engine", key.engine);
			key.ap = value.second.get<QString>("ap");
			key.sd = value.second.get<QString>("sd");
			key.op = value.second.get<QString>("op");
			key.serialNumber = value.second.get<ulong>("secret_serial_number", key.serialNumber);
			key.bankSerialNumber = value.second.get<ulong>("serial_number");
			key.publicKeyPath = value.second.get<QString>("public_key");
			key.secretKeyPath = value.second.get<QString>("secret_key");
			key.secretPassword = value.second.get<QString>("secret_password");
			key.description = value.second.get<QString>("description", QString());
			key.isValid = true;

			if (keys.contains(key.id))
			{
				toLog(LogLevel::Error, QString("There is already a key with id = %1.").arg(key.id));
			}
			else
			{
				keys.insert(key.id, key);
			}
		}
		catch (std::exception & e)
		{
			toLog(LogLevel::Error, QString("Failed to load key data: %1.").arg(e.what()));
		}
	}

	return keys;
}

//---------------------------------------------------------------------------
void TerminalSettings::setKey(const SKeySettings & aKey, bool aReplaceIfExists)
{
	auto applyConfig = [&](TPtree & aPair) {
		aPair.put("ap", aKey.ap);
		aPair.put("sd", aKey.sd);
		aPair.put("op", aKey.op);
		aPair.put("engine", aKey.engine);
		aPair.put("secret_serial_number", aKey.serialNumber);
		aPair.put("serial_number", aKey.bankSerialNumber);
		aPair.put("public_key", aKey.publicKeyPath);
		aPair.put("secret_key", aKey.secretKeyPath);
		aPair.put("secret_password", aKey.secretPassword);
		aPair.put("description", aKey.description);
	};

	TPtree empty;
	BOOST_FOREACH(TPtree::value_type & value, mProperties.get_child("keys", empty))
	{
		try
		{
			if (value.first == "<xmlattr>")
			{
				continue;
			}

			if (value.second.get<int>("<xmlattr>.id") == aKey.id)
			{
				if (aReplaceIfExists)
				{
					// Перезаписываем существующий ключ.
					applyConfig(value.second);

					return;
				}
			}
		}
		catch (std::exception & e)
		{
			toLog(LogLevel::Error, QString("setKey error: %1 .").arg(e.what()));
		}
	}

	if (aKey.isValid)
	{
		// Добавляем новую пару ключей.
		TPtree pair;

		pair.put("<xmlattr>.id", aKey.id);
		applyConfig(pair);

		mProperties.add_child("keys.pair", pair);
	}
}

//---------------------------------------------------------------------------
void TerminalSettings::cleanKeys()
{
	TPtree empty;
	while (mProperties.get_child("keys", empty).erase("pair") > 0);
}

//---------------------------------------------------------------------------
SCurrencySettings TerminalSettings::getCurrencySettings() const
{
	SCurrencySettings currencySettings;
	currencySettings.id = mProperties.get("system.currency.id", -1);

	if (currencySettings.id != -1)
	{
		currencySettings.code = mProperties.get("system.currency.code", QString());
		currencySettings.name = mProperties.get("system.currency.name", QString());

		if (currencySettings.code.isEmpty() || currencySettings.name.isEmpty())
		{
			toLog(LogLevel::Warning, QString("For currency id = %1 code or name is not found.").arg(currencySettings.id));
		}

		foreach (auto coin, mProperties.get("system.currency.coins", QString()).split(","))
		{
			currencySettings.coins << Currency::Nominal(coin.toDouble());
		}

		foreach (auto note, mProperties.get("system.currency.notes", QString()).split(","))
		{
			currencySettings.notes << Currency::Nominal(note.toInt());
		}
	}

	return currencySettings;
}

//----------------------------------------------------------------------------
const QSet<int> & TerminalSettings::getCriticalErrors() const
{
	return mCriticalErrors;
}

//---------------------------------------------------------------------------
QString TerminalSettings::getKeygenURL() const
{
	QString url = mProperties.get("system.common.keygen_url", QString());

	if (url.isEmpty())
	{
		toLog(LogLevel::Error, "Keygen url is not set!");
	}

	return url;
}

//---------------------------------------------------------------------------
QString TerminalSettings::getReceiptMailURL() const
{
	QString url = mProperties.get("system.common.receipt_mail_url", QString());

	if (url.isEmpty())
	{
		toLog(LogLevel::Error, "Url for receipt mail is not set!");
	}

	return url;
}

//---------------------------------------------------------------------------
QString TerminalSettings::getFeedbackURL() const
{
	QString url = mProperties.get("system.common.feedback_url", QString());

	if (url.isEmpty())
	{
		toLog(LogLevel::Error, "Url for feedback is not set!");
	}

	return url;
}

//---------------------------------------------------------------------------
QVariantMap TerminalSettings::getChargeProviderAccess() const
{
	QVariantMap result;

	TPtree empty;
	BOOST_FOREACH(const TPtree::value_type & value, mProperties.get_child("system.charge_access", empty))
	{
		result.insert(QString::fromStdString(value.first), value.second.get_value<QString>().split(","));
	}

	return result;
}

//---------------------------------------------------------------------------
SAppEnvironment TerminalSettings::getAppEnvironment() const
{
	SAppEnvironment environment;

	environment.userDataPath = mProperties.get("environment.user_data_path", QString());
	environment.contentPath = mProperties.get("environment.content_path", QString());
	environment.interfacePath = mProperties.get("environment.interface_path", QString());
	environment.adPath = mProperties.get("environment.ad_path", QString());
	environment.version = mProperties.get("environment.version", QString());

	return environment;
}

//---------------------------------------------------------------------------
void TerminalSettings::setAppEnvironment(const SAppEnvironment & aEnv)
{
	mProperties.put("environment.user_data_path", aEnv.userDataPath);
	mProperties.put("environment.content_path", aEnv.contentPath);
	mProperties.put("environment.interface_path", aEnv.interfacePath);
	mProperties.put("environment.ad_path", aEnv.adPath);
	mProperties.put("environment.version", aEnv.version);
}

//---------------------------------------------------------------------------
SCommonSettings TerminalSettings::getCommonSettings() const
{
	SCommonSettings settings;

	if (mProperties.get_child("config.hardware", TPtree()).empty())
	{
		settings.isValid = false;

		return settings;
	}

	settings.setBlockOn(SCommonSettings::ValidatorError, mProperties.get("config.hardware.validator_settings.block_terminal_on_error", true));
	settings.autoEncachement = mProperties.get("config.hardware.validator_settings.auto_encashment", settings.autoEncachement);
	settings.printFailedReceipts = mProperties.get("config.hardware.printer_settings.print_failed_receipts", settings.printFailedReceipts);
	settings.randomReceiptsID = mProperties.get("config.hardware.printer_settings.random_receipts_id", settings.randomReceiptsID);
	settings.enableBlankFiscalData = mProperties.get("config.hardware.printer_settings.enable_blank_fiscal_data", settings.enableBlankFiscalData);

	QString defaultZReportTime = (!settings.autoZReportTime.isNull() && settings.autoZReportTime.isValid()) ?
		settings.autoZReportTime.toString("hh:mm") : "";
	settings.autoZReportTime = QTime::fromString(mProperties.get("config.hardware.printer_settings.auto_z_report_time", defaultZReportTime), "hh:mm");

	settings.timeZoneOffset = mProperties.get_optional<int>("config.terminal.timezone");

	// Получаем минимальный разрешенный номинал.
	auto minNote = mProperties.get_optional<double>("config.hardware.validator_settings.min_note");
	if (minNote.is_initialized())
	{
		settings.minPar = minNote.get();

		// Парсим список активированных купюр.
		foreach (const QString & str, mProperties.get("config.hardware.validator_settings.notes", QString()).split(",", QString::SkipEmptyParts))
		{
			auto nominal = Currency::Nominal(str.toDouble());

			if (nominal >= settings.minPar)
			{
				settings.enabledParNotesList << nominal;
				settings.enabledParCoinsList << nominal;
			}
		}
	}
	else // Новый формат с раздельным указанием купюр и монет
	{
		settings.minPar = 10000;

		// Парсим список активированных монет.
		foreach (auto str, mProperties.get("config.hardware.validator_settings.coins", QString()).split(",", QString::SkipEmptyParts))
		{
			auto nominal = Currency::Nominal(str.toDouble());
			settings.minPar = qMin(nominal, settings.minPar);
			settings.enabledParCoinsList << nominal;
		}

		// Парсим список активированных купюр.
		foreach (auto str, mProperties.get("config.hardware.validator_settings.notes", QString()).split(",", QString::SkipEmptyParts))
		{
			auto nominal = Currency::Nominal(str.toInt());
			settings.minPar = qMin(nominal, settings.minPar);
			settings.enabledParNotesList.insert(nominal);
		}
	}

	settings.skipCheckWhileNetworkError = mProperties.get("config.terminal.skip_check_while_network_error", settings.skipCheckWhileNetworkError);

	settings.setBlockOn(SCommonSettings::PrinterError, mProperties.get("config.hardware.printer_settings.block_terminal_on_error", true));
	settings.setBlockOn(SCommonSettings::CardReaderError, mProperties.get("config.hardware.cardreader_settings.block_terminal_on_error", false));
	settings.setBlockOn(SCommonSettings::AccountBalance, getMonitoringSettings().isBlockByAccountBalance());

	switch (mProperties.get("config.terminal.block_by_penetration", 0))
	{
	case 2: settings.penetrationEventLevel = EEventType::Critical; break;
	case 1: settings.penetrationEventLevel = EEventType::Warning; break;
	default: settings.penetrationEventLevel = EEventType::OK; break;
	}
	
	settings.setBlockOn(SCommonSettings::Penetration, settings.penetrationEventLevel == EEventType::Critical);

	auto updateBlockNotes = [&settings](quint32 aNominal, quint32 aInterval, quint32 aRepeat)
	{
		if (aInterval && aRepeat)
		{
			settings.blockNotes << SBlockByNote(aNominal, aInterval, aRepeat);
		}
	};

	settings.blockCheatedPayment = mProperties.get("config.terminal.block_cheated_payment", settings.blockCheatedPayment);

	TPtree empty;
	BOOST_FOREACH(const TPtree::value_type & notes, mProperties.get_child("config.terminal.block_by_note", empty))
	{
		BOOST_FOREACH(const TPtree::value_type & note, notes.second)
		{
			if (note.first == "<xmlattr>")
			{
				updateBlockNotes(
					note.second.get<int>("nominal"),
					note.second.get<int>("interval"),
					note.second.get<int>("repeat"));
			}
		}
	}

	settings.disableAmountOverflow = mProperties.get("config.hardware.validator_settings.disable_amount_overflow", settings.disableAmountOverflow);

	return settings;
}

//---------------------------------------------------------------------------
SServiceMenuPasswords TerminalSettings::getServiceMenuPasswords() const
{
	SServiceMenuPasswords passwords;

	passwords.phone = mProperties.get("config.service_menu.phone", QString());
	passwords.operatorId = mProperties.get("config.service_menu.operator", -1);

	std::tr1::array<QString, 4> passwordTypes =
	{
		CServiceMenuPasswords::Service,
		CServiceMenuPasswords::Screen,
		CServiceMenuPasswords::Collection,
		CServiceMenuPasswords::Technician
	};

	for (size_t i = 0; i < passwordTypes.size(); i++)
	{
		try
		{
			passwords.passwords[passwordTypes[i]] = mProperties.get<QString>((QString("config.service_menu.") + passwordTypes[i]).toStdString());
		}
		catch (std::exception & e)
		{
			toLog(LogLevel::Error, QString("Error %1.").arg(e.what()));
		}
	}

	return passwords;
}

//---------------------------------------------------------------------------
SServiceMenuSettings TerminalSettings::getServiceMenuSettings() const
{
	SServiceMenuSettings settings;

	settings.allowAnyKeyPair = mProperties.get("config.service_menu.alloy_any_keypair", false);

	return settings;
}

//---------------------------------------------------------------------------
QString TerminalSettings::getPrinterForReceipt(const QString & aReceiptType)
{
	auto receipts = mProperties.get_child("terminal.receipts", TPtree());
	auto deviceAlias = receipts.get(aReceiptType.toStdString(), QString());

	// Ищем девайс с таким алиасом в списке всех устройств.
	auto devices = mProperties.get_child("terminal.hardware", TPtree());
	return devices.get(deviceAlias.toStdString(), QString());
}

//---------------------------------------------------------------------------
bool TerminalSettings::isValid() const
{
	QStringList configs = QStringList()
		<< "environment"
		<< "keys"
		<< "terminal"
		<< "system"
		<< "config";
	QStringList errorConfigs;

	foreach(const QString & aConfig, configs)
	{
		if (mProperties.get_child(aConfig.toStdString(), TPtree()).empty())
		{
			errorConfigs << aConfig;
		}
	}

	if (!errorConfigs.isEmpty())
	{
		toLog(LogLevel::Error, "Failed to validate configuration settings due to bad section(s): " + errorConfigs.join(", "));
		return false;
	}

	return true;
}

//---------------------------------------------------------------------------
int TerminalSettings::getLogsMaxSize() const
{
	return mProperties.get("config.terminal.logs.<xmlattr>.max_size", 100);
}

//---------------------------------------------------------------------------
QStringList TerminalSettings::getUpdaterUrls() const
{
	QStringList result;
	
	result << mProperties.get("system.updater.url", QString())
		<< mProperties.get("system.updater.data_url", QString());

	return result;
}

//---------------------------------------------------------------------------
QString TerminalSettings::getAdProfile() const
{
	return mProperties.get("config.terminal.ad.<xmlattr>.profile", QString());
}

//---------------------------------------------------------------------------
QTime TerminalSettings::autoUpdate() const
{
	if (mProperties.get<bool>("config.terminal.check_update", false))
	{
		return QTime::fromString(mProperties.get<QString>("config.terminal.check_update.<xmlattr>.start", ""), "hh:mm");
	}

	return QTime();
}

//---------------------------------------------------------------------------
QString TerminalSettings::energySave() const
{
	QStringList result = QStringList()
		<< mProperties.get<QString>("config.hardware.energy_save.<xmlattr>.from", "")
		<< mProperties.get<QString>("config.hardware.energy_save.<xmlattr>.till", "")
		<< mProperties.get<QString>("config.hardware.energy_save", "");

	return result.join(";");
}

//---------------------------------------------------------------------------
}} // SDK::PaymentProcessor
