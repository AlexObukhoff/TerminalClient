/* @file Базовый ФР на протоколе АТОЛ. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// Project
#include "AtolFRBase.h"

using namespace SDK::Driver;
using namespace ProtocolUtils;

//--------------------------------------------------------------------------------
AtolFRBase::AtolFRBase()
{
	using namespace SDK::Driver::IOPort::COM;

	// данные порта
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR115200);   // default
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR4800);     // default after resetting to zero
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR57600);
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR38400);
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR19200);
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);

	mPortParameters[EParameters::Parity].append(EParity::No);

	// теги
	mTagEngine = Tags::PEngine(new CAtolFR::TagEngine());

	// данные устройства
	mLineFeed = false;
	mMode = CAtolFR::InnerModes::NoMode;
	mSubmode = CAtolFR::InnerSubmodes::NoSubmode;
	mLocked = false;
	mNonNullableAmount = 0;

	// количество строк шапки (по умолчанию 4) - изменено для корректной печати на фискализированных аппаратах (как и было раньше)
	mDocumentCapStrings = 6;

	// регистры
	mRegisterData.add(CAtolFR::Registers::PaymentAmount,    '\x03', 6);
	mRegisterData.add(CAtolFR::Registers::PaymentCount,     '\x06', 2);
	mRegisterData.add(CAtolFR::Registers::MoneyInCash,      '\x0A', 7);
	mRegisterData.add(CAtolFR::Registers::CurrentDateTime,  '\x11', 6);
	mRegisterData.add(CAtolFR::Registers::SessionInfo,      '\x12', 7);
	mRegisterData.add(CAtolFR::Registers::PrintingSettings, '\x18', 1);

	// команды
	using namespace CAtolFR::Commands;

	mCommandData.add(GetModelInfo,   true,  false, false);
	mCommandData.add(GetLongStatus,  false, true);
	mCommandData.add(GetShortStatus, false, true);
	mCommandData.add(EnterToMode,    true,  true);

	mCommandData.add(GetFRRegister,  30 * 1000);
	mCommandData.add(OpenDocument,    5 * 1000);
	mCommandData.add(CloseDocument,  20 * 1000);
	mCommandData.add(Encashment,     10 * 1000);
	mCommandData.add(CancelDocument, 20 * 1000);
	mCommandData.add(ZReport,        30 * 1000);
	mCommandData.add(ExitFromMode,   10 * 1000);
	mCommandData.add(OpenFRSession,  10 * 1000);

	// ошибки
	mUnprocessedErrorData.add(CAtolFR::Commands::GetFRParameter, CAtolFR::Errors::WrongSeriesNumber);

	// налоги
	mTaxData.add(10, 2);
	mTaxData.add(18, 3);
	mTaxData.add( 0, 4);
}

//--------------------------------------------------------------------------------
void AtolFRBase::setInitialData()
{
	TSerialFRBase::setInitialData();

	mFRBuild = 0;
}

//--------------------------------------------------------------------------------
QDateTime AtolFRBase::getDateTime()
{
	QByteArray data;

	if (getLongStatus(data))
	{
		QString dateDataTime = data.mid(3, 6).toHex().prepend("20");

		return QDateTime::fromString(dateDataTime, CAtolFR::DateTimeFormat);
	}

	return QDateTime();
}

//--------------------------------------------------------------------------------
bool AtolFRBase::checkTaxValue(TVAT aVAT, CFR::Taxes::SData & aData, const CAtolFR::FRParameters::TData & aFRParameterData, bool aCanCorrectTaxValue)
{
	QByteArray taxData;

	if (!getFRParameter(aFRParameterData(aData.group), taxData))
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to get tax value for %1 tax group").arg(aData.group));
		return false;
	}

	bool OK;
	int FRValue = TVAT(taxData.toHex().toInt(&OK));

	if (!OK)
	{
		FRValue = -1;
	}
	else
	{
		aData.deviceVAT = FRValue / 100;
	}

	int value = int(aVAT * 100);

	if (value != FRValue)
	{
		QString log = QString("%1% (%2 tax group)").arg(value/100.0, 5, 'f', 2, ASCII::Zero).arg(aData.group);

		if (!aCanCorrectTaxValue)
		{
			toLog(LogLevel::Error, mDeviceName + ": Cannot to set tax value = " + log);
			return false;
		}
		else if (!setFRParameter(aFRParameterData(aData.group), getBCD(double(aVAT) * 100.0, 2)))
		{
			toLog(LogLevel::Error, mDeviceName + ": Failed to set tax value = " + log);
			return false;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
bool AtolFRBase::updateParameters()
{
	exitInnerMode();
	processDeviceData();

	setConfigParameter(CHardware::Printer::FeedingAmount, mModelData.feedingAmount);

	// если у нас Меркурий - открываем руками смену
	if ((mDeviceName == CAtolFR::Models::Mercury140F) && !openFRSession())
	{
		toLog(LogLevel::Error, "AtolFR: Failed to open fiscal session");
		return false;
	}

	// программируем параметры модели
	if (!enterInnerMode(CAtolFR::InnerModes::Programming) || !setFRParameters())
	{
		toLog(LogLevel::Error, "AtolFR: Failed to set FR parameters");
		return false;
	}

	if (!getPrintingSettings())
	{
		return false;
	}

	if (!isFiscal())
	{
		return true;
	}

	TLoadSectionName loadSectionName = [&] (int aIndex, QByteArray & aData) -> bool { return getFRParameter(CAtolFR::FRParameters::SectionName(aIndex), aData); };

	if (mOperatorPresence && !loadSectionNames(loadSectionName))
	{
		return false;
	}

	mZBufferError = !enterExtendedMode();

	return checkTaxes();
}

//--------------------------------------------------------------------------------
void AtolFRBase::finaliseInitialization()
{
	if (!mCanProcessZBuffer)
	{
		enterInnerMode(CAtolFR::InnerModes::Register);
	}

	exitInnerMode();

	TSerialFRBase::finaliseInitialization();
}

//--------------------------------------------------------------------------------
bool AtolFRBase::isConnected()
{
	QByteArray answer;

	if (!processCommand(CAtolFR::Commands::GetModelInfo, &answer))
	{
		return false;
	}

	CAtolFR::TModelKey modelKey = getModelKey(answer);
	CAtolFR::CModelData modelData;

	if (!modelData.data().keys().contains(modelKey))
	{
		toLog(LogLevel::Error, "AtolFR: Unknown model");
	}

	mModelData = modelData[modelKey];
	mDeviceName = mModelData.name;
	mVerified = mModelData.verified;
	mCanProcessZBuffer = mModelData.ZBufferSize;
	mModelCompatibility = mSupportedModels.contains(mDeviceName);
	setConfigParameter(CHardware::Printer::NeedCutting, mModelData.cutter);

	return true;
}

//--------------------------------------------------------------------------------
bool AtolFRBase::getPrintingSettings()
{
	if ((mDeviceName == CAtolFR::Models::TriumF)      ||
		(mDeviceName == CAtolFR::Models::FelixRF)     ||
		(mDeviceName == CAtolFR::Models::Felix02K)    ||
		(mDeviceName == CAtolFR::Models::Mercury140F) ||
		(mDeviceName == CAtolFR::Models::Mercury130)  ||
		(mDeviceName == CAtolFR::Models::MicroFR01K)  ||
		(mDeviceName == CAtolFR::Models::Flaton11K))
	{
		toLog(LogLevel::Normal, mDeviceName + ": Cannot get printing settings");
		return true;
	}

	QByteArray data;

	if (!getFRParameter(CAtolFR::FRParameters::PrintingSettings, data))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to get get printing settings");
		return true;
	}

	int FRTypeLength = data.toHex().toInt();
	int modelTypeLength = mModelData.maxStringSize;
	mLineSize = ((FRTypeLength <= modelTypeLength) && FRTypeLength) ? FRTypeLength : modelTypeLength;

	return true;
}

//--------------------------------------------------------------------------------
QByteArray AtolFRBase::getBCD(double aValue, int aSize, int aPrecision, int aMantissa) const
{
	qint64 factor = qint64(qPow(10.0, abs(aPrecision + 1)));
	aValue = (aPrecision >= 0) ? (double(qRound64(aValue * factor)) / factor) : (qint64(aValue / factor) * factor);

	aMantissa = aMantissa > aPrecision ? aMantissa : aPrecision;
	qint64 value = qRound64(aValue * qPow(10.0, aMantissa));
	QString stringValue = QString("%1").arg(value, aSize * 2, 10, QChar(ASCII::Zero));

	QByteArray result;

	for (int i = 0; i < aSize; ++i)
	{
		result.append(uchar(stringValue.mid(2 * i, 2).toInt(0, 16)));
	}

	return result;
}

//--------------------------------------------------------------------------------
TResult AtolFRBase::execCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer)
{
	// для пересчета ошибки виртуального конца буфера Z-отчетов:
	// если открываем чек, либо делаем выплату (- операции, открывающие смену) и сессия сейчас закрыта, то
	// после открытия чека она будет открыта открытием чека. Тогда потом просто обновим время последнего открытия смены.
	bool isSessionInZBufferOpened = false;

	if (mCanProcessZBuffer && ((aCommand[0] == CAtolFR::Commands::OpenDocument) || (aCommand[0] == CAtolFR::Commands::Encashment)))
	{
		isSessionInZBufferOpened = getSessionState() == ESessionState::Closed;
	}

	QByteArray commandData = aCommand + aCommandData;
	QByteArray answer;
	mLastCommandResult = performCommand(commandData, answer, mCommandData[aCommand].timeout);

	if ((mLastCommandResult != CommandResult::Transport) && aAnswer)
	{
		*aAnswer = answer;
	}

	if (!mLastCommandResult)
	{
		return mLastCommandResult;
	}

	CAtolFR::SCommadData commandInfo = mCommandData[aCommand];
	int minAnswerSize = int(commandInfo.error) + int(commandInfo.prefix);

	if (answer.size() < minAnswerSize)
	{
		toLog(LogLevel::Error, "AtolFR: Data in packet is less than " + QString::number(minAnswerSize));
		return CommandResult::Answer;
	}

	char error = getError(aCommand, answer);

	if (uchar(error) < CAtolFR::MinErrorCode)
	{
		// смена открыта, а ранее она была закрыта - обновляем метку времени открытия смены
		if (isSessionInZBufferOpened)
		{
			mLastOpenSession = QDateTime::currentDateTime();
		}

		return CommandResult::OK;
	}

	mLastError = error;
	mLastCommand = aCommand;

	toLog(LogLevel::Error, mDeviceName + ": Error: " + mErrorData->value(mLastError).description);

	if (!isErrorUnprocessed(aCommand, mLastError))
	{
		setErrorFlags(aCommand, mLastError);
	}

	if (!mProcessingErrors.isEmpty() && (mProcessingErrors.last() == mLastError))
	{
		return CommandResult::Device;
	}

	if (isErrorUnprocessed(aCommand, error) || !processAnswer(aCommand, error))
	{
		mLastError = error;
		mLastCommand = aCommand;

		return CommandResult::Device;
	}

	TResult result = processCommand(aCommand, aCommandData, aAnswer);

	if (result)
	{
		mProcessingErrors.pop_back();
	}

	return result;
}

//--------------------------------------------------------------------------------
char AtolFRBase::getError(char aCommand, const QByteArray & aAnswer)
{
	return getError(QByteArray(1, aCommand), aAnswer);
}

//--------------------------------------------------------------------------------
char AtolFRBase::getError(const QByteArray & aCommand, const QByteArray & aAnswer)
{
	CAtolFR::SCommadData commandData = mCommandData[aCommand];

	     if (!commandData.error) return ASCII::NUL;
	else if (commandData.prefix) return aAnswer[1];

	return aAnswer[0];
}

//--------------------------------------------------------------------------------
void AtolFRBase::setErrorFlags(const QByteArray & /*aCommand*/, char aError)
{
	if (aError == CAtolFR::Errors::PrinterHeadOverheat)
	{
		mPrinterCollapse = true;
	}
}

