/* @file Сервис, владеющий платёжными потоками. */

// Boost
#include <boost/foreach.hpp>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegExp>
#include <QtCore/QMutexLocker>
#include <QtCore/QCryptographicHash>
#include <QtConcurrent/QtConcurrentRun>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Core/EventTypes.h>
#include <SDK/PaymentProcessor/Core/IFundsService.h>
#include <SDK/PaymentProcessor/Core/ITerminalService.h>
#include <SDK/PaymentProcessor/Core/ServiceParameters.h>
#include <SDK/PaymentProcessor/Core/HookConstants.h>
#include <SDK/PaymentProcessor/Payment/Step.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>
#include <SDK/PaymentProcessor/Payment/IPayment.h>
#include <SDK/PaymentProcessor/Payment/IPaymentFactory.h>
#include <SDK/PaymentProcessor/Settings/DealerSettings.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

// Модули
#include <Crypt/ICryptEngine.h>

// Проект
#include "DatabaseUtils/IPaymentDatabaseUtils.h"

#include "Services/ServiceNames.h"
#include "Services/PluginService.h"
#include "Services/SettingsService.h"
#include "Services/DatabaseService.h"
#include "Services/CryptService.h"
#include "Services/EventService.h"
#include "Services/PaymentService.h"
#include "Services/HookService.h"
#include "Services/ServiceCommon.h"
#include "Services/NetworkService.h"

namespace PPSDK = SDK::PaymentProcessor;

using namespace std::placeholders;

//---------------------------------------------------------------------------
namespace CPaymentService
{
	/// Название нити, обрабатывающей оффлайн платежи.
	const char ThreadName[] = "PaymentThread";

	/// Таймаут обработки очереди оффлайн платежей.
	const int ProcessOfflineTimeout = 1 * 1000;

	/// Таймаут онлайн платежа, когда он гарантированно переводится в BAD
	const int OnlinePaymentOvertime = 30 * 60 * 1000;

	/// Таймаут обработки очереди оффлайн платежей при отсутствии связи.
	const int CheckNetworkConnectionTimeout = 5 * 1000;

	/// Тип платежа, в который будет добавляться неизрасходованная сдача.
	const char ChangePaymentType[] = "cyberplat";

	/// Признак того, что в платеже хранится неизрасходованная сдача.
	const char ChangePaymentParam[] = "UNUSED_CHANGE";
}

//---------------------------------------------------------------------------
PaymentService * PaymentService::instance(IApplication * aApplication)
{
	return static_cast<PaymentService *>(aApplication->getCore()->getService(CServices::PaymentService));
}

//---------------------------------------------------------------------------
PaymentService::PaymentService(IApplication * aApplication)
	: ILogable("Payments"),
	  mApplication(aApplication),
	  mEnabled(false),
	  mPaymentLock(QMutex::Recursive),
	  mOfflinePaymentID(-1),
	  mOfflinePaymentLock(QMutex::Recursive),
	  mCommandMutex(QMutex::Recursive)
{
	qRegisterMetaType<EPaymentCommandResult::Enum>("EPaymentCommandResult");

	mPaymentThread.setObjectName(CPaymentService::ThreadName);

	mPaymentTimer.setSingleShot(true);
	mPaymentTimer.moveToThread(&mPaymentThread);

	connect(&mPaymentThread, SIGNAL(started()), SLOT(onPaymentThreadStarted()), Qt::DirectConnection);
	connect(&mPaymentThread, SIGNAL(finished()), &mPaymentTimer, SLOT(stop()), Qt::DirectConnection);

	connect(&mPaymentTimer, SIGNAL(timeout()), SLOT(onProcessPayments()), Qt::DirectConnection);
}

//---------------------------------------------------------------------------
bool PaymentService::initialize()
{
	connect(mApplication->getCore()->getFundsService()->getAcceptor(), SIGNAL(amountUpdated(qint64, double, double)), SLOT(onAmountUpdated(qint64, double, double)));
	connect(mApplication->getCore()->getFundsService()->getDispenser(), SIGNAL(dispensed(double)), SLOT(onAmountDispensed(double)));

	mCommandIndex = 1;

	mDBUtils = DatabaseService::instance(mApplication)->getDatabaseUtils<IPaymentDatabaseUtils>();
	if (!mDBUtils)
	{
		toLog(LogLevel::Error, "Failed to get database utils.");

		return false;
	}

	QStringList factories =
		PluginService::instance(mApplication)->getPluginLoader()->getPluginList(QRegExp("PaymentProcessor\\.PaymentFactory\\..*"));

	foreach (const QString & path, factories)
	{
		SDK::Plugin::IPlugin * plugin = PluginService::instance(mApplication)->getPluginLoader()->createPlugin(path);

		SDK::PaymentProcessor::IPaymentFactory * factory = dynamic_cast<SDK::PaymentProcessor::IPaymentFactory *>(plugin);
		if (factory)
		{
			factory->setSerializer(std::bind(&PaymentService::savePayment, this, _1));

			foreach (const QString & type, factory->getSupportedPaymentTypes())
			{
				mFactoryByType[type] = factory;
			}

			mFactories << factory;
		}
		else
		{
			PluginService::instance(mApplication)->getPluginLoader()->destroyPlugin(plugin);
		}
	}

	// Ищем всех провайдеров с неподдерживаемым типом процессинга
	auto dealerSettings = SettingsService::instance(mApplication)->getAdapter<SDK::PaymentProcessor::DealerSettings>();

	foreach (const QString & processingType, dealerSettings->getProviderProcessingTypes().toSet().subtract(mFactoryByType.keys().toSet()))
	{
		// И удаляем их
		foreach (qint64 providerId, dealerSettings->getProviders(processingType))
		{
			toLog(LogLevel::Error, QString("Drop provider %1. Unsupported processing type: '%2'.").arg(providerId).arg(processingType));

			dealerSettings->disableProvider(providerId);
		}
	}

	return true;
}

//------------------------------------------------------------------------------
void PaymentService::finishInitialize()
{
	foreach (auto factory, mFactories)
	{
		if (!factory->initialize())
		{
			toLog(LogLevel::Error, QString("Failed initialize payment factory plugin for payment types: %1.").arg(factory->getSupportedPaymentTypes().join(", ")));
		}
	}

	// Запускаем поток на обработку оффлайн платежей.
	mPaymentThread.start();

	mEnabled = true;
}

//---------------------------------------------------------------------------
bool PaymentService::canShutdown()
{
	return true;
}

