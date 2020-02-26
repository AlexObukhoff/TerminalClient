/* Реализация протокола с EFTPOS 3.0 компании Ucs. */

#pragma once

#include <numeric>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTimer>
#include <QtCore/QTimerEvent>
#include <QtCore/QLibrary>
#include <QtConcurrent/QtConcurrentRun>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Core/IGUIService.h>
#include <SDK/PaymentProcessor/Core/ISchedulerService.h>
#include "SDK/PaymentProcessor/Settings/ISettingsAdapter.h"
#include <SDK/PaymentProcessor/Core/IPrinterService.h>
#include <SDK/PaymentProcessor/Scripting/Core.h>
#include <SDK/PaymentProcessor/Scripting/PrinterService.h>
#include <SDK/GUI/IGraphicsHost.h>


// Project headers
#include "Ucs.h"
#include "API.h"
#include "Responses.h"
#include "Utils.h"
#include "UscEncashTask.h"

#pragma comment(lib, "psapi.lib")

namespace CUcs
{
	const char * DefaultTerminalID = "0000000000";
	const int ReconnectTimeout = 5000;
	const int PollTimeout = 30 * 60 * 1000; // Проверяем раз в 30 мин.
	const int LoginTimeout = 60 * 60 * 1000; // Проверяем раз в 1 час.
	const int ExecuteEncashmentTimeout = 100;

	const char * LibraryName = "ucs_ms";

	const int USE_STATUS = 0;  // Запрещаем выполнение запроса статуса, т.к. для новых прошивок POS она кривая.
	const int ReceiveBufferSize = 320;
}