//--------------------------------------------------------------------------------
bool AtolFRBase::openSession()
{
	char mode = mMode;

	if (!enterInnerMode(CAtolFR::InnerModes::Register))
	{
		return false;
	}

	QByteArray commandData(1, CAtolFR::FiscalFlags::ExecutionMode);
	bool result = processCommand(CAtolFR::Commands::OpenFRSession, commandData);
	enterInnerMode(mode);

	return result;
}

//--------------------------------------------------------------------------------
bool AtolFRBase::processAnswer(const QByteArray & aCommand, char aError)
{
	switch (aError)
	{
		// эта модель не может выполнить эту команду.
		case CAtolFR::Errors::CannotExecCommand :
		case CAtolFR::Errors::BadModeForCommand :
		{
			/*
			Если еще не было ни 1 запроса статуса (значит, идет инициализация), но
			не можем выполнить команду, значит, что-то не так либо с ЭКЛЗ, либо с ФП, и ККМ фискализирована.
			Последующие запросы статуса должны прояснить ситуацию.
			*/
			if ((aError == CAtolFR::Errors::BadModeForCommand) &&
			    (mMode    == CAtolFR::InnerModes::NoMode) &&
			    (mSubmode == CAtolFR::InnerSubmodes::NoSubmode))
			{
				mFiscalized = true;
				mFiscalCollapse = true;
			}

			//если режим - не выбор, поэтому, возможно, не выполнили команду, т.к. уже находимся в режиме
			//тогда выходим из режима и снова заходим в нужный режим
			if ((aCommand[0] == CAtolFR::Commands::EnterToMode) ||
				(aCommand[0] == CAtolFR::Commands::PrintString) ||
				(aCommand[0] == CAtolFR::Commands::Cut))
			{
				mProcessingErrors.append(aError);

				return exitInnerMode(true);
			}

			//TODO: разобраться со спецификой модели Феликс80к, сделать завязку на эту модель
			break;
		}
		//--------------------------------------------------------------------------------
		// необходимо закрыть чек - отменяем
		case CAtolFR::Errors::NeedCloseDocument :
		case CAtolFR::Errors::NeedCloseSaleDocument :
		{
			mProcessingErrors.append(aError);

			return processCommand(CAtolFR::Commands::CancelDocument);
		}
		//--------------------------------------------------------------------------------
		// со времени открытия смены прошло более 24 часа - надо закрыть смену
		//TODO: проверить на предмет обработки ошибок выполнения отдельных команд
		//TODO: для Меркурия-140Ф надо руками открывать смену, автооткрытия по первому чеку у него нет.
		//самое удобное - сделать это сразу после команды закрытия смены
		case CAtolFR::Errors::NeedZReport :
		case CAtolFR::Errors::NeedCloseSession :
		{
			mProcessingErrors.append(aError);

			return execZReport(true);
		}
		//--------------------------------------------------------------------------------
		// нет бамаги - это совсем не значит, что бумаги нет. Это может означать, к примеру, что забился буфер Z-отчетов.
		// или бумага замялась. или головка перегрелась. или еще что-то...
		case CAtolFR::Errors::NoPaper :
		{
			if (!mCommandData[aCommand].status)
			{
				simplePoll();

				if ((aCommand[0] == CAtolFR::Commands::ZReport) && mCanProcessZBuffer)
				{
					bool & ZBufferFlag = (getSessionState() == ESessionState::Expired) ? mZBufferOverflow : mZBufferFull;
					ZBufferFlag = true;
				}
				else
				{
					mProcessingErrors.append(aError);
				}
			}
			else
			{
				mProcessingErrors.append(aError);
			}

			break;
		}
		//--------------------------------------------------------------------------------
		// если не смогли открыть чек с ошибкой "Вход в режим заблокирован" - ФР залочен
		case CAtolFR::Errors::EnterToModeIsLocked :
		{
			mLocked = mLocked || (aCommand[0] == CAtolFR::Commands::OpenDocument);

			break;
		}
	}

	return false;
}