//---------------------------------------------------------------------------
bool PaymentService::shutdown()
{
	mEnabled = false;

	// Спрашиваем сетевой сервис может ли он закрыться. Тем самым сбрасывая все сетевые задачи.
	mApplication->getCore()->getService(CServices::NetworkService)->canShutdown();

	disconnect(mApplication->getCore()->getFundsService()->getAcceptor());
	disconnect(mApplication->getCore()->getFundsService()->getDispenser());

	mActivePaymentSynchronizer.waitForFinished();

	SafeStopServiceThread(&mPaymentThread, 3000, getLog());

	if (mChangePayment)
	{
		mChangePayment.reset();
	}

	setPaymentActive(std::shared_ptr<PPSDK::IPayment>());

	while (!mFactories.isEmpty())
	{
		PluginService::instance(mApplication)->getPluginLoader()->destroyPlugin(
			dynamic_cast<SDK::Plugin::IPlugin *>(mFactories.takeFirst()));
	}

	return true;
}

//---------------------------------------------------------------------------
QString PaymentService::getName() const
{
	return CServices::PaymentService;
}

//---------------------------------------------------------------------------
const QSet<QString> & PaymentService::getRequiredServices() const
{
	static QSet<QString> requiredServices = QSet<QString>()
		<< CServices::SettingsService
		<< CServices::EventService
		<< CServices::PluginService
		<< CServices::DatabaseService
		<< CServices::CryptService;

	return requiredServices;
}

//---------------------------------------------------------------------------
QVariantMap PaymentService::getParameters() const
{
	// TODO Заполнить параметры
	QVariantMap parameters;
	parameters[PPSDK::CServiceParameters::Payment::UnprocessedPaymentCount] = "";
	parameters[PPSDK::CServiceParameters::Payment::PaymentsPerDay] = "";

	return parameters;
}

//---------------------------------------------------------------------------
void PaymentService::resetParameters(const QSet<QString> &)
{
}

//---------------------------------------------------------------------------
void PaymentService::setPaymentActive(std::shared_ptr<SDK::PaymentProcessor::IPayment> aPayment)
{
	QMutexLocker lock(&mPaymentLock);

	mActivePayment = aPayment;
}

//---------------------------------------------------------------------------
bool PaymentService::isPaymentActive(std::shared_ptr<PPSDK::IPayment> aPayment)
{
	QMutexLocker lock(&mPaymentLock);

	return aPayment == mActivePayment;
}

//---------------------------------------------------------------------------
qint64 PaymentService::createPayment(qint64 aProvider)
{
	using namespace std::placeholders;

	if (getActivePayment() != -1)
	{
		setPaymentActive(std::shared_ptr<PPSDK::IPayment>());
	}

	toLog(LogLevel::Normal, QString("Creating payment. Provider: %1.").arg(aProvider));

	PPSDK::SProvider provider = SettingsService::instance(mApplication)->getAdapter<SDK::PaymentProcessor::DealerSettings>()->getProvider(aProvider);
	if (provider.isNull())
	{
		toLog(LogLevel::Error, QString("Failed to get settings for provider %1.").arg(aProvider));

		return -1;
	}

	if (mFactoryByType.contains(provider.processor.type))
	{
		PPSDK::IPaymentFactory * factory = mFactoryByType[provider.processor.type];

		std::shared_ptr<PPSDK::IPayment> payment(factory->createPayment(provider.processor.type), std::bind(&PPSDK::IPaymentFactory::releasePayment, factory, _1));
		if (!payment)
		{
			toLog(LogLevel::Error, "Failed to create payment object.");

			return -1;
		}

		QList<PPSDK::IPayment::SParameter> parameters;

		parameters
			<< PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::ID, mDBUtils->createDummyPayment(), true)
			<< PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::CreationDate, QDateTime::currentDateTime(), true)
			<< PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::Type, provider.processor.type, true)
			<< PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::Provider, provider.id, true)
			<< PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::Status, PPSDK::EPaymentStatus::Init, true)
			<< PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::Priority, PPSDK::IPayment::Online, true);

		if (!payment->restore(parameters))
		{
			toLog(LogLevel::Error, "Failed to initialize payment object.");

			return -1;
		}

		if (payment->getID() == -1)
		{
			toLog(LogLevel::Error, "Failed to create payment ID.");

			return -1;
		}

		// Добавляем платёж в список активных
		setPaymentActive(payment);

		return payment->getID();
	}

	toLog(LogLevel::Error, QString("Failed to find factory for payment type %1.").arg(provider.processor.type));

	return -1;
}

//---------------------------------------------------------------------------
bool PaymentService::savePayment(PPSDK::IPayment * aPayment)
{
	if (mDBUtils->savePayment(aPayment, createSignature(aPayment)))
	{
		EventService::instance(mApplication)->sendEvent(PPSDK::Event(PPSDK::EEventType::PaymentUpdated, CServices::PaymentService, aPayment->getID()));

		return true;
	}
	else
	{
		QString amountAll = aPayment->getParameter(PPSDK::CPayment::Parameters::AmountAll).value.toString();
		QString msg = QString("Payment [%1] error write to database (AMOUNT_ALL=%2).").arg(aPayment->getInitialSession()).arg(amountAll);

		// Выставить ошибочный статус устройства "терминал"
		mApplication->getCore()->getEventService()->sendEvent(
			SDK::PaymentProcessor::Event(SDK::PaymentProcessor::EEventType::Critical, CServices::DatabaseService, msg));

		return false;
	}
}

//---------------------------------------------------------------------------
qint64 PaymentService::getActivePayment() const
{
	return mActivePayment ? mActivePayment->getID() : -1;
}

//---------------------------------------------------------------------------
void PaymentService::deactivatePayment()
{
	if (mActivePayment)
	{
		QMutexLocker lock(&mPaymentLock);

		savePayment(mActivePayment.get());

		mActivePayment.reset();
	}

	// Если существует пустой платеж "сдача", нужно его пометить в БД как удаленный
	if (mChangePayment && qFuzzyIsNull(getChangeAmount()))
	{
		resetChange();
	}
}