namespace Ucs
{

//---------------------------------------------------------------------------
API::API(SDK::PaymentProcessor::ICore * aCore, ILog * aLog) : 
	ILogable(aLog),
	mCore(aCore),
	mTerminalSettings(static_cast<PPSDK::TerminalSettings *>(aCore->getSettingsService()->getAdapter(PPSDK::CAdapterNames::TerminalAdapter))),
	mTerminalID(CUcs::DefaultTerminalID),
	mLastLineReceived(false),
	mTimerEncashID(0),
	mRuntimeInit(false),
	mPySelf(nullptr),
	mEftpCreate(nullptr),
	mEftpDestroy(nullptr),
	mEftpDo(nullptr),
	mTerminalState(APIState::None),
	mMaxAmount(0.0),
	mNeedEncashment(false),
	mNeedPrintAllEncashmentReports(false),
	mDatabase(mCore->getDatabaseService(), ILog::getInstance(Ucs::LogName))
{
	toLog(LogLevel::Normal, QString("UCS API created."));
	
	connect(&mResponseWatcher, SIGNAL(finished()), this, SLOT(onResponseFinished()));

	killOldUCSProcess();

	//Заводим задачу в планировщике на ежесуточную инкассацию
	mEncashmentTask = new UscEncashTask(Ucs::EncashmentTask, getLog()->getName(), QString());
	connect(mEncashmentTask, SIGNAL(finished(const QString &, bool)), this, SLOT(onEncashTaskFinished(const QString &, bool)));
	mCore->getSchedulerService()->registerTaskType(Ucs::EncashmentTask, mEncashmentTask);
}

//---------------------------------------------------------------------------
QSharedPointer<API> API::getInstance(SDK::PaymentProcessor::ICore * aCore, ILog * aLog)
{
	static QSharedPointer<API> gApi = QSharedPointer<API>(new API(aCore, aLog));

	return gApi;
}

//---------------------------------------------------------------------------
bool API::setupRuntime(const QString & aRuntimePath)
{
	if (mRuntimeInit)
	{
		return true;
	}

	QString libPath = QString("%1/%2").arg(aRuntimePath).arg(CUcs::LibraryName);
	QLibrary lib(libPath);
	if (!lib.load())
	{
		toLog(LogLevel::Error, QString("Could not load library %1").arg(libPath));
		return false;
	}
	
	mEftpCreate = (EftpCreate)lib.resolve("eftp_create");
	if (!mEftpCreate)
	{
		toLog(LogLevel::Error, QString("Could not load resolve function 'eftp_create'"));
		return false;
	}

	mEftpDestroy = (EftpDestroy)lib.resolve("eftp_destroy");
	if (!mEftpDestroy)
	{
		toLog(LogLevel::Error, QString("Could not load resolve function 'eftp_destroy'"));
		return false;
	}
	
	mEftpDo = (EftpDo)lib.resolve("eftp_do");
	if (!mEftpDo)
	{
		toLog(LogLevel::Error, QString("Could not load resolve function 'eftp_do'"));
		return false;
	}

	mRuntimeInit = true;

	mTimerEncashID = startTimer(CUcs::PollTimeout);

	// первый раз статус дергаем сразу после запуска
	QTimer::singleShot(CUcs::ReconnectTimeout, this, SLOT(status()));

	connect(mCore->getPrinterService(), SIGNAL(receiptPrinted(int, bool)), this, SLOT(onReceiptPrinted(int, bool)));

	return true;
}

//---------------------------------------------------------------------------
bool API::isReady() const
{
	return mRuntimeInit;
}

//---------------------------------------------------------------------------
bool API::enable(PPSDK::TPaymentAmount aAmount)
{
	if (mTerminalState != APIState::None)
	{
		toLog(LogLevel::Error, QString("API is busy (state=%1). Could not enable charge provider.").arg(mTerminalState));

		return false;
	}

	mTerminalState = APIState::Sale;

	mMaxAmount = aAmount;
	mCurrentReceipt.clear();

	eftpCleanup();

	return true;
}

//---------------------------------------------------------------------------
void API::disable()
{
	mTerminalState = APIState::None;
	mLoggedIn = QDateTime();

	mCurrentReceipt.clear();

	eftpCleanup();

	if (mNeedEncashment && mNeedPrintAllEncashmentReports)
	{
		encashment(false);
	}
}

//---------------------------------------------------------------------------
void API::eftpCleanup()
{
	if (mPySelf)
	{
		mEftpDestroy(mPySelf);
		mPySelf = nullptr;

		toLog(LogLevel::Debug, "EFTP Object destroyed.");
	}
}

//---------------------------------------------------------------------------
void API::onResponseFinished()
{
	TResponse result = mResponseWatcher.result();

	QByteArray responseBuffer = result.response;

	QList<std::function<bool(API &, BaseResponsePtr)>> responseHandlers;
	responseHandlers
		<< std::mem_fun_ref(&API::isErrorResponse)
		<< std::mem_fun_ref(&API::isConsoleResponse)
		<< std::mem_fun_ref(&API::isLoginResponse)
		<< std::mem_fun_ref(&API::isEnchashmentResponse)				
		<< std::mem_fun_ref(&API::isInitialResponse)
		<< std::mem_fun_ref(&API::isPINRequiredResponse)
		<< std::mem_fun_ref(&API::isOnlineRequiredResponse)
		<< std::mem_fun_ref(&API::isAuthResponse)
		<< std::mem_fun_ref(&API::isPrintLineResponse)
		<< std::mem_fun_ref(&API::isBreakResponse)
		<< std::mem_fun_ref(&API::isHoldResponse)
		<< std::mem_fun_ref(&API::isMessageResponse);

	while (!responseBuffer.isEmpty())
	{
		// Нам сюда могут прилететь пачкой несколько ответов POS терминала, разбираем их по одному.
		QByteArray nextAnswer = responseBuffer.left(CUcs::ReceiveBufferSize);
		responseBuffer.remove(0, CUcs::ReceiveBufferSize);

		BaseResponsePtr response = BaseResponse::createResponse(nextAnswer);

		if (!response)
		{
			toLog(LogLevel::Debug, QString("BaseResponsePtr corrupted."));
			continue;;
		}

		// Переносим статус API из полученного сырого буфера в обработанный результат
		response->mAPIState = result.state;

		toLog(LogLevel::Debug, QString("BASE RESPONSE packet from EFTPOS. Class='%1' Code='%2'")
			.arg(response->mClass)
			.arg(response->mCode));

		if (!response->isValid())
		{
			toLog(LogLevel::Error, QString("Receive unknown packet from EFTPOS. Class='%1' Code='%2'")
				.arg(response->mClass)
				.arg(response->mCode));

			toLog(LogLevel::Error, QString("Raw response: %1").arg(QString::fromLatin1(responseBuffer.toHex())));
		}
		else
		{
			foreach (auto handler, responseHandlers)
			{
				if (handler(*this, response))
				{
					emit doComplete(false);
					break;
				}
			}
		}
	}
}

//---------------------------------------------------------------------------
void API::onReceiptPrinted(int aJobIndex, bool aError)
{
	if (mEncashmentInPrint.contains(aJobIndex))
	{
		auto encashment = mEncashmentInPrint.value(aJobIndex);

		if (!aError)
		{
			toLog(LogLevel::Normal, QString("Encashment [%1] printed.").arg(encashment.date.toString("yyyy.MM.dd hh:mm:ss")));

			mDatabase.markAsPrinted(encashment);
		}
		else
		{
			toLog(LogLevel::Error, QString("Encashment [%1] print FAILED.").arg(encashment.date.toString("yyyy.MM.dd hh:mm:ss")));
		}

		mEncashmentInPrint.remove(aJobIndex);
	}
}

//---------------------------------------------------------------------------
void API::login()
{
	if (!isLoggedInExpired())
	{
		emit ready();
		return;
	}
	
	mLoggedIn = QDateTime();
	mTerminalState = APIState::Login;

	toLog(LogLevel::Normal, QString("> Login."));

	QByteArray buffer;
	buffer.append(mTerminalID);
	buffer.append("01"); // Length
	buffer.append("1"); // Info Code
	
	mResponseWatcher.setFuture(QtConcurrent::run(
		this, &API::send, makeRequest(Ucs::Class::Session, Ucs::Login::CodeRequest, buffer), true));
}

//---------------------------------------------------------------------------
void API::encashment(bool aOnDemand)
{
	doEncashment(aOnDemand, false);
}

//---------------------------------------------------------------------------
void API::doEncashment(bool aOnDemand, bool aSkipPrintReceipt)
{
	if (!mNeedEncashment && aOnDemand)
	{
		emit encashmentComplete();
		return;
	}

	if (!aOnDemand)
	{
		toLog(LogLevel::Normal, "Start manual encashment.");

		mNeedEncashment = true;
		mNeedPrintAllEncashmentReports = !aSkipPrintReceipt;
	}

	if (mTerminalState != APIState::None && mTerminalState != APIState::Encashment)
	{
		toLog(LogLevel::Normal, "Wait for starting manual encashment...");
		return;
	}

	mTerminalState = APIState::Encashment;
	
	toLog(LogLevel::Normal, QString("> Encashment."));

	QByteArray buffer;
	buffer.append(mTerminalID);
	buffer.append("00"); // Length

	mResponseWatcher.setFuture(QtConcurrent::run(
		this, &API::send, makeRequest(Ucs::Class::Service, Ucs::Encashment::CodeRequest, buffer), true));
}

//---------------------------------------------------------------------------
void API::sale(double aAmount)
{
	mTerminalState = APIState::Sale;

	toLog(LogLevel::Normal, QString("> Sale."));

	QByteArray buffer;
	buffer.append(mTerminalID);
	buffer.append("0C"); // Length
	buffer.append(QString("%1").arg(qFloor((aAmount * 1000 + 0.001) / 10.0), 12, 10, QChar('0')).toLatin1());

	mResponseWatcher.setFuture(QtConcurrent::run(
		this, &API::send, aAmount ? makeRequest(Ucs::Class::AuthRequest, Ucs::Sale::CodeRequest, buffer) : QByteArray(), mLastLineReceived));
}

//---------------------------------------------------------------------------
void API::breakSale()
{
	toLog(LogLevel::Normal, "> Break sale.");

	mTerminalState = APIState::Sale;

	mResponseWatcher.setFuture(QtConcurrent::run(
		this, &API::send, makeRequest(Ucs::Class::Session, Ucs::Break::CodeRequest), true));
}

//---------------------------------------------------------------------------
void API::status()
{
	if (mTerminalState != APIState::None)
	{
		toLog(LogLevel::Debug, QString("Skip get status. State=%1.").arg(mTerminalState));

		return;
	}

	if (isLoggedInExpired())
	{
		login();

		return;
	}

	if (CUcs::USE_STATUS)
	{
		toLog(LogLevel::Normal, "> Request status.");

		mTerminalState = APIState::Status;

		QByteArray buffer;
		buffer.append(mTerminalID);
		buffer.append("05"); // Length
		buffer.append("1"); // Last message
		buffer.append("JOU0"); // Processing Method

		mResponseWatcher.setFuture(QtConcurrent::run(
			this, &API::send, makeRequest(Ucs::Class::Session, Ucs::Information::CodeRequest, buffer), true));
	}
}

//---------------------------------------------------------------------------
void API::onEncashTaskFinished(const QString & aName, bool aComplete)
{
	toLog(LogLevel::Debug, "Scheduled task ready to execute encashment.");

	if (isReady())
	{
		toLog(LogLevel::Normal, "Execute encashment by task.");
		doEncashment(false, true);
	}
}

//---------------------------------------------------------------------------
void API::printAllEncashments()
{
	mNeedPrintAllEncashmentReports = false;

	auto printingService = mCore->getPrinterService();
	QVariantMap parameters;

	toLog(LogLevel::Normal, "Start print all unprinted encashment.");

	foreach (UcsDB::Encashment encashment, mDatabase.getAllNotPrinted())
	{
		parameters["EMV_DATA"] = encashment.receipt.join("[br]");

		int job = printingService->printReceipt("", parameters, "emv_encashment", DSDK::EPrintingModes::Continuous);
		
		if (job)
		{
			toLog(LogLevel::Normal, QString("Encashment [%1] print started.").arg(encashment.date.toString("yyyy.MM.dd hh:mm:ss")));

			mEncashmentInPrint[job] = encashment;
		}
		else
		{
			toLog(LogLevel::Error, QString("Failed start encashment [%1] print.").arg(encashment.date.toString("yyyy.MM.dd hh:mm:ss")));
		}
	}
}

//---------------------------------------------------------------------------
void API::killOldUCSProcess()
{
	unsigned long processes[2048], cbNeeded, cProcesses;
	QSet<unsigned long> lprocess;

	if (EnumProcesses(processes, sizeof(processes), &cbNeeded))
	{
		cProcesses = cbNeeded / sizeof(processes[0]);

		for (unsigned int i = 0; i < cProcesses; i++)
		{
			if (processes[i] == 0)
				continue;

			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, processes[i]);
			wchar_t buffer[MAX_PATH] = {0};
			GetModuleBaseName(hProcess, 0, buffer, MAX_PATH);
			CloseHandle(hProcess);

			QString processPath = QString::fromWCharArray(buffer);

			if (processPath.toLower().contains("ucs_"))
			{
				hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, processes[i]);
				if (hProcess != 0)
				{
					if (!TerminateProcess(hProcess, 0))
					{
						toLog(LogLevel::Error, QString("Error TerminateProcess(%1, 0) (%2): #%3.")
							.arg(processes[i]).arg(processPath).arg(GetLastError(), 0, 16));
					}

					CloseHandle(hProcess);
				}
				else
				{
					toLog(LogLevel::Error, QString("Error OpenProcess(PROCESS_ALL_ACCESS, 0, %1) (%2): #%3.")
						.arg(processes[i]).arg(processPath).arg(GetLastError(), 0, 16));
				}
			}
		}
	}
	else
	{
		toLog(LogLevel::Error, QString("Error call EnumProcesses: #%1.").arg(GetLastError(), 0, 16));
	}
}