//---------------------------------------------------------------------------
bool AtolFRBase::checkTaxes()
{
	char mode = mMode;

	if (!enterInnerMode(CAtolFR::InnerModes::Programming))
	{
		return false;
	}

	bool result = TSerialFRBase::checkTaxes();
	enterInnerMode(mode);

	return result;
}

//--------------------------------------------------------------------------------
bool AtolFRBase::printLine(const QByteArray & aString)
{
	if (mProcessingErrors.contains(CAtolFR::Errors::NoPaper))
	{
		return false;
	}

	if (!processCommand(CAtolFR::Commands::PrintString, aString))
	{
		toLog(LogLevel::Error, "AtolFR: Failed to process print line command for AtolFR");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool AtolFRBase::processReceipt(const QStringList & aReceipt, bool aProcessing)
{
	bool result = TSerialFRBase::processReceipt(aReceipt, aProcessing);

	if (aProcessing)
	{
		SleepHelper::msleep(CAtolFR::Timeouts::EndNotFiscalPrint);
	}

	return result;
}

//--------------------------------------------------------------------------------
bool AtolFRBase::getCommonStatus(TStatusCodes & aStatusCodes)
{
	if (!getShortStatus(aStatusCodes))
	{
		return false;
	}

	QByteArray data = performStatus(aStatusCodes, CAtolFR::Commands::GetLongStatus, 9);

	if (data == CFR::Result::Fail)
	{
		return false;
	}
	else if ((data != CFR::Result::Error) && (data[9] & CAtolFR::States::CoverIsOpened))
	{
		aStatusCodes.insert(DeviceStatusCode::Error::CoverIsOpened);
	}

	return true;
}

//--------------------------------------------------------------------------------
bool AtolFRBase::getStatus(TStatusCodes & aStatusCodes)
{
	// если находимся в ошибке сбоя буфера Z-отчета - пытаемся исправиться
	// на случай, если принтер отваливается, но потом опять появляется, не используем последние статусы
	if (mZBufferError)
	{
		mZBufferError = !enterExtendedMode();
	}

	if (mCanProcessZBuffer)
	{
		checkZBufferState();

		// если Z-буфер есть и уже заполнен - запрашиваем время, чтобы понять - переполнен ли он.
		if (mZBufferFull && !mZBufferOverflow)
		{
			int sessionInterval = mLastOpenSession.secsTo(QDateTime::currentDateTime()) / 60;
			mZBufferOverflow = sessionInterval >= (24 * 60 - CPrinters::ZBufferVirtualOverflow);
		}
	}

	if (!getCommonStatus(aStatusCodes))
	{
		return false;
	}

	bool buildOK = !mFRBuild || (mFRBuild >= mModelData.build);

	if (!buildOK)
	{
		aStatusCodes.insert(DeviceStatusCode::Warning::Firmware);
	}

	if (mProcessingErrors.contains(CAtolFR::Errors::NeedZReport))
	{
		if (!mCanProcessZBuffer)
		{
			mNeedCloseSession = true;
		}
		else if (!mWhiteSpaceZBuffer)
		{
			mZBufferOverflow = true;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
void AtolFRBase::execTags(Tags::SLexeme & aTagLexeme, QVariant & aLine)
{
	QByteArray data = mCodec->fromUnicode(aTagLexeme.data);

	if (aTagLexeme.tags.contains(Tags::Type::DoubleWidth))
	{
		Tags::TTypes types;
		types.insert(Tags::Type::DoubleWidth);

		for (int i = 0; i < data.size(); i = i + 2)
		{
			data.insert(i, mTagEngine->getTag(types, Tags::Direction::Open));
		}
	}

	aLine = aLine.toByteArray() + data;

	//TODO: реализовать теги командой Печать поля (87h):
	// для PayVKP-80   : inverse, bold и underline
	// для PayPPU-700  : inverse, bold
	// для PayCTS-2000 : inverse
}

//--------------------------------------------------------------------------------
bool AtolFRBase::performFiscal(const QStringList & aReceipt, const SPaymentData & aPaymentData, quint32 * /*aFDNumber*/)
{
	if (!enterInnerMode(CAtolFR::InnerModes::Register) || (!openDocument(aPaymentData.payOffType) && !mLocked))
	{
		return false;
	}

	bool result = processReceipt(aReceipt, false);

	if (result)
	{
		foreach (auto unitData, aPaymentData.unitDataList)
		{
			result = result && sale(unitData);
		}

		if (result)
		{
			result = setOFDParameters() && closeDocument(aPaymentData.payType);
		}
		else if (aPaymentData.back() && (mLastError == CAtolFR::Errors::NoMoneyForPayout))
		{
			emitStatusCode(FRStatusCode::Error::NoMoney, EFRStatus::NoMoneyForSellingBack);
		}
	}

	if (!result)
	{
		processCommand(CAtolFR::Commands::CancelDocument);
	}

	exitInnerMode();

	return result;
}

//--------------------------------------------------------------------------------
bool AtolFRBase::isFiscalReady(bool aOnline, EFiscalPrinterCommand::Enum aCommand)
{
	if (!TSerialFRBase::isFiscalReady(aOnline, aCommand))
	{
		return false;
	}
	else if (aCommand == EFiscalPrinterCommand::Encashment)
	{
		int sessionInterval = mLastOpenSession.secsTo(QDateTime::currentDateTime()) / 60;

		if (sessionInterval >= 24 * 60)
		{
			return false;
		}

		MutexLocker locker(&mExternalMutex);

		return enterInnerMode(CAtolFR::InnerModes::Register);
	}

	return true;
}

//--------------------------------------------------------------------------------
double AtolFRBase::getAmountInCash()
{
	QByteArray data;

	if (!getRegister(CAtolFR::Registers::MoneyInCash, data))
	{
		return -1;
	}

	bool OK;
	double result = data.toHex().toInt(&OK) / 100.0;

	return OK ? result : -1;
}

//--------------------------------------------------------------------------------
bool AtolFRBase::processPayout(double aAmount)
{
	char innerMode = mMode;

	if (!enterInnerMode(CAtolFR::InnerModes::Register))
	{
		return false;
	}

	QByteArray commandData;
	commandData.append(CAtolFR::FiscalFlags::ExecutionMode);
	commandData.append(getBCD(aAmount / 10.0, 5, 2, 3));

	bool result = processCommand(CAtolFR::Commands::Encashment, commandData);

	enterInnerMode(innerMode);

	return result;
}

//--------------------------------------------------------------------------------
bool AtolFRBase::cut()
{
	return processCommand(CAtolFR::Commands::Cut, QByteArray(1, CAtolFR::FullCutting));
}

//--------------------------------------------------------------------------------
bool AtolFRBase::printDeferredZReports()
{
	if (!exitInnerMode())
	{
		return false;
	}

	int command = getConfigParameter(CHardware::FR::Commands::PrintingDeferredZReports).toInt();

	if (!processCommand(char(command)))
	{
		toLog(LogLevel::Error, "AtolFR: Failed to print deferred Z-reports");
		return false;
	}

	if (!waitForChangeZReportMode())
	{
		toLog(LogLevel::Error, "AtolFR: Waiting error when printing deferred Z-reports");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool AtolFRBase::performZReport(bool aPrintDeferredReports)
{
	toLog(LogLevel::Normal, "AtolFR: Processing Z-report");

	bool printDeferredZReportSuccess = true;

	// если ККМ работает в расширенном режиме - печатаем отложенные Z-отчеты
	if (mCanProcessZBuffer && aPrintDeferredReports)
	{
		toLog(LogLevel::Normal, "AtolFR: Printing deferred Z-reports");

		printDeferredZReportSuccess = printDeferredZReports();
	}

	bool ZReportSuccess = execZReport(false);

	// ошибки наполнения и переполнения буфера Z-Отчетов можно сбросить
	checkZBufferState();

	exitInnerMode();

	bool result = ZReportSuccess || (printDeferredZReportSuccess && aPrintDeferredReports);

	if ((ZReportSuccess || (printDeferredZReportSuccess && aPrintDeferredReports)) || (mWhiteSpaceZBuffer > 1))
	{
		mZBufferOverflow = false;
	}

	return result;
}

//--------------------------------------------------------------------------------
bool AtolFRBase::openDocument(EPayOffTypes::Enum aPayOffType)
{
	QByteArray commandData;
	commandData.append(CAtolFR::FiscalFlags::ExecutionMode);
	commandData.append(CAtolFR::PayOffTypeData[aPayOffType]);

	if (processCommand(CAtolFR::Commands::OpenDocument, commandData))
	{
		return true;
	}

	if (mLocked)
	{
		toLog(LogLevel::Normal, "AtolFR: FR is locked.");
		return false;
	}

	EFiscalDocumentState::Enum state;

	if (!getFiscalDocumentState(state) || (state != EFiscalDocumentState::Opened))
	{
		toLog(LogLevel::Error, "AtolFR: Failed to open document.");
		return false;
	}

	toLog(LogLevel::Normal, "AtolFR: There was an error at the check opening, but the document is opened");

	return true;
}

//--------------------------------------------------------------------------------
bool AtolFRBase::closeDocument(EPayTypes::Enum aPayType)
{
	QByteArray commandData;
	commandData.append(CAtolFR::FiscalFlags::ExecutionMode);
	commandData.append(mPayTypeData[aPayType].value);
	commandData.append(QByteArray(5, ASCII::NUL));    // сумма = 0, т.к. сдачу терминал не дает

	if (!processCommand(CAtolFR::Commands::CloseDocument, commandData))
	{
		EFiscalDocumentState::Enum state;

		if (!getFiscalDocumentState(state) || (state != EFiscalDocumentState::Closed))
		{
			toLog(LogLevel::Error, "AtolFR: Failed to close document");
			cancelDocument(true);

			return false;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
bool AtolFRBase::sale(const SUnitData & aUnitData)
{
	if (!aUnitData.name.isEmpty())
	{
		printLine(mCodec->fromUnicode(aUnitData.name));
	}

	if (!aUnitData.VAT)
	{
		QString withoutTaxes = getConfigParameter(CHardware::FR::Strings::WithoutTaxes).toString();
		printLine(mCodec->fromUnicode(withoutTaxes));
	}

	int taxGroup = (aUnitData.section == -1) ? mTaxData[aUnitData.VAT].group : aUnitData.section;

	QByteArray commandData;
	commandData.append(CAtolFR::SaleFlags);                       // флаги
	commandData.append(getBCD(aUnitData.sum / 10.0, 5, 2, 3));    // сумма
	commandData.append(getBCD(10, 5, 2));                         // количество = 1 штука
	commandData.append(uchar(taxGroup));                          // отдел (== налоговая ставка)

	if (processCommand(CAtolFR::Commands::Sale, commandData))
	{
		return true;
	}

	toLog(LogLevel::Error, QString("%1: Failed to sale for %2 (%3, VAT = %4)").arg(mDeviceName).arg(aUnitData.sum, 0, 'f', 2).arg(aUnitData.name).arg(aUnitData.VAT));

	EFiscalDocumentState::Enum state = EFiscalDocumentState::Other;
	bool documentOpened = !getFiscalDocumentState(state) || (state == EFiscalDocumentState::Opened) || (state == EFiscalDocumentState::Sale);
	cancelDocument(documentOpened);

	return false;
}

//--------------------------------------------------------------------------------
bool AtolFRBase::setDocumentCapAmount(char aAmount)
{
	// войдем в режим программирования
	if (!enterInnerMode(CAtolFR::InnerModes::Programming))
	{
		return false;
	}

	// ставим количество строк шапки
	if (!setFRParameter(CAtolFR::FRParameters::DocumentCapStringsAmount, char(aAmount)))
	{
		toLog(LogLevel::Error, "AtolFR: Failed to set document cap strings amount");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
void AtolFRBase::cancelDocument(bool aDocumentIsOpened)
{
	bool result = true;

	//если фискальный документ открыт - пытаемся аннулировать. Не вышло или не открыт - проматываем, если надо, и режем
	if (aDocumentIsOpened)
	{
		//аннулируем фискальный документ
		result = processCommand(CAtolFR::Commands::CancelDocument);
	}

	if (!(aDocumentIsOpened && result))
	{
		receiptProcessing();
	}

	// выходим из режима
	exitInnerMode();
}

//--------------------------------------------------------------------------------
bool AtolFRBase::getLastSessionDT(QDateTime & aLastSessionDT)
{
	QByteArray data;

	// Делаем запрос состояния смены только если ЭКЛЗ активирована, иначе вернется мусор
	if (!mFiscalized || !getRegister(CAtolFR::Registers::SessionInfo, data))
	{
		return false;
	}

	QString dateDataTime = data.mid(1, 6).toHex().insert(4, "20");
	aLastSessionDT = QDateTime::fromString(dateDataTime, CAtolFR::SessionDTFormat);

	return aLastSessionDT.isValid();
}

//--------------------------------------------------------------------------------
void AtolFRBase::processDeviceData()
{
	setDeviceParameter(CDeviceData::ModelNumber, mModelData.modelNumber);

	// Вытаскиваем инфо по софта ФР и БЛ запросами версий софта
	CAtolFR::SSoftInfo softInfo;
	removeDeviceParameter(CDeviceData::Firmware);
	removeDeviceParameter(CDeviceData::BootFirmware);

	if (getSoftVersion(CAtolFR::FRSubSystems::FR, softInfo))
	{
		setDeviceParameter(CDeviceData::FR::Language, softInfo.language);
		setDeviceParameter(CDeviceData::Version, softInfo.version, CDeviceData::Firmware);
		setDeviceParameter(CDeviceData::Build, softInfo.build, CDeviceData::Firmware);
	}

	if (getSoftVersion(CAtolFR::FRSubSystems::BL, softInfo))
	{
		setDeviceParameter(CDeviceData::Version, softInfo.version, CDeviceData::BootFirmware);
		setDeviceParameter(CDeviceData::Build, softInfo.build, CDeviceData::BootFirmware);
	}

	//запрашиваем статус для получения режима.подрежима
	TStatusCodes statusCodes;
	getShortStatus(statusCodes);
	QByteArray data;

	QDateTime FRDateTime = getDateTime();

	if (FRDateTime.isValid())
	{
		setDeviceParameter(CDeviceData::FR::CurrentDate, FRDateTime.toString(CFR::DateTimeLogFormat));
	}

	if (getRegister(CAtolFR::Registers::SerialNumber, data))
	{
		mSerial = CFR::serialToString(data.toHex());
	}

	removeDeviceParameter(CDeviceData::FR::Session);
	QDateTime lastSessionDT;

	if (getLastSessionDT(lastSessionDT) && getLongStatus(data))
	{
		QString sessionDT = lastSessionDT.toString(CFR::DateTimeLogFormat);

		if (data[9] & CAtolFR::States::SessionOpened)
		{
			mLastOpenSession = lastSessionDT.addDays(-1);

			setDeviceParameter(CDeviceData::FR::Session, CDeviceData::Values::Opened);
			setDeviceParameter(CDeviceData::FR::FutureClosingDate, sessionDT, CDeviceData::FR::Session);
		}
		else
		{
			setDeviceParameter(CDeviceData::FR::Session, CDeviceData::Values::Closed);
			setDeviceParameter(CDeviceData::FR::LastClosingDate, sessionDT, CDeviceData::FR::Session);
		}
	}
}

//--------------------------------------------------------------------------------
bool AtolFRBase::setFRParameter(const CAtolFR::FRParameters::SData & aData, const QVariant & aValue)
{
	QByteArray commandData;
	commandData.append(aData.table);
	commandData.append(uchar(aData.series << 8));
	commandData.append(uchar(aData.series << 0));
	commandData.append(aData.field);

	     if (aValue.type() == QVariant::ByteArray) commandData.append(aValue.toByteArray());
	else if (aValue.type() == QVariant::String) commandData.append(mCodec->fromUnicode(aValue.toString()));
	else commandData.append(char(aValue.toInt()));

	if (!processCommand(CAtolFR::Commands::SetFRParameters, commandData))
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to set %1 = %2").arg(aData.log()).arg(aValue.toString()));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool AtolFRBase::setFRParameters()
{
	if (!setFRParameter(CAtolFR::FRParameters::TaxType, CAtolFR::CustomSaleTax) ||    // тип налога - на каждую покупку
		!setFRParameter(CAtolFR::FRParameters::Cut, true)                       ||    // автоотрезка фискального чека - да. Нефискальный - режем сами
		!setFRParameter(CAtolFR::FRParameters::PrintNotFiscalData, true))             // печатать текст командой Печать строки - да
	{
		return false;
	}

	QByteArray data;

	if (!getFRParameter(CAtolFR::FRParameters::ReportMode, data))
	{
		return false;
	}

	char FRReportMode = data[0];
	char reportMode = CAtolFR::ReportMode;
	QString nullingSumInCash = getConfigParameter(CHardwareSDK::FR::NullingSumInCash).toString();

	if (nullingSumInCash == CHardwareSDK::Values::Auto)
	{
		reportMode &= ~CAtolFR::NullingSumInCashMask;
		reportMode |= FRReportMode & CAtolFR::NullingSumInCashMask;
	}
	else if (nullingSumInCash == CHardwareSDK::Values::Use)
	{
		reportMode |= CAtolFR::NullingSumInCashMask;
	}
	else if (nullingSumInCash == CHardwareSDK::Values::NotUse)
	{
		reportMode &= ~CAtolFR::NullingSumInCashMask;
	}

	if (mOperatorPresence)
	{
		reportMode |= CAtolFR::LongReportMask;
	}

	if (reportMode != FRReportMode)
	{
		// выставляем параметры формата печати Z-отчета
		setFRParameter(CAtolFR::FRParameters::ReportMode, reportMode);
	}

	// размеры "ИТОГ" на чеке
	setFRParameter(CAtolFR::FRParameters::ResumeSize, CAtolFR::ResumeSize);

	// печатать имя кассира
	setFRParameter(CAtolFR::FRParameters::PrintCashier, true);

	// сквозная нумерация на чеке - нет
	setFRParameter(CAtolFR::FRParameters::ContinuousDocumentNumbering, false);

	// обнулять счетчик чеков при закрытии смены - нет
	setFRParameter(CAtolFR::FRParameters::AutoNullingChequeCounter, false);

	// печатать номер секции - нет
	setFRParameter(CAtolFR::FRParameters::PrintSectionNumber, false);

	// печатать название секции - в зависимости от наличия оператора
	setFRParameter(CAtolFR::FRParameters::PrintSectionName, mOperatorPresence);

	// печатать документ открытия смены - нет
	setFRParameter(CAtolFR::FRParameters::OpeningSessionDocument, false);

	// количество строк шапки
	setFRParameter(CAtolFR::FRParameters::DocumentCapStringsAmount, mDocumentCapStrings);

	// межстрочный интервал
	if (mModelData.lineSpacing && containsConfigParameter(CHardware::Printer::Settings::LineSpacing))
	{
		int linespacing = getConfigParameter(CHardware::Printer::Settings::LineSpacing).toInt();
		setFRParameter(CAtolFR::FRParameters::LineSpacing, char(linespacing));
	}

	return true;
}

//--------------------------------------------------------------------------------
bool AtolFRBase::getFRParameter(const CAtolFR::FRParameters::SData & aData, QByteArray & aValue)
{
	QByteArray commandData;
	commandData.append(aData.table);
	commandData.append(uchar(aData.series << 8));
	commandData.append(uchar(aData.series << 0));
	commandData.append(aData.field);

	if (!processCommand(CAtolFR::Commands::GetFRParameter, commandData, &aValue))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to get " + aData.log());
		return false;
	}

	aValue = aValue.mid(2);

	return true;
}

//--------------------------------------------------------------------------------
TResult AtolFRBase::getRegister(const QString & aRegister, QByteArray & aData, char aParameter1, char aParameter2)
{
	CAtolFR::Registers::SData registerData = mRegisterData[aRegister];
	toLog(LogLevel::Normal, mDeviceName + QString(": Begin to get FR register %1 (%2)").arg(toHexLog(registerData.number)).arg(aRegister));

	QByteArray commandData;
	commandData.append(registerData.number);
	commandData.append(aParameter1);
	commandData.append(aParameter2);

	TResult result = processCommand(CAtolFR::Commands::GetFRRegister, commandData, &aData);

	if (!CORRECT(result) || (result == CommandResult::Device))
	{
		return result;
	}

	if (aData.size() < (2 + registerData.size))
	{
		toLog(LogLevel::Error, mDeviceName + QString(": invalid answer size for register %1: %2, need %3 minimum")
			.arg(toHexLog(registerData.number)).arg(aData.size()).arg(2 + registerData.size));
		return CommandResult::Answer;
	}

	aData = aData.mid(2);

	return CommandResult::OK;
}

//--------------------------------------------------------------------------------
bool AtolFRBase::waitForChangeXReportMode()
{
	//запускаем цикл опроса ККМ, ставим таймер на 20 с
	//(код состояния = 2.2 - печатается X отчет)
	QTime clockTimer;
	clockTimer.start();

	do
	{
		QTime clock = QTime::currentTime();
		TStatusCodes statusCodes;

		// Получаем статус, пауза 0.5 секунды
		if (getShortStatus(statusCodes))
		{
			if (getStatusCollection(statusCodes).size(EWarningLevel::Error))
			{
				// при распечатке отчета произошла ошибка, выходим с ошибкой
				toLog(LogLevel::Error, "AtolFR: unexpected error occured when printing Z-report, exit!");
				return false;
			}
			else if ((mMode == CAtolFR::InnerModes::Choice) ||
			        ((mMode == CAtolFR::InnerModes::NotCancel) && (mSubmode == 0)))
			{
				// все нормально, выходим
				return true;
			}
			else if ((mMode == CAtolFR::InnerModes::NotCancel) && (mSubmode == 2))
			{
				toLog(LogLevel::Normal, "AtolFR: service X-report process, wait...");
			}
			else
			{
				// режим не тот, который ожидаем в соответствии с протоколом, выходим с ошибкой
				toLog(LogLevel::Error, QString("AtolFR: X-report, unknown mode.submode = %1.%2").arg(int(mMode)).arg(int(mSubmode)));
				return false;
			}
		}

		//спим до периода опроса
		int sleepTime = CAtolFR::Timeouts::XReportPoll - abs(clock.msecsTo(QTime::currentTime()));

		if (sleepTime > 0)
		{
			SleepHelper::msleep(sleepTime);
		}
	}
	while(clockTimer.elapsed() < CAtolFR::Timeouts::XReportNoAnswer);

	//вышли по таймауту, значит, не смогли дождаться нужного режима/подрежима
	return false;
}

//--------------------------------------------------------------------------------
bool AtolFRBase::waitForChangeZReportMode()
{
	//запускаем цикл опроса ККМ, ставим таймер на 20 с
	//(код состояния = 3.2 - печатается Z отчет) -> (код состояния = 7.1 - гасятся регистры)
	QTime clockTimer;
	clockTimer.start();

	do
	{
		QTime clock = QTime::currentTime();
		TStatusCodes statusCodes;

		// Получаем статус, пауза 0.5 секунды
		if (getShortStatus(statusCodes))
		{
			if (getStatusCollection(statusCodes).size(EWarningLevel::Error))
			{
				// при распечатке отчета(ов) произошла ошибка, выходим с ошибкой
				toLog(LogLevel::Error, "AtolFR: unexpected error occured when printing Z-report, exit!");
				return false;
			}
			else if ((mMode == CAtolFR::InnerModes::Choice) ||
			        ((mMode == CAtolFR::InnerModes::Cancel) && (mSubmode == 0)))
			{
				// все нормально, выходим
				return true;
			}
			else if (((mMode == CAtolFR::InnerModes::ExtraCommand) && (mSubmode == 1)) ||
			        (((mMode == CAtolFR::InnerModes::Cancel) ||
			          (mMode == CAtolFR::InnerModes::NotCancel))   && (mSubmode == 2)))
			{
				toLog(LogLevel::Normal, "AtolFR: service Z-report process, wait...");
			}
			else
			{
				// режим не тот, который ожидаем в соответствии с протоколом, выходим с ошибкой
				toLog(LogLevel::Error, QString("AtolFR: Z-report, unknown mode.submode = %1.%2").arg(int(mMode)).arg(int(mSubmode)));
				return false;
			}
		}

		//спим до периода опроса
		int sleepTime = CAtolFR::Timeouts::ZReportPoll - abs(clock.msecsTo(QTime::currentTime()));

		if (sleepTime > 0)
		{
			SleepHelper::msleep(sleepTime);
		}
	}
	while(clockTimer.elapsed() < CAtolFR::Timeouts::ZReportNoAnswer);

	//вышли по таймауту, значит, не смогли дождаться нужного режима/подрежима
	return false;
}

//--------------------------------------------------------------------------------
bool AtolFRBase::enterInnerMode(char aInnerMode)
{
	// если хотим войти в такой же режим, в котором сейчас находимся - выходим
	if (mMode == aInnerMode)
	{
		return true;
	}
	else if (mMode)
	{
		bool result = exitInnerMode();

		// если хотим войти в режим выбора, либо не вышло выйти из режима - выходим
		if (!result || (aInnerMode == CAtolFR::InnerModes::Choice))
		{
			return result;
		}
	}

	toLog(LogLevel::Normal, QString("AtolFR: Entering to %1 mode").arg(int(aInnerMode)));

	QByteArray commandData;
	commandData.append(aInnerMode);
	commandData.append(ASCII::NUL);
	commandData.append(ASCII::NUL);
	commandData.append(ASCII::NUL);
	commandData.append(CAtolFR::Users::SysAdmin);

	bool result = processCommand(CAtolFR::Commands::EnterToMode, commandData);

	if (result)
	{
		mMode = aInnerMode;
	}

	return result;
}

//--------------------------------------------------------------------------------
bool AtolFRBase::exitInnerMode(bool aForce)
{
	//выходим, если выходить дальше некуда
	if (!aForce && (mMode == CAtolFR::InnerModes::Choice))
	{
		return true;
	}

	toLog(LogLevel::Normal, "AtolFR: Exiting from mode");

	if (!processCommand(CAtolFR::Commands::ExitFromMode))
	{
		return false;
	}

	mMode = CAtolFR::InnerModes::Choice;

	return true;
}

//--------------------------------------------------------------------------------
bool AtolFRBase::getSoftVersion(char aSoftType, CAtolFR::SSoftInfo & aSoftInfo)
{
	QByteArray answer;

	if (!processCommand(CAtolFR::Commands::GetSoftInfo, QByteArray(1, aSoftType), &answer))
	{
		return false;
	}

	aSoftInfo.version = QString("%1.%2")
		.arg(uchar(answer.mid(2, 1).toHex().toUShort()), 1, 10, QChar(ASCII::Zero))
		.arg(uchar(answer.mid(3, 1).toHex().toUShort()), 2, 10, QChar(ASCII::Zero));
	aSoftInfo.build = answer.mid(5, 2).toHex().toUShort();

	if (aSoftType == CAtolFR::FRSubSystems::FR)
	{
		aSoftInfo.language = CAtolFR::Languages[uchar(answer.mid(4, 1).toHex().toUShort())];

		if (!mIsOnline)
		{
			mFRBuild = aSoftInfo.build;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
bool AtolFRBase::processXReport()
{
	// запомнили режим
	char innerMode = mMode;

	// заходим в режим гашения. не можем зайти - что-то критичное, выходим
	if (!enterInnerMode(CAtolFR::InnerModes::NotCancel))
	{
		return false;
	}

	bool result = false;

	if (processCommand(CAtolFR::Commands::XReport, QByteArray(1, CAtolFR::Balances::XReport)))
	{
		result = waitForChangeXReportMode();
	}

	enterInnerMode(innerMode);

	return result;
}

//--------------------------------------------------------------------------------
bool AtolFRBase::execZReport(bool aAuto)
{
	toLog(LogLevel::Normal, mDeviceName + QString(": Begin processing %1Z-report").arg(aAuto ? "auto-" : ""));
	ESessionState::Enum sessionState = getSessionState();

	ExitAction exitAction([&] () { mZBufferError = !enterExtendedMode(); });

	     if (sessionState == ESessionState::Error)  return false;
	else if (sessionState == ESessionState::Closed) return true;

	bool cannotAutoZReport = !mCanProcessZBuffer || (mOperatorPresence && !getConfigParameter(CHardware::FR::ForcePerformZReport).toBool());

	if (aAuto && cannotAutoZReport)
	{
		toLog(LogLevel::Error, mDeviceName + (mOperatorPresence ?
			": Failed to process auto-Z-report due to presence of the operator." :
			": has no Z-buffer, so it is impossible to perform auto-Z-report."));

		mNeedCloseSession = mNeedCloseSession || (sessionState == ESessionState::Expired);

		return false;
	}

	char innerMode = mMode;

	if (!enterInnerMode(CAtolFR::InnerModes::Cancel))
	{
		return false;
	}

	QVariantMap outData;
	QByteArray data;
	char FDType = CAtolFR::PayOffTypeData[EPayOffTypes::Debit];

	if (getRegister(CAtolFR::Registers::PaymentCount, data, FDType))
	{
		outData.insert(CFiscalPrinter::PaymentCount, data.toHex().toUShort());
	}

	double paymentAmount = 0;

	if (getRegister(CAtolFR::Registers::PaymentAmount, data, FDType, CAtolFR::PaymentSource::Cash))
	{
		paymentAmount = qlonglong(data.toHex().toULongLong()) / 100.0;
		outData.insert(CFiscalPrinter::PaymentAmount, paymentAmount);
	}

	mNonNullableAmount += paymentAmount;
	outData.insert(CFiscalPrinter::NonNullableAmount, mNonNullableAmount);

	if (getLongStatus(data))
	{
		int closedSessionNumber = data.mid(20, 2).toHex().toUShort();
		outData.insert(CFiscalPrinter::ZReportNumber, closedSessionNumber + 1);
	}

	outData.insert(CFiscalPrinter::Serial, mSerial);
	outData.insert(CFiscalPrinter::RNM, mRNM);

	QDateTime FRDateTime = getDateTime();

	if (FRDateTime.isValid())
	{
		outData.insert(CFiscalPrinter::FRDateTime, FRDateTime.toString(CFR::DateTimeLogFormat));
	}

	QString systemDateTime = QDateTime::currentDateTime().toString(CFR::DateTimeLogFormat);
	outData.insert(CFiscalPrinter::SystemDateTime, systemDateTime);

	// делаем закрытие смены и ждем смены состояний
	mNeedCloseSession = false;
	bool success = processCommand(CAtolFR::Commands::ZReport) && waitForChangeZReportMode();
	mNeedCloseSession = getSessionState() == ESessionState::Expired;

	if (!mNeedCloseSession)
	{
		mProcessingErrors.removeAll(CAtolFR::Errors::NeedZReport);
	}

	if (success)
	{
		emit FRSessionClosed(outData);
	}

	enterInnerMode(innerMode);

	return success && exitAction.reset();
}

//--------------------------------------------------------------------------------
void AtolFRBase::checkZBufferState()
{
	if (mCanProcessZBuffer)
	{
		if (!mWhiteSpaceZBuffer)
		{
			mZBufferFull = true;
		}
		else if (mWhiteSpaceZBuffer > 1)
		{
			mZBufferFull = false;
		}
	}
}

//--------------------------------------------------------------------------------
ESessionState::Enum AtolFRBase::getSessionState()
{
	QByteArray data;

	if (!getLongStatus(data))
	{
		return ESessionState::Error;
	}

	if (~data[9] & CAtolFR::States::SessionOpened)
	{
		return ESessionState::Closed;
	}

	QString dateDataTime = data.mid(3, 6).toHex().prepend("20");
	QDateTime currentDT = QDateTime::fromString(dateDataTime, CAtolFR::DateTimeFormat);
	QDateTime lastSessionDT;

	if (!currentDT.isValid() || !getLastSessionDT(lastSessionDT))
	{
		return ESessionState::Error;
	}

	return (lastSessionDT.addDays(1) <= currentDT) ? ESessionState::Expired : ESessionState::Opened;
}

//--------------------------------------------------------------------------------
bool AtolFRBase::getFiscalDocumentState(EFiscalDocumentState::Enum & aState)
{
	QByteArray data;

	if (!getLongStatus(data))
	{
		return false;
	}

	char fiscalState = data[22] & 0x07;

	switch (fiscalState)
	{
		case  0: aState = EFiscalDocumentState::Closed;       break;
		case  1: aState = EFiscalDocumentState::Opened;       break;
		case  3: aState = EFiscalDocumentState::Cancellation; break;
		default: aState = EFiscalDocumentState::Other;        break;
	}

	qlonglong sum = data.mid(23, 5).toHex().toULongLong();

	if (sum && (aState == EFiscalDocumentState::Opened))
	{
		aState = EFiscalDocumentState::Sale;
	}

	return true;
}

//--------------------------------------------------------------------------------
EDocumentState::Enum AtolFRBase::getDocumentState()
{
	EFiscalDocumentState::Enum state;

	if (!getFiscalDocumentState(state))
	{
		return EDocumentState::Error;
	}

	return (state == EFiscalDocumentState::Closed) ? EDocumentState::Closed : EDocumentState::Opened;
}

//--------------------------------------------------------------------------------
bool AtolFRBase::getLongStatus(QByteArray & aData)
{
	return processCommand(CAtolFR::Commands::GetLongStatus, &aData) && (aData.size() >= 28);
}

//--------------------------------------------------------------------------------
bool AtolFRBase::getShortStatus(TStatusCodes & aStatusCodes)
{
	QByteArray data = performStatus(aStatusCodes, CAtolFR::Commands::GetShortStatus, 2);

	if (data == CFR::Result::Fail)
	{
		return false;
	}
	else if (data != CFR::Result::Error)
	{
		mMode    = (data[1] >> 0) & CAtolFR::ModeMask;
		mSubmode = (data[1] >> 4) & CAtolFR::ModeMask;

		parseShortStatusFlags(data[2], aStatusCodes);
	}

	return true;
}

//--------------------------------------------------------------------------------
void AtolFRBase::parseShortStatusFlags(char aFlags, TStatusCodes & aStatusCodes)
{
	for (auto it = CAtolFR::ShortFlags.data().begin(); it != CAtolFR::ShortFlags.data().end(); ++it)
	{
		if (aFlags & it.key())
		{
			aStatusCodes.insert(it.value());
		}
	}
}

//--------------------------------------------------------------------------------