//---------------------------------------------------------------------------
QString PaymentService::createSignature(PPSDK::IPayment * aPayment)
{
	if (!aPayment)
	{
		toLog(LogLevel::Error, "Failed to create payment signature. No payment specified.");

		return "";
	}

	QString signature;

	PPSDK::SProvider provider = SettingsService::instance(mApplication)->getAdapter<SDK::PaymentProcessor::DealerSettings>()->
		getProvider(aPayment->getProvider(false));

	PPSDK::SKeySettings keys = SettingsService::instance(mApplication)->getAdapter<PPSDK::TerminalSettings>()->
		getKeys()[provider.processor.keyPair];

	signature += QString("SD: %1, AP: %2, OP: %3\n").arg(keys.sd).arg(keys.ap).arg(keys.op);

	// В подпись включаем все суммы платежа.
	QStringList controlParameters;
	controlParameters
		<< PPSDK::CPayment::Parameters::Amount
		<< PPSDK::CPayment::Parameters::AmountAll
		<< PPSDK::CPayment::Parameters::Change
		<< PPSDK::CPayment::Parameters::Fee;

	signature += "Sums:\n";

	foreach (const QString & parameter, controlParameters)
	{
		signature += parameter + " : " + aPayment->getParameter(parameter).value.toString() + "\n";
	}

	signature += "Provider fields:\n";

	// Включаем в подпись все поля данных оператора
	QStringList providerFields;

	foreach (const PPSDK::SProviderField & field, provider.fields)
	{
		if (!field.keepEncrypted())
		{
			providerFields << field.id;
		}
	}

	if (providerFields.isEmpty())
	{
		// Описания оператора нет, берём названия полей из базы.
		providerFields = aPayment->getParameter(PPSDK::CPayment::Parameters::ProviderFields).value.toString()
			.split(PPSDK::CPayment::Parameters::ProviderFieldsDelimiter);
	}

	providerFields.sort();

	foreach (const QString & field, providerFields)
	{
		signature += field + " : " + aPayment->getParameter(field).value.toString() + "\n";
	}

	// Включаем в подпись основные поля платежа.
	signature += "id\tcreate_date\tinitial_session\tsession\toperator\tstatus\tstep\n";

	signature += QString::number(aPayment->getID());
	signature += "\t";
	signature += aPayment->getCreationDate().toString("yyyy-MM-dd hh:mm:ss.zzz");
	signature += "\t";
	signature += aPayment->getInitialSession();
	signature += "\t";
	signature += aPayment->getSession();
	signature += "\t";
	signature += QString::number(aPayment->getProvider(false));
	signature += "\t";
	signature += QString::number(aPayment->getStatus());
	signature += "\t";
	signature += aPayment->getParameter(PPSDK::CPayment::Parameters::Step).value.toString();

#ifndef _DEBUG
	signature = QString::fromUtf8(QCryptographicHash::hash(signature.toUtf8(), QCryptographicHash::Md5).toHex().toUpper());

	ICryptEngine * crypt = static_cast<ICryptEngine *>(CryptService::instance(mApplication)->getCryptEngine());

	QByteArray encodedSignature;

	QString error;

	if (!crypt->encryptLong(-1, signature.toUtf8(), encodedSignature, error))
	{
		toLog(LogLevel::Error, QString("Payment %1. Failed to encrypt signature. Error: %2.").arg(aPayment->getID()).arg(error));

		return QString();
	}

	return QString::fromUtf8(encodedSignature);
#else
	return signature;
#endif // _DEBUG
}

//---------------------------------------------------------------------------
bool PaymentService::verifySignature(PPSDK::IPayment * aPayment)
{
	QString signature = aPayment->getParameter(PPSDK::CPayment::Parameters::Signature).value.toString();

	if (signature.isEmpty())
	{
		toLog(LogLevel::Error, QString("Payment %1. No signature.").arg(aPayment->getID()));
		return false;
	}

	QString runtimeSignature = createSignature(aPayment);

#ifndef _DEBUG
	ICryptEngine * crypt = static_cast<ICryptEngine *>(CryptService::instance(mApplication)->getCryptEngine());

	QByteArray decodedSignature;
	QByteArray decodedRuntimeSignature;

	QString error;

	if (!crypt->decryptLong(-1, signature.toUtf8(), decodedSignature, error))
	{
		toLog(LogLevel::Warning, QString("Payment %1. Failed to decrypt signature. Error: %2.").arg(aPayment->getID()).arg(error));

		return false;
	}

	if (!crypt->decryptLong(-1, runtimeSignature.toUtf8(), decodedRuntimeSignature, error))
	{
		toLog(LogLevel::Warning, QString("Payment %1. Failed to decrypt runtime signature. Error: %2.").arg(aPayment->getID()).arg(error));

		return false;
	}

	return (decodedSignature == decodedRuntimeSignature);
#else
	return (signature == runtimeSignature);
#endif // _DEBUG
}

//---------------------------------------------------------------------------
std::shared_ptr<PPSDK::IPayment> PaymentService::getPayment(qint64 aID)
{
	{
		QMutexLocker lock(&mPaymentLock);

		// Если спрашиваем активный платёж, не перезагружаем его из базы.
		if (mActivePayment && (mActivePayment->getID() == aID))
		{
			return mActivePayment;
		}
	}

	if (aID == -1)
	{
		toLog(LogLevel::Debug, QString("Payment %1. Payment not exist.").arg(aID));

		return std::shared_ptr<PPSDK::IPayment>();
	}
	
	toLog(LogLevel::Normal, QString("Payment %1. Loading...").arg(aID));

	QList<PPSDK::IPayment::SParameter> parameters = mDBUtils->getPaymentParameters(aID);

	PPSDK::IPayment::SParameter type = PPSDK::IPayment::parameterByName(PPSDK::CPayment::Parameters::Type, parameters);

	if (!type.isNull() && mFactoryByType.contains(type.value.toString()))
	{
		std::shared_ptr<PPSDK::IPayment> payment(
			mFactoryByType[type.value.toString()]->createPayment(type.value.toString()),
			std::bind(&PPSDK::IPaymentFactory::releasePayment, mFactoryByType[type.value.toString()], _1));
		if (!payment)
		{
			toLog(LogLevel::Normal, QString("Payment %1. Failed to create payment object.").arg(aID));

			return std::shared_ptr<PPSDK::IPayment>();
		}

		if (!payment->restore(parameters))
		{
			toLog(LogLevel::Normal, QString("Payment %1. Failed to restore payment object.").arg(aID));

			return std::shared_ptr<PPSDK::IPayment>();
		}

		if (!verifySignature(payment.get()))
		{
			toLog(LogLevel::Warning, QString("Payment %1. Cheated.").arg(payment->getID()));

			payment->setStatus(PPSDK::EPaymentStatus::Cheated);

			savePayment(payment.get());
		}

		return payment;
	}

	return std::shared_ptr<PPSDK::IPayment>();
}