//---------------------------------------------------------------------------
bool API::isErrorResponse(BaseResponsePtr aResponse)
{
	auto errorResponse = qSharedPointerDynamicCast<ErrorResponse, BaseResponse>(aResponse);
	if (errorResponse)
	{
		QString e = QTextCodec::codecForName("windows-1251")->toUnicode(errorResponse->getErrorMessage().toLatin1());
		
		toLog(LogLevel::Error, QString("< Error: 0x%1 (%2)")
			.arg(errorResponse->getError()).arg(e));

		mTerminalState = APIState::None;

		emit error(e);

		mLastError = errorResponse->getError();

		if (mLastError == "10") // необходима инкассация
		{
			// Инкассировать будем в новой сессии библиотеки
			eftpCleanup();

			mTerminalState = APIState::None;
			mNeedEncashment = true;
			mNeedPrintAllEncashmentReports = false;

			QTimer::singleShot(CUcs::ExecuteEncashmentTimeout, this, SLOT(encashment()));
		}
	}

	return errorResponse;
}

//---------------------------------------------------------------------------
bool API::isLoginResponse(BaseResponsePtr aResponse)
{
	auto loginResponse = qSharedPointerDynamicCast<LoginResponse, BaseResponse>(aResponse);
	if (loginResponse)
	{
		toLog(LogLevel::Normal, QString("< Login. TID: %1. Encashment state: %2").arg(loginResponse->getTerminalID()).arg(loginResponse->needEncashment()));

		mNeedEncashment = loginResponse->needEncashment();
		mTerminalID = loginResponse->getTerminalID();

		if (mTerminalID != CUcs::DefaultTerminalID && loginResponse->getStatusCode() == "0" && !mNeedEncashment)
		{
			mLoggedIn = QDateTime::currentDateTime();
		}

		// Логин с валидными параметрами создадим с новым объектом библиотеки
		eftpCleanup();

		mTerminalState = APIState::None;

		if (mNeedEncashment)
		{
			toLog(LogLevel::Normal, "Encashment required.");

			QTimer::singleShot(CUcs::ExecuteEncashmentTimeout, this, SLOT(encashment()));
		}
		else
		{
			emit ready();
		}
	}

	return loginResponse;
}

