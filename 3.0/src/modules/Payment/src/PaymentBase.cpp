/* @file Базовый класс для класса реализующего платёж. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/qmath.h>
#include <QtCore/QReadLocker>
#include <QtCore/QWriteLocker>
#include <QtScript/QScriptEngine>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/PaymentProcessor/Core/IServiceState.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Core/ICryptService.h>
#include <SDK/PaymentProcessor/Settings/ISettingsAdapter.h>
#include <SDK/PaymentProcessor/Settings/DealerSettings.h>
#include <SDK/PaymentProcessor/Payment/IPayment.h>
#include <SDK/PaymentProcessor/Payment/IPaymentFactory.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>
#include <SDK/PaymentProcessor/Payment/Step.h>

// Modules
#include <Crypt/ICryptEngine.h>

// Project
#include "PaymentBase.h"

// Thirdparty
#if QT_VERSION < 0x050000
#include <Qt5Port/qt5port.h>
#endif

namespace PPSDK = SDK::PaymentProcessor;

//------------------------------------------------------------------------------
PaymentBase::PaymentBase(SDK::PaymentProcessor::IPaymentFactory * aFactory, SDK::PaymentProcessor::ICore * aCore) :
	ILogable(aFactory->getLog()),
	mFactory(aFactory),
	mCore(aCore),
	mIsRestoring(false),
	mParametersLock(QReadWriteLock::Recursive)
{
	mParameters.insert(SParameter(PPSDK::CPayment::Parameters::CreationDate, QDateTime::currentDateTime(), true));
	mParameters.insert(SParameter(PPSDK::CPayment::Parameters::Status, PPSDK::EPaymentStatus::Init, true));
	mParameters.insert(SParameter(PPSDK::CPayment::Parameters::Step, CPayment::Steps::Init, true));
	mParameters.insert(SParameter(PPSDK::CPayment::Parameters::InitialSession, createPaymentSession(), true));
	mParameters.insert(SParameter(PPSDK::CPayment::Parameters::AmountAll, 0, true));
	mParameters.insert(SParameter(PPSDK::CPayment::Parameters::PayTool, 0, true));

	QStringList states;

	foreach(PPSDK::IService * service, aCore->getServices())
	{
		if (PPSDK::IServiceState * ss = dynamic_cast<PPSDK::IServiceState *>(service))
		{
			states << ss->getState();
		}
	}

	if (!states.isEmpty())
	{
#if QT_VERSION < 0x050000
		QString crc = QString::fromLatin1(CCryptographicHash::hash(states.join(";").toLatin1(), CCryptographicHash::Sha256).toHex());
#else
		QString crc = QString::fromLatin1(QCryptographicHash::hash(states.join(";").toLatin1(), QCryptographicHash::Sha256).toHex());
#endif
		mParameters.insert(SParameter(PPSDK::CPayment::Parameters::CRC, crc, true, true, true));
	}
}

//------------------------------------------------------------------------------
bool PaymentBase::isNull() const
{
	double amountAll = getParameter(PPSDK::CPayment::Parameters::AmountAll).value.toDouble();

	return qFuzzyIsNull(amountAll);
}

//------------------------------------------------------------------------------
PPSDK::IPaymentFactory * PaymentBase::getFactory() const
{
	return mFactory;
}

//------------------------------------------------------------------------------
void PaymentBase::setParameter(const SParameter & aParameter)
{
	updateParameter(aParameter);

	if (limitsDependOnParameter(aParameter))
	{
		if (calculateLimits())
		{
			calculateSums();
		}
	}
}

//------------------------------------------------------------------------------
PPSDK::IPayment::SParameter PaymentBase::getParameter(const QString & aName) const
{
	QReadLocker lock(&mParametersLock);

	TParameterList::index<NameTag>::type::iterator it = mParameters.find(aName);

	return it == mParameters.end() ? SParameter() : *it;
}

//------------------------------------------------------------------------------
QList<PPSDK::IPayment::SParameter> PaymentBase::getParameters() const
{
	QReadLocker lock(&mParametersLock);

	QList<SParameter> result;

	for (TParameterList::index<NameTag>::type::iterator it = mParameters.begin(); it != mParameters.end(); ++it)
	{
		result << *it;
	}

	return result;
}

//------------------------------------------------------------------------------
qint64 PaymentBase::getID() const
{
	return getParameter(PPSDK::CPayment::Parameters::ID).value.toLongLong();
}

//------------------------------------------------------------------------------
QString PaymentBase::getType() const
{
	return getParameter(PPSDK::CPayment::Parameters::Type).value.toString();
}

//------------------------------------------------------------------------------
QString PaymentBase::getSession() const
{
	return getParameter(PPSDK::CPayment::Parameters::Session).value.toString();
}

//------------------------------------------------------------------------------
QString PaymentBase::getInitialSession() const
{
	return getParameter(PPSDK::CPayment::Parameters::InitialSession).value.toString();
}

//------------------------------------------------------------------------------
QDateTime PaymentBase::getCreationDate() const
{
	return getParameter(PPSDK::CPayment::Parameters::CreationDate).value.toDateTime();
}

//------------------------------------------------------------------------------
QDateTime PaymentBase::getCompleteDate() const
{
	return getParameter(PPSDK::CPayment::Parameters::CompleteDate).value.toDateTime();
}

//------------------------------------------------------------------------------
QDateTime PaymentBase::getLastUpdateDate() const
{
	return getParameter(PPSDK::CPayment::Parameters::LastUpdateDate).value.toDateTime();
}

//------------------------------------------------------------------------------
int PaymentBase::getStatus() const
{
	return getParameter(PPSDK::CPayment::Parameters::Status).value.toInt();
}

//------------------------------------------------------------------------------
void PaymentBase::setStatus(int aStatus)
{
	setParameter(SParameter(PPSDK::CPayment::Parameters::Status, aStatus, true));
}

//------------------------------------------------------------------------------
PaymentBase::Priority PaymentBase::getPriority() const
{
	return static_cast<Priority>(getParameter(PPSDK::CPayment::Parameters::Priority).value.toInt());
}

//------------------------------------------------------------------------------
void PaymentBase::setPriority(Priority aPriority)
{
	setParameter(SParameter(PPSDK::CPayment::Parameters::Priority, aPriority, true));
}

//------------------------------------------------------------------------------
QString PaymentBase::getAmount() const
{
	return getParameter(PPSDK::CPayment::Parameters::Amount).value.toString();
}

//------------------------------------------------------------------------------
QString PaymentBase::getAmountAll() const
{
	return getParameter(PPSDK::CPayment::Parameters::AmountAll).value.toString();
}

//------------------------------------------------------------------------------
QDateTime PaymentBase::getNextTryDate() const
{
	return getParameter(PPSDK::CPayment::Parameters::NextTryDate).value.toDateTime();
}

//------------------------------------------------------------------------------
void PaymentBase::setNextTryDate(const QDateTime & aDate)
{
	setParameter(SParameter(PPSDK::CPayment::Parameters::NextTryDate, aDate, true));
}

//------------------------------------------------------------------------------
void PaymentBase::setCompleteDate(const QDateTime & aDate)
{
	if (!getCompleteDate().isValid())
	{
		setParameter(SParameter(PPSDK::CPayment::Parameters::CompleteDate, aDate.toUTC(), true));
	}
}

//------------------------------------------------------------------------------
qint64 PaymentBase::getProvider(bool aMNP) const
{
	qint64 myProvider = getParameter(PPSDK::CPayment::Parameters::Provider).value.toLongLong();

	if (!aMNP)
	{
		return myProvider;
	}

	qint64 getewayIn = getParameter(PPSDK::CPayment::Parameters::MNPGetewayIn).value.toLongLong();
	qint64 getewayOut = getParameter(PPSDK::CPayment::Parameters::MNPGetewayOut).value.toLongLong();

	PPSDK::DealerSettings * dealerSettings =
		dynamic_cast<PPSDK::DealerSettings *>(mCore->getSettingsService()->getAdapter(PPSDK::CAdapterNames::DealerAdapter));

	Q_ASSERT(dealerSettings);

	PPSDK::SProvider provider = dealerSettings->getMNPProvider(myProvider, getewayIn, getewayOut);

	return provider.isNull() ? myProvider : provider.id;
}

//---------------------------------------------------------------------------
bool PaymentBase::canProcessOffline() const
{
	return !getProviderSettings().processor.payOnline;
}

//------------------------------------------------------------------------------
bool PaymentBase::getBlockUpdateLimits() const
{
	auto param = getParameter(PPSDK::CPayment::Parameters::BlockUpdateLimits);

	return !param.isNull() && param.value.toBool();
}

//------------------------------------------------------------------------------
void PaymentBase::setBlockUpdateLimits(bool aBlock)
{
	if (aBlock && !getBlockUpdateLimits())
	{
		// Прикрепляет параметр, что бы он попал в БД
		auto pinParameter = [&](const QString & aName) {
			auto p = getParameter(aName);

			if (!p.isNull())
			{
				p.updated = true;
				updateParameter(p);
			}
		};
	
		pinParameter(PPSDK::CPayment::Parameters::MinAmount);
		pinParameter(PPSDK::CPayment::Parameters::MaxAmount);
		pinParameter(PPSDK::CPayment::Parameters::MaxAmountAll);

		updateParameter(SParameter(PPSDK::CPayment::Parameters::BlockUpdateLimits, true, true));
	}
}

//------------------------------------------------------------------------------
void PaymentBase::updateParameter(const SParameter & aParameter)
{
	{
		QWriteLocker lock(&mParametersLock);

		TParameterList::index<NameTag>::type::iterator it = mParameters.find(aParameter.name);
		if (it != mParameters.end())
		{
			mParameters.modify(it, SParameterModifier(aParameter));
		}
		else
		{
			mParameters.insert(aParameter);
		}
	}

	if (aParameter.name == PPSDK::CPayment::Parameters::Provider)
	{
		mProviderSettings = getProviderSettings(getProvider(false));

		if (!mProviderSettings.isNull())
		{
			if (!mIsRestoring)
			{
				// При установке провайдера, считаем по нему ограничения сумм.
				calculateLimits();
			}

			// В базе всегда храним список полей провайдера на случай, если провайдера удалят из конфига, а платежи по нему ещё останутся.
			QStringList providerFields;

			foreach (const PPSDK::SProviderField & field, mProviderSettings.fields)
			{
				providerFields << field.id;
			}

			setParameter(SParameter(PPSDK::CPayment::Parameters::ProviderFields, providerFields.join(PPSDK::CPayment::Parameters::ProviderFieldsDelimiter), true));
		}
	}
}

//------------------------------------------------------------------------------
bool PaymentBase::getLimits(double & aMinAmount, double & aMaxAmount)
{
	if (getProviderSettings().isNull())
	{
		return false;
	}

	auto limits = getMNPProviderSettings().limits;

	QString minLimit = limits.min;
	QString maxLimit = limits.max;

	QRegExp macroPattern("\\{(.+)\\}");
	macroPattern.setMinimal(true);

	while (macroPattern.indexIn(minLimit) != -1)
	{
		minLimit.replace(macroPattern.cap(0), getParameter(macroPattern.cap(1)).value.toString());
	}

	while (macroPattern.indexIn(maxLimit) != -1)
	{
		maxLimit.replace(macroPattern.cap(0), getParameter(macroPattern.cap(1)).value.toString());
	}

	QScriptEngine myEngine;
	aMinAmount = myEngine.evaluate(minLimit).toString().toDouble();

	// Если не получилось вычислить максимальный лимит, то берем системный
	aMaxAmount = myEngine.evaluate(maxLimit.isEmpty() ? limits.system : maxLimit).toString().toDouble();

	return true;
}

//------------------------------------------------------------------------------
inline bool lessOrEqual(double aValue, double bValue)
{
	return aValue < bValue || qFuzzyCompare(aValue, bValue);
}

//------------------------------------------------------------------------------
inline bool greatOrEqual(double aValue, double bValue)
{
	return aValue > bValue || qFuzzyCompare(aValue, bValue);
}

//------------------------------------------------------------------------------
bool PaymentBase::calculateLimits()
{
	PPSDK::DealerSettings * dealerSettings =
		dynamic_cast<PPSDK::DealerSettings *>(mCore->getSettingsService()->getAdapter(PPSDK::CAdapterNames::DealerAdapter));

	Q_ASSERT(dealerSettings);

	double minAmount = 0.0;
	double maxAmount = 0.0;

	if (!getLimits(minAmount, maxAmount))
	{
		return false;
	}

	if (getBlockUpdateLimits())
	{
		return true;
	}

	const bool isFixedAmount = qFuzzyCompare(minAmount, maxAmount);
	const auto provider = getMNPProviderSettings();
	double systemMax = provider.limits.system.toDouble();
	systemMax = maxAmount > systemMax ? systemMax : maxAmount;

	if (!isFixedAmount)
	{
		// Корректируем максимальный платеж в зависимости от системного лимита
		if (qFuzzyIsNull(maxAmount) || qFuzzyIsNull(systemMax))
		{
			// Если какой-либо из лимитов не задан, то берем тот, который задан.
			maxAmount = qMax(systemMax, maxAmount);
		}
		else
		{
			// Иначе берем нижнюю границу.
			maxAmount = qMin(systemMax, maxAmount);
		}
	}

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

	foreach (const PPSDK::SProviderField & field, getProviderSettings().fields)
	{
		credentials.insert(field.id, getParameter(field.id).value.toString());
	}

	PPSDK::Commission com(dealerSettings->getCommission(getProvider(true), credentials, maxAmount));

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

		if (com.getType() == PPSDK::Commission::Percent && com.getValue())
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
		else if (com.getType() == PPSDK::Commission::Absolute && !qFuzzyIsNull(com.getValue()))
		{
			aAmount -= com.getValue();
			amountAll = aAmount;
		}
		else if (!qFuzzyIsNull(com.getMinCharge()))
		{
			amountAll = aAmount - com.getMinCharge();
		}

		return amountAll;
	};

	double localAmountAllLimit = calcAmountAll(maxAmount);
	double maxAmountAll = 0.0;

	if (isFixedAmount)
	{
		// если лимиты равны и комиссия больше системного лимита, то пропускаем эти лимиты дальше.
		maxAmountAll = localAmountAllLimit;
	}
	else if (localAmountAllLimit > systemMax)
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

	updateParameter(SParameter(PPSDK::CPayment::Parameters::MinAmount, minAmount));
	updateParameter(SParameter(PPSDK::CPayment::Parameters::MaxAmount, maxAmount));
	updateParameter(SParameter(PPSDK::CPayment::Parameters::MaxAmountAll, maxAmountAll));

	return true;
}

//------------------------------------------------------------------------------
QList<PPSDK::IPayment::SParameter> PaymentBase::calculateCommission(const QList<PPSDK::IPayment::SParameter> & aParameters)
{
	double maxAmount = getParameter(PPSDK::CPayment::Parameters::MaxAmount).value.toDouble();
	double maxAmountAll = getParameter(PPSDK::CPayment::Parameters::MaxAmountAll).value.toDouble();

	double amountAll = 0.f;
	double amount = 0.f;
	double commission = 0.f;
	double change = 0.f;

	foreach(auto p, aParameters)
	{
		if (p.name == PPSDK::CPayment::Parameters::AmountAll)
		{
			amountAll = p.value.toDouble();
		}
	}

	// Формируем массив заполненных полей для подсчёта комиссий.
	QVariantMap credentials;

	foreach (const PPSDK::SProviderField & field, getProviderSettings().fields)
	{
		credentials.insert(field.id, getParameter(field.id).value.toString());
	}

	const auto limits = getMNPProviderSettings().limits;
	PPSDK::DealerSettings * dealerSettings =
		dynamic_cast<PPSDK::DealerSettings *>(mCore->getSettingsService()->getAdapter(PPSDK::CAdapterNames::DealerAdapter));

	Q_ASSERT(dealerSettings);

	if (!qFuzzyIsNull(amountAll))
	{
		PPSDK::Commission com;
		double targetSum = 0.f;

		// Случай платежа с фиксированной суммой
		if (limits.min == limits.max)
		{
			if (lessOrEqual(amountAll, maxAmountAll))
			{
				targetSum = amountAll;

				com = dealerSettings->getCommission(getProvider(true), credentials, amountAll > maxAmount ? maxAmount : amountAll);
			}
			else
			{
				targetSum = maxAmountAll;

				com = dealerSettings->getCommission(getProvider(true), credentials, maxAmount);
			}
		}
		// Случай "плавающего" платежа
		else
		{
			if (greatOrEqual(amountAll, maxAmountAll))
			{
				targetSum = maxAmountAll;

				com = dealerSettings->getCommission(getProvider(true), credentials, maxAmount);
			}
			else if (amountAll > maxAmount && lessOrEqual(amountAll, maxAmountAll))
			{
				targetSum = amountAll;

				com = dealerSettings->getCommission(getProvider(true), credentials, maxAmount);
			}
			else
			{
				targetSum = amountAll;

				com = dealerSettings->getCommission(getProvider(true), credentials, targetSum);
			}
		}

		commission = com.getValueFor(targetSum, getMNPProviderSettings().processor.feeType == PPSDK::SProvider::FeeByAmount);
	}

	if (greatOrEqual(amountAll, maxAmountAll))
	{
		amount = qFuzzyCompare(maxAmount, limits.system.toDouble()) ? maxAmount - commission : maxAmount;
	}
	else
	{
		amount = amountAll - commission;
	}

	if (amount < 0.f)
	{
		amount = 0.f;
	}

	if (getMNPProviderSettings().processor.rounding)
	{
		amount = qRound(amount);

		// А если у нас сдача 0 рублей 58 копеек, то все в комиссию кидаем
		if (amount > amountAll)
		{
			amount = 0.f;
		}

		if (amountAll > maxAmountAll)
		{
			commission = maxAmountAll - amount;
		}
		else
		{
			commission = amountAll - amount;
		}
	}

	if (!qFuzzyCompare(amount + commission, amountAll) && (amount + commission < amountAll))
	{
		// amountAll превышает допустимую сумму
		change = amountAll - maxAmountAll;
		amountAll = maxAmountAll;
	}

	// Если комиссия превышает внесённую сумму, уравниваем.
	if (!qFuzzyCompare(commission, amountAll) && (commission > amountAll))
	{
		commission = amountAll;
	}

	// Для банковских платежей добавляем комиссию дилера и банка.
	double processingCommission = dealerSettings->getProcessingCommission(getProvider(true)).getValue(amount, amountAll);
	int vat = dealerSettings->getVAT(getProvider(true));

	QList<SParameter> result;

	auto calcFees = [](double aProcessingFee, double aFee) -> QPair<double, double>
	{
		double dealerFee = aFee - aProcessingFee;

		if (dealerFee > 0)
		{
			return QPair<double, double>(aProcessingFee, dealerFee);
		}
		else
		{
			return QPair<double, double>(aFee, 0.0);
		}
	};

	result
		<< SParameter(PPSDK::CPayment::Parameters::AmountAll, QString::number(amountAll, 'f', 2), true)
		<< SParameter(PPSDK::CPayment::Parameters::Amount, QString::number(amount, 'f', 2), true)
		<< SParameter(PPSDK::CPayment::Parameters::Fee, QString::number(commission, 'f', 2), true)
		<< SParameter(PPSDK::CPayment::Parameters::Change, QString::number(change, 'f', 2), true)
		<< SParameter(PPSDK::CPayment::Parameters::ProcessingFee, QString::number(calcFees(processingCommission, commission).first, 'f', 2), true)
		<< SParameter(PPSDK::CPayment::Parameters::DealerFee, QString::number(calcFees(processingCommission, commission).second, 'f', 2), true)
		<< SParameter(PPSDK::CPayment::Parameters::Vat, vat, true);

	return result;
}

//------------------------------------------------------------------------------
void PaymentBase::calculateSums()
{
	foreach (auto p, calculateCommission(QList<SParameter>() << getParameter(PPSDK::CPayment::Parameters::AmountAll)))
	{
		updateParameter(p);
	}
}

//------------------------------------------------------------------------------
bool PaymentBase::limitsDependOnParameter(const SParameter & aParameter)
{
	return
		!mIsRestoring &&
		!getMNPProviderSettings().isNull() &&
		(aParameter.name == PPSDK::CPayment::Parameters::AmountAll ||
		aParameter.name == PPSDK::CPayment::Parameters::Provider ||
		getMNPProviderSettings().limits.min.contains(QString("{%1}").arg(aParameter.name)) ||
		getMNPProviderSettings().limits.max.contains(QString("{%1}").arg(aParameter.name)));
}

//------------------------------------------------------------------------------
QString PaymentBase::createPaymentSession() const
{
	return QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz")
		+ QString("%1").arg(qrand() % 1000, 3, 10, QChar('0'));
}

//------------------------------------------------------------------------------
bool PaymentBase::isCriticalError(int aError) const
{
	PPSDK::TerminalSettings * terminalSettings =
		dynamic_cast<PPSDK::TerminalSettings *>(mCore->getSettingsService()->getAdapter(PPSDK::CAdapterNames::TerminalAdapter));

	Q_ASSERT(terminalSettings);

	return terminalSettings->getCriticalErrors().contains(aError);
}

//------------------------------------------------------------------------------
const PPSDK::SProvider PaymentBase::getProviderSettings(qint64 aProvider) const
{
	if (aProvider == -1)
	{
		return mProviderSettings;
	}

	if (!mCore)
	{
		toLog(LogLevel::Error, QString("Payment %1. Failed to get core object.").arg(getID()));

		return PPSDK::SProvider();
	}

	PPSDK::ISettingsService * settingsService = mCore->getSettingsService();
	if (!settingsService)
	{
		toLog(LogLevel::Error, QString("Payment %1. Failed to get settings service.").arg(getID()));

		return PPSDK::SProvider();
	}

	PPSDK::DealerSettings * dealerSettings =
		dynamic_cast<PPSDK::DealerSettings *>(settingsService->getAdapter(PPSDK::CAdapterNames::DealerAdapter));
	if (!dealerSettings)
	{
		toLog(LogLevel::Error, QString("Payment %1. Failed to get dealer settings.").arg(getID()));

		return PPSDK::SProvider();
	}

	return getFactory()->getProviderSpecification(dealerSettings->getProvider(aProvider));
}

//------------------------------------------------------------------------------
const SDK::PaymentProcessor::SProvider PaymentBase::getMNPProviderSettings() const
{
	return getProviderSettings(getProvider(true));
}

//------------------------------------------------------------------------------
const PPSDK::SKeySettings & PaymentBase::getKeySettings() const
{
	return mKeySettings;
}

//------------------------------------------------------------------------------
bool PaymentBase::restore(const QList<SParameter> & aParameters)
{
	mIsRestoring = true;

	try
	{
		if (!mCore)
		{
			throw QString("failed to get core object");
		}

		foreach (SParameter parameter, aParameters)
		{
			parameter.updated = false;

			// Расшифровываем параметры ключом терминала.
			if (parameter.crypted)
			{
				ICryptEngine * cryptEngine = mCore->getCryptService()->getCryptEngine();

				QByteArray decryptedValue;

				QString error;

				if (cryptEngine->decryptLong(-1, parameter.value.toString().toLatin1(), decryptedValue, error))
				{
					parameter.value = QString::fromUtf8(decryptedValue);
				}
				else
				{
					toLog(LogLevel::Error, QString("Payment %1. Failed to decrypt parameter %2 while restoring. Error: %3.")
						.arg(getID()).arg(parameter.name).arg(error));
				}
			}

			updateParameter(parameter);
		}

		QStringList requiredParameters;

		requiredParameters
			<< PPSDK::CPayment::Parameters::ID
			<< PPSDK::CPayment::Parameters::Type
			<< PPSDK::CPayment::Parameters::CreationDate
			<< PPSDK::CPayment::Parameters::Provider
			<< PPSDK::CPayment::Parameters::Status
			<< PPSDK::CPayment::Parameters::Priority;

		foreach (const QString & requiredParameter, requiredParameters)
		{
			QReadLocker lock(&mParametersLock);

			if (mParameters.find(requiredParameter) == mParameters.end())
			{
				throw QString("required parameter missing: %1").arg(requiredParameter);
			}
		}

		if (getProviderSettings().isNull())
		{
			qint64 id = getID();

			if (id == -1)
			{
				toLog(LogLevel::Debug, QString("Payment %1. Restore provider settings failed.").arg(id));
			}
			else
			{
				// Проведение платежа невозможно, но данные из него извлечь можно, а в случае необходимости и поменять
				// провайдера.
				toLog(LogLevel::Warning, QString("Payment %1. Failed to load provider settings.").arg(id));
			}
		}

		PPSDK::ISettingsService * settingsService = mCore->getSettingsService();
		if (!settingsService)
		{
			throw QString("failed to get settings service");
		}

		PPSDK::TerminalSettings * terminalSettings =
			dynamic_cast<PPSDK::TerminalSettings *>(settingsService->getAdapter(PPSDK::CAdapterNames::TerminalAdapter));
		if (!terminalSettings)
		{
			throw QString("failed to get terminal settings");
		}

		mKeySettings = terminalSettings->getKeys()[mProviderSettings.processor.keyPair];
		if (!mKeySettings.isValid)
		{
			throw QString("failed to get key pair settings");
		}

		// После загрузки всех параметров пересчитываем лимиты
		calculateLimits();
	}
	catch (QString & error)
	{
		toLog(LogLevel::Error, QString("Payment %1. Failed to restore. Error: %2.").arg(getID()).arg(error));

		mIsRestoring = false;

		return false;
	}

	mIsRestoring = false;

	return true;
}

//------------------------------------------------------------------------------
QList<SDK::PaymentProcessor::IPayment::SParameter> PaymentBase::serialize() const
{
	QReadLocker lock(&mParametersLock);

	const TParameterList::index<UpdateTag>::type & index = mParameters.get<UpdateTag>();

	TParameterList::index<UpdateTag>::type::iterator i = index.lower_bound(true);
	TParameterList::index<UpdateTag>::type::iterator j = index.upper_bound(true);

	QList<SParameter> result;

	while (i != j)
	{
		SParameter parameter = *i++;

		// Шифруем параметры ключом терминала.
		if (parameter.crypted)
		{
			ICryptEngine * cryptEngine = mCore->getCryptService()->getCryptEngine();

			QByteArray encryptedValue;

			QString error;

			if (cryptEngine->encryptLong(-1, parameter.value.toString().toUtf8(), encryptedValue, error))
			{
				parameter.value = encryptedValue;
			}
			else
			{
				toLog(LogLevel::Error, QString("Payment %1. Failed to encrypt parameter %2 while serializing. Error: %3.")
					.arg(getID()).arg(parameter.name).arg(error));

				parameter.value.clear();
			}
		}

		result << parameter;
	}

	return result;
}

//------------------------------------------------------------------------------
bool PaymentBase::remove()
{
	// Можем отменить платёж, только если не было запроса PAY.
	if (getParameter(PPSDK::CPayment::Parameters::Step).value.toInt() == CPayment::Steps::Init)
	{
		setStatus(PPSDK::EPaymentStatus::Deleted);

		toLog(LogLevel::Normal, QString("Payment %1. Deleted.").arg(getID()));

		return true;
	}

	toLog(LogLevel::Warning, QString("Payment %1. Failed to delete.").arg(getID()));

	return false;
}

//------------------------------------------------------------------------------
bool PaymentBase::cancel()
{
	// Можем отменить платёж, только если не было запроса PAY.
	if (getParameter(PPSDK::CPayment::Parameters::Step).value.toInt() == CPayment::Steps::Init)
	{
		setStatus(PPSDK::EPaymentStatus::Canceled);

		toLog(LogLevel::Normal, QString("Payment %1. Canceled.").arg(getID()));

		return true;
	}

	toLog(LogLevel::Warning, QString("Payment %1. Failed to cancel.").arg(getID()));

	return false;
}

//------------------------------------------------------------------------------
