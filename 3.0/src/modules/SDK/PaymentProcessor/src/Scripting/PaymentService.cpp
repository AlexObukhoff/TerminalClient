/* @file Прокси класс для работы с платежами в скриптах. */

// stl
#include <algorithm>

#include <Common/QtHeadersBegin.h>
#include <QtCore/QTextCodec>
#include <QtCore/QDateTime>
#include <QtCore/QScopedPointer>
#include <Common/QtHeadersEnd.h>

// Boost
#pragma push_macro("foreach")
#undef foreach
#include <boost/noncopyable.hpp>
#pragma pop_macro("foreach")
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IPaymentService.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Core/INetworkService.h>
#include <SDK/PaymentProcessor/Core/IDatabaseService.h>
#include <SDK/PaymentProcessor/Core/ITerminalService.h>
#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/CyberPlat/ErrorCodes.h>
#include <SDK/PaymentProcessor/Settings/DealerSettings.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>
#include <SDK/PaymentProcessor/Settings/Directory.h>
#include <SDK/PaymentProcessor/Payment/Security.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>
#include <SDK/PaymentProcessor/Scripting/PaymentService.h>
#include <SDK/PaymentProcessor/Scripting/PaymentService.h>

#include <DatabaseProxy/IDatabaseQuery.h>

namespace PPSDK = SDK::PaymentProcessor;

typedef boost::property_tree::basic_ptree<std::string, std::string> TPtreeOperators;

//---------------------------------------------------------------------------
namespace CMultistage
{
	const QString Type = "multistage";

	const QString Step = "MULTISTAGE_STEP";            /// текущий шаг 
	const QString StepFields = "MULTISTAGE_FIELDS_%1"; /// список полей для конкретного шага
	const QString History = "MULTISTAGE_HISTORY";      /// история шагов
}

namespace SDK {
namespace PaymentProcessor {
namespace Scripting {

//------------------------------------------------------------------------------
ScriptArray * ProviderField::getEnumItems()
{
	ScriptArray * list = new ScriptArray(this);
	
	foreach (const SProviderField::SEnumItem & item, mField.enumItems)
	{
		list->append(new EnumItem(item, list));
	}

	return list;
}

//------------------------------------------------------------------------------
Provider::Provider(const SProvider & aProvider, QObject * aParent)
	: QObject(aParent),
		mProvider(aProvider)
{
	QObjectList fields;

	foreach (const SProviderField & field, mProvider.fields)
	{
		fields << new ProviderField(field, this);
	}

	mFields["0"] = fields;
}

//------------------------------------------------------------------------------
bool Provider::isCheckStepSettingsOK()
{
	QStringList limits;
	limits << mProvider.limits.min << mProvider.limits.max << mProvider.limits.externalMin << mProvider.limits.externalMax;

	QRegExp rx("\\{(.+)\\}");

	QStringList limits2;
	foreach(QString l, limits)
	{
		int pos = 0;

		while ((pos = rx.indexIn(l, pos)) != -1)
		{
			limits2 << rx.cap(1);
			pos += rx.matchedLength();
		}
	}
	
	QStringList responseFields;

	foreach (SProvider::SProcessingTraits::SRequest::SField f, mProvider.processor.requests.value("CHECK").responseFields)
	{
		responseFields << f.name;
	}

	return limits2.toSet().intersect(responseFields.toSet()).isEmpty() ? true : (mProvider.processor.showAddInfo ? true : mProvider.processor.skipCheck == false);
}

//------------------------------------------------------------------------------
QString Provider::applySecurityFilter(const QString aId, const QString & aValueRaw, const QString & aValueDisplay) const
{
	PPSDK::SecurityFilter filter(mProvider, PPSDK::SProviderField::SecuritySubsystem::Display);

	QString masked = aValueRaw;

	if (filter.haveFilter(aId))
	{
		masked = filter.apply(aId, aValueRaw);
	}
	else
	{
		masked = aValueDisplay;
	}

	return masked;
}

//------------------------------------------------------------------------------
QString Provider::xmlFields2Json(const QString & aXmlFields)
{
	TPtreeOperators ptFields;
	const TPtreeOperators emptyTree;
	TProviderFields fields;

	QByteArray buffer = aXmlFields.toUtf8();
	std::stringstream stream(std::string(buffer.data(), buffer.size()));

	try
	{
		boost::property_tree::read_xml(stream, ptFields);
	}
	catch (boost::property_tree::xml_parser_error & e)
	{
		qDebug() << QString("xmlFields2Json: XML parser error: %1.").arg(QString::fromStdString(e.message()));

		return QString();
	}

	std::function<void(SProviderField::TEnumItems &, const TPtreeOperators &)> loadProviderEnumItems;
	loadProviderEnumItems = [&](SProviderField::TEnumItems & aItemList, const TPtreeOperators & aTree)
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
	};

