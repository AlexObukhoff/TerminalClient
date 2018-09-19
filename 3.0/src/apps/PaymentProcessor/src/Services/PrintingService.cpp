/* @file Сервис печати и формирования чеков. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegExp>
#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QFuture>
#include <QtCore/QMutexLocker>
#include <QtCore/QDir>
#include <QtCore/QDate>
#include <QtCore/QBuffer>
#include <QtQml/QJSEngine>
#include <QtQml/QJSValue>
#include <QtCore/QTextCodec>
#include <QtXML/QDomDocument>
#include <QtCore/QXmlStreamWriter>
#include <Common/QtHeadersEnd.h>

// Thirdparty
#include <Common/QtHeadersBegin.h>
#include <qzint.h>
#include <Common/QtHeadersEnd.h>

// PaymentProcessor SDK
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/IDeviceService.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Core/ReceiptTypes.h>
#include <SDK/PaymentProcessor/Core/ServiceParameters.h>
#include <SDK/PaymentProcessor/Settings/DealerSettings.h>
#include <SDK/PaymentProcessor/Settings/ExtensionsSettings.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>
#include <SDK/PaymentProcessor/Payment/Security.h>

// Driver SDK
#include <SDK/Drivers/IFiscalPrinter.h>
#include <SDK/Drivers/FR/FiscalPrinterConstants.h>
#include <SDK/Drivers/Components.h>
#include <SDK/Drivers/HardwareConstants.h>

// Project
#include "System/IApplication.h"

#include "Services/ServiceNames.h"
#include "Services/PrintConstants.h"
#include "Services/PrintingService.h"
#include "Services/SettingsService.h"
#include "Services/DatabaseService.h"
#include "Services/EventService.h"
#include "Services/PluginService.h"
#include "Services/PaymentService.h"

#include "PrintingCommands.h"
#include "FRReportConstants.h"

namespace PPSDK = SDK::PaymentProcessor;

namespace CPrintingService
{
	const QString ReceiptDateTimeFormat = "dd.MM.yyyy hh:mm:ss";
	const QString ConditionTag = "@@";

	const QString MaskedFieldPostfix = "_MASKED";
	const QString DislayPostfix = "_DISPLAY";
}

//---------------------------------------------------------------------------
PrintingService::PrintingService(IApplication * aApplication) :
	mApplication(aApplication),
	mDatabaseUtils(nullptr),
	mDeviceService(nullptr),
	mContinuousMode(false),
	mServiceOperation(false),
	mRandomReceiptsID(false),
	mNextReceiptIndex(1),
	mRandomGenerator(static_cast<unsigned>(QDateTime::currentDateTime().currentMSecsSinceEpoch())),
	mEnableBlankFiscalData(false),
	mFiscalRegister(nullptr)
{
	setLog(mApplication->getLog());
}

//---------------------------------------------------------------------------
PrintingService::~PrintingService()
{
}

//---------------------------------------------------------------------------
QString PrintingService::getName() const
{
	return CServices::PrintingService;
}

//---------------------------------------------------------------------------
const QSet<QString> & PrintingService::getRequiredServices() const
{
	static QSet<QString> requiredServices = QSet<QString>()
		<< CServices::DeviceService
		<< CServices::DatabaseService
		<< CServices::SettingsService
		<< CServices::PluginService
		<< CServices::PaymentService;

	return requiredServices;
}

//---------------------------------------------------------------------------
QVariantMap PrintingService::getParameters() const
{
	QVariantMap parameters;

	parameters[PPSDK::CServiceParameters::Printing::ReceiptCount] = getReceiptCount();

	return parameters;
}

//---------------------------------------------------------------------------
void PrintingService::resetParameters(const QSet<QString> & aParameters)
{
	if (aParameters.contains(PPSDK::CServiceParameters::Printing::ReceiptCount))
	{
		mDatabaseUtils->setDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal, PPSDK::CDatabaseConstants::Parameters::ReceiptCount, 0);
	}
}

//---------------------------------------------------------------------------
bool PrintingService::initialize()
{
	// Запрашиваем доступные устройства.
	mDeviceService = mApplication->getCore()->getDeviceService();

	loadReceiptTemplates();
	loadTags();

	updateHardwareConfiguration();
	createFiscalRegister();

	mDatabaseUtils = DatabaseService::instance(mApplication)->getDatabaseUtils<IHardwareDatabaseUtils>();

	connect(mDeviceService, SIGNAL(configurationUpdated()), SLOT(updateHardwareConfiguration()));
	connect(&mFutureWatcher, SIGNAL(finished()), this, SLOT(taskFinished()));

	return true;
}

//------------------------------------------------------------------------------
void PrintingService::finishInitialize()
{
}

//---------------------------------------------------------------------------
bool PrintingService::canShutdown()
{
	return true;
}

//---------------------------------------------------------------------------
bool PrintingService::shutdown()
{
	mFutureWatcher.waitForFinished();

	foreach (DSDK::IPrinter * printer, mPrinterDevices)
	{
		mDeviceService->releaseDevice(printer);
	}

	if (mFiscalRegister)
	{
		SDK::Plugin::IPluginLoader * pluginLoader = PluginService::instance(mApplication)->getPluginLoader();
		
		pluginLoader->destroyPlugin(dynamic_cast<SDK::Plugin::IPlugin *>(mFiscalRegister));
	}

	return true;
}

//---------------------------------------------------------------------------
PrintCommand * PrintingService::getPrintCommand(const QString & aReceiptType)
{
	if (aReceiptType == PPSDK::CReceiptType::Payment)
	{
		return new PrintPayment(aReceiptType, this);
	}
	else if (aReceiptType == PPSDK::CReceiptType::Balance || aReceiptType == PPSDK::CReceiptType::XReport)
	{
		return new PrintBalance(aReceiptType, this);
	}
	else if (aReceiptType == PPSDK::CReceiptType::DispenserBalance || aReceiptType == PPSDK::CReceiptType::DispenserEncashment)
	{
		auto command = new PrintBalance(aReceiptType, this);
		command->setFiscal(false);
		return command;
	}
	else if (aReceiptType == PPSDK::CReceiptType::Encashment)
	{
		return new PrintEncashment(aReceiptType, this);
	}
	else if (aReceiptType == PPSDK::CReceiptType::ZReport)
	{
		return new PrintZReport(aReceiptType, this, false);
	}
	else if (aReceiptType == PPSDK::CReceiptType::ZReportFull)
	{
		return new PrintZReport(aReceiptType, this, true);
	}
	else
	{
		return new PrintReceipt(aReceiptType, this);
	}
}

//---------------------------------------------------------------------------
bool PrintingService::canPrintReceipt(const QString & aReceiptType, bool aRealCheck)
{
	DSDK::IPrinter * printer = takePrinter(aReceiptType, aRealCheck);
	giveBackPrinter(printer);

	return printer != nullptr;
}

//---------------------------------------------------------------------------
int PrintingService::printReceipt(const QString & aReceiptType, const QVariantMap & aParameters, const QString & aReceiptTemplate, bool aContinuousMode, bool aServiceOperation)
{
	mContinuousMode = aContinuousMode;
	mServiceOperation = aServiceOperation;

	QStringList receiptTemplates;
	receiptTemplates
		// Извлекаем шаблон 'PROCESSING_TYPE'_aReceiptTemplate
		<< (QString("%1_%2").arg(aParameters[PPSDK::CPayment::Parameters::Type].toString()).arg(aReceiptTemplate)).toLower()
		// Извлекаем обычный шаблон для чека нужного типа.
		<< aReceiptTemplate.toLower()
		// Резервный шаблон
		<< SDK::PaymentProcessor::CReceiptType::Payment;

	foreach (auto templateName, receiptTemplates)
	{
		if (templateName == SDK::PaymentProcessor::CReceiptType::Disabled)
		{
			break;
		}

		if (mCachedReceipts.contains(templateName))
		{
			auto printCommand = getPrintCommand(aReceiptType);
			printCommand->setReceiptTemplate(templateName);

			return performPrint(printCommand, aParameters, mCachedReceipts[templateName]);
		}
	}

	toLog(LogLevel::Error, QString("Failed to print receipt. Missing receipt template : %1.").arg(aReceiptTemplate));

	QMetaObject::invokeMethod(this, "printEmptyReceipt", Qt::QueuedConnection, Q_ARG(int, 0), Q_ARG(bool, true));

	return 0;
}

//---------------------------------------------------------------------------
void PrintingService::printEmptyReceipt(int aJobIndex, bool aError)
{
	emit receiptPrinted(aJobIndex, aError);
}

//---------------------------------------------------------------------------
bool PrintingService::printReceiptDirected(DSDK::IPrinter * aPrinter, const QString & aReceiptTemplate, const QVariantMap & aParameters)
{
	mContinuousMode = false;

	// Извлекаем шаблон для чека нужного типа.
	if (!mCachedReceipts.contains(aReceiptTemplate.toLower()))
	{
		toLog(LogLevel::Error, QString("Missing receipt template : %1.").arg(aReceiptTemplate));
		return false;
	}

	QScopedPointer<PrintCommand> printCommand(getPrintCommand(QString()));
	printCommand->setReceiptTemplate(aReceiptTemplate);

	QVariantMap configuration;
	configuration.insert(CHardwareSDK::Printer::ContinuousMode, mContinuousMode);
	configuration.insert(CHardwareSDK::Printer::ServiceOperation, mServiceOperation);
	aPrinter->setDeviceConfiguration(configuration);

	// TODO: нужно ли увеличивать счетчик чеков?
	return printCommand->print(aPrinter, aParameters);
}

//---------------------------------------------------------------------------
template <class ResultT, class T>
ResultT & joinMap(ResultT & aResult, const T & aParameters2)
{
	for (auto it = aParameters2.begin(); it != aParameters2.end(); ++it)
	{
		aResult.insert(it.key(), it.value());
	}

	return aResult;
}

//---------------------------------------------------------------------------
int PrintingService::performPrint(PrintCommand * aCommand, const QVariantMap & aParameters, QStringList aReceiptTemplate)
{
	// Функция, в которой прозводится печать. Должно быть исключено обращение к общим для разных принтеров данным.
	mPrintingFunction =
		[this, aReceiptTemplate](int aJobIndex, PrintCommand * aCommand, QVariantMap aParameters) -> bool
		{
			auto printer = takePrinter(aCommand->getReceiptType(), false);

			if (!printer)
			{
				delete aCommand;

				return false;
			}

			QVariantMap staticParameters;

			QVariantMap configuration;
			configuration.insert(CHardwareSDK::Printer::ContinuousMode, mContinuousMode);
			configuration.insert(CHardwareSDK::Printer::ServiceOperation, mServiceOperation);
			configuration.insert(CHardwareSDK::Printer::TemplateParameters, aParameters);
			configuration.insert(CHardwareSDK::Printer::ReceiptParameters, joinMap(staticParameters, mStaticParameters));
			configuration.insert(CHardwareSDK::Printer::ReceiptTemplate, aReceiptTemplate);
			printer->setDeviceConfiguration(configuration);

			bool result = aCommand->print(printer, joinMap(aParameters, staticParameters));

			if (result)
			{
				incrementReceiptCount(printer);
			}

			giveBackPrinter(printer);

			delete aCommand;

			emit receiptPrinted(aJobIndex, !result);

			return result;
		};

	int taskIndex = mNextReceiptIndex.fetchAndAddOrdered(1);
	
	if (taskIndex != 1 && !mFutureWatcher.isFinished())
	{
		Task task = {taskIndex, aCommand, aParameters};

		mQueue.enqueue(task);
	}
	else
	{
		mFutureWatcher.setFuture(QtConcurrent::run(mPrintingFunction, taskIndex, aCommand, aParameters));
	}

	return taskIndex;
}

//---------------------------------------------------------------------------
void PrintingService::taskFinished()
{
	if (!mQueue.isEmpty())
	{
		Task task = mQueue.dequeue();

		mFutureWatcher.setFuture(QtConcurrent::run(mPrintingFunction, task.index, task.command, task.parameters));
	}
}

//---------------------------------------------------------------------------
int PrintingService::printReport(const QString & aReceiptType, const QVariantMap & aParameters)
{
	auto printCommand = getPrintCommand(aReceiptType);

	PrintBalance * balanceCommand = dynamic_cast<PrintBalance *>(printCommand);
	if (balanceCommand && aParameters.contains(CPrintConstants::NoFiscal))
	{
		// Включаем нефискальный режим
		balanceCommand->setFiscal(false);
	}

	printCommand->setReceiptTemplate(aReceiptType);

	QVariantMap parameters;

	return performPrint(printCommand, joinMap(joinMap(parameters, mStaticParameters), aParameters));
}

//---------------------------------------------------------------------------
void PrintingService::giveBackPrinter(DSDK::IPrinter * aPrinter)
{
	if (aPrinter)
	{
		QMutexLocker lock(&mAvailablePrintersMutex);

		mAvailablePrinters.insert(aPrinter);

		mPrintersAvailable.wakeOne();
	}
}

//---------------------------------------------------------------------------
DSDK::IPrinter * PrintingService::takePrinter(const QString & aReceiptType, bool aCheckOnline)
{
	if (mPrinterDevices.empty())
	{
		toLog(LogLevel::Error, "Printers are not found in current configuration.");
		return 0;
	}

	// Пытаемся найти предпочтительный принтер.
	auto settings = SettingsService::instance(mApplication)->getAdapter<SDK::PaymentProcessor::TerminalSettings>();
	QString preferredName = settings->getPrinterForReceipt(aReceiptType);

	QList<DSDK::IPrinter *> printers;
	auto printCommand = getPrintCommand(aReceiptType);
	DSDK::IPrinter * preferred = nullptr;

	if (!preferredName.isEmpty())
	{
		preferred = dynamic_cast<DSDK::IPrinter *>(mDeviceService->acquireDevice(preferredName));

		if (preferred && mPrinterDevices.contains(preferred) && printCommand->canPrint(preferred, aCheckOnline))
		{
			printers << preferred;
		}
	}

	foreach(auto printer, mPrinterDevices)
	{
		if (printer && (printer != preferred) && printCommand->canPrint(printer, aCheckOnline))
		{
			printers << printer;
		}
	}

	delete printCommand;

	if (printers.isEmpty())
	{
		toLog(LogLevel::Error, QString("No any available printer for type %1 with %2 checking.").arg(aReceiptType).arg(aCheckOnline ? "online" : "offline"));
		return nullptr;
	}

	DSDK::IPrinter * printer = printers.first();

	// Ждем, пока принтер не появится в списке доступных.
	QMutexLocker lock(&mAvailablePrintersMutex);

	if (!mAvailablePrinters.contains(printer))
	{
		mPrintersAvailable.wait(&mAvailablePrintersMutex);
	}

	mAvailablePrinters.remove(printer);

	return printer;
}

//---------------------------------------------------------------------------
void PrintingService::incrementReceiptCount(DSDK::IPrinter * aPrinter) const
{
	QString deviceConfigName = mDeviceService->getDeviceConfigName(aPrinter);

	// Увеличиваем количество напечатанных чеков для конкртетного принтера.
	QVariant receiptCount = mDatabaseUtils->getDeviceParam(deviceConfigName, PPSDK::CDatabaseConstants::Parameters::ReceiptCount);
	receiptCount = QVariant::fromValue(receiptCount.toInt() + 1);
	mDatabaseUtils->setDeviceParam(deviceConfigName, PPSDK::CDatabaseConstants::Parameters::ReceiptCount, receiptCount);

	// Увеличиваем количество напечатанных чеков для терминала.
	receiptCount = mDatabaseUtils->getDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal, PPSDK::CDatabaseConstants::Parameters::ReceiptCount);
	receiptCount = QVariant::fromValue(receiptCount.toInt() + 1);
	mDatabaseUtils->setDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal, PPSDK::CDatabaseConstants::Parameters::ReceiptCount, receiptCount);
}

//---------------------------------------------------------------------------
unsigned PrintingService::getReceiptID() const
{
	if (mRandomReceiptsID)
	{
		return mRandomGenerator();
	}
	else
	{
		return getReceiptCount() + 1;
	}
}

//---------------------------------------------------------------------------
int PrintingService::getReceiptCount() const
{
	QVariant receiptCount = mDatabaseUtils->getDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal, PPSDK::CDatabaseConstants::Parameters::ReceiptCount);

	return receiptCount.isNull() ? 0 : receiptCount.toInt();
}

//---------------------------------------------------------------------------
bool PrintingService::loadTags()
{
	mStaticParameters.clear();

	SettingsService * settingsService = SettingsService::instance(mApplication);

	PPSDK::TerminalSettings * terminalSettings = settingsService->getAdapter<PPSDK::TerminalSettings>();

	PPSDK::SKeySettings key0 = terminalSettings->getKeys().value(0);

	if (key0.ap.isEmpty())
	{
		toLog(LogLevel::Error, "Failed to retrieve terminal number from configs.");
		return false;
	}
	else
	{
		mStaticParameters.insert(CPrintConstants::TermNumber, key0.ap);
	}

	PPSDK::DealerSettings * dealerSettings = settingsService->getAdapter<PPSDK::DealerSettings>();

	// TODO: проверить на валидность.
	PPSDK::SPersonalSettings dealer = dealerSettings->getPersonalSettings();

	joinMap(mStaticParameters, dealer.mPrintingParameters);

	mStaticParameters.insert(CPrintConstants::DealerName, dealer.name);
	mStaticParameters.insert(CPrintConstants::DealerPhone, dealer.phone);
	mStaticParameters.insert(CPrintConstants::DealerSupportPhone, dealer.phone);
	mStaticParameters.insert(CPrintConstants::DealerAddress, dealer.address);
	mStaticParameters.insert(CPrintConstants::DealerBusinessAddress, dealer.businessAddress.isEmpty() ? dealer.address : dealer.businessAddress);
	mStaticParameters.insert(CPrintConstants::DealerInn, dealer.inn);
	mStaticParameters.insert(CPrintConstants::DealerKbk, dealer.kbk);
	mStaticParameters.insert(CPrintConstants::DealerIsBank, dealer.isBank);
	mStaticParameters.insert(CPrintConstants::PointAddress, dealer.pointAddress);
	mStaticParameters.insert(CPrintConstants::PointName, dealer.pointName);
	mStaticParameters.insert(CPrintConstants::PointExternalID, dealer.pointExternalID);
	mStaticParameters.insert(CPrintConstants::BankName, dealer.bankName);
	mStaticParameters.insert(CPrintConstants::BankBik, dealer.bankBik);
	mStaticParameters.insert(CPrintConstants::BankPhone, dealer.bankPhone);
	mStaticParameters.insert(CPrintConstants::BankAddress, dealer.bankAddress);
	mStaticParameters.insert(CPrintConstants::BankInn, dealer.bankInn);

	QString currency = terminalSettings->getCurrencySettings().code;

	if (!currency.isEmpty())
	{
		mStaticParameters.insert(CPrintConstants::Currency, terminalSettings->getCurrencySettings().name);
	}
	else
	{
		toLog(LogLevel::Warning, "Failed to retrieve currency settings from configs. Tag <CURRENCY> will be printed empty on all receipts!");
		return false;
	}

	return true;
}

//---------------------------------------------------------------------------
QStringList PrintingService::getReceipt(const QString & aReceiptTemplate, const QVariantMap & aParameters)
{
	if (!mCachedReceipts.contains(aReceiptTemplate.toLower()))
	{
		toLog(LogLevel::Error, QString("Missing receipt template %1.").arg(aReceiptTemplate));
	}

	QStringList receipt = mCachedReceipts.value(aReceiptTemplate.toLower());
	expandTags(receipt, aParameters);

	return receipt;
}

//---------------------------------------------------------------------------
QString maskedString(const QString & aString, bool aNeedMask)
{
	if (aNeedMask)
	{
		int oneFourthLen = qMin(aString.size() / 4, 4);

		return aString.left(oneFourthLen) + QString("*").repeated(aString.size() - oneFourthLen * 2) + aString.right(oneFourthLen);
	}

	return  aString;
}

//---------------------------------------------------------------------------
QString PrintingService::convertImage2base64(const QString & aString)
{
	QString result = aString;

	for (int extLen = 3; extLen <= 4; extLen++)
	{
		// \[img\s*\].*((?:[\w]\:|\\)?((\\|/)?[a-z_\-\s0-9\.]+)+\.[a-z]{3,4})
		QRegExp imgPattern = QRegExp(QString("\\[img\\s*\\].*((?:[\\w]\\:|\\\\)?((\\\\|/)?[a-z_\\-\\s0-9\\.]+)+\\.[a-z]{%1})").arg(extLen), Qt::CaseInsensitive);

		imgPattern.setMinimal(true);

		int offset = 0;
		while ((offset = imgPattern.indexIn(result, offset)) != -1)
		{
			QString img = "<image>";

			QFile file(imgPattern.cap(1));
			if (file.open(QIODevice::ReadOnly))
			{
				img = QString::fromLatin1(file.readAll().toBase64());
			}
			else
			{
				toLog(LogLevel::Error, QString("Error load image '%1': %2").arg(imgPattern.cap(1)).arg(file.errorString()));
			}

			result.replace(imgPattern.pos(1), imgPattern.cap(1).length(), img);
			offset = imgPattern.pos(1) + img.size();
		}
	}

	return result;
}

//---------------------------------------------------------------------------
QString PrintingService::generateQR(const QString & aString)
{
	QString result = aString;

	auto generateQRCode = [=](const QString & aText, int aSize, int aLeftMargin) -> QString 
	{
		QImage image(QSize(aSize + aLeftMargin, aSize), QImage::Format_ARGB32);
		image.fill(QColor("transparent"));

		Zint::QZint zint;

		zint.setWidth(0);
		zint.setHeight(aSize);
		zint.setText(aText);
		zint.setWhitespace(0);
		zint.setBorderType(Zint::QZint::NO_BORDER);
		zint.setInputMode(DATA_MODE);
		zint.setHideText(true);
		zint.setSymbol(BARCODE_QRCODE);
		zint.setFgColor(QColor("black"));
		zint.setBgColor(QColor("white"));

		{
			QPainter painter(&image);
			painter.fillRect(QRectF(0, 0, image.width(), image.height()), QColor("white"));
			zint.render(painter, QRectF(aLeftMargin, 0, image.width() - aLeftMargin, image.height()), Zint::QZint::KeepAspectRatio);
			painter.end();
		}

		if (zint.hasErrors())
		{
			toLog(LogLevel::Error, QString("Failed render QR code: %1.").arg(zint.lastError()));
		}
		else
		{
			QBuffer buffer;
			if (buffer.open(QIODevice::WriteOnly))
			{
				image.save(&buffer, "png");

				return buffer.data().toBase64();
			}
		}

		return "";
	};

	QRegExp qrPattern("\\[qr(\\s*(\\w+)\\s*=\\s*(\\d+)\\s*)?(\\s*(\\w+)\\s*=\\s*(\\d+)\\s*)?\\](.*)\\[/qr\\]", Qt::CaseInsensitive);

	qrPattern.setMinimal(true);

	int offset = 0;
	while ((offset = qrPattern.indexIn(result, offset)) != -1)
	{
		int size = 200;
		int left_margin = 0;

		for (int i = 2; i < 6; i += 3)
		{
			if (qrPattern.cap(i).toLower() == "size")
			{
				size = qrPattern.cap(i + 1).toInt() ? qrPattern.cap(i + 1).toInt() : size;
			}
			else if (qrPattern.cap(i).toLower() == "left_margin")
			{
				left_margin = qrPattern.cap(i + 1).toInt() ? qrPattern.cap(i + 1).toInt() : left_margin;
			}
		}

		QString content = qrPattern.cap(7);
		QString qrImage = generateQRCode(content, size, left_margin);

		QString img = qrImage.isEmpty() ? "<qr-code>" : QString("[img]%1[/img]").arg(qrImage);

		result.replace(offset, qrPattern.cap(0).length(), img);
		offset += img.size();
	}

	return result;
}

//---------------------------------------------------------------------------
QString PrintingService::generatePDF417(const QString & aString)
{
	QString result = aString;

	auto generatePDFCode = [=](const QString & aText, int aSize, int aLeftMargin) -> QString 
	{
		int divider = aText.size() > 100 ? 2 : 3;

		QImage image(QSize(aSize + aLeftMargin, aSize / divider), QImage::Format_ARGB32);
		image.fill(QColor("transparent"));

		Zint::QZint zint;

		zint.setWidth(aSize);
		zint.setHeight(aSize / divider);
		zint.setText(aText);
		zint.setWhitespace(0);
		zint.setBorderType(Zint::QZint::NO_BORDER);
		zint.setInputMode(UNICODE_MODE);
		zint.setHideText(true);
		zint.setSymbol(BARCODE_PDF417);
		zint.setFgColor(QColor("black"));
		zint.setBgColor(QColor("white"));

		QPainter painter(&image);
		painter.fillRect(QRectF(0, 0, image.width(), image.height()), QColor("white"));
		zint.render(painter, QRectF(aLeftMargin, 0, image.width() - aLeftMargin, image.height()));
		painter.end();

		if (zint.hasErrors())
		{
			toLog(LogLevel::Error, QString("Failed render PDF-417 code: %1.").arg(zint.lastError()));
		}
		else
		{
			QBuffer buffer;
			if (buffer.open(QIODevice::WriteOnly))
			{
				image.save(&buffer, "png");

				return buffer.data().toBase64();
			}
		}

		return "";
	};

	QRegExp qrPattern("\\[pdf417(\\s*(\\w+)\\s*=\\s*(\\d+)\\s*)?(\\s*(\\w+)\\s*=\\s*(\\d+)\\s*)?\\](.*)\\[/pdf417\\]", Qt::CaseInsensitive);

	qrPattern.setMinimal(true);

	int offset = 0;
	while ((offset = qrPattern.indexIn(result, offset)) != -1)
	{
		int size = 200;
		int left_margin = 0;

		for (int i = 2; i < 6; i += 3)
		{
			if (qrPattern.cap(i).toLower() == "size")
			{
				size = qrPattern.cap(i + 1).toInt() ? qrPattern.cap(i + 1).toInt() : size;
			}
			else if (qrPattern.cap(i).toLower() == "left_margin")
			{
				left_margin = qrPattern.cap(i + 1).toInt() ? qrPattern.cap(i + 1).toInt() : left_margin;
			}
		}

		QString content = qrPattern.cap(7);
		QString qrImage = generatePDFCode(content, size, left_margin);

		QString img = qrImage.isEmpty() ? "<pdf417-code>" : QString("[img]%1[/img]").arg(qrImage);

		result.replace(offset, qrPattern.cap(0).length(), img);
		offset += img.size();
	}

	return result;
}

//---------------------------------------------------------------------------
QString PrintingService::generate1D(const QString & aString)
{
	QString result = aString;

	auto generatePDFCode = [=](const QString & aText, int aSize, int aLeftMargin) -> QString
	{
		int divider = aText.size() > 100 ? 2 : 3;

		QImage image(QSize(aSize + aLeftMargin, aSize / divider), QImage::Format_ARGB32);
		image.fill(QColor("transparent"));

		Zint::QZint zint;

		zint.setWidth(aSize);
		zint.setHeight(aSize / divider);
		zint.setText(aText);
		zint.setWhitespace(0);
		zint.setBorderType(Zint::QZint::NO_BORDER);
		zint.setInputMode(UNICODE_MODE);
		zint.setHideText(true);
		zint.setSymbol(BARCODE_CODE128);
		zint.setFgColor(QColor("black"));
		zint.setBgColor(QColor("white"));

		QPainter painter(&image);
		painter.fillRect(QRectF(0, 0, image.width(), image.height()), QColor("white"));
		zint.render(painter, QRectF(aLeftMargin, 0, image.width() - aLeftMargin, image.height()));
		painter.end();

		if (zint.hasErrors())
		{
			toLog(LogLevel::Error, QString("Failed render PDF-417 code: %1.").arg(zint.lastError()));
		}
		else
		{
			QBuffer buffer;
			if (buffer.open(QIODevice::WriteOnly))
			{
				image.save(&buffer, "png");

				return buffer.data().toBase64();
			}
		}

		return "";
	};

	QRegExp qrPattern("\\[1d(\\s*(\\w+)\\s*=\\s*(\\d+)\\s*)?(\\s*(\\w+)\\s*=\\s*(\\d+)\\s*)?\\](.*)\\[/1d\\]", Qt::CaseInsensitive);

	qrPattern.setMinimal(true);

	int offset = 0;
	while ((offset = qrPattern.indexIn(result, offset)) != -1)
	{
		int size = 200;
		int left_margin = 0;

		for (int i = 2; i < 6; i += 3)
		{
			if (qrPattern.cap(i).toLower() == "size")
			{
				size = qrPattern.cap(i + 1).toInt() ? qrPattern.cap(i + 1).toInt() : size;
			}
			else if (qrPattern.cap(i).toLower() == "left_margin")
			{
				left_margin = qrPattern.cap(i + 1).toInt() ? qrPattern.cap(i + 1).toInt() : left_margin;
			}
		}

		QString content = qrPattern.cap(7);
		QString qrImage = generatePDFCode(content, size, left_margin);

		QString img = qrImage.isEmpty() ? "<pdf417-code>" : QString("[img]%1[/img]").arg(qrImage);

		result.replace(offset, qrPattern.cap(0).length(), img);
		offset += img.size();
	}

	return result;
}

//---------------------------------------------------------------------------
QString PrintingService::generateLine(const QString & aString)
{
	QString result = aString;

	auto generateLine = [](int aSize, int aHeight, int aDense) -> QString
	{
		QImage image(QSize(aSize, aHeight), QImage::Format_ARGB32);
		image.fill(QColor("transparent"));

		Qt::BrushStyle style;

		if (aDense <= 0) style = Qt::SolidPattern;
		else if (aDense >= 6) style = Qt::Dense7Pattern;
		else style = static_cast<Qt::BrushStyle>(aDense + 1);

		QPainter painter(&image);
		painter.setBrush(style);
		painter.setPen(QColor("transparent"));
		painter.drawRect(0, 0, image.width(), image.height());
		painter.end();

		QBuffer buffer;
		if (buffer.open(QIODevice::WriteOnly))
		{
			image.save(&buffer, "png");
			return buffer.data().toBase64();
		}

		return QString();
	};

	QRegExp qrPattern("\\[hr(\\s*(\\w+)\\s*=\\s*(\\d+)\\s*)?(\\s*(\\w+)\\s*=\\s*(\\d+)\\s*)?\\](.*)\\[/hr\\]", Qt::CaseInsensitive);

	qrPattern.setMinimal(true);

	int offset = 0;
	while ((offset = qrPattern.indexIn(result, offset)) != -1)
	{
		int size = 220;
		int height = 1;
		int dense = 0;

		for (int i = 2; i < 6; i += 3)
		{
			if (qrPattern.cap(i).toLower() == "size")
			{
				size = qrPattern.cap(i + 1).toInt() ? qrPattern.cap(i + 1).toInt() : size;
			}
			else if (qrPattern.cap(i).toLower() == "height")
			{
				height = qrPattern.cap(i + 1).toInt() ? qrPattern.cap(i + 1).toInt() : height;
			}
			else if (qrPattern.cap(i).toLower() == "ds")
			{
				dense = qrPattern.cap(i + 1).toInt() ? qrPattern.cap(i + 1).toInt() : dense;
			}
		}

		QString qrImage = generateLine(size, height, dense);

		QString img = qrImage.isEmpty() ? "<hr-code>" : QString("[img]%1[/img]").arg(qrImage);

		result.replace(offset, qrPattern.cap(0).length(), img);
		offset += img.size();
	}

	return result;
}

//---------------------------------------------------------------------------
void PrintingService::expandTags(QStringList & aReceipt, const QVariantMap & aParameters)
{
	int operatorFieldIndex = 0;

	QVariantMap userParameters = aParameters;

	// Если есть дата создания платежа и она валидна, то подставляем на чек дату платежа
	userParameters[CPrintConstants::DateTime] = aParameters.contains(PPSDK::CPayment::Parameters::CreationDate) && 
		aParameters.value(PPSDK::CPayment::Parameters::CreationDate, "").toDateTime().isValid() ?
			aParameters.value(PPSDK::CPayment::Parameters::CreationDate, "").toDateTime().toString(CPrintingService::ReceiptDateTimeFormat) :
			QDateTime::currentDateTime().toLocalTime().toString(CPrintingService::ReceiptDateTimeFormat);

	// Добавляем поля, зависящие от конкретного вызова.
	userParameters[CPrintConstants::ReceiptNumber] = getReceiptID();

	int providerId = aParameters.value(PPSDK::CPayment::Parameters::Provider, -1).toInt();
	PPSDK::SProvider provider = SettingsService::instance(mApplication)->getAdapter<PPSDK::DealerSettings>()->getProvider(providerId);
	PPSDK::SProvider mnpProvider = SettingsService::instance(mApplication)->getAdapter<PPSDK::DealerSettings>()->getMNPProvider(providerId,
		aParameters.value(PPSDK::CPayment::Parameters::MNPGetewayIn, 0).toLongLong(),
		aParameters.value(PPSDK::CPayment::Parameters::MNPGetewayOut, 0).toLongLong());

	if (!mnpProvider.isNull())
	{
		userParameters[CPrintConstants::OpBrand] = mnpProvider.name;
	}

	PPSDK::SecurityFilter filter(mnpProvider, PPSDK::SProviderField::SecuritySubsystem::Printer);

	// Набор строк, полученных в результате применения всех подстановок.
	QStringList result;

	// Для каждого тега в шаблоне чека, заменяем его значением из параметров.
	for (auto it = aReceipt.begin(); it != aReceipt.end(); ++it)
	{
		QRegExp tagPattern("%(.*)%", Qt::CaseInsensitive);

		tagPattern.setMinimal(true);

		int offset = 0;

		while ((offset = tagPattern.indexIn(*it, offset)) != -1)
		{
			QString tag = tagPattern.cap(1);

			// %% заменяем на %
			if (tag.isEmpty())
			{
				it->replace(offset, tagPattern.cap(0).length(), "%");
				offset += 1;
				continue;
			}

			bool isMasked = tag.endsWith(CPrintingService::MaskedFieldPostfix);
			if (isMasked)
			{
				tag.remove(CPrintingService::MaskedFieldPostfix);
			}

			// Параметр пользователя?
			auto userParameter = userParameters.find(tag);

			if (userParameter != userParameters.end())
			{
				// Параметр список?
				if (tag.startsWith("[") && tag.endsWith("]"))
				{
					// удаляем строку с параметром-списком
					aReceipt.erase(it, it);
					it++;

					QStringList listItems = userParameter.value().toStringList();
					QStringList::const_iterator listIt;
					for (listIt = listItems.constBegin(); listIt != listItems.constEnd(); listIt++)
					{
						result.append(filter.apply(tag, *listIt));
					}
				}
				else
				{
					QString masked = isMasked ?
						maskedString(userParameter.value().toString(), isMasked) :
						filter.apply(userParameter.key(), userParameter.value().toString());

					it->replace(offset, tagPattern.cap(0).length(), masked);
					offset += masked.length();
				}

				continue;
			}

			// Статический параметр?
			auto staticParameter = mStaticParameters.find(tag);

			if (staticParameter != mStaticParameters.end())
			{
				QString masked = isMasked ?
					maskedString(staticParameter.value(), isMasked) :
					filter.apply(staticParameter.key(), staticParameter.value());

				it->replace(offset, tagPattern.cap(0).length(), masked);
				offset += masked.length();

				continue;
			}

			// Название или значение поля оператора?
			QString targetParameter;

			QRegExp operatorFieldPattern("FIELD_(.+)", Qt::CaseInsensitive);

			operatorFieldPattern.setMinimal(true);

			if (operatorFieldPattern.indexIn(tag) != -1)
			{
				targetParameter = operatorFieldPattern.cap(1);

				if (!filter.haveFilter(targetParameter))
				{
					targetParameter += CPrintingService::DislayPostfix;
				}
			}
			else
			{
				operatorFieldPattern.setPattern("RAWFIELD_(.+)");

				if (operatorFieldPattern.indexIn(tag) != -1)
				{
					targetParameter = operatorFieldPattern.cap(1);
				}
			}

			if (!targetParameter.isEmpty())
			{
				if (!aParameters.contains(targetParameter))
				{
					toLog(LogLevel::Error, QString("Operator parameter %1 required in receipt but missing in parameters list.").arg(targetParameter));
				}
				else
				{
					QString masked = isMasked ?
						maskedString(aParameters[targetParameter].toString(), isMasked) :
						filter.apply(targetParameter, aParameters[targetParameter].toString());

					it->replace(offset, tagPattern.cap(0).length(), masked);
					offset += masked.length();

					continue;
				}
			}

			// Тег OPERATOR_FIELD
			if (tag == "OPERATOR_FIELD")
			{
				if (provider.isNull())
				{
					toLog(LogLevel::Error, QString("Failed to expand operator field. Provider id %1 is not valid.").arg(providerId));
				}

				// Вставляем поля оператора.
				if (operatorFieldIndex < provider.fields.size())
				{
					PPSDK::SProviderField field = provider.fields.at(operatorFieldIndex);

					QString fieldValue = aParameters.value(field.id, QString()).toString();

					bool findNextField = true;

					while (fieldValue.isEmpty())
					{
						if (operatorFieldIndex >= provider.fields.size())
						{
							findNextField = false;
							break;
						}

						field = provider.fields.at(operatorFieldIndex);
						fieldValue = aParameters.value(field.id, QString()).toString();

						if (fieldValue.isEmpty())
						{
							++operatorFieldIndex;
						}
					}

					if (findNextField)
					{
						QString masked;

						if (filter.haveFilter(field.id))
						{
							masked = filter.apply(field.id, aParameters.value(field.id, QString()).toString());
						}
						else
						{
							QString value = aParameters.value(field.id + CPrintingService::DislayPostfix, QString()).toString();
							masked = isMasked ? maskedString(value, isMasked) : value;
						}

						QString replaceString = QString("%1: %2").arg(field.title).arg(masked);

						it->replace(offset, tagPattern.cap(0).size(), replaceString);
						offset = replaceString.length();

						++operatorFieldIndex;
					}
				}
				else
				{
					it->replace(tagPattern.cap(0), QString());
				}
			}

			// Оставляем поле пустым.
			it->replace(tagPattern.cap(0), QString());
		}

		// Предзагружаем содержимое тегов [IMG]
		*it = convertImage2base64(*it);

		// Преобразуем [QR] теги в [IMG]
		*it = generateQR(*it);

		// Преобразуем [PDF417] теги в [IMG]
		*it = generatePDF417(*it);

		// Преобразуем [1d] теги в [IMG]
		*it = generate1D(*it);

		// Преобразуем [hr] тег в [IMG]
		*it = generateLine(*it);

		// Обработаем тег с условием
		if (it->contains(CPrintingService::ConditionTag))
		{
			QStringList l = it->split(CPrintingService::ConditionTag);
			
			toLog(LogLevel::Debug, QString("Evaluate receipt condition %1").arg(l.join(";")));
			
			if (QJSEngine().evaluate(l.first()).toBool())
			{
				toLog(LogLevel::Debug, QString("Evaluate receipt result %1").arg(l.last()));
				result.append(l.last());
			}
			else
			{
				toLog(LogLevel::Debug, QString("Evaluate condition nothing for %1").arg(l.last()));
			}

			continue;
		}

		if (it->length() > 0)
		{
			result.append(*it);
		}
	}

	aReceipt = result;
}

//---------------------------------------------------------------------------
void PrintingService::loadReceiptTemplates()
{
	auto loadReceiptTemplate = [&](const QFileInfo & aFileInfo)
	{
		if (!aFileInfo.suffix().compare("xml", Qt::CaseInsensitive))
		{
			QDomDocument xmlFile(aFileInfo.fileName());
			QFile file(aFileInfo.filePath());
			if (!file.open(QIODevice::ReadOnly))
			{
				toLog(LogLevel::Error, QString("Failed to open file %1").arg(aFileInfo.filePath()));
				return;
			}

			xmlFile.setContent(&file);

			QDomElement body = xmlFile.documentElement();

			QStringList receiptContents;

			for (QDomNode node = body.firstChild(); !node.isNull(); node = node.nextSibling())
			{
				QDomElement row = node.toElement();
				
				if (row.tagName() == "string" || row.tagName() == "else")
				{
					QString prefix = row.attribute("if").isEmpty() ? "" : QString("%1%2").arg(row.attribute("if")).arg(CPrintingService::ConditionTag);
					receiptContents.append(QString("%1%2").arg(prefix).arg(row.text()));
				}
				else if (row.tagName() == "hr")
				{
					receiptContents.append("[hr]-[/hr]");
				}
			}

			if (!receiptContents.isEmpty())
			{
				mCachedReceipts.insert(aFileInfo.baseName().toLower(), receiptContents);
			}
			else
			{
				toLog(LogLevel::Error, QString("Bad receipt template '%1': parse xml error").arg(aFileInfo.fileName()));
			}
		}
		else
		{
			toLog(LogLevel::Error, QString("Bad receipt template file extension : %1").arg(aFileInfo.fileName()));
		}
	};

	// Загружаем все шаблоны чеков в папке ./receipts
	QDir receiptDirectory;

	receiptDirectory.setPath(mApplication->getWorkingDirectory() + "/data/receipts");

	// Загружаем все файлы, которые есть в каталоге.
	foreach (const QFileInfo & fileInfo, receiptDirectory.entryInfoList(QDir::Files))
	{
		loadReceiptTemplate(fileInfo);
	}

	// загружаем все шаблоны из папки пользовательских шаблонов чеков
	receiptDirectory.setPath(mApplication->getWorkingDirectory() + "/user/receipts");

	// Загружаем все файлы, которые есть в каталоге.
	foreach (const QFileInfo & fileInfo, receiptDirectory.entryInfoList(QDir::Files))
	{
		loadReceiptTemplate(fileInfo);
	}
}

//---------------------------------------------------------------------------
void PrintingService::saveReceiptContent(const QString & aReceiptName, const QStringList & aContents)
{
	// Получаем имя папки с чеками.
	QString suffix = QDate::currentDate().toString("yyyy.MM.dd");

	QDir path(mApplication->getWorkingDirectory() + "/receipts/" + suffix);

	if (!path.exists())
	{
		if (!QDir().mkpath(path.path()))
		{
			toLog(LogLevel::Error, "Failed to create printed receipts folder.");
			return;
		}
	}

	auto fileName = path.path() + QDir::separator() + aReceiptName + ".txt";
	QFile file(fileName);

	// Сохраняем чек.
	if (file.open(QIODevice::WriteOnly))
	{
		file.write(aContents.join("\r\n").toUtf8());
		file.close();
	}
	else
	{
		toLog(LogLevel::Error, QString("Failed to open file %1 for receipt.").arg(fileName));
	}
}

//---------------------------------------------------------------------------
void PrintingService::saveReceipt(const QVariantMap & aParameters, const QString & aReceiptTemplate)
{
	QStringList receipt = getReceipt(aReceiptTemplate, aParameters);

	QString fileName = QTime::currentTime().toString("hhmmsszzz");

	if (aParameters.contains(PPSDK::CPayment::Parameters::ID))
	{
		fileName += QString("_%1").arg(aParameters[PPSDK::CPayment::Parameters::ID].toString());
	}

	fileName += "_not_printed";

	saveReceiptContent(fileName, receipt);
}

//---------------------------------------------------------------------------
QString & replaceTags(QString & aMessage)
{
	aMessage.replace("[br]", "\n", Qt::CaseInsensitive);
	aMessage.remove(QRegExp("\\[(b|dw|dh)\\]", Qt::CaseInsensitive));
	aMessage.remove(QRegExp("\\[/(b|dw|dh)\\]", Qt::CaseInsensitive));

	aMessage.remove(QRegExp("\\[img.?\\].*\\[/img\\]"));

	return aMessage;
}

//---------------------------------------------------------------------------
QString PrintingService::loadReceipt(qint64 aPaymentId)
{
	// Получаем имя папки с чеками.
	QString suffix = QDate::currentDate().toString("yyyy.MM.dd");

	QDir path(mApplication->getWorkingDirectory() + "/receipts/" + suffix);

	if (!path.exists())
	{
		toLog(LogLevel::Error, "Failed to find printed receipts folder.");
		return QString();
	}

	QDir dir(path.path() + QDir::separator());
	QStringList receiptFiles;
	receiptFiles
		<< QString("*_%1.txt").arg(aPaymentId)
		<< QString("*_%1_*.txt").arg(aPaymentId);

	QStringList receiptsBody;

	QStringList receipts = dir.entryList(receiptFiles);
	while (!receipts.isEmpty())
	{
		QFile f(dir.absolutePath() + QDir::separator() + receipts.takeFirst());
		if (f.open(QIODevice::ReadOnly))
		{
			receiptsBody << replaceTags(QString::fromUtf8(f.readAll()));
		}
	}

	return receiptsBody.join("\n------------------------------\n");
}

//---------------------------------------------------------------------------
void PrintingService::onStatusChanged(DSDK::EWarningLevel::Enum aWarningLevel, const QString & /*aTranslation*/, int /*aStatus*/)
{
	emit printerStatus(aWarningLevel == DSDK::EWarningLevel::OK);
}