//---------------------------------------------------------------------------
bool API::isPrintLineResponse(BaseResponsePtr aResponse)
{
	auto printLineResponse = qSharedPointerDynamicCast<PrintLineResponse, BaseResponse>(aResponse);
	if (printLineResponse)
	{
		toLog(LogLevel::Normal, QString("< Print line: %1.").arg(printLineResponse->getText()));

		mCurrentReceipt << printLineResponse->getText();

		if (aResponse->mAPIState == APIState::Encashment)
		{
			if (printLineResponse->isLast())
			{
				saveEncashmentReport();
			}
		}
		else
		{
			if (printLineResponse->isLast())
			{
				mLastLineReceived = true;
				printReceipt();

				if (mAuthResponse)
				{
					emit saleComplete(mAuthResponse->mTransactionSumm / 100.0,
						mAuthResponse->mCurrency,
						mAuthResponse->mRRN,
						mAuthResponse->mConfirmation);

					mAuthResponse.clear();
				}
			}
		}
	}

	return printLineResponse;
}

//---------------------------------------------------------------------------
bool API::isConsoleResponse(BaseResponsePtr aResponse)
{
	auto consoleResponse = qSharedPointerDynamicCast<ConsoleResponse, BaseResponse>(aResponse);
	if (consoleResponse)
	{
		toLog(LogLevel::Normal,
			QString("< Console response: %1.").arg(consoleResponse->getMessage()));

		emit message(consoleResponse->getMessage());

		if (mTerminalState == APIState::Encashment)
		{
			toLog(LogLevel::Normal, "Wait encashment result.");

			QTimer::singleShot(100, this, SLOT(encashment()));
		}
	}

	return consoleResponse;
}