	BOOST_FOREACH(const auto & fieldIt, ptFields.get_child("add_fields", emptyTree))
	{
		try
		{
			SProviderField field;

			if (fieldIt.first == "field")
			{
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

				loadProviderEnumItems(field.enumItems, fieldIt.second.get_child("enum", emptyTree));

				fields << field;
			}
		}
		catch (std::runtime_error & error)
		{
			qDebug() << error.what();
		}
	}
	
	return SProvider::fields2Json(fields);
}

//------------------------------------------------------------------------------
QObjectList Provider::getFields()
{
	PaymentService * paymentService = qobject_cast<PaymentService *>(parent());

	if (!paymentService)
	{
		return mFields["0"]; 
	}

	QString currentStep = paymentService->currentStep();

	if (mProvider.processor.type == CMultistage::Type)
	{
		TProviderFields fields;
		if (paymentService->loadFieldsForStep(fields))
		{
			QObjectList resultList;

			foreach (const SProviderField & field, fields)
			{
				resultList << new ProviderField(field, this);
			}

			mFields[currentStep] = resultList;
		}
	}

	return mFields[currentStep];
}

//------------------------------------------------------------------------------
PaymentService::PaymentService(ICore * aCore) :
	mCore(aCore),
	mPaymentService(mCore->getPaymentService()),
	mProviderWithExternalLimits(-1),
	mForcePayOffline(false)
{
	connect(mPaymentService, SIGNAL(stepCompleted(qint64, int, bool)), SLOT(onStepCompleted(qint64, int, bool)));
	connect(mPaymentService, SIGNAL(amountUpdated(qint64)), SIGNAL(amountUpdated(qint64)));
	connect(mPaymentService, SIGNAL(changeUpdated(double)), SIGNAL(changeUpdated(double)));

	mDirectory = static_cast<PPSDK::Directory *>(aCore->getSettingsService()->getAdapter(CAdapterNames::Directory));
	mDealerSettings = static_cast<PPSDK::DealerSettings *>(aCore->getSettingsService()->getAdapter(CAdapterNames::DealerAdapter));
	mCommonSettings = static_cast<PPSDK::TerminalSettings *>(aCore->getSettingsService()->getAdapter(CAdapterNames::TerminalAdapter))->getCommonSettings();
}

//------------------------------------------------------------------------------
qint64 PaymentService::create(qint64 aProvider)
{
	return mPaymentService->createPayment(aProvider);
}

//------------------------------------------------------------------------------
qint64 PaymentService::getActivePaymentID()
{
	return mPaymentService->getActivePayment();
}

//------------------------------------------------------------------------------
qint64 PaymentService::getLastPaymentID()
{
	auto payments = mPaymentService->getBalance().payments;

	while (!payments.isEmpty())
	{
		qint64 payment = payments.takeLast();

		// Проверяем что платеж это не сдача
		if (mPaymentService->getPaymentField(payment, PPSDK::CPayment::Parameters::Provider).value.toLongLong() > 0)
		{
			return payment;
		}
	}

	return -1;
}

//------------------------------------------------------------------------------
QVariant PaymentService::getParameter(const QString & aName)
{
	return mPaymentService->getPaymentField(getActivePaymentID(), aName).value;
}

//------------------------------------------------------------------------------
QVariantMap PaymentService::getParameters()
{
	return getParameters(getActivePaymentID());
}

//------------------------------------------------------------------------------
QVariantMap PaymentService::calculateCommission(const QVariantMap & aParameters)
{
	QList<PPSDK::IPayment::SParameter> input;

	foreach (auto p, aParameters.keys())
	{
		input << PPSDK::IPayment::SParameter(p, aParameters.value(p));
	}

	if (mPaymentService)
	{
		QVariantMap result;

		foreach (auto p, mPaymentService->calculateCommission(input))
		{
			result.insert(p.name, p.value);
		}

		return result;
	}

	return QVariantMap();
}

//------------------------------------------------------------------------------
QVariantMap PaymentService::calculateLimits(const QString & aAmount)
{
	//TODO Код из paymentbase::calculateLimits

	PPSDK::DealerSettings * dealerSettings =
		dynamic_cast<PPSDK::DealerSettings *>(mCore->getSettingsService()->getAdapter(PPSDK::CAdapterNames::DealerAdapter));

	double minAmount = aAmount.toDouble();
	double maxAmount = aAmount.toDouble();

	const qint64 providerID = getParameter(PPSDK::CPayment::Parameters::Provider).toLongLong();
	const bool isFixedAmount = qFuzzyCompare(minAmount, maxAmount);
	const auto provider = mPaymentService->getProvider(providerID);
	double systemMax = qMin(provider.limits.system.toDouble(), maxAmount);

	// Проверяем значение системного лимита
	if (qFuzzyIsNull(systemMax))
	{
		systemMax = maxAmount;
	}

	// Если провайдер требует округления - применим округление к пограничным значениям
	if (provider.processor.rounding)
	{
		maxAmount = qCeil(maxAmount);
		minAmount = isFixedAmount ? maxAmount : qCeil(minAmount);
	}

	// Формируем массив заполненных полей для подсчёта комиссий.
	QVariantMap credentials;

	foreach(const PPSDK::SProviderField & field, provider.fields)
	{
		credentials.insert(field.id, getParameter(field.id).toString());
	}

	PPSDK::Commission com(dealerSettings->getCommission(providerID, credentials, maxAmount));

	auto calcAmountAll = [&com, &provider](double aAmount) -> double {
		double amountAll = 0.0;

		if (com.getType() == PPSDK::Commission::Percent)
		{
			double amountAllLimit1 = 0.0;

			if (provider.processor.feeType == PPSDK::SProvider::FeeByAmount ||
				com.getBase() == PPSDK::Commission::Amount)
			{
				amountAllLimit1 = aAmount * (1 + 0.01 * com.getValue());
			}
			else
			{
				amountAllLimit1 = aAmount / (1 - 0.01 * com.getValue());
			}

			amountAll = amountAllLimit1;

			if (!qFuzzyIsNull(com.getMinCharge()))
			{
				double amountAllLimit2 = aAmount + com.getMinCharge();
				amountAll = amountAllLimit1 > amountAllLimit2 ? amountAllLimit1 : amountAllLimit2;
			}

			if (!qFuzzyIsNull(com.getMaxCharge()))
			{
				double amountAllLimit3 = aAmount + com.getMaxCharge();
				amountAll = amountAll > amountAllLimit3 ? amountAllLimit3 : amountAll;
			}
		}
		else
		{
			amountAll = aAmount + com.getValue();
		}

		return amountAll;
	};

	auto calcAmountAllByAmountAll = [&com, &provider](double & aAmount) -> double {
		double amountAll = 0.0;

		if (com.getType() == PPSDK::Commission::Percent)
		{
			double amountAllLimit1 = 0.0;

			if (provider.processor.feeType == PPSDK::SProvider::FeeByAmount ||
				com.getBase() == PPSDK::Commission::Amount)
			{
				amountAllLimit1 = aAmount / (1 + 0.01 * com.getValue());
			}
			else
			{
				amountAllLimit1 = aAmount * (1 - 0.01 * com.getValue());
			}

			amountAll = amountAllLimit1;
			double fee = aAmount - amountAllLimit1;

			if (com.getMinCharge() && fee < com.getMinCharge())
			{
				double amountAllLimit2 = aAmount - com.getMinCharge();
				amountAll = amountAllLimit1 > amountAllLimit2 ? amountAllLimit1 : amountAllLimit2;
			}

			if (com.getMaxCharge() && fee > com.getMaxCharge())
			{
				double amountAllLimit3 = aAmount - com.getMaxCharge();
				amountAll = qMax(amountAllLimit3, amountAll);
			}
		}
		else
		{
			aAmount -= com.getValue();
			amountAll = aAmount;
		}

		return amountAll;
	};

	double localAmountAllLimit = calcAmountAll(maxAmount);
	double maxAmountAll = 0.0;

	if (localAmountAllLimit > systemMax)
	{
		maxAmountAll = systemMax;
		maxAmount = calcAmountAllByAmountAll(maxAmount);
	}
	else
	{
		maxAmountAll = qMax(localAmountAllLimit, maxAmount);
	}

	// Округлим результат после 2ого знака чтобы не было расхождений в копейках
	maxAmountAll = qRound64(maxAmountAll * 100) / double(100);

	// Если провайдер требует округления - применим округление к пограничным значениям
	if (provider.processor.rounding)
	{
		maxAmountAll = qCeil(maxAmountAll);
	}

	QVariantMap result;

	result.insert(PPSDK::CPayment::Parameters::MaxAmount, maxAmount);
	result.insert(PPSDK::CPayment::Parameters::MaxAmountAll, maxAmountAll);

	return result;
}

//------------------------------------------------------------------------------
QVariantMap PaymentService::getParameters(qint64 aPaymentId)
{
	QVariantMap result;

	if (mPaymentService)
	{
		foreach (auto parameter, mPaymentService->getPaymentFields(aPaymentId))
		{
			result.insert(parameter.name, parameter.value);
		}
	}

	return result;
}

//------------------------------------------------------------------------------
QObject * PaymentService::getProvider()
{
	return getProvider(getParameter(PPSDK::CPayment::Parameters::Provider).toLongLong());
}

//------------------------------------------------------------------------------
QObject * PaymentService::getProvider(qint64 aID)
{
	return new Provider(updateSkipCheckFlag(mPaymentService->getProvider(aID)), this);
}

//------------------------------------------------------------------------------
QObject * PaymentService::getMNPProvider()
{
	return new Provider(updateSkipCheckFlag(mDealerSettings->getMNPProvider(
		getParameter(PPSDK::CPayment::Parameters::Provider).toLongLong(),
		getParameter(PPSDK::CPayment::Parameters::MNPGetewayIn).toLongLong(),
		getParameter(PPSDK::CPayment::Parameters::MNPGetewayOut).toLongLong())), this);
}

//------------------------------------------------------------------------------
QObject * PaymentService::getProviderByGateway(qint64 aCID)
{
	auto providers = mDealerSettings->getProvidersByCID(aCID);

	return new Provider(providers.isEmpty() ? PPSDK::SProvider() : updateSkipCheckFlag(providers.at(0)), this);
}

//------------------------------------------------------------------------------
QObject * PaymentService::getProviderForNumber(qint64 aNumber)
{
	ScriptArray * result = new ScriptArray(this);

	foreach (const SProvider & p, mDealerSettings->getProvidersByRange(mDirectory->getRangesForNumber(aNumber), mDirectory->getOverlappedIDs()))
	{
		result->append(new PPSDK::Scripting::Provider(updateSkipCheckFlag(p), this));
	}

	return result;
}

//------------------------------------------------------------------------------
void PaymentService::setExternalParameter(const QString & aName, const QVariant & aValue)
{
	mPaymentService->updatePaymentField(getActivePaymentID(), IPayment::SParameter(aName, aValue, true, false, true));
}

//------------------------------------------------------------------------------
QString PaymentService::findAliasFromRequest(const QString & aParamName, const QString & aRequestName)
{
	auto provider = mPaymentService->getProvider(getParameter(PPSDK::CPayment::Parameters::Provider).toLongLong());

	auto fields = provider.processor.requests.value(aRequestName).requestFields;
	foreach(auto field, fields)
	{
		if (field.name == aParamName)
		{
			QRegExp macroPattern("\\{(.+)\\}");
			macroPattern.setMinimal(true);

			QString result = field.value;

			if (macroPattern.indexIn(result) != -1)
			{
				result.replace(macroPattern.cap(0), macroPattern.cap(1));
			}

			return result;
		}
	}

	return QString();
}

//------------------------------------------------------------------------------
void PaymentService::setParameter(const QString & aName, const QVariant & aValue, bool aCrypted /*= false*/)
{
	mPaymentService->updatePaymentField(getActivePaymentID(), IPayment::SParameter(aName, aValue, true, aCrypted));
}

//------------------------------------------------------------------------------
void PaymentService::setParameters(const QVariantMap & aParameters)
{
	auto provider = mPaymentService->getProvider(getParameter(PPSDK::CPayment::Parameters::Provider).toLongLong());

	// функция проверки - является ли параметр значением поля, содержащим пароль
	auto keepEncrypted = [&provider](const QString & aParamName) -> bool
	{
		foreach (auto field, provider.fields)
		{
			if (aParamName.contains(field.id))
			{
				return field.keepEncrypted();
			}
		}

		return false;
	};

	QList<PPSDK::IPayment::SParameter> parameters;

	for (auto it = aParameters.begin(); it != aParameters.end(); ++it)
	{
		parameters << PPSDK::IPayment::SParameter(it.key(), it.value(), true, keepEncrypted(it.key()));
	}

	mPaymentService->updatePaymentFields(getActivePaymentID(), parameters);
}

//------------------------------------------------------------------------------
void PaymentService::updateLimits(qint64 aProviderId, double aExternalMin, double aExternalMax)
{
	mDealerSettings->setExternalLimits(aProviderId, aExternalMin, aExternalMax);

	mProviderWithExternalLimits = aProviderId;
}

//------------------------------------------------------------------------------
bool PaymentService::canProcessOffline()
{
	return mPaymentService->canProcessPaymentOffline(getActivePaymentID());
}

//------------------------------------------------------------------------------
void PaymentService::check()
{
	return processStep(EPaymentStep::DataCheck);
}

//------------------------------------------------------------------------------
void PaymentService::stepForward()
{
	return processStep(EPaymentStep::GetStep);
}

//------------------------------------------------------------------------------
void PaymentService::processStep(int aStep)
{
	mPaymentService->processPaymentStep(getActivePaymentID(), static_cast<EPaymentStep::Enum>(aStep));
}

//------------------------------------------------------------------------------
PaymentService::EProcessResult PaymentService::process(bool aOnline)
{
	double amount = getParameter(PPSDK::CPayment::Parameters::Amount).toDouble();
	double minAmount = getParameter(PPSDK::CPayment::Parameters::MinAmount).toDouble();

	if ((amount < minAmount) && !qFuzzyCompare(amount, minAmount))
	{
		cancel();

		return LowMoney;
	}

	if (!aOnline && !canProcessOffline())
	{
		cancel();

		return OfflineIsNotSupported;
	}

	if (!aOnline &&
		mCommonSettings.blockCheatedPayment &&
		getParameter(PPSDK::CPayment::Parameters::Cheated).toInt())
	{
		stop(PPSDK::CyberPlat::EServerError::Cheated, "Cheated");

		resetChange();

		return BadPayment;
	}

	checkStatus();

	return mPaymentService->processPayment(getActivePaymentID(), aOnline) ? OK : BadPayment;
}

//------------------------------------------------------------------------------
bool PaymentService::stop(int aError, const QString & aErrorMessage)
{
	return mPaymentService->stopPayment(getActivePaymentID(), aError, aErrorMessage);
}

//------------------------------------------------------------------------------
bool PaymentService::cancel()
{
	return mPaymentService->cancelPayment(getActivePaymentID());
}

//------------------------------------------------------------------------------
double PaymentService::getChangeAmount()
{
	return mPaymentService->getChangeAmount();
}

//------------------------------------------------------------------------------
void PaymentService::useChange()
{
	mPaymentService->moveChangeToPayment(getActivePaymentID());
}

//------------------------------------------------------------------------------
void PaymentService::useChangeBack()
{
	mPaymentService->movePaymentToChange(getActivePaymentID());
}

//------------------------------------------------------------------------------
void PaymentService::resetChange()
{
	mPaymentService->resetChange();
}

//------------------------------------------------------------------------------
QObject * PaymentService::getPaymentNotes()
{
	ScriptArray * notes = new ScriptArray(this);

	foreach (const SNote & note, mPaymentService->getPaymentNotes(getActivePaymentID()))
	{
		notes->append(new Note(note, notes));
	}

	return notes;
}

//------------------------------------------------------------------------------
QObject * PaymentService::notesFromBalance(const PPSDK::SBalance & aBalance)
{
	ScriptArray * notes = new ScriptArray(this);

	foreach (const SBalance::SAmounts & amounts, aBalance.detailedSums)
	{
		foreach (const SBalance::SAmounts::SAmount & amount, amounts.amounts)
		{
			foreach (QString serial, amount.serials.split(","))
			{
				SNote note(amounts.type, amount.value.toDouble(), amounts.currency, serial);
				notes->append(new Note(note, notes));
			}
		}
	}

	return notes;
}

//------------------------------------------------------------------------------
QObject * PaymentService::getBalanceNotes()
{
	return notesFromBalance(mPaymentService->getBalance());
}

//------------------------------------------------------------------------------
QObject * PaymentService::getLastEncashmentNotes()
{
	return notesFromBalance(mPaymentService->getLastEncashment().balance);
}

//------------------------------------------------------------------------------
QString PaymentService::currentStep()
{
	qint64 providerId = getParameter(PPSDK::CPayment::Parameters::Provider).toLongLong();
	SProvider provider = mPaymentService->getProvider(providerId);
	
	if (provider.processor.type == CMultistage::Type)
	{
		QString step = getParameter(CMultistage::Step).toString();
		
		return step.isEmpty() ? "0" : step;
	}

	return "0";
}

//------------------------------------------------------------------------------
void PaymentService::reset()
{
	// Установить шаг на первый
	if (currentStep() != "0")
	{
		setParameter(CMultistage::Step, "");
		setParameter(CMultistage::History, "");
	}

	mPaymentService->deactivatePayment();

	// Вернем платежные лимиты по умолчанию
	mDealerSettings->setExternalLimits(mProviderWithExternalLimits, 0.0, 0.0);
}

//------------------------------------------------------------------------------
bool PaymentService::isFinalStep()
{
	return (getParameter(CMultistage::Step).toString() == "FINAL_STEP");
}

//------------------------------------------------------------------------------
void PaymentService::stepBack()
{
	if (currentStep() != "0")
	{
		QStringList history = getParameter(CMultistage::History).toString().split(";", QString::SkipEmptyParts);
		if (!history.isEmpty())
		{
			setParameter(CMultistage::Step, history.takeLast());
			setParameter(CMultistage::History, history.join(";"));
		}
	}
}

//------------------------------------------------------------------------------
bool PaymentService::loadFieldsForStep(TProviderFields & aFields)
{
	QString step = currentStep();
	if (step != "0")
	{
		aFields = SProvider::json2Fields(getParameter(CMultistage::StepFields.arg(step)).toString());

		return true;
	}

	return false;
}

//------------------------------------------------------------------------------
QStringList PaymentService::findPayments(const QDate & aDate, const QString & aPhoneNumber)
{
	QStringList result;

	foreach (auto id, mPaymentService->findPayments(aDate, aPhoneNumber))
	{
		result << QString::number(id);
	}

	return result;
}

//------------------------------------------------------------------------------
bool PaymentService::convert(const QString & aTargetType)
{
	return mPaymentService->convertPayment(mPaymentService->getActivePayment(), aTargetType);
}

//------------------------------------------------------------------------------
void PaymentService::setForcePayOffline(bool aForcePayOffline)
{
	mForcePayOffline = aForcePayOffline;
}

//------------------------------------------------------------------------------
bool isEmptyReqiredResponseFields(const SProvider & aProvider)
{
	int count = 0;

	foreach(auto f, aProvider.processor.requests["CHECK"].responseFields)
	{
		count = f.required ? count + 1 : count;
	}

	foreach(auto f, aProvider.processor.requests["PAY"].responseFields)
	{
		count = f.required ? count + 1 : count;
	}

	return (count == 0);
}

//------------------------------------------------------------------------------
PPSDK::SProvider PaymentService::updateSkipCheckFlag(SProvider aProvider)
{
	if (!aProvider.processor.payOnline && mForcePayOffline)
	{
		// Изменяем параметр проверки платежа для возможности проведения его в офлайне
		if (!mCore->getNetworkService()->isConnected(true) &&
			!aProvider.processor.skipCheck)
		{
			aProvider.processor.skipCheck = isEmptyReqiredResponseFields(aProvider);
		}
	}

	return aProvider;
}

//------------------------------------------------------------------------------
void PaymentService::onStepCompleted(qint64 aPayment, int aStep, bool aError)
{
	if (aStep == PPSDK::EPaymentStep::DataCheck && aError)
	{
		auto provider = updateSkipCheckFlag(mPaymentService->getProvider(getParameter(PPSDK::CPayment::Parameters::Provider).toLongLong()));

		if (mForcePayOffline && isEmptyReqiredResponseFields(provider) && !provider.processor.payOnline)
		{
			// проверка на сетевую ошибку, константа далеко закопана в проекте
			if (getParameter(PPSDK::CPayment::Parameters::ServerError).toInt() == -1)
			{
				emit stepCompleted(aPayment, aStep, false);

				return;
			}
		}
	}

	emit stepCompleted(aPayment, aStep, aError);
}

//------------------------------------------------------------------------------
QVariantMap PaymentService::getStatistic()
{
	QVariantMap statistic;
	QMapIterator<qint64, unsigned> i(mPaymentService->getStatistic());
	
	while (i.hasNext()) 
	{
		i.next();
		statistic.insert(QString::number(i.key()), i.value());
	}

	return statistic;
}

//------------------------------------------------------------------------------
void PaymentService::checkStatus()
{
	PPSDK::TerminalSettings * terminalSettings = static_cast<PPSDK::TerminalSettings *>
		(mCore->getSettingsService()->getAdapter(CAdapterNames::TerminalAdapter));
	
	QList<SDK::PaymentProcessor::SBlockByNote> notes = terminalSettings->getCommonSettings().blockNotes;

	QString currencyName = terminalSettings->getCurrencySettings().name;
	
	PPSDK::IDatabaseService * db = mCore->getDatabaseService();

	QString queryStr = QString("SELECT `create_date` FROM `device_status` WHERE `fk_device_id` = 1 AND `description` = 'Unlocked' ORDER BY 1 DESC LIMIT 1");
	QSharedPointer<IDatabaseQuery> query(db->createAndExecQuery(queryStr));

	quint32 deltaTime = 0;

	if (query && query->first())
	{
		deltaTime = qAbs(QDateTime::currentDateTime().secsTo(query->value(0).toDateTime()));
	}

	foreach(SDK::PaymentProcessor::SBlockByNote note, notes)
	{
		QString queryStr = QString("SELECT COUNT(*) FROM `payment_note` WHERE ((NOT `ejection`) OR (`ejection` IS NULL)) AND `type` <> 2 AND `nominal` = %1 AND `date` >= DATETIME('now', 'localtime', '-%2 seconds')").arg(note.nominal).arg(deltaTime ? qMin(deltaTime, note.interval) : note.interval);
		
		QSharedPointer<IDatabaseQuery> query(db->createAndExecQuery(queryStr));

		if (query && query->first() && query->value(0).toUInt() > note.repeat)
		{
			setExternalParameter(PPSDK::CPayment::Parameters::Cheated, PPSDK::EPaymentCheatedType::NotesCount);

			mCore->getEventService()->sendEvent(PPSDK::Event(PPSDK::EEventType::TerminalLock));
			mCore->getEventService()->sendEvent(PPSDK::Event(SDK::PaymentProcessor::EEventType::Critical, "Payment",
				QString("Terminal blocked by nominal limit reached: %1 %2 * %3").arg(note.nominal).arg(currencyName).arg(query->value(0).toInt())));
			
			return;
		}
	}
}

//------------------------------------------------------------------------------
}}} // Scripting::PaymentProcessor::SDK