//---------------------------------------------------------------------------
void PrintingService::onFRSessionClosed(const QVariantMap & aParameters)
{
	// Получаем имя папки с отчётами.
	QDir path(mApplication->getWorkingDirectory() + "/receipts/reports");

	if (!path.exists())
	{
		if (!QDir().mkpath(path.path()))
		{
			toLog(LogLevel::Error, "Failed to create printed reports folder.");
			return;
		}
	}

	auto fileName = path.path() + QDir::separator() + QDateTime::currentDateTime().toString("yyyy.MM.dd hhmmsszzz") + ".xml";
	QFile file(fileName);

	// Сохраняем отчёт.
	if (file.open(QIODevice::WriteOnly))
	{
		using namespace SDK::Driver;

		QXmlStreamWriter stream(&file);
		stream.setAutoFormatting(true);
		stream.writeStartDocument();

		stream.writeStartElement(CFRReport::ZReport);
		stream.writeAttribute(CFRReport::Number, aParameters[CFiscalPrinter::ZReportNumber].toString());

		stream.writeStartElement(CFRReport::FR);
		stream.writeAttribute(CFRReport::Serial, aParameters[CFiscalPrinter::Serial].toString());
		stream.writeAttribute(CFRReport::RNM, aParameters[CFiscalPrinter::RNM].toString());
		stream.writeEndElement(); // CFRReport::FR

		stream.writeTextElement(CFRReport::PaymentCount, aParameters[CFiscalPrinter::PaymentCount].toString());
		double amount = aParameters[CFiscalPrinter::PaymentAmount].toDouble();
		stream.writeTextElement(CFRReport::PaymentAmount, QString::number(amount, 'f', 2));
		amount = aParameters[CFiscalPrinter::NonNullableAmount].toDouble();
		stream.writeTextElement(CFRReport::NonNullableAmount, QString::number(amount, 'f', 2));
		stream.writeTextElement(CFRReport::FRDateTime, aParameters[CFiscalPrinter::FRDateTime].toString());
		stream.writeTextElement(CFRReport::SystemDateTime, aParameters[CFiscalPrinter::SystemDateTime].toString());

		stream.writeEndElement(); // CFRReport::ZReport
		stream.writeEndDocument();
		
		file.flush();
		file.close();
	}
}

