/* @file Настройки дилера: операторы, комиссии, персональная инфа и т.п. */

// Stl
#include <algorithm>

// Boost
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

//Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QFile>
#include <QtCore/QStringList>
#include <QtCore/QStack>
#include <QtCore/QElapsedTimer>
#include <QtCore/QXmlStreamReader>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/ILog.h>

// Project
#include "DealerSettings.h"

namespace SDK {
namespace PaymentProcessor {

typedef boost::property_tree::basic_ptree<std::string, std::string> TPtreeOperators;

//---------------------------------------------------------------------------
DealerSettings::DealerSettings(TPtree & aProperties)
	: mProperties(aProperties.get_child(CAdapterNames::DealerAdapter, aProperties)), mProvidersLock(QReadWriteLock::Recursive), mIsValid(false)
{
}

//---------------------------------------------------------------------------
DealerSettings::~DealerSettings()
{
}

//---------------------------------------------------------------------------
QString DealerSettings::getAdapterName()
{
	return CAdapterNames::DealerAdapter;
}

//---------------------------------------------------------------------------
void DealerSettings::initialize()
{
	bool r1 = loadProviders();
	bool r2 = loadCommissions();
	bool r3 = loadPersonalSettings();

	mIsValid = r1 && r2 && r3;

	// Проверяем наличие групп.
	if (mProperties.get_child("groups", TPtree()).empty())
	{
		mIsValid = false;
	}

	mProperties.clear();
}

//---------------------------------------------------------------------------
inline QString & encodeLTGT(QString & value)
{
	return value.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;");
}

//---------------------------------------------------------------------------
inline QString & encodeQUOT(QString & value)
{
	return value.replace("\"", "&quot;");
}

//---------------------------------------------------------------------------
bool DealerSettings::loadOperatorsXML(const QString & aFileName)
{
	QFile inputFile(aFileName);

	if (!inputFile.open(QIODevice::ReadOnly))
	{
		toLog(LogLevel::Error, QString("Failed to open file: %1.").arg(aFileName));
		return false;
	}

	std::string op;
	qint64 opID = 0;
	QString processing;
	QSet<qint64> cids;
	double operatorsVersion = 0.;
	QStack<QString> tags;

	op.reserve(4096);

	QXmlStreamReader xmlReader(&inputFile);

	while (!xmlReader.atEnd())
	{
		QXmlStreamReader::TokenType token = xmlReader.readNext();

		switch (token)
		{
			// Встретили открывающий тег.
			case QXmlStreamReader::StartElement:
			{
				tags << xmlReader.name().toString().toLower();

				bool isOP = (tags.top() == "operator");

				if (isOP)
				{
					op = "<?xml version=\"1.0\" encoding=\"utf-8\"?><operator version=\"" + 
						QString::number(operatorsVersion, 'f').toStdString() + "\"";

					opID = 0;
					processing.clear();
					cids.clear();
				}
				else
				{
					op += "<" + tags.top().toLatin1();
				}

				// Обрабатываем список атрибутов, если такие имеются.
				QXmlStreamAttributes attributes = xmlReader.attributes();

				if (!attributes.isEmpty())
				{
					QMap<QString, QString> attrs;

					foreach(const QXmlStreamAttribute & attribute, attributes)
					{
						QString value = attribute.value().toString();
						attrs[attribute.name().toString().toLower()].swap(encodeQUOT(value));
					}

					if (tags.top() == "operators")
					{
						operatorsVersion = attrs.value("version", "0").toDouble();
					}
					else if (isOP)
					{
						opID = attrs.value("id").toLongLong();

						if (operatorsVersion < 2.0)
						{
							cids << attrs.value("cid").toLongLong();
						}
					}
					else if (tags.top() == "processor")
					{
						// Процессинг вида тип_процессинга#имя является алиасом для стандартных типов
						processing = attrs.value("type").split("#").takeFirst();
					}

					foreach(auto name, attrs.keys())
					{
						op += " " + name.toLatin1() + "=\"" + attrs.value(name).toUtf8() + "\"";
					}
				}

				op += ">";

				break;
			}

			// Текст внутри тегов.
			case QXmlStreamReader::Characters:
			{
				if (!xmlReader.isWhitespace())
				{
					QString text = xmlReader.text().toString();
					op += encodeLTGT(text).toUtf8();

					if (operatorsVersion >= 2.0)
					{
						if (tags.size() > 2 && tags.at(tags.size() - 2) == "operator")
						{
							if (tags.top() == "cid")
							{
								cids << xmlReader.text().toString().toLongLong();
							}
						}
					}

					if (tags.top() == "tt_list")
					{
						foreach(auto cid, xmlReader.text().toString().split(","))
						{
							cids << cid.trimmed().toLongLong();
						}
					}
				}

				break;
			}

			// Встретили закрывающий тег.
			case QXmlStreamReader::EndElement:
			{
				QString key = xmlReader.name().toString().toLower();

				if (key == "operator")
				{
					op += "</operator>";

					if (mProviderRawBuffer.contains(opID))
					{
						//qDebug() << "Skip existing operator " << opID;
					}
					else
					{
						mProviderRawBuffer[opID].swap(op);
						foreach (qint64 cid, cids)
						{
							mProviderGeteways.insert(cid, opID);
						}
						// operatorsProcessing[opID].swap(processing);
						// operatorsHash[opID].swap(QString::fromLatin1(CCryptographicHash::hash(op, CCryptographicHash::Sha256).toHex()));
						mProvidersProcessingIndex.insert(processing, opID);
					}

					cids.clear();
					op.reserve(4096);
					processing.clear();
					opID = 0;
				}
				else
				{
					op += "</" + key.toStdString() + ">";
				}

				tags.pop();
				break;
			}

			// Ошибка в формате документа.
			case QXmlStreamReader::Invalid:
			{
				toLog(LogLevel::Error, QString("'%1' parsing error: %2, line %3, column %4.")
					.arg(aFileName)
					.arg(xmlReader.errorString())
					.arg(xmlReader.lineNumber())
					.arg(xmlReader.columnNumber()));

				return false;
			}
		}
	}

	return true;
}

//---------------------------------------------------------------------------
bool DealerSettings::loadProviders()
{
	const TPtree emptyTree;

	toLog(LogLevel::Normal, "Loading providers.");

	QElapsedTimer elapsed;
	elapsed.start();

	BOOST_FOREACH(const TPtree::value_type & operators, mProperties.get_child("", emptyTree))
	{
		if (operators.first != "operators")
		{
			continue;
		}

		QString operatorsPath = QString::fromStdWString(operators.second.get_value(L""));

		toLog(LogLevel::Normal, QString("Loading %1.").arg(operatorsPath));

		loadOperatorsXML(operatorsPath);
	}

	toLog(LogLevel::Normal, QString("Total providers loaded: %1, elapsed %2 ms.").arg(mProviderRawBuffer.size()).arg(elapsed.elapsed()));

	return !mProviderRawBuffer.isEmpty();
}

//---------------------------------------------------------------------------
void loadProviderEnumItems(SProviderField::TEnumItems & aItemList, const TPtreeOperators & aTree)
{
	auto searchBounds = aTree.equal_range("item");

	for (auto itemIt = searchBounds.first; itemIt != searchBounds.second; ++itemIt)
	{
		SProviderField::SEnumItem item;

		auto attr = itemIt->second.get_child("<xmlattr>");

		item.title = attr.get<QString>("name");
		item.value = attr.get<QString>("value", QString());
		item.id = attr.get<QString>("id", QString());
		item.sort = attr.get<int>("sort", 65535);

		loadProviderEnumItems(item.subItems, itemIt->second);

		aItemList << item;
	}

	qStableSort(aItemList.begin(), aItemList.end(), [](const SProviderField::SEnumItem & a, const SProviderField::SEnumItem & b){return a.sort < b.sort; });
}

//---------------------------------------------------------------------------
bool DealerSettings::loadProvidersFromBuffer(const std::string & aBuffer, SProvider & aProvider)
{
	TPtreeOperators operators;
	const TPtreeOperators emptyTree;

	try
	{
		std::stringstream stream(aBuffer);

		boost::property_tree::read_xml(stream, operators);
	}
	catch (boost::property_tree::xml_parser_error & e)
	{
		toLog(LogLevel::Error, QString("XML parser error: %1.").arg(QString::fromStdString(e.message())));

		return false;
	}

	BOOST_FOREACH(const auto & value, operators.get_child("", emptyTree))
	{
		try
		{
			double operatorsVersion = value.second.get<double>("<xmlattr>.version", 0);
			aProvider.id = value.second.get<qint64>("<xmlattr>.id");
			aProvider.cid = value.second.get<qint64>("<xmlattr>.cid", -1);

			// Для совместимости с operators.xml version=2.0
			if (operatorsVersion >= 2.0)
			{
				aProvider.cid = value.second.get<qint64>("cid", -1);

				QString terminalShow = value.second.get<QString>("terminal_show", "1").trimmed();
				terminalShow = terminalShow.isEmpty() ? "1" : terminalShow;

				QString enabled = value.second.get<QString>("enabled", "1").trimmed();
				enabled = enabled.isEmpty() ? "1" : enabled;

				if (terminalShow.toInt() == 0 || enabled.toInt() == 0)
				{
					return false;
				}
			}

			QString ttList = value.second.get<QString>("tt_list", QString());
			foreach(auto cid, ttList.split(",", QString::SkipEmptyParts))
			{
				aProvider.ttList.insert(cid.trimmed().toLongLong());
			}

			aProvider.type = value.second.get<QString>("<xmlattr>.type", "cyberplat");
			aProvider.name = value.second.get<QString>("name");
			aProvider.comment = value.second.get<QString>("comment", QString());

			// Прогружаем лимиты
			TPtreeOperators limitBranch = value.second.get_child("limit.<xmlattr>");

			aProvider.limits.min = limitBranch.get<QString>("min");
			aProvider.limits.max = limitBranch.get<QString>("max");
			aProvider.limits.system = limitBranch.get<QString>("system", QString());
			aProvider.limits.check = limitBranch.get<QString>("check", QString());

			// Проверяем корректность лимитов.
			if (aProvider.limits.min.isEmpty() || (aProvider.limits.system.isEmpty() && aProvider.limits.max.isEmpty()))
			{
				throw std::runtime_error(QString("operator %1: unspecified max or min limit").arg(aProvider.id).toStdString());
			}

			// Парсим процессор
			TPtreeOperators processorBranch = value.second.get_child("processor");

			auto processorBranchAttr = processorBranch.get_child("<xmlattr>");

			aProvider.processor.type = processorBranchAttr.get<QString>("type");
			aProvider.processor.keyPair = processorBranchAttr.get<int>("keys", 0);
			aProvider.processor.clientCard = processorBranchAttr.get<int>("client_card", 0);
			if (!aProvider.processor.clientCard)
			{
				// читаем атрибут по старому пути, если не нашли его в разделе processor
				aProvider.processor.clientCard = value.second.get<int>("client_card", 0);
			}
			aProvider.processor.skipCheck = processorBranchAttr.get<bool>("skip_check", true);
			aProvider.processor.payOnline = processorBranchAttr.get<bool>("pay_online", false);
			aProvider.processor.askForRetry = processorBranchAttr.get<bool>("ask_for_retry", false);
			aProvider.processor.requirePrinter = processorBranchAttr.get<bool>("require_printer", false);
			aProvider.processor.feeType = processorBranchAttr.get<QString>("fee_type", "amount_all") == "amount" ? SProvider::FeeByAmount : SProvider::FeeByAmountAll;
			aProvider.processor.rounding = processorBranchAttr.get<bool>("rounding", false);
			aProvider.processor.showAddInfo = processorBranchAttr.get<bool>("show_add_info", false);

			auto requests = processorBranch.equal_range("request");

			for (auto requestIt = requests.first; requestIt != requests.second; ++requestIt)
			{
				SProvider::SProcessingTraits::SRequest request;

				request.url = requestIt->second.get<QString>("url");

				// Отправляемые поля
				auto searchBounds = requestIt->second.equal_range("request_property");

				for (auto fieldIt = searchBounds.first; fieldIt != searchBounds.second; ++fieldIt)
				{
					SProvider::SProcessingTraits::SRequest::SField field;

					auto attr = fieldIt->second.get_child("<xmlattr>");

					field.name = attr.get<QString>("name");
					field.value = attr.get<QString>("value");
					field.crypted = attr.get<bool>("crypted", field.crypted);

					QString algorithm = attr.get<QString>("algorithm", "ipriv").toLower();
					if (algorithm == "md5")
					{
						field.algorithm = SProvider::SProcessingTraits::SRequest::SField::Md5;
					}
					else if (algorithm == "sha1")
					{
						field.algorithm = SProvider::SProcessingTraits::SRequest::SField::Sha1;
					}
					else if (algorithm == "sha256")
					{
						field.algorithm = SProvider::SProcessingTraits::SRequest::SField::Sha256;
					}

					request.requestFields << field;
				}

				// Ожидаемые поля
				searchBounds = requestIt->second.equal_range("receive_property");

				for (auto fieldIt = searchBounds.first; fieldIt != searchBounds.second; ++fieldIt)
				{
					SProvider::SProcessingTraits::SRequest::SResponseField field;

					auto attr = fieldIt->second.get_child("<xmlattr>");

					field.name = attr.get<QString>("name");
					field.crypted = attr.get<bool>("crypted", field.crypted);
					field.required = attr.get<bool>("required", field.required);

					field.setCodepage(attr.get<QString>("codepage", ""));
					field.setEncoding(attr.get<QString>("encoding", ""));

					request.responseFields << field;
				}

				aProvider.processor.requests[requestIt->second.get<QString>("<xmlattr>.name").toUpper()] = request;
			}

			BOOST_FOREACH(const auto & fieldIt, value.second.get_child("fields"))
			{
				SProviderField field;

				auto attr = fieldIt.second.get_child("<xmlattr>");

				field.id = attr.get<QString>("id");
				field.type = attr.get<QString>("type");
				field.keyboardType = attr.get<QString>("keyboard_type", QString());
				field.isRequired = attr.get<bool>("required", true);
				field.sort = attr.get<int>("sort", 65535);
				field.minSize = attr.get<int>("min_size", -1);
				field.maxSize = attr.get<int>("max_size", -1);
				field.language = attr.get<QString>("lang", QString());
				field.letterCase = attr.get<QString>("case", QString());
				field.behavior = attr.get<QString>("behavior", QString());
				field.defaultValue = attr.get<QString>("default_value", QString());

				field.title = fieldIt.second.get<QString>("name");
				field.comment = fieldIt.second.get<QString>("comment", QString());
				field.extendedComment = fieldIt.second.get<QString>("extended_comment", QString());
				field.mask = fieldIt.second.get<QString>("mask", QString());
				field.isPassword = fieldIt.second.get<bool>("mask.<xmlattr>.password", false);
				field.format = fieldIt.second.get<QString>("format", QString());

				field.url = fieldIt.second.get<QString>("url", QString());
				field.html = fieldIt.second.get<QString>("html", QString());
				field.backButton = fieldIt.second.get<QString>("back_button", QString());
				field.forwardButton = fieldIt.second.get<QString>("forward_button", QString());

				field.dependency = fieldIt.second.get<QString>("dependency", QString());

				QString externalDataHandler = fieldIt.second.get<QString>("on_external_data", QString());
				if (!externalDataHandler.isEmpty())
				{
					aProvider.externalDataHandler = externalDataHandler;
				}

				loadProviderEnumItems(field.enumItems, fieldIt.second.get_child("enum", emptyTree));

				auto security = fieldIt.second.get_child("security", emptyTree);
				if (!security.empty())
				{
					field.security.insert(SProviderField::SecuritySubsystem::Default, security.get<QString>("<xmlattr>.hidemask", QString()));

					if (field.security.value(SProviderField::SecuritySubsystem::Default, "").isEmpty())
					{
						throw std::runtime_error("empty security@hideMask atribute");
					}

					BOOST_FOREACH(auto subSystem, security.get_child("", emptyTree))
					{
						if (subSystem.first == "printer")
						{
							field.security.insert(SProviderField::SecuritySubsystem::Printer, subSystem.second.get<QString>("<xmlattr>.hidemask", QString()));
						}
						else if (subSystem.first == "logger")
						{
							field.security.insert(SProviderField::SecuritySubsystem::Log, subSystem.second.get<QString>("<xmlattr>.hidemask", QString()));
						}
						else if (subSystem.first == "display")
						{
							field.security.insert(SProviderField::SecuritySubsystem::Display, subSystem.second.get<QString>("<xmlattr>.hidemask", QString()));
						}
					}
				}

				aProvider.fields << field;
			}

			qStableSort(aProvider.fields.begin(), aProvider.fields.end(), [](const SProviderField & a, const SProviderField & b){return a.sort < b.sort; });

			// Типы и параметры чеков
			BOOST_FOREACH(const auto & receiptIt, value.second.get_child("receipts", emptyTree))
			{
				if (receiptIt.first == "parameter")
				{
					aProvider.receiptParameters.insert(receiptIt.second.get<QString>("<xmlattr>.name"), receiptIt.second.get<QString>("<xmlattr>.value", QString()));
				}
				else if (receiptIt.first == "receipt")
				{
					aProvider.receipts.insert(receiptIt.second.get<QString>("<xmlattr>.type"), receiptIt.second.get<QString>("<xmlattr>.template", QString()));
				}
			}
		}
		catch (std::runtime_error & error)
		{
			toLog(LogLevel::Error, QString("Failed to load provider (id:%1, cid:%2). Error: %3.").arg(aProvider.id).arg(aProvider.cid).arg(error.what()));

			return false;
		}

		break;
	}

	return true;
}

//---------------------------------------------------------------------------
void DealerSettings::disableProvider(qint64 aId)
{
	QWriteLocker locker(&mProvidersLock);

	mProviders.remove(aId);
	mProviderRawBuffer.remove(aId);
	mProvidersProcessingIndex.remove(mProvidersProcessingIndex.key(aId), aId);

	foreach (auto cid, mProviderGeteways.keys())
	{
		mProviderGeteways.remove(cid, aId);
	}
}

//----------------------------------------------------------------------------
bool DealerSettings::loadCommissions()
{
	try
	{
		const TPtree emptyTree;

		toLog(LogLevel::Normal, "Loading commissions.");

		BOOST_FOREACH (const TPtree::value_type & commissions, mProperties.get_child("", emptyTree))
		{
			if (commissions.first != "commissions")
			{
				continue;
			}

			if (!mCommissions.isValid())
			{
				mCommissions = Commissions::fromSettings(commissions.second);
			}
			else
			{
				mCommissions.appendFromSettings(commissions.second);
			}
		}
	}
	catch (std::runtime_error & error)
	{
		toLog(LogLevel::Error, QString("Failed to load commissions. Error: %1.").arg(error.what()));
		return false;
	}

	try
	{
		toLog(LogLevel::Normal, "Loading customers.");

		BOOST_FOREACH (const TPtree::value_type & value, mProperties.get_child("customers"))
		{
			if (value.first == "<xmlattr>")
			{
				continue;
			}

			TPtree operatorSettings = value.second.get_child("operator");

			SCustomer customer;

			customer.blocked = operatorSettings.get<bool>("<xmlattr>.blocked", false);

			std::pair<TPtree::const_assoc_iterator, TPtree::const_assoc_iterator> searchBounds = operatorSettings.equal_range("field");
			for (TPtree::const_assoc_iterator it = searchBounds.first; it != searchBounds.second; ++it)
			{
				customer.addValue(it->second.get<QString>("<xmlattr>.value").trimmed());
			}

			if (!customer.blocked)
			{
				customer.commissions = Commissions::fromSettings(operatorSettings);
			}

			if (!customer.isEmpty())
			{
				mCustomers << customer;
			}
		}
	}
	catch (std::runtime_error & error)
	{
		toLog(LogLevel::Error, QString("Failed to load customers. Error: %1.").arg(error.what()));
	}

	return true;
}

//----------------------------------------------------------------------------
bool DealerSettings::loadPersonalSettings()
{
	toLog(LogLevel::Normal, "Loading personal settings.");

	try
	{
		TPtree & branch = mProperties.get_child("config.dealer");

		mPersonalSettings.pointName = branch.get("point_name", QString());
		mPersonalSettings.pointAddress = branch.get("point_address", QString());
		mPersonalSettings.pointExternalID = branch.get("external_id_for_cash_collection", QString());
		mPersonalSettings.name = branch.get("dealer_name", QString());
		mPersonalSettings.address = branch.get("dealer_address", QString());
		mPersonalSettings.businessAddress = branch.get("dealer_business_address", QString());
		mPersonalSettings.inn = branch.get("dealer_inn", QString());
		mPersonalSettings.kbk = branch.get("dealer_kbk", QString());
		mPersonalSettings.phone = branch.get("dealer_support_phone", QString());
		mPersonalSettings.isBank = branch.get("dealer_is_bank", QString("0"));

		mPersonalSettings.operatorName = branch.get("operator_name", QString());
		mPersonalSettings.operatorAddress = branch.get("operator_address", QString());
		mPersonalSettings.operatorInn = branch.get("operator_inn", QString());
		mPersonalSettings.operatorContractNumber = branch.get("operator_contract_number", QString());

		mPersonalSettings.bankName = branch.get("bank_name", QString());
		mPersonalSettings.bankAddress = branch.get("bank_address", QString());
		mPersonalSettings.bankBik = branch.get("bank_bik", QString());
		mPersonalSettings.bankInn = branch.get("bank_inn", QString());
		mPersonalSettings.bankPhone = branch.get("bank_phone", QString());
		mPersonalSettings.bankContractNumber = branch.get("contract_number", QString());

		const TPtree emptyTree;

		BOOST_FOREACH(const TPtree::value_type & parameter, mProperties.get_child("config.printing.parameters", emptyTree))
		{
			mPersonalSettings.mPrintingParameters.insert(
				QString::fromStdString(parameter.first).toUpper(), 
				parameter.second.get_value(QString()).replace("\\n", "\n"));
		}
	}
	catch (std::runtime_error & error)
	{
		toLog(LogLevel::Error, QString("Failed to load personal settigns : %1.").arg(error.what()));
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------
SProvider DealerSettings::getProvider(qint64 aId)
{
	if (mProviderRawBuffer.contains(aId))
	{
		QWriteLocker locker(&mProvidersLock);

		SProvider provider;
		if (loadProvidersFromBuffer(mProviderRawBuffer[aId], provider))
		{
			mProviders.insert(aId, provider);
			mProviderRawBuffer.remove(aId);
		}
		else
		{
			toLog(LogLevel::Error, QString("Error parse provider %1 from buffer.").arg(aId));
		}
	}

	QReadLocker locker(&mProvidersLock);

	if (mProviders.contains(aId))
	{
		SProvider provider = mProviders.value(aId);
		
		// Лимиты из описания оператора могут быть переопределены снаружи
		provider.limits.min = !qFuzzyIsNull(provider.limits.externalMin.toDouble()) ? provider.limits.externalMin : provider.limits.min;
		provider.limits.max = !qFuzzyIsNull(provider.limits.externalMax.toDouble()) ? provider.limits.externalMax : provider.limits.max;
		
		return provider;
	}

	return SProvider();
}

//----------------------------------------------------------------------------
SProvider DealerSettings::getMNPProvider(qint64 aId, qint64 aCidIn, qint64 aCidOut)
{
	auto provider = getProvider(aId);

	if (aCidIn == 0 && aCidOut == 0)
	{
		return provider;
	}
	
	if (aCidOut && (provider.cid == aCidOut || provider.ttList.contains(aCidOut)))
	{
		return provider;
	}

	auto providers = getProvidersByCID(aCidOut);
	
	if (providers.isEmpty() && aCidIn > 0)
	{
		providers = getProvidersByCID(aCidIn);
	}

	return providers.isEmpty() ? provider : providers.at(0);
}

//----------------------------------------------------------------------------
QList<SProvider> DealerSettings::getProvidersByCID(qint64 aCid)
{
	QList<SProvider> providers;

	foreach (auto id, mProviderGeteways.values(aCid))
	{
		providers << getProvider(id);
	}

	return providers;
}

//---------------------------------------------------------------------------
const QList<qint64> DealerSettings::getProviders(const QString & aProcessingType)
{
	return mProvidersProcessingIndex.values(aProcessingType);
}

//---------------------------------------------------------------------------
QStringList DealerSettings::getProviderProcessingTypes()
{
	return mProvidersProcessingIndex.keys().toSet().toList();
}

//---------------------------------------------------------------------------
void DealerSettings::setExternalLimits(qint64 aProviderId, double aMinExternalLimit, double aMaxExternalLimit)
{
	SProvider provider = getProvider(aProviderId);

	if (!provider.isNull())
	{
		mProviders[aProviderId].limits.externalMin = QString::number(aMinExternalLimit);
		mProviders[aProviderId].limits.externalMax = QString::number(aMaxExternalLimit);
	}
}

//---------------------------------------------------------------------------
const SPersonalSettings & DealerSettings::getPersonalSettings() const
{
	return mPersonalSettings;
}

//---------------------------------------------------------------------------
DealerSettings::TCustomers::iterator DealerSettings::findCustomer(const QVariantMap & aParameters)
{
	QSet<QString> parametersValueSet;

	foreach (auto value, aParameters.values())
	{
		parametersValueSet << value.toString();
	}

	auto isItRightCustomer = [&](const SCustomer & aCustomer) -> bool {
		return aCustomer.contains(parametersValueSet);
	};

	return std::find_if(mCustomers.begin(), mCustomers.end(), isItRightCustomer);
}

//---------------------------------------------------------------------------
void DealerSettings::setExternalCommissions(const Commissions & aCommissions)
{
	mExternalCommissions = aCommissions;
}

//---------------------------------------------------------------------------
void DealerSettings::resetExternalCommissions()
{
	mExternalCommissions.clear();
}

//---------------------------------------------------------------------------
bool DealerSettings::isCustomerAllowed(const QVariantMap & aParameters)
{
	TCustomers::iterator it = findCustomer(aParameters);

	return it == mCustomers.end() ? true : !it->blocked;
}

//---------------------------------------------------------------------------
TCommissions DealerSettings::getCommissions(qint64 aProvider, const QVariantMap & aParameters)
{
	if (mExternalCommissions.isValid() && mExternalCommissions.contains(aProvider))
	{
		return mExternalCommissions.getCommissions(aProvider);
	}
	
	TCustomers::iterator it = findCustomer(aParameters);

	// Игнорирум настройки customer комисии если комиссия процессинга не нулевая.
	return it == mCustomers.end() ? mCommissions.getCommissions(aProvider) : it->commissions.getCommissions(aProvider);
}

//---------------------------------------------------------------------------
Commission DealerSettings::getCommission(qint64 aProvider, const QVariantMap & aParameters, double aSum)
{
	if (mExternalCommissions.isValid() && mExternalCommissions.contains(aProvider))
	{
		return mExternalCommissions.getCommission(aProvider, aSum);
	}

	TCustomers::iterator it = findCustomer(aParameters);

	// Игнорирум настройки customer комисии если комиссия процессинга не нулевая.
	return it == mCustomers.end() ? mCommissions.getCommission(aProvider, aSum) : it->commissions.getCommission(aProvider, aSum);
}

//---------------------------------------------------------------------------
ProcessingCommission DealerSettings::getProcessingCommission(qint64 aProvider)
{
	if (mExternalCommissions.isValid() && mExternalCommissions.contains(aProvider))
	{
		return mExternalCommissions.getProcessingCommission(aProvider);
	}

	return mCommissions.getProcessingCommission(aProvider);
}

//---------------------------------------------------------------------------
int DealerSettings::getVAT(qint64 aProvider)
{
	if (mExternalCommissions.isValid() && mExternalCommissions.contains(aProvider))
	{
		return mExternalCommissions.getVAT(aProvider);
	}

	return mCommissions.getVAT(aProvider);
}

//---------------------------------------------------------------------------
QList<SProvider> DealerSettings::getProvidersByRange(QList<SRange> aRanges, QSet<qint64> aExclude)
{
	QList<SProvider> providers;

	foreach (SRange range, aRanges)
	{
		if (!range.ids.isEmpty())
		{
			foreach (qint64 id, range.ids)
			{
				SProvider p = getProvider(id);

				if (!p.isNull())
				{
					providers << p;
				}
			}
		}
		else
		{
			foreach (qint64 cid, range.cids)
			{
				foreach (const SProvider & p, getProvidersByCID(cid))
				{
					if (!aExclude.contains(p.id))
					{
						providers << p;
					}
				}
			}
		}
	}

	return providers;
}

//---------------------------------------------------------------------------
bool DealerSettings::isValid() const
{
	return mIsValid;
}

//---------------------------------------------------------------------------
}} // SDK::PaymentProcessor