//---------------------------------------------------------------------------
bool API::isEnchashmentResponse(BaseResponsePtr aResponse)
{
	auto encashmentResponse = qSharedPointerDynamicCast<EncashmentResponse, BaseResponse>(aResponse);
	if (encashmentResponse)
	{
		mNeedEncashment = false;
		
		toLog(LogLevel::Normal, QString("< Encashment response OK."));
	}

	return encashmentResponse;
}

//---------------------------------------------------------------------------
bool API::isInitialResponse(BaseResponsePtr aResponse)
{
	auto initialResponse = qSharedPointerDynamicCast<InitialResponse, BaseResponse>(aResponse);
	if (initialResponse)
	{
		toLog(LogLevel::Normal, "< Initial response: OK.");

		if (mTerminalState == APIState::Status)
		{
			// Wait 3-8 response
			mResponseWatcher.setFuture(QtConcurrent::run(this, &API::send, QByteArray(), true));
		}
		else
		{
			mTerminalState = APIState::Sale;

			emit readyToCard();
		}
	}

	return initialResponse;
}

//---------------------------------------------------------------------------
bool API::isBreakResponse(BaseResponsePtr aResponse)
{
	auto breakResponse = qSharedPointerDynamicCast<BreakResponse, BaseResponse>(aResponse);
	if (breakResponse)
	{
		toLog(breakResponse->isComplete() ? LogLevel::Normal : LogLevel::Error, 
			QString("< Break response: %1.").arg(breakResponse->isComplete() ? "OK" : "DENY"));

		// TODO - надо как-то реагировать на невозможность отмены транзакции!
		if (breakResponse->isComplete())
		{
			mTerminalState = APIState::None;

			emit breakComplete();
		}
	}

	return breakResponse;
}