//---------------------------------------------------------------------------
void PrintingService::updateHardwareConfiguration()
{
	PPSDK::TerminalSettings * settings = SettingsService::instance(mApplication)->getAdapter<PPSDK::TerminalSettings>();

	// Получаем информацию о принтерах из конфигов.
	QString regExpData = QString("(%1|%2|%3)")
		.arg(DSDK::CComponents::Printer).arg(DSDK::CComponents::DocumentPrinter).arg(DSDK::CComponents::FiscalRegistrator);
	QStringList printerNames = settings->getDeviceList().filter(QRegExp(regExpData));

	auto commonSettings = settings->getCommonSettings();

	mRandomReceiptsID = commonSettings.randomReceiptsID;
	QTime autoZReportTime = commonSettings.autoZReportTime;
	mEnableBlankFiscalData = commonSettings.enableBlankFiscalData;

	mPrinterDevices.clear();
	mAvailablePrinters.clear();

	// Запрашиваем устройства.
	foreach (const QString & printerName, printerNames)
	{
		DSDK::IPrinter * device = dynamic_cast<DSDK::IPrinter *>(mDeviceService->acquireDevice(printerName));

		if (device)
		{
			QVariantMap dealerSettings;
			if (mStaticParameters.contains(CPrintConstants::DealerTaxSystem)) dealerSettings.insert(CHardwareSDK::FR::DealerTaxSystem, mStaticParameters[CPrintConstants::DealerTaxSystem]);
			if (mStaticParameters.contains(CPrintConstants::DealerAgentFlag)) dealerSettings.insert(CHardwareSDK::FR::DealerAgentFlag, mStaticParameters[CPrintConstants::DealerAgentFlag]);

			mPrinterDevices.append(device);

			// Подписываемся на события принтера.
			device->subscribe(SDK::Driver::IDevice::StatusSignal, this, SLOT(onStatusChanged(SDK::Driver::EWarningLevel::Enum, const QString &, int)));

			// Подписываемся на события фискальника.
			if (dynamic_cast<DSDK::IFiscalPrinter *>(device))
			{
				device->subscribe(SDK::Driver::IFiscalPrinter::FRSessionClosedSignal, this, SLOT(onFRSessionClosed(const QVariantMap &)));

				if (autoZReportTime.isValid() && !autoZReportTime.isNull())
				{
					toLog(LogLevel::Normal, QString("Setup auto z-report time: %1.").arg(autoZReportTime.toString("hh:mm:ss")));

					dealerSettings.insert(CHardwareSDK::FR::ZReportTime, autoZReportTime);
				}
			}

			device->setDeviceConfiguration(dealerSettings);
		}
		else
		{
			toLog(LogLevel::Error, QString("Failed to acquire device %1 .").arg(printerName));
		}
	}

	mAvailablePrinters = mPrinterDevices.toSet();
}