//---------------------------------------------------------------------------
std::shared_ptr<PPSDK::IPayment> PaymentService::getPayment(const QString & aInitialSession)
{
	qint64 id = mDBUtils->getPaymentByInitialSession(aInitialSession);

	if (id != -1)
	{
		return getPayment(id);
	}

	return std::shared_ptr<PPSDK::IPayment>();
}

//---------------------------------------------------------------------------
PPSDK::SProvider PaymentService::getProvider(qint64 aID)
{
	PPSDK::SProvider provider = SettingsService::instance(mApplication)->getAdapter<SDK::PaymentProcessor::DealerSettings>()->getProvider(aID);
	if (!provider.isNull())
	{
		if (mFactoryByType.contains(provider.processor.type))
		{
			provider = mFactoryByType[provider.processor.type]->getProviderSpecification(provider);
		}
		else
		{
			toLog(LogLevel::Error, QString("Provider #%1: has unknown payment processor type: %2.").arg(aID).arg(provider.processor.type));
		}
	}

	return provider;
}

//---------------------------------------------------------------------------
PPSDK::IPayment::SParameter PaymentService::getPaymentField(qint64 aPayment, const QString & aName)
{
	return PPSDK::IPayment::parameterByName(aName, getPaymentFields(aPayment));
}

//---------------------------------------------------------------------------
TPaymentParameters PaymentService::getPaymentFields(qint64 aPayment)
{
	if (aPayment == getActivePayment())
	{
		std::shared_ptr<PPSDK::IPayment> payment = getPayment(aPayment);

		return payment ? payment->getParameters() : QList<PPSDK::IPayment::SParameter>();
	}
	else
	{
		return mDBUtils->getPaymentParameters(aPayment);
	}
}

//------------------------------------------------------------------------------
QMap<qint64, TPaymentParameters> PaymentService::getPaymentsFields(const QList<qint64> & aIds)
{
	QMap<qint64, TPaymentParameters> result = mDBUtils->getPaymentParameters(aIds); 

	if (aIds.contains(getActivePayment()))
	{
		std::shared_ptr<PPSDK::IPayment> payment = getPayment(getActivePayment());

		if (payment)
		{
			// объединяем два списка параметров
			QMap<QString, SDK::PaymentProcessor::IPayment::SParameter> parameters;

			foreach (auto param, payment->getParameters())
			{
				parameters.insert(param.name, param);
			}

			// из базы более старые параметры
			foreach (auto param, result.value(getActivePayment()))
			{
				if (!parameters.contains(param.name))
				{
					parameters.insert(param.name, param);
				}
			}

			result.insert(getActivePayment(), parameters.values());
		}
	}

	return result;
}

//---------------------------------------------------------------------------
QList<PPSDK::IPayment::SParameter> PaymentService::calculateCommission(const QList<PPSDK::IPayment::SParameter> & aParameters)
{
	if (mActivePayment)
	{
		return mActivePayment->calculateCommission(aParameters);
	}

	return QList<PPSDK::IPayment::SParameter>();
}

//---------------------------------------------------------------------------
bool PaymentService::updatePaymentField(qint64 aID, const PPSDK::IPayment::SParameter & aField, bool aForceUpdate)
{
	QList<PPSDK::IPayment::SParameter> parameters;

	parameters.append(aField);

	return updatePaymentFields(aID, parameters, aForceUpdate);
}

//---------------------------------------------------------------------------
bool PaymentService::updatePaymentFields(qint64 aID, const QList<SDK::PaymentProcessor::IPayment::SParameter> & aFields, bool aForceUpdate)
{
	QMutexLocker lock(&mOfflinePaymentLock);

	if (mOfflinePaymentID != aID)
	{
		doUpdatePaymentFields(aID, getPayment(aID), aFields);
	}
	else if (aForceUpdate)
	{
		// mOfflinePaymentLock гарантирует что мы попали на запись параметра ДО сохранения оффлайн платежа в БД
		// сохраняем параметр в объект, обслуживаемый в оффлайне
		doUpdatePaymentFields(aID, mOfflinePayment, aFields, aForceUpdate);
		// тут же сохраняем объект в базу напрямую
		doUpdatePaymentFields(aID, getPayment(aID), aFields, aForceUpdate);
	}
	else
	{
		lock.unlock();

		// Создаем команду на обновление поля.
		QMutexLocker lock(&mCommandMutex);

		auto command = [aID, aFields](PaymentService * aService) -> EPaymentCommandResult::Enum
		{
			aService->doUpdatePaymentFields(aID, aService->getPayment(aID), aFields);
			aService->mPaymentHaveUnsavedParameters.remove(aID);
			return EPaymentCommandResult::OK;
		};

		mCommands << qMakePair(mCommandIndex++, std::function<EPaymentCommandResult::Enum(PaymentService *)>(command));
		mPaymentHaveUnsavedParameters.insert(aID);
	}

	// TODO: убрать возвращаемое значение.
	return true;
}

//---------------------------------------------------------------------------
void PaymentService::processPaymentStep(qint64 aPayment, SDK::PaymentProcessor::EPaymentStep::Enum aStep, bool aBlocking)
{
	if (aBlocking)
	{
		std::shared_ptr<PPSDK::IPayment> payment(getPayment(aPayment));
		if (!payment)
		{
			toLog(LogLevel::Error, QString("Payment %1. Failed to perform step.").arg(aPayment));

			emit stepCompleted(aPayment, aStep, true);
		}
		else
		{
			if (aStep == PPSDK::EPaymentStep::Pay)
			{
				payment->setCompleteDate(QDateTime::currentDateTime());
			}

			bool result = payment->performStep(aStep);

			savePayment(payment.get());

			emit stepCompleted(aPayment, aStep, !result);
		}
	}
	else
	{
		mActivePaymentSynchronizer.addFuture(QtConcurrent::run(this, &PaymentService::processPaymentStep, aPayment, aStep, true));
	}
}