//---------------------------------------------------------------------------
bool API::isPINRequiredResponse(BaseResponsePtr aResponse)
{
	auto pinRequiredResponse = qSharedPointerDynamicCast<PINRequiredResponse, BaseResponse>(aResponse);
	if (pinRequiredResponse)
	{
		toLog(LogLevel::Normal, "< PIN required.");

		mTerminalState = APIState::Sale;

		emit pinRequired();
	}

	return pinRequiredResponse;
}

//---------------------------------------------------------------------------
bool API::isOnlineRequiredResponse(BaseResponsePtr aResponse)
{
	auto onlineRequiredResponse = qSharedPointerDynamicCast<OnlineRequiredResponse, BaseResponse>(aResponse);
	if (onlineRequiredResponse)
	{
		toLog(LogLevel::Normal, "< Online required.");

		mTerminalState = APIState::Sale;

		emit onlineRequired();
	}

	return onlineRequiredResponse;
}

//---------------------------------------------------------------------------
bool API::isAuthResponse(BaseResponsePtr aResponse)
{
	auto authResponse = qSharedPointerDynamicCast<AuthResponse, BaseResponse>(aResponse);
	if (authResponse)
	{
		toLog(LogLevel::Normal, QString("< Auth response. %1.").arg(authResponse->toString()));

		if (authResponse->isOK())
		{
			mAuthResponse = authResponse;

			emit hold();
		}
	}

	return authResponse;
}

//---------------------------------------------------------------------------
bool API::isHoldResponse(BaseResponsePtr aResponse)
{
	auto holdResponse = qSharedPointerDynamicCast<HoldResponse, BaseResponse>(aResponse);
	if (holdResponse)
	{
		toLog(LogLevel::Normal, "< Hold.");

		emit hold();
	}

	return holdResponse;
}

//---------------------------------------------------------------------------
bool API::isMessageResponse(BaseResponsePtr aResponse)
{
	auto msgResponse = qSharedPointerDynamicCast<MessageResponse, BaseResponse>(aResponse);
	if (msgResponse)
	{
		toLog(LogLevel::Normal, "< Status message.");

		mNeedEncashment = msgResponse->needEncashment();

		disable();

		if (mNeedEncashment)
		{
			toLog(LogLevel::Normal, "Encashment required.");

			QTimer::singleShot(CUcs::ExecuteEncashmentTimeout, this, SLOT(encashment()));
		}
	}

	return msgResponse;
}

//---------------------------------------------------------------------------
void API::printReceipt()
{
	QVariantMap parameters;
	parameters["EMV_DATA"] = mCurrentReceipt.join("").replace("||", "|\n|").replace("\n", "[br]").replace("|", "");
	
	SDK::PaymentProcessor::Scripting::Core * scriptingCore = static_cast<SDK::PaymentProcessor::Scripting::Core *>
		(dynamic_cast<SDK::GUI::IGraphicsHost *>(mCore->getGUIService())->getInterface<QObject>(SDK::PaymentProcessor::Scripting::CProxyNames::Core));

	SDK::PaymentProcessor::Scripting::PrinterService * ps = static_cast<SDK::PaymentProcessor::Scripting::PrinterService *>(scriptingCore->property("printer").value<QObject *>());

	ps->printReceiptExt("", parameters, "emv", DSDK::EPrintingModes::Glue);
}