//---------------------------------------------------------------------------
void PrintingService::createFiscalRegister()
{
	if (!mFiscalRegister)
	{
		// Получаем информацию о фискальных регистраторах.
		PPSDK::ExtensionsSettings * extSettings = SettingsService::instance(mApplication)->getAdapter<PPSDK::ExtensionsSettings>();
		SDK::Plugin::IPluginLoader * pluginLoader = PluginService::instance(mApplication)->getPluginLoader();

		foreach(auto fr, pluginLoader->getPluginList(QRegExp(PPSDK::CComponents::FiscalRegister)))
		{
			auto plugin = pluginLoader->createPlugin(fr);
			if (!plugin)
			{
				continue;
			}

			PPSDK::IFiscalRegister * frPlugin = dynamic_cast<PPSDK::IFiscalRegister *>(plugin);

			if (!frPlugin)
			{
				toLog(LogLevel::Error, QString("FR %1 not have IFiscalRegister interface.").arg(fr));
				pluginLoader->destroyPlugin(plugin);
				continue;
			}

			auto parameters = extSettings->getSettings(plugin->getPluginName());

			if (parameters.isEmpty())
			{
				toLog(LogLevel::Warning, QString("FR %1 not have extensions settings. Skip it. (check config.xml).").arg(plugin->getPluginName()));
				pluginLoader->destroyPlugin(plugin);
				continue;
			}

			connect(dynamic_cast<QObject *>(frPlugin), SDK::Driver::IFiscalPrinter::FRSessionClosedSignal, this, SLOT(onFRSessionClosed(const QVariantMap &)));

			if (!frPlugin->initialize(parameters))
			{
				toLog(LogLevel::Warning, QString("FR %1 error initialize. Skip it.").arg(plugin->getPluginName()));
				pluginLoader->destroyPlugin(plugin);
				continue;
			}

			mFiscalRegister = frPlugin;
			toLog(LogLevel::Normal, QString("FR %1 loaded successful.").arg(plugin->getPluginName()));
			break;
		}
	}
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::IFiscalRegister * PrintingService::getFiscalRegister() const
{
	return mFiscalRegister;
}

//---------------------------------------------------------------------------
void PrintingService::setFiscalNumber(qint64 aPaymentId, const QVariantMap & aParameters)
{
	QList<PPSDK::IPayment::SParameter> parameters;

	foreach (auto name, aParameters.keys())
	{
		parameters.push_back(PPSDK::IPayment::SParameter(name, aParameters.value(name), true));
	}

	if (!PaymentService::instance(mApplication)->updatePaymentFields(aPaymentId, parameters))
	{
		toLog(LogLevel::Error, QString("Payment %1: Error update fiscal parameters.").arg(aPaymentId));
	}
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::SCurrencySettings PrintingService::getCurrencySettings() const
{
	SettingsService * settingsService = SettingsService::instance(mApplication);

	PPSDK::TerminalSettings * terminalSettings = settingsService->getAdapter<PPSDK::TerminalSettings>();

	return terminalSettings->getCurrencySettings();
}

//---------------------------------------------------------------------------