//---------------------------------------------------------------------------
bool PaymentService::convertPayment(qint64 aPayment, const QString & aTargetType)
{
	if (mFactoryByType.contains(aTargetType))
	{
		std::shared_ptr<PPSDK::IPayment> payment(getPayment(aPayment));

		if (payment)
		{
			if (mFactoryByType[aTargetType]->convertPayment(aTargetType, payment.get()))
			{
				savePayment(payment.get());

				if (isPaymentActive(payment))
				{
					QMutexLocker lock(&mPaymentLock);

					// Заменяем активный платёж на новый экземпляр с другим типом процессинга
					mActivePayment.reset();
					setPaymentActive(getPayment(aPayment));
				}

				toLog(LogLevel::Normal, QString("Payment %1. Converted to type %2.").arg(aPayment).arg(aTargetType));

				return true;
			}
		}
	}

	toLog(LogLevel::Error, QString("Payment %1. Failed convert to %2 type.").arg(aPayment).arg(aTargetType));

	return false;
}

//---------------------------------------------------------------------------
bool PaymentService::processPayment(qint64 aPayment, bool aOnline)
{
	std::shared_ptr<PPSDK::IPayment> payment(getPayment(aPayment));

	if (!payment)
	{
		toLog(LogLevel::Error, QString("Payment %1 not avaible.").arg(aPayment));
		return false;
	}

	// Блокируем проведение платежа с "финальными" статусами
	switch (payment->getStatus())
	{
	case PPSDK::EPaymentStatus::Init:
	case PPSDK::EPaymentStatus::ReadyForCheck:
	case PPSDK::EPaymentStatus::ProcessError:
		break;
	case PPSDK::EPaymentStatus::Completed: // А сюда можем попасть досрочно, если обработка платежа происходит в плагине.
		return true;
	default:
		toLog(LogLevel::Normal, QString("Payment %1 has status %2. Process deny.").arg(aPayment).arg(payment->getStatus()));
		return false;
	}

	if (!aOnline)
	{
		toLog(LogLevel::Normal, QString("Payment %1. Offline mode.").arg(aPayment));

		payment->setPriority(PPSDK::IPayment::Offline);
		payment->setStatus(PPSDK::EPaymentStatus::ReadyForCheck);
		auto stamp = QDateTime::currentDateTime();
		payment->setNextTryDate(stamp);
		payment->setCompleteDate(stamp);

		savePayment(payment.get());
	}
	else
	{
		toLog(LogLevel::Normal, QString("Payment %1. Online mode.").arg(aPayment));

		processPaymentStep(aPayment, SDK::PaymentProcessor::EPaymentStep::Pay, false);
	}

	return true;
}

//---------------------------------------------------------------------------
bool PaymentService::cancelPayment(qint64 aPayment)
{
	bool result = false;

	std::shared_ptr<PPSDK::IPayment> payment(getPayment(aPayment));

	if (payment)
	{
		result = payment->cancel() && savePayment(payment.get());
	}

	return result;
}

//---------------------------------------------------------------------------
bool PaymentService::stopPayment(qint64 aPayment, int aError, const QString & aErrorMessage)
{
	bool result = false;

	std::shared_ptr<PPSDK::IPayment> payment(getPayment(aPayment));

	if (payment)
	{
		toLog(LogLevel::Normal, QString("Payment %1. Stopped because of '%2'. Error code:%3.").arg(aPayment).arg(aErrorMessage).arg(aError));

		payment->setStatus(PPSDK::EPaymentStatus::BadPayment);
		payment->setParameter(PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::ServerError, aError, true));
		payment->setParameter(PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::ErrorMessage, aErrorMessage, true));
		result = savePayment(payment.get());
	}

	return result;
}

//---------------------------------------------------------------------------
bool PaymentService::removePayment(qint64 aPayment)
{
	bool result = false;

	std::shared_ptr<PPSDK::IPayment> payment(getPayment(aPayment));

	if (payment)
	{
		result = payment->remove() && savePayment(payment.get());
	}

	return result;
}
//---------------------------------------------------------------------------
bool PaymentService::canProcessPaymentOffline(qint64 aPayment)
{
	std::shared_ptr<PPSDK::IPayment> payment(getPayment(aPayment));

	if (payment)
	{
		return payment->canProcessOffline();
	}

	return false;
}

//---------------------------------------------------------------------------
void PaymentService::hangupProcessing()
{
	QMetaObject::invokeMethod(this, "onProcessPayments", Qt::QueuedConnection);
}

//---------------------------------------------------------------------------
void PaymentService::doUpdatePaymentFields(quint64 aID, std::shared_ptr<PPSDK::IPayment> aPayment, const QList<SDK::PaymentProcessor::IPayment::SParameter> & aFields, bool aForce)
{
	if (!aPayment)
	{
		toLog(LogLevel::Error, QString("Payment %1. Failed to update parameters.").arg(aID));
		return;
	}

	foreach (const PPSDK::IPayment::SParameter & parameter, aFields)
	{
		toLog(LogLevel::Normal, QString("Payment %1. %2pdating parameter: name '%3', value '%4'.")
			.arg(aID)
			.arg(aForce ? "Force u" : "U")
			.arg(parameter.name)
			.arg(parameter.crypted ? "** CRYPTED **" : parameter.value.toString()));

		if ((parameter.name == PPSDK::CPayment::Parameters::Amount) ||
			(parameter.name == PPSDK::CPayment::Parameters::AmountAll) ||
			(parameter.name == PPSDK::CPayment::Parameters::Change) ||
			(parameter.name == PPSDK::CPayment::Parameters::Fee) ||
			(parameter.name == PPSDK::CPayment::Parameters::ID))
		{
			toLog(LogLevel::Error, QString("Payment %1. Cannot update money related field manually.").arg(aID));

			continue;
		}

		aPayment->setParameter(parameter);
	}

	if (!savePayment(aPayment.get()))
	{
		toLog(LogLevel::Error, QString("Payment %1. Failed to save updated payment.").arg(aID));
	}
}

//---------------------------------------------------------------------------
void PaymentService::onAmountUpdated(qint64 aPayment, double /*aTotalAmount*/, double aAmount)
{
	std::shared_ptr<PPSDK::IPayment> payment(getPayment(aPayment));

	if (payment)
	{
		// Обновляем информацию о внесённых средствах, платёж должен пересчитать суммы, включая сдачу.
		double amountAll = payment->getParameter(PPSDK::CPayment::Parameters::AmountAll).value.toDouble() +
				payment->getParameter(PPSDK::CPayment::Parameters::Change).value.toDouble() +
				aAmount;

		payment->setParameter(PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::AmountAll, QString::number(amountAll, 'f', 2), true));

		if (!savePayment(payment.get()))
		{
			QString msg = QString("Payment [%1] error write to database; AMOUNT_ALL=%2;").arg(payment->getInitialSession()).arg(amountAll, 0, 'f', 2);

			mApplication->getCore()->getTerminalService()->sendFeedback(CServices::PaymentService, msg);
		}

		// Получаем изменение сдачи.
		double change = payment->getParameter(PPSDK::CPayment::Parameters::Change).value.toDouble();

		// Если сдача не 0, сохраняем.
		if (!qFuzzyIsNull(change))
		{
			setChangeAmount(change, payment);
		}

		emit amountUpdated(aPayment);
	}
}