//---------------------------------------------------------------------------
void API::saveEncashmentReport()
{
	toLog(LogLevel::Normal, QString("Encashment report complete.\n%1").arg(mCurrentReceipt.join("\n")));

	UcsDB::Encashment enc;
	
	enc.date = QDateTime::currentDateTime();
	qSwap(enc.receipt, mCurrentReceipt);

	mDatabase.save(enc);

	emit encashmentComplete();

	if (mNeedPrintAllEncashmentReports)
	{
		printAllEncashments();
	}
	else
	{
		QVariantMap parameters;

		parameters["EMV_DATA"] = enc.receipt.join("\n");

		mCore->getPrinterService()->saveReceipt(parameters, "emv_encashment");
	}
}

//---------------------------------------------------------------------------
bool API::isLoggedIn() const
{
	return mTerminalID != CUcs::DefaultTerminalID && !mLoggedIn.isNull();
}

//---------------------------------------------------------------------------
bool API::isLoggedInExpired() const
{
	return !isLoggedIn() || mLoggedIn.addMSecs(CUcs::LoginTimeout) < QDateTime::currentDateTime();
}

//---------------------------------------------------------------------------
QByteArray API::makeRequest(char aClass, char aCode, const QByteArray & aData)
{
	QByteArray request;

	request.append(aClass);
	request.append(aCode);
	/*QString dataLength = QString::number(aData.size(), 16);
	if (dataLength.size() == 1)
	{
		dataLength.insert(0, '0');
	}
	request.append(dataLength.toLatin1());*/
	request.append(aData);

	return request;
}

//---------------------------------------------------------------------------
void API::timerEvent(QTimerEvent * aEvent)
{
	if (mTimerEncashID == aEvent->timerId())
	{
		status();
	}
}

//---------------------------------------------------------------------------
API::TResponse API::send(const QByteArray & aRequest, bool aWaitOperationComplete /*= true*/)
{
	if (!mRuntimeInit)
	{
		toLog(LogLevel::Error, "USC runtime was not loaded. Send command error.");
		return TResponse();
	}
	
	if (!mPySelf)
	{
		mPySelf = mEftpCreate("");

		if (!mPySelf)
		{
			toLog(LogLevel::Error, "Could not create EFTP Object. Request aborted.");
			return TResponse();
		}

		toLog(LogLevel::Debug, "EFTP Object created.");
	}
	
	toLog(LogLevel::Debug, QString("REQUEST: %1").arg(QString::fromLatin1(aRequest)));
	
	mLastError.clear();

	auto readTo1B = [&](const QByteArray & aBuffer, QTextCodec * aCodec) -> QString
	{
		QString result;

		for (int i = 0; i < aBuffer.size() && aBuffer[i]; i++)
		{
			char c = aBuffer.at(i);
			result.append(aCodec->toUnicode(&c, 1).at(0));
		}

		return result;
	};

	QByteArray responseBuffer(CUcs::ReceiveBufferSize, 0);
	int result = mEftpDo(mPySelf, aRequest.isEmpty() ? 0 : (char *)aRequest.data(), responseBuffer.data(), NULL, NULL);

	toLog(LogLevel::Debug, QString("RESPONSE: [%1, %2]")
		.arg(result)
		.arg(readTo1B(responseBuffer, QTextCodec::codecForName("windows-1251"))));

	if (aWaitOperationComplete && result == 0)
	{
		do
		{
			QByteArray responseBuffer2(CUcs::ReceiveBufferSize, 0);
			result = mEftpDo(mPySelf, 0, responseBuffer2.data(), NULL, NULL);

			toLog(LogLevel::Debug, QString("RESPONSE (loop): [%1, %2]")
				.arg(result)
				.arg(readTo1B(responseBuffer2, QTextCodec::codecForName("windows-1251"))));

			if (responseBuffer2.size() != responseBuffer2.count('\0'))
			{
				responseBuffer.append(responseBuffer2);
			}
		} while (result == 0);
	}

	// Создаем объект TResponse тут, т.к. метод disable() сбрасывает mTerminalState
	TResponse response(result, mTerminalState, responseBuffer);

	if (aWaitOperationComplete)
	{
		if (mTerminalState == APIState::Sale && result == 9)
		{
			emit doComplete(true);
		}
		if (mTerminalState == APIState::Encashment)
		{
			mNeedEncashment = false;
		}

		mLastLineReceived = false;

		disable();
	}

	return response;
}

} // Ucs

//---------------------------------------------------------------------------