//---------------------------------------------------------------------------
void PaymentService::onAmountDispensed(double aAmount)
{
	if (qFuzzyIsNull(aAmount))
	{
		toLog(LogLevel::Warning, QString("Dispensed zero summ %1.").arg(aAmount, 0, 'f', 2));

		return;
	}

	if (mChangePayment)
	{
		double amountAll = mChangePayment->getParameter(PPSDK::CPayment::Parameters::AmountAll).value.toDouble();

		if (amountAll >= aAmount)
		{
			setChangeAmount(amountAll - aAmount, mActivePayment);

			toLog(LogLevel::Normal, QString("Dispensed %1. Change %2").arg(aAmount, 0, 'f', 2).arg(amountAll - aAmount, 0, 'f', 2));
		}
		else
		{
			// Выдали денег больше чем есть в сдаче
			toLog(LogLevel::Fatal, QString("Dispensed %1, but change payment contain only %2.").arg(aAmount, 0, 'f', 2).arg(amountAll, 0, 'f', 2));

			setChangeAmount(0, mActivePayment);
		}
	}
	else
	{
		// Ругаемся, т.к. выдали денег, а сдачи реально нет.
		toLog(LogLevel::Error, QString("Dispensed %1, but change payment is NULL.").arg(aAmount, 0, 'f', 2));
	}
}

//---------------------------------------------------------------------------
void PaymentService::onPaymentThreadStarted()
{
	mPaymentTimer.start(CPaymentService::ProcessOfflineTimeout);
}

//---------------------------------------------------------------------------
bool PaymentService::processPaymentInternal(std::shared_ptr<PPSDK::IPayment> aPayment)
{
	// Пропускаем платежи с неизрасходованной сдачей.
	if (mChangePayment && (mChangePayment->getID() == aPayment->getID()))
	{
		mDBUtils->suspendPayment(aPayment->getID(), 15);

		return false;
	}

	// Пропускаем платёж, если он активен.
	if (isPaymentActive(aPayment) && (aPayment->getPriority() == PPSDK::IPayment::Online))
	{
		mDBUtils->suspendPayment(aPayment->getID(), 15);

		return false;
	}

	// Платеж можно удалить только в том случае, если в нем нет денег и купюр/монет
	if (aPayment->isNull() && getPaymentNotes(aPayment->getID()).empty())
	{
		toLog(LogLevel::Normal, QString("Payment %1. Is null.").arg(aPayment->getID()));

		mDBUtils->removePayment(aPayment->getID());

		return false;
	}

	// Запоминаем id платежа, находяшегося в обработке.
	{
		QMutexLocker lock(&mOfflinePaymentLock);
		mOfflinePaymentID = aPayment->getID();
		mOfflinePayment = aPayment;
	}

	// Проверка на неиспользованный остаток
	if ((aPayment->getStatus() == PPSDK::EPaymentStatus::Init) && !aPayment->getParameter(CPaymentService::ChangePaymentParam).isNull())
	{
		aPayment->setStatus(PPSDK::EPaymentStatus::LostChange);
	}

	if (aPayment->canProcessOffline())
	{
		aPayment->setPriority(PPSDK::IPayment::Offline);
		aPayment->process();
	}
	else
	{
		toLog(LogLevel::Normal, QString("Payment %1. Online payment can't process with offline mode.").arg(aPayment->getID()));

		if (aPayment->getStatus() == PPSDK::EPaymentStatus::ProcessError)
		{
			aPayment->setStatus(PPSDK::EPaymentStatus::BadPayment);
		}
		else if (aPayment->getCreationDate().msecsTo(QDateTime::currentDateTime()) > CPaymentService::OnlinePaymentOvertime)
		{
			aPayment->setStatus(PPSDK::EPaymentStatus::BadPayment);
		}
		else
		{
			toLog(LogLevel::Warning, QString("Suspending online payment %1.").arg(aPayment->getID()));

			aPayment->setNextTryDate(QDateTime::currentDateTime().addSecs(5 * 60)); // 5min
		}
	}

	// Отпускаем платеж.
	{
		QMutexLocker lock(&mOfflinePaymentLock);

		mOfflinePaymentID = -1;
		mOfflinePayment.reset();

		// Сохраняем платёж внутри защищенного блока для избежания записи параметров оfflline платежа
		savePayment(aPayment.get());
	}

	/// сообщаем об окончании процесса обработки платежа
	emit stepCompleted(aPayment->getID(), SDK::PaymentProcessor::EPaymentStep::Pay, aPayment->getStatus() != PPSDK::EPaymentStatus::Completed);

	return (aPayment->getStatus() == PPSDK::EPaymentStatus::Completed);
}

//---------------------------------------------------------------------------
void PaymentService::onProcessPayments()
{
	// Выгружаем старые платежи раз в день.
	if (mLastBackupDate.addDays(1) <= QDateTime::currentDateTime())
	{
		mDBUtils->backupOldPayments();

		mLastBackupDate = QDateTime::currentDateTime();
	}

	// Блокируем offline проведение платежей до установления связи
	if (!mApplication->getCore()->getNetworkService()->isConnected(true))
	{
		toLog(LogLevel::Warning, "Waiting network connection for payment processing.");

		mPaymentTimer.start(CPaymentService::CheckNetworkConnectionTimeout);

		return;
	}

	QList<qint64> payments = mDBUtils->getPaymentQueue();

	if (payments.size())
	{
		qint64 id = payments.takeFirst();

		if (mEnabled)
		{
			std::shared_ptr<PPSDK::IPayment> payment(getPayment(id));

			// Если платеж не прогрузился, останавливаем его обработку на 15 минут.
			if (!payment)
			{
				toLog(LogLevel::Warning, QString("Suspending bad payment %1.").arg(id));

				mDBUtils->suspendPayment(id, 15);
			}
			else
			{
				processPaymentInternal(payment);
			}
		}
	}

	QMutexLocker lock(&mCommandMutex);

	// Обрабатываем очередь команд.
	foreach (auto & command, mCommands)
	{
		emit paymentCommandComplete(command.first, command.second(this));
	}

	mCommands.clear();

	mPaymentTimer.start(CPaymentService::ProcessOfflineTimeout);
}

//---------------------------------------------------------------------------
int PaymentService::registerForcePaymentCommand(const QString & aInitialSession, const QVariantMap & aParameters)
{
	QMutexLocker lock(&mCommandMutex);

	auto command = [aInitialSession, aParameters](PaymentService * aService) -> EPaymentCommandResult::Enum
	{
		if (!aService->mEnabled)
		{
			return EPaymentCommandResult::Error;
		}

		auto payment = aService->getPayment(aInitialSession);

		if (!payment)
		{
			return EPaymentCommandResult::NotFound;
		}

		foreach (const QString & parameter, aParameters.keys())
		{
			// берем значения атрибутов crypted и external у предыдущего значения параметра
			auto oldParameter = payment->getParameter(parameter);

			payment->setParameter(PPSDK::IPayment::SParameter(parameter, aParameters.value(parameter), true, oldParameter.crypted, oldParameter.external));
		}

		payment->setParameter(PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::NumberOfTries, 1, true));
		payment->setParameter(PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::NextTryDate, QDateTime::currentDateTime(), true));
		payment->setStatus(PPSDK::EPaymentStatus::ProcessError);

		return aService->processPaymentInternal(payment) ? EPaymentCommandResult::OK : EPaymentCommandResult::Error;
	};

	mCommands << qMakePair(mCommandIndex++, std::function<EPaymentCommandResult::Enum(PaymentService *)>(command));

	auto parametersToString = [aParameters]() -> QString
	{
		QStringList list;

		foreach (const QString & parameter, aParameters.keys())
		{
			list << QString("%1=%2").arg(parameter).arg(aParameters.value(parameter).toString());
		}

		return list.join(";");
	};

	toLog(LogLevel::Normal, QString("Register command 'process payment %1'. Internal command id: %2. Parameters: %3")
		.arg(aInitialSession).arg(mCommandIndex).arg(parametersToString()));

	return mCommands.last().first;
}

//---------------------------------------------------------------------------
int PaymentService::registerRemovePaymentCommand(const QString & aInitialSession)
{
	QMutexLocker lock(&mCommandMutex);

	auto command = [aInitialSession](PaymentService * aService) -> EPaymentCommandResult::Enum
	{
		qint64 id = aService->mDBUtils->getPaymentByInitialSession(aInitialSession);

		if (id < 0)
		{
			return EPaymentCommandResult::NotFound;
		}

		return aService->removePayment(id) ? EPaymentCommandResult::OK : EPaymentCommandResult::Error;
	};

	mCommands << qMakePair(mCommandIndex++, std::function<EPaymentCommandResult::Enum(PaymentService *)>(command));

	toLog(LogLevel::Normal, QString("Register command 'remove payment %1'. Internal command id: %2.").arg(aInitialSession).arg(mCommandIndex));

	return mCommands.last().first;
}

//---------------------------------------------------------------------------
PPSDK::SBalance PaymentService::getBalance()
{
	return mDBUtils->getBalance();
}

//---------------------------------------------------------------------------
PPSDK::EncashmentResult::Enum PaymentService::performEncashment(const QVariantMap & aParameters, PPSDK::SEncashment & aEncashment)
{
	PPSDK::SBalance balance = mDBUtils->getBalance();

	{
		QMutexLocker lock(&mCommandMutex);

		// не можем выполнить инкассацию в случае несохраненных в БД данных
		if (!balance.notPrintedPayments.intersect(mPaymentHaveUnsavedParameters).isEmpty())
		{
			return PPSDK::EncashmentResult::TryLater;
		}
	}

	aEncashment = mDBUtils->performEncashment(aParameters);

	return aEncashment.isValid() ? PPSDK::EncashmentResult::OK : PPSDK::EncashmentResult::Error;
}

//---------------------------------------------------------------------------
PPSDK::SEncashment PaymentService::getLastEncashment()
{
	auto encashments = mDBUtils->getLastEncashments(1);

	return encashments.isEmpty() ? PPSDK::SEncashment() : encashments.at(0);
}

//---------------------------------------------------------------------------
QList<PPSDK::SEncashment> PaymentService::getEncashmentList(int aDepth)
{
	return mDBUtils->getLastEncashments(aDepth);
}

//---------------------------------------------------------------------------
double PaymentService::getChangeAmount()
{
	if (mChangePayment)
	{
		return mChangePayment->getParameter(PPSDK::CPayment::Parameters::AmountAll).value.toDouble();
	}

	return 0.0;
}

//---------------------------------------------------------------------------
void PaymentService::moveChangeToPayment(qint64 aPayment)
{
	if (mChangePayment)
	{
		double change = getChangeAmount();

		if (!qFuzzyIsNull(change))
		{
			std::shared_ptr<PPSDK::IPayment> payment(getPayment(aPayment));
			if (payment)
			{
				toLog(LogLevel::Normal, QString("Payment %1. Using change: %2.").arg(aPayment).arg(change, 0, 'f', 2));

				double amountAll = payment->getParameter(PPSDK::CPayment::Parameters::AmountAll).value.toDouble() + change;

				payment->setParameter(PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::AmountAll, QString::number(amountAll, 'f', 2), true, false));

				mChangePayment->setParameter(PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::AmountAll, payment->getParameter(PPSDK::CPayment::Parameters::Change).value, true));

				// Использовал мошенническую сдачу - сам мошенник
				auto originalPayment = getPayment(getChangeSessionRef());
				auto cheatedParameter = (originalPayment ? originalPayment : mChangePayment)->getParameter(PPSDK::CPayment::Parameters::Cheated);

				if (!cheatedParameter.isNull())
				{
					cheatedParameter.updated = true;
					payment->setParameter(cheatedParameter);
				}

				savePayment(mChangePayment.get());
				savePayment(payment.get());

				double newChange = payment->getParameter(PPSDK::CPayment::Parameters::Change).value.toDouble();
				toLog(LogLevel::Normal, QString("Payment %1. Updating change: %2.").arg(aPayment).arg(newChange, 0, 'f', 2));

				emit amountUpdated(aPayment);
				emit changeUpdated(newChange);
			}
		}
	}
}

//---------------------------------------------------------------------------
void PaymentService::movePaymentToChange(qint64 aPayment)
{
	std::shared_ptr<PPSDK::IPayment> payment(getPayment(aPayment));
	if (payment)
	{
		double change = getChangeAmount() + payment->getParameter(PPSDK::CPayment::Parameters::AmountAll).value.toDouble();

		if (!qFuzzyIsNull(change))
		{
			if (setChangeAmount(change, payment))
			{
				toLog(LogLevel::Normal, QString("Payment %1. Move amount to change: %2.").arg(aPayment).arg(change, 0, 'f', 2));

				payment->setParameter(PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::AmountAll, QString::number(0.00, 'f', 2), true, false));
				
				savePayment(payment.get());
			}
		}
	}
}

//---------------------------------------------------------------------------
void PaymentService::resetChange()
{
	// Если есть активный платёж со сдачей, отправляем его на мониторинг и создаём новый.
	if (mChangePayment)
	{
		bool remove = qFuzzyIsNull(getChangeAmount());
		qint64 changePaymentID = mChangePayment->getID();

		auto originalPayment = getPayment(getChangeSessionRef());
		auto cheatedParameter = (originalPayment ? originalPayment : mChangePayment)->getParameter(PPSDK::CPayment::Parameters::Cheated);

		toLog(LogLevel::Normal, QString("Payment %1. Reset %3 change amount: %2.")
			.arg(changePaymentID).arg(getChangeAmount(), 0, 'f', 2)
			.arg(cheatedParameter.isNull() ? "" : "CHEATED"));

		// Если есть платёж "сдача" с нулевой суммой, то не оставляем его в БД "как есть", а помечаем как удалённый.
		mChangePayment->setStatus(remove ? PPSDK::EPaymentStatus::Deleted : PPSDK::EPaymentStatus::LostChange);

		savePayment(mChangePayment.get());

		mChangePayment.reset();

		if (remove)
		{
			mDBUtils->removePayment(changePaymentID);
		}
	}

	emit changeUpdated(0.0);
}

//---------------------------------------------------------------------------
QString PaymentService::getChangeSessionRef()
{
	if (mChangePayment)
	{
		return mChangePayment->getParameter(PPSDK::CPayment::Parameters::OriginalPayment).value.toString().split(PPSDK::CPayment::Parameters::ProviderFieldsDelimiter).takeFirst();
	}

	return QString();
}

//---------------------------------------------------------------------------
bool PaymentService::setChangeAmount(double aChange, std::shared_ptr<PPSDK::IPayment> aPaymentSource)
{
	if (!mChangePayment)
	{
		if (mFactoryByType.contains(CPaymentService::ChangePaymentType))
		{
			mChangePayment = std::shared_ptr<PPSDK::IPayment>(mFactoryByType[CPaymentService::ChangePaymentType]->createPayment(CPaymentService::ChangePaymentType),
				std::bind(&PPSDK::IPaymentFactory::releasePayment, mFactoryByType[CPaymentService::ChangePaymentType], _1));

			if (mChangePayment)
			{
				QList<PPSDK::IPayment::SParameter> parameters;

				parameters
					<< PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::ID, mDBUtils->createDummyPayment(), true)
					<< PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::CreationDate, QDateTime::currentDateTime(), true)
					<< PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::Type, CPaymentService::ChangePaymentType, true)
					<< PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::Provider, -1, true)
					<< PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::Status, PPSDK::EPaymentStatus::Init, true)
					<< PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::Priority, PPSDK::IPayment::Offline, true)
					<< PPSDK::IPayment::SParameter(CPaymentService::ChangePaymentParam, 1, true);

				if (!mChangePayment->restore(parameters))
				{
					toLog(LogLevel::Error, "Failed to register storage for the change.");

					mChangePayment.reset();

					return false;
				}
			}
			else
			{
				toLog(LogLevel::Error, "Failed to create storage for the change.");

				return false;
			}
		}
		else
		{
			toLog(LogLevel::Error, "Failed to create storage for the change. Required plugin is missing.");

			return false;
		}
	}

	// В сдачу засовываем все данные из родительского платежа
	if (aPaymentSource)
	{
		QMutexLocker lock(&mPaymentLock);

		QStringList paramValues(aPaymentSource->getParameter(PPSDK::CPayment::Parameters::InitialSession).value.toString());

		foreach (auto param, aPaymentSource->getParameter(PPSDK::CPayment::Parameters::ProviderFields).value.toString()
			.split(PPSDK::CPayment::Parameters::ProviderFieldsDelimiter, QString::SkipEmptyParts))
		{
			paramValues << aPaymentSource->getParameter(param).value.toString();
		}

		// Если платеж с подозрением на мошенничество, сдача тоже мошенническая
		auto cheatedParameter = aPaymentSource->getParameter(PPSDK::CPayment::Parameters::Cheated);
		if (!cheatedParameter.isNull())
		{
			cheatedParameter.updated = true;
			mChangePayment->setParameter(cheatedParameter);
		}

		mChangePayment->setParameter(PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::OriginalPayment,
			paramValues.join(PPSDK::CPayment::Parameters::ProviderFieldsDelimiter), true));

		mChangePayment->setParameter(PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::ProviderFields, 
			PPSDK::CPayment::Parameters::OriginalPayment, true));
	}

	mChangePayment->setParameter(PPSDK::IPayment::SParameter(PPSDK::CPayment::Parameters::AmountAll, QString::number(aChange, 'f', 2), true));
	
	toLog(LogLevel::Normal, QString("Payment %1. Change: %2.").arg(getActivePayment()).arg(aChange, 0, 'f', 2));

	emit changeUpdated(aChange);

	return savePayment(mChangePayment.get());
}

//---------------------------------------------------------------------------
QList<PPSDK::SNote> PaymentService::getPaymentNotes(qint64 aID) const
{
	return mDBUtils->getPaymentNotes(aID);
}

//---------------------------------------------------------------------------
QList<qint64> PaymentService::getPayments(const QSet<PPSDK::EPaymentStatus::Enum> & aStates)
{
	return mDBUtils->getPayments(aStates);
}

//---------------------------------------------------------------------------
QList<qint64> PaymentService::findPayments(const QDate & aDate, const QString & aPhoneNumber)
{
	return mDBUtils->findPayments(aDate, aPhoneNumber);
}

//------------------------------------------------------------------------------
QMap<qint64, quint32> PaymentService::getStatistic() const
{
	return mDBUtils->getStatistic();
}

//------------------------------------------------------------------------------
