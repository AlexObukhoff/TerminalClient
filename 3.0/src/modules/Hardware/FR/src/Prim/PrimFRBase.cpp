/* @file Принтеры семейства ПРИМ. */

// Modules
#include "Hardware/Printers/POSPrinterData.h"
#include "Hardware/Protocols/FR/PrimFR.h"

// Project
#include "PrimFRBase.h"

using namespace PrinterStatusCode;
using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//--------------------------------------------------------------------------------
PrimFRBase::PrimFRBase() : mMode(EFRMode::Fiscal)
{
	// теги
	mTagEngine = Tags::PEngine(new CPrimFR::TagEngine());

	// данные моделей
	mDeviceName = CPrimFR::DefaultModelName;
	mModels = CPrimFR::CommonModels();
	mModel = CPrimFR::Models::Unknown;
	mOffline = true;

	setConfigParameter(CHardware::Printer::Commands::Cutting, CPOSPrinter::Command::Cut);
	mErrorData = PErrorData(new CPrimFR::Errors::Specification);

	// типы оплаты
	mPayTypeData.add(EPayTypes::Cash,         0);
	mPayTypeData.add(EPayTypes::EMoney,       1);
	mPayTypeData.add(EPayTypes::PostPayment,  2);

	// данные команд
	mCommandTimouts.append(CPrimFR::Commands::SetFRParameters,  3 * 1000);
	mCommandTimouts.append(CPrimFR::Commands::SetEjectorAction, 3 * 1000);
	mCommandTimouts.append(CPrimFR::Commands::ZReport,          3 * 1000);
	mCommandTimouts.append(CPrimFR::Commands::AFD,             10 * 1000);

	// налоги
	mTaxData.add( 0, 0, "БЕЗ НДС");
	mTaxData.add(10, 4, "НДС 10%");
	mTaxData.add(18, 5, "НДС 18%");

	// данные порта
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR115200);   // preferable for work
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);     // default
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR4800);     // default after resetting to zero
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR57600);
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR38400);
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR19200);

	mPortParameters[EParameters::RTS].clear();
	mPortParameters[EParameters::RTS].append(ERTSControl::Disable);

	mPortParameters[EParameters::DTR].clear();
	mPortParameters[EParameters::DTR].append(EDTRControl::Handshake);

	mPortParameters[EParameters::Parity].append(EParity::No);
}

//--------------------------------------------------------------------------------
bool PrimFRBase::updateParameters()
{
	if (!mOperatorPresence && isFiscal() && !checkParameters())
	{
		return false;
	}

	if (!checkControlSettings() || !checkTaxes())
	{
		return false;
	}

	CPrimFR::TData commandData;
	QString payment = getConfigParameter(CHardware::FR::Strings::Payment).toString();
	commandData << mCodec->fromUnicode(payment) << " " << " ";

	// устанавливаем названия строк фискального чека
	if (!mOperatorPresence && isFiscal() && !commandData.isEmpty() && !processCommand(CPrimFR::Commands::SetFDTypeNames, commandData))
	{
		toLog(LogLevel::Error, "PrimPrinters: Failed to set fiscal receipt type name");
		return false;
	}

	//GetLastCVCNumber -> EDocument, для получения предыдущего чека

	return true;
}

//--------------------------------------------------------------------------------
bool PrimFRBase::checkParameters()
{
	CPrimFR::TData answer;

	if (!processCommand(CPrimFR::Commands::GetFRParameters, &answer) || (answer.size() < 8))
	{
		toLog(LogLevel::Error, "PrimPrinters: Failed to get FR parameters");
		return false;
	}

	ushort FRParameter1 = QString(answer[5]).toUShort(0, 16);
	ushort FRParameter2 = QString(answer[6]).toUShort(0, 16);
	ushort FRParameter3 = QString(answer[7]).toUShort(0, 16);

	ushort parameter1 = CPrimFR::Parameter1;
	ushort parameter2 = CPrimFR::Parameter2;
	ushort parameter3 = 0;

	using namespace CHardware::Printer;

	if (mIsOnline)
	{
		parameter1 &=  CPrimFR::Parameter1Mask;
		parameter1 |= ~CPrimFR::Parameter1Mask & FRParameter1;
	}

	QString printDocumentCap = getConfigParameter(Settings::DocumentCap).toString();
	parameter2 &= ~CPrimFR::NeedPrintFiscalCapMask;

	if ((printDocumentCap == CHardware::Values::Use) ||
	   ((printDocumentCap == CHardware::Values::NoChange) && (FRParameter2 & CPrimFR::NeedPrintFiscalCapMask)))
	{
		parameter2 |= CPrimFR::NeedPrintFiscalCapMask;
	}

	if (!getConfigParameter(RetractorEnable).toBool())
	{
		parameter3 = ushort(getConfigParameter(Settings::LineSpacing).toInt());
	}
	else if (getConfigParameter(Settings::NotTakenReceipt).toString() == Values::Retract)
	{
		parameter3 = ushort(getConfigParameter(Settings::LeftReceiptTimeout).toInt());
	}

	parameter3 = qToBigEndian(parameter3);

	if ((FRParameter1 == parameter1) &&
		(FRParameter2 == parameter2) &&
		(FRParameter3 == parameter3))
	{
		return true;
	}

	auto getCommandData = [] (ushort aData) -> QByteArray {return QString("%1").arg(aData, 4, 16, QChar(ASCII::Zero)).toUpper().toLatin1(); };
	CPrimFR::TData commandData = CPrimFR::TData()
		<< getCommandData(CPrimFR::Parameter1)  // т.к. старший байт - только 0-й бит
		<< getCommandData(parameter2)
		<< getCommandData(parameter3);

	if (!processCommand(CPrimFR::Commands::SetFRParameters, commandData))
	{
		toLog(LogLevel::Error, "PrimPrinters: Failed to set FR parameters");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool PrimFRBase::checkControlSettings()
{
	CPrimFR::TData answer;

	if (!processCommand(CPrimFR::Commands::GetFRControlSettings, &answer) || (answer.size() < 11))
	{
		toLog(LogLevel::Error, "PrimPrinters: Failed to get control settings");
		return false;
	}

	if (bool(answer[10].toInt()) == CPrimFR::DateTimeInCommand)
	{
		return true;
	}

	CPrimFR::TData commandData = answer.mid(6);
	commandData[4] = int2ByteArray(CPrimFR::DateTimeInCommand);

	if (!processCommand(CPrimFR::Commands::FRControl, commandData))
	{
		toLog(LogLevel::Error, "PrimPrinters: Failed to set control settings");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool PrimFRBase::getTaxData(int aGroup, CPrimFR::Taxes::SData & aData)
{
	CPrimFR::TData answer;

	if (!processCommand(CPrimFR::Commands::GetTaxRate, CPrimFR::TData() << int2ByteArray(aGroup), &answer) || (answer.size() < 8))
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to get data for %1 tax group").arg(aGroup));
		return false;
	}

	bool groupOK;
	bool valueOK;
	int group = answer[5].toInt(&groupOK);
	aData.description = mCodec->toUnicode(answer[6]);
	aData.value = TVAT(answer[7].toDouble(&valueOK));
	aData.extraData = answer.mid(8);

	if (!groupOK || !valueOK)
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to parse data for %1 tax group").arg(aGroup));
		return false;
	}

	if (aData.extraData.isEmpty())
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to parse data for %1 tax group due to extra data is empty").arg(aGroup));
		return false;
	}

	if (group != aGroup)
	{
		toLog(LogLevel::Error, mDeviceName + QString("tax group = %1, need %2").arg(group).arg(aGroup));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool PrimFRBase::setTaxData(int aGroup, const CPrimFR::Taxes::SData & aData)
{
	CPrimFR::TData commandData = CPrimFR::TData()
		<< int2ByteArray(aGroup)
		<< mCodec->fromUnicode(aData.description)
		<< QString("%1").arg(aData.value, 5, 'f', 2, QLatin1Char(ASCII::Zero)).toLatin1()
		<< aData.extraData;

	if (!processCommand(CPrimFR::Commands::SetTaxRate, commandData))
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to set data for %1 tax group").arg(aGroup));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool PrimFRBase::checkTax(TVAT aVAT, const CFR::Taxes::SData & aData)
{
	CPrimFR::Taxes::SData taxData;

	if (!getTaxData(aData.group, taxData))
	{
		return false;
	}

	QStringList log;

	if (taxData.value != aVAT)
	{
		log << QString("tax value = %1%, need %2%").arg(taxData.value/100.0, 5, 'f', 2, ASCII::Zero).arg(aVAT, 5, 'f', 2, ASCII::Zero);
	}

	if (taxData.description != aData.description)
	{
		log << QString("tax description = %1, need %2").arg(taxData.description).arg(aData.description);
	}

	if (log.isEmpty())
	{
		return true;
	}

	toLog(LogLevel::Error, mDeviceName + QString(": Wrong %1 for %2 tax group").arg(log.join("; ")).arg(aData.group));

	if (mIsOnline && (taxData.value != aVAT))
	{
		return false;
	}

	taxData.value = aVAT;
	taxData.description = aData.description;

	bool result = setTaxData(aData.group, taxData);

	return result || (taxData.value == aVAT);
}

//--------------------------------------------------------------------------------
bool PrimFRBase::isConnected()
{
	QByteArray answer;

	while (mIOPort->read(answer, 5) && !answer.isEmpty())
	{
	}

	TStatusCodes statusCodes;
	CPrimFR::TData answerData;

	if (!getStatusInfo(statusCodes, answerData) || (answerData.size() < 8))
	{
		return false;
	}

	QByteArray softVersion = answerData[6];
	mModel = CPrimFR::ModelNames[answerData[5].simplified()];

	if (softVersion.indexOf(CPrimFR::FirmarePRIM21_03) != -1)
	{
		mModel = CPrimFR::Models::PRIM_21K_03;
	}

	setDeviceParameter(CDeviceData::Firmware, softVersion);

	CPrimFR::SModelParameters modelData = CPrimFR::ModelData[mModel];
	mVerified = modelData.verified;
	mModelCompatibility = mModels.contains(mModel);
	mCanProcessZBuffer = !mIsOnline && modelData.hasBuffer;
	mDeviceName = modelData.name;

	setConfigParameter(CHardware::Printer::FeedingAmount, modelData.feed);

	return true;
}

//--------------------------------------------------------------------------------
QStringList PrimFRBase::getModelList()
{
	return CPrimFR::getModelList(CPrimFR::CommonModels());
}

//--------------------------------------------------------------------------------
TResult PrimFRBase::processCommand(char aCommand, CPrimFR::TData * aAnswer)
{
	CPrimFR::TData commandData;

	return processCommand(aCommand, commandData, aAnswer);
}

//--------------------------------------------------------------------------------
TResult PrimFRBase::processCommand(char aCommand, const CPrimFR::TData & aCommandData, CPrimFR::TData * aAnswer)
{
	mProtocol.setPort(mIOPort);
	mProtocol.setLog(mLog);

	QVariantMap configuration;
	configuration.insert(CHardware::Port::DeviceModelName, "PRIM");
	mIOPort->setDeviceConfiguration(configuration);

	QByteArray commandData = QByteArray(1, aCommand).toHex().toUpper();
	CPrimFR::TData data;

	if (CPrimFR::Commands::DateTimeIn.contains(aCommand))
	{
		QTime currentTime = QTime::currentTime();
		QString time = QString("%1%2")
			.arg(currentTime.hour(),   2, 10, QLatin1Char(ASCII::Zero))
			.arg(currentTime.minute(), 2, 10, QLatin1Char(ASCII::Zero));

		data << QDate::currentDate().toString("ddMMyy").toLatin1() << time.toLatin1();
	}

	data << aCommandData;

	foreach(auto dataItem, data)
	{
		commandData += CPrimFR::Separator + dataItem;
	}

	QByteArray answer;
	TResult result = mProtocol.processCommand(commandData, answer, mCommandTimouts[aCommand]);

	if (mConnected && (result == CommandResult::NoAnswer) && (answer.size() == 1) && (~answer[0] & CPrimFR::CommandResultMask::PrinterMode))
	{
		mMode = EFRMode::Printer;
		mIOPort->write(QByteArray(2, ASCII::LF));
		performReceipt(QStringList() << CPrimFR::EndPrinterModeText, true);

		if (setMode(EFRMode::Fiscal))
		{
			result = mProtocol.processCommand(commandData, answer, mCommandTimouts[aCommand]);
		}
	}

	if (aCommand == CPrimFR::Commands::SetFDTypeNames)
	{
		SleepHelper::msleep(CPrimFR::Pause::Programming);
	}

	CPrimFR::TData answerData;
	result = checkAnswer(result, answer, answerData);

	if (!result)
	{
		return result;
	}

	ushort state = QString(answerData[2]).toUShort(0, 16);
	mSessionOpened = qToBigEndian(state) & CPrimFR::SessionOpenedMask;

	if (aAnswer)
	{
		*aAnswer = answerData;
	}

	char error = char(qToBigEndian(QString(answerData[3]).toUShort(0, 16)));

	if (!error)
	{
		if (aCommand == CPrimFR::Commands::ZReport)
		{
			mNeedCloseSession = false;
		}

		return CommandResult::OK;
	}

	if (!mProcessingErrors.isEmpty() && (mProcessingErrors.last() == error))
	{
		return CommandResult::Device;
	}

	QString canAutoCloseSession = getConfigParameter(CHardware::FR::CanAutoCloseSession).toString();

	if ((error == CPrimFR::Errors::NeedZReport) && (mInitialized == ERequestStatus::InProcess))
	{
		if (canAutoCloseSession == CHardware::Values::Auto)
		{
			setConfigParameter(CHardware::FR::CanAutoCloseSession, CHardware::Values::NotUse);

			emit configurationChanged();
		}
		else if (canAutoCloseSession == CHardware::Values::NotUse)
		{
			return CommandResult::OK;
		}
	}

	if (!processAnswer(error))
	{
		return CommandResult::Device;
	}

	mProcessingErrors.pop_back();

	return processCommand(aCommand, aCommandData, aAnswer);
}

//--------------------------------------------------------------------------------
TResult PrimFRBase::checkAnswer(TResult aResult, const QByteArray & aAnswer, CPrimFR::TData & aAnswerData)
{
	if (aResult == CommandResult::NoAnswer)
	{
		char commandResultAnswer;
		TResult result = mProtocol.getCommandResult(commandResultAnswer);

		if (result == CommandResult::Port)
		{
			return CommandResult::Port;
		}
		else if (result == CommandResult::OK)
		{
			TStatusCodes statusCodes = parseRTStatus(0, commandResultAnswer);

			if (statusCodes.contains(PrinterStatusCode::Error::PrinterFR))
			{
				toLog(LogLevel::Error, "FR printer is in error, going to offline mode and exiting");
				mOffline = true;
			}

			if (statusCodes.contains(DeviceStatusCode::Warning::OperationError))
			{
				toLog(LogLevel::Error, "Last command not identify");
			}
		}

		return CommandResult::NoAnswer;
	}
	else if (!aResult)
	{
		return aResult;
	}

	aAnswerData = aAnswer.split(CPrimFR::Separator);
	int size = aAnswerData.size();

	if (size < 5)
	{
		toLog(LogLevel::Error, QString("Failed to process command because too few sections in answer = %1, need 5 minimum").arg(size));
		return CommandResult::Answer;
	}

	ushort error = qToBigEndian(QString(aAnswerData[3]).toUShort(0, 16));
	char errorCode = char(error);
	char errorReason = char(error >> 8);

	if (errorCode)
	{
		toLog(LogLevel::Error, "PRIMFR: Error: " + mErrorData->getDescription(error));
	}
	else if (errorReason == 1)
	{
		toLog(LogLevel::Warning, "PRIMFR: OK, but the document was not printed");
	}

	return CommandResult::OK;
}

//--------------------------------------------------------------------------------
bool PrimFRBase::printLine(const QByteArray & aString)
{
	if (!TSerialFRBase::printLine(aString))
	{
		return false;
	}

	if (mModel == CPrimFR::Models::PRIM_07K)
	{
		SleepHelper::msleep(CPrimFR::Pause::LinePrinting);
	}

	return true;
}

//--------------------------------------------------------------------------------
bool PrimFRBase::performReceipt(const QStringList & aReceipt, bool aProcessing)
{
	QVariantMap configuration;
	configuration.insert(CHardware::Port::IOLogging, QVariant().fromValue(ELoggingType::ReadWrite));
	mIOPort->setDeviceConfiguration(configuration);

	bool result = TSerialFRBase::processReceipt(aReceipt, aProcessing);

	configuration.insert(CHardware::Port::IOLogging, QVariant().fromValue(mIOMessageLogging));
	mIOPort->setDeviceConfiguration(configuration);

	return result;
}

//--------------------------------------------------------------------------------
bool PrimFRBase::processReceipt(const QStringList & aReceipt, bool aProcessing)
{
	if (!setMode(EFRMode::Printer))
	{
		return false;
	}

	//TODO: лишняя команда для наследников, которые печатают через драйвер принтера
	mIOPort->write(CPOSPrinter::Command::AlignLeft);
	bool result = performReceipt(aReceipt, aProcessing);

	setMode(EFRMode::Fiscal);

	return result;
}

//--------------------------------------------------------------------------------
void PrimFRBase::getRTStatuses(TStatusCodes & aStatusCodes)
{
	aStatusCodes.clear();

	foreach (auto command, CPrimFR::ModelData[mModel].statusData.keys())
	{
		if (aStatusCodes.contains(DeviceStatusCode::Error::NotAvailable))
		{
			break;
		}

		aStatusCodes.unite(getRTStatus(command));
	}

	bool notAvailabled = aStatusCodes.contains(DeviceStatusCode::Error::NotAvailable);
	bool oldNotAvailabled = mStatusCollection.isEmpty() || mStatusCollection.contains(DeviceStatusCode::Error::NotAvailable);
	bool error = std::find_if(aStatusCodes.begin(), aStatusCodes.end(), [&] (int aCode) -> bool
		{ return mStatusCodesSpecification->value(aCode).warningLevel == EWarningLevel::Error; }) != aStatusCodes.end();

	mOffline = notAvailabled || (!oldNotAvailabled && error);
}

//--------------------------------------------------------------------------------
bool PrimFRBase::getStatusInfo(TStatusCodes & aStatusCodes, CPrimFR::TData & aAnswer)
{
	bool printerMode = mMode == EFRMode::Printer;

	if (mOffline || printerMode)
	{
		getRTStatuses(aStatusCodes);
	}

	if (!mOffline && !printerMode)
	{
		if (!processCommand(CPrimFR::Commands::GetKKMInfo, &aAnswer))
		{
			if (!aAnswer.isEmpty() && !aAnswer[0].isEmpty())
			{
				getRTStatuses(aStatusCodes);
			}

			return false;
		}

		TStatusCodes commandStatus = getRTStatus(0);
		aStatusCodes.unite(commandStatus);
	}

	return !aStatusCodes.contains(DeviceStatusCode::Error::NotAvailable);
}

//--------------------------------------------------------------------------------
bool PrimFRBase::getStatus(TStatusCodes & aStatusCodes)
{
	if (mNeedCloseSession)
	{
		aStatusCodes.insert(FRStatusCode::Error::NeedCloseSession);
	}

	if (mMode == EFRMode::Printer)
	{
		return true;
	}

	CPrimFR::TData answerData;

	if (!getStatusInfo(aStatusCodes, answerData))
	{
		return false;
	}

	char fixedStatus = char(answerData[1].toUShort(0, 16));

	foreach(auto bit, CPrimFR::StatusInfo.data().keys())
	{
		if (fixedStatus & (1 << bit))
		{
			//aStatusCodes.insert(CPrimFR::StatusInfo[bit]);
		}
	}

	QByteArray printerStatuses = answerData[4];
	int byte = 0;

	while (byte < (printerStatuses.size() / 2))
	{
		char status = char(printerStatuses.mid(byte * 2, 2).toUShort(0, 16));
		aStatusCodes += parseRTStatus(++byte, status);
	}

	return true;
}

//--------------------------------------------------------------------------------
int PrimFRBase::getVerificationCode()
{
	CPrimFR::TData answer;

	if (!processCommand(CPrimFR::Commands::GetLastCVCNumber, &answer) || (answer.size() < 6))
	{
		toLog(LogLevel::Error, "PrimPrinters: Failed to get last CVC number");
		return 0;
	}

	return answer[5].toHex().toInt(0, 16);
}

//--------------------------------------------------------------------------------
void PrimFRBase::makeAFDReceipt(QStringList & aReceipt)
{
	aReceipt = simplifyReceipt(aReceipt);

	Tags::TLexemeReceipt lexemeReceipt;
	makeLexemeReceipt(aReceipt, lexemeReceipt);
	aReceipt.clear();

	for (int i = 0; i < lexemeReceipt.size(); ++i)
	{
		QString line;

		for (int j = 0; j < lexemeReceipt[i].size(); ++j)
		{
			for (int k = 0; k < lexemeReceipt[i][j].size(); ++k)
			{
				line += lexemeReceipt[i][j][k].data;
			}
		}

		aReceipt << line;
	}

	if (!aReceipt.isEmpty())
	{
		toLog(LogLevel::Normal, "Printing fiscal document, receipt:\n" + aReceipt.join("\n"));
	}

	for (int i = 0; i < aReceipt.size(); ++i)
	{
		QString line = aReceipt.takeAt(i--);

		while (!line.isEmpty())
		{
			aReceipt.insert(++i, line.left(CPrimFR::MaxLengthGField));
			line = line.mid(CPrimFR::MaxLengthGField);
		}
	}
}

//--------------------------------------------------------------------------------
void PrimFRBase::setFiscalData(CPrimFR::TData & aCommandData, CPrimFR::TDataList & aAdditionalAFDData, const SPaymentData & aPaymentData, int aReceiptSize)
{
	QString depositing     = getConfigParameter(CHardware::FR::Strings::Depositing).toString();
	QString serialNumber   = getConfigParameter(CHardware::FR::Strings::SerialNumber).toString();
	QString documentNumber = getConfigParameter(CHardware::FR::Strings::DocumentNumber).toString();
	QString INN            = getConfigParameter(CHardware::FR::Strings::INN).toString();
	QString date           = getConfigParameter(CHardware::FR::Strings::Date).toString();
	QString time           = getConfigParameter(CHardware::FR::Strings::Time).toString();
	QString amount         = getConfigParameter(CHardware::FR::Strings::Amount).toString();

	QByteArray sum = QString::number(qRound64(getTotalAmount(aPaymentData) * 100.0) / 100.0, '0', 2).toLatin1();
	QString cashier = getConfigParameter(CHardware::FiscalFields::Cashier).toString();
	QString operatorId = cashier.isEmpty() ? CPrimFR::OperatorID : cashier;

	// обязательные G-поля
	aCommandData
		<< addGFieldToBuffer(aReceiptSize + 1, serialNumber.size() + 2)                             // серийный номер
		<< addGFieldToBuffer(aReceiptSize + 1, serialNumber.size() + 14 + documentNumber.size())    // номер документа
		<< addGFieldToBuffer(aReceiptSize + 3, date.size() + 2)                                     // дата
		<< addGFieldToBuffer(aReceiptSize + 3, date.size() + 16 + time.size())                      // время
		<< addGFieldToBuffer(aReceiptSize + 2, INN.size() + 2)                                      // ИНН
		<< addGFieldToBuffer(aReceiptSize + 3, 7) << mCodec->fromUnicode(operatorId)                // ID оператора
		<< addGFieldToBuffer(aReceiptSize + 4, amount.size() + 2) << sum;                           // сумма

	// произворльные G-поля
	aAdditionalAFDData
		<< addArbitraryFieldToBuffer(aReceiptSize + 1, 1, serialNumber)
		<< addArbitraryFieldToBuffer(aReceiptSize + 1, serialNumber.size() + 12, documentNumber)
		<< addArbitraryFieldToBuffer(aReceiptSize + 2, 1, INN)
		<< addArbitraryFieldToBuffer(aReceiptSize + 3, 1, date)
		<< addArbitraryFieldToBuffer(aReceiptSize + 3, date.size() + 15, time)
		<< addArbitraryFieldToBuffer(aReceiptSize + 4, 1, amount);
}

//--------------------------------------------------------------------------------
bool PrimFRBase::performFiscal(const QStringList & aReceipt, const SPaymentData & aPaymentData, TFiscalPaymentData & aFPData, TComplexFiscalPaymentData & aPSData)
{
	QStringList receipt(aReceipt);
	makeAFDReceipt(receipt);
	int receiptSize = receipt.size();

	int verificationCode = getVerificationCode();
	QByteArray FDType = aPaymentData.back ? CPrimFR::FDTypes::SaleBack : CPrimFR::FDTypes::Sale;
	QByteArray payTypeData = int2ByteArray(mPayTypeData[aPaymentData.payType].value);

	CPrimFR::TData commandData = CPrimFR::TData()
		<< FDType               // тип чека
		<< payTypeData          // тип оплаты
		<< CPrimFR::Font        // шрифт - не используется, по умолчанию
		<< CPrimFR::Copies      // количество копий
		<< CPrimFR::Copies      // количество копий по горизонтали
		<< CPrimFR::Copies      // количество копий по вертикали
		<< CPrimFR::CopуGrid    // шаг сетки копий по горизонтали
		<< CPrimFR::CopуGrid    // шаг сетки копий по вертикали
		<< CPrimFR::LineGrid;   // шаг строк

	CPrimFR::TDataList additionalAFDData;
	setFiscalData(commandData, additionalAFDData, aPaymentData, receiptSize);

	for (int i = 0; i < additionalAFDData.size(); ++i)
	{
		commandData << additionalAFDData[i];
	}

	// терминальный чек
	for (int i = 0; i < receiptSize; ++i)
	{
		commandData << addArbitraryFieldToBuffer(i + 1, 1, receipt[i]);
	}

	// количество полей
	commandData.insert(32, int2ByteArray(receiptSize + additionalAFDData.size()));

	bool result = processCommand(CPrimFR::Commands::AFD, commandData);

	mPaperInPresenter = QDateTime::currentDateTime();

	int newVerificationCode = getVerificationCode();

	if (verificationCode && newVerificationCode)
	{
		return newVerificationCode > verificationCode;
	}

	return result;
}

//--------------------------------------------------------------------------------
bool PrimFRBase::processXReport()
{
	return processCommand(CPrimFR::Commands::XReport);
}

//--------------------------------------------------------------------------------
double PrimFRBase::getAmountInCash()
{
	CPrimFR::TData answer;

	if (!processCommand(CPrimFR::Commands::EReport, &answer) || (answer.size() < 24))
	{
		toLog(LogLevel::Error, "PrimPrinters: Failed to process E-Report");
		return -1;
	}

	bool OK;
	double result = answer[23].toDouble(&OK);

	return OK ? result : -1;
}

//--------------------------------------------------------------------------------
bool PrimFRBase::processPayout(double aAmount)
{
	QByteArray data = QString::number(floor(aAmount * 100) / 100, '0', 2).toLatin1();
	CPrimFR::TData commandData = CPrimFR::TData() << data << CPrimFR::OperatorID;

	return processCommand(CPrimFR::Commands::Encashment, commandData);
}

//--------------------------------------------------------------------------------
bool PrimFRBase::setStartZReportNumber(int aNumber, const CPrimFR::TData & aExtraData)
{
	toLog(LogLevel::Normal, "PrimPrinters: Set start Z-report number: " + QString::number(aNumber));

	CPrimFR::Taxes::SData taxData(0, CPrimFR::LastTaxRateName, aExtraData);
	taxData.extraData[0] = QString("%1").arg(aNumber, 5, 'f', 2, QLatin1Char(ASCII::Zero)).toLatin1();

	return setTaxData(CPrimFR::LastTaxRate, taxData);
}

//--------------------------------------------------------------------------------
int PrimFRBase::getStartZReportNumber(CPrimFR::TData & aExtraData)
{
	toLog(LogLevel::Normal, "PrimPrinters: Get start Z-report number");
	CPrimFR::Taxes::SData taxData;

	if (!getTaxData(CPrimFR::LastTaxRate, taxData))
	{
		return -1;
	}

	QString strName   = taxData.description.simplified();
	QString strNumber = taxData.extraData[0];
	aExtraData = taxData.extraData;

	if (strName.toLower() != QString(CPrimFR::LastTaxRateName).toLower())
	{
		toLog(LogLevel::Error, QString("PRIM: Tax rate name not valid: %1, need %2").arg(strName).arg(CPrimFR::LastTaxRateName));
		return -1;
	}

	bool OK;
	int startZReport = int(strNumber.toDouble(&OK));

	if (!OK)
	{
		toLog(LogLevel::Error, QString("PRIM: Failed to convert begin Z-report number: %1").arg(strNumber));
		return -1;
	}

	toLog(LogLevel::Normal, "Start Z-report number: " + QString::number(startZReport));

	return startZReport;
}

//--------------------------------------------------------------------------------
int PrimFRBase::getEndZReportNumber()
{
	toLog(LogLevel::Normal, "PrimPrinters: Get end Z-report number");
	CPrimFR::TData answer;

	if (!processCommand(CPrimFR::Commands::GetStatus, &answer) || (answer.size() < 8))
	{
		toLog(LogLevel::Error, "PrimPrinters: Failed to get end number of Z-report");
		return -1;
	}

	bool isOK;
	int endZReport = qToBigEndian(answer[7].toUShort(&isOK, 16));

	if (!isOK)
	{
		toLog(LogLevel::Error, "PRIM: Wrong end number of ZReport = %1" + QString(answer[7]));
		return -1;
	}

	toLog(LogLevel::Normal, "End Z-report number: " + QString::number(endZReport));

	return endZReport;
}

//--------------------------------------------------------------------------------
bool PrimFRBase::printDeferredZReport(int aNumber)
{
	toLog(LogLevel::Normal, "PrimPrinters: try to print deferred Z-report #" + QString::number(aNumber));

	CPrimFR::TData commandData = CPrimFR::TData() << QString("%1").arg(qToBigEndian(ushort(aNumber)), 4, 16, QChar(ASCII::Zero)).toUpper().toLatin1();

	if (!processCommand(CPrimFR::Commands::PrintDeferredZReports, commandData))
	{
		toLog(LogLevel::Error, "PrimPrinters: Failed to print deferred Z-report #" + QString::number(aNumber));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
TResult PrimFRBase::doZReport(bool aAuto)
{
	CPrimFR::TData commandData;

	if (aAuto)
	{
		commandData << CPrimFR::DontPrintFD;
	}

	return processCommand(CPrimFR::Commands::ZReport, commandData);
}

//--------------------------------------------------------------------------------
bool PrimFRBase::execZReport(bool aAuto)
{
	bool cannotAutoZReport = !CPrimFR::ModelData[mModel].hasBuffer || (mOperatorPresence && !getConfigParameter(CHardware::FR::ForcePerformZReport).toBool());

	if (aAuto && cannotAutoZReport)
	{
		toLog(LogLevel::Error, mDeviceName + (mOperatorPresence ?
			": Failed to process auto-Z-report due to presence of the operator." :
			": has no Z-buffer, so it is impossible to perform auto-Z-report."));
		return false;
	}

	toLog(LogLevel::Normal, QString("PrimPrinters: Begin processing %1Z-report").arg(aAuto ? "auto-" : ""));

	if (!doZReport(aAuto))
	{
		toLog(LogLevel::Error, "PrimPrinters: Failed to process Z-report");
		return false;
	}

	mNeedCloseSession = false;
	mProcessingErrors.removeAll(CPrimFR::Errors::NeedZReport);

	return true;
}

//--------------------------------------------------------------------------------
bool PrimFRBase::performZReport(bool aPrintDeferredReports)
{
	bool ZReportOK = execZReport(false);

	int needPrintDeferred = aPrintDeferredReports && CPrimFR::ModelData[mModel].hasBuffer;
	int endZReport = needPrintDeferred ? getEndZReportNumber() : 0;

	if (!needPrintDeferred || (endZReport <= 0))
	{
		return ZReportOK;
	}

	CPrimFR::TData extraData;
	int startZReport = getStartZReportNumber(extraData);

	if ((startZReport <= 0) || (startZReport > endZReport))
	{
		startZReport = 1;
		setStartZReportNumber(startZReport, extraData);
	}

	toLog(LogLevel::Normal, QString("PrimPrinters: Begin printing deferred Z-reports from %1 to %2").arg(startZReport).arg(endZReport));
	bool deferred = true;

	for (int i = startZReport; i <= endZReport; ++i)
	{
		if (!printDeferredZReport(i) || !setStartZReportNumber(i, extraData))
		{
			deferred = false;
		}
	}

	if (deferred)
	{
		toLog(LogLevel::Normal, "PrimPrinters: clear SKL.");

		if (!processCommand(CPrimFR::Commands::ClearZBuffer))
		{
			toLog(LogLevel::Error, "PrimPrinters: Failed to clear SKL.");
			return false;
		}
	}

	return ZReportOK || (aPrintDeferredReports && deferred);
}

//--------------------------------------------------------------------------------
bool PrimFRBase::setMode(EFRMode::Enum aMode)
{
	if (mMode == aMode)
	{
		return true;
	}

	if (aMode == EFRMode::Printer)
	{
		if (!processCommand(CPrimFR::Commands::SetPrinterMode))
		{
			toLog(LogLevel::Error, "PrimPrinters: Failed to set printer mode");
			return false;
		}
	}
	else if (aMode == EFRMode::Fiscal)
	{
		QByteArray answer;
		TResult result = mProtocol.execCommand(CPrimFR::Commands::SetFiscalMode, answer, CPrimFR::SetFiscalModeTimeout, EPrimFRCommandConditions::PrinterMode);

		CPrimFR::TData answerData;
		result = checkAnswer(result, answer, answerData);

		if (!result)
		{
			toLog(LogLevel::Error, "PrimPrinters: Failed to set fiscal mode");
			return false;
		}
	}

	mMode = aMode;

	return true;
}

//--------------------------------------------------------------------------------
CPrimFR::TData PrimFRBase::addGFieldToBuffer(int aX, int aY)
{
	return CPrimFR::TData()
		<< QString("%1").arg(qToBigEndian(unsigned short(aX)), 4, 16, QLatin1Char(ASCII::Zero)).toLatin1()    // позиция реквизита по X
		<< QString("%1").arg(qToBigEndian(unsigned short(aY)), 4, 16, QLatin1Char(ASCII::Zero)).toLatin1()    // позиция реквизита по Y
		<< "01"; // шрифт, см. ESC !
}

//--------------------------------------------------------------------------------
CPrimFR::TData PrimFRBase::addArbitraryFieldToBuffer(int aX, int aY, const QString & aData)
{
	return CPrimFR::TData()
		<< QString("%1").arg(qToBigEndian(unsigned short(aX)), 4, 16, QLatin1Char(ASCII::Zero)).toLatin1()    // позиция реквизита по X
		<< QString("%1").arg(qToBigEndian(unsigned short(aY)), 4, 16, QLatin1Char(ASCII::Zero)).toLatin1()    // позиция реквизита по Y
		<< "01"    // шрифт, см. ESC !
		<< "01"    // Печать произвольного реквизита
		<< "00"    // N вывода на контрольную ленту
		<< mCodec->fromUnicode(aData);    // данные
}

//--------------------------------------------------------------------------------
bool PrimFRBase::processAnswer(char aError)
{
	switch (aError)
	{
		case CPrimFR::Errors::InvalidStateForCommand:
		{
			mProcessingErrors.push_back(aError);

			return processCommand(CPrimFR::Commands::CancelDocument);
		}
		//--------------------------------------------------------------------------------
		case CPrimFR::Errors::NeedBeginSession:
		{
			mProcessingErrors.push_back(aError);

			return processCommand(CPrimFR::Commands::OpenSession);
		}
		//--------------------------------------------------------------------------------
		case CPrimFR::Errors::NeedBeginFRSession:
		{
			mProcessingErrors.push_back(aError);

			return openFRSession();
		}
		//--------------------------------------------------------------------------------
		case CPrimFR::Errors::NeedZReport :
		{
			mProcessingErrors.push_back(aError);

			mNeedCloseSession = true;

			return execZReport(true);
		}
	}

	return false;
}

//--------------------------------------------------------------------------------
TResult PrimFRBase::openFRSession()
{
	CPrimFR::TData commandData = CPrimFR::TData() << "";    // реквизиты смены

	return processCommand(CPrimFR::Commands::OpenFRSession, commandData);
}

//--------------------------------------------------------------------------------
TStatusCodes PrimFRBase::getRTStatus(int aCommand)
{
	mRTProtocol.setPort(mIOPort);
	mRTProtocol.setLog(mLog);

	QVariantMap configuration;
	configuration.insert(CHardware::Port::IOLogging, QVariant().fromValue(ELoggingType::ReadWrite));
	mIOPort->setDeviceConfiguration(configuration);

	char answer;
	TStatusCodes result;

	if (mRTProtocol.processCommand(aCommand, answer))
	{
		result = parseRTStatus(aCommand, answer);
	}
	else
	{
		result.insert(DeviceStatusCode::Error::NotAvailable);
		mOffline = true;
	}

	configuration.insert(CHardware::Port::IOLogging, QVariant().fromValue(ELoggingType::None));
	mIOPort->setDeviceConfiguration(configuration);

	return result;
}

//--------------------------------------------------------------------------------
TStatusCodes PrimFRBase::parseRTStatus(int aCommand, char aAnswer)
{
	TStatusCodes result;
	CPrimFR::TStatusBitShifts shifts = CPrimFR::ModelData[mModel].statusData[aCommand];

	foreach(auto bit, shifts.keys())
	{
		if (bool(aAnswer & (1 << bit)) == bool(aCommand))
		{
			result.insert(shifts[bit]);
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
QString PrimFRBase::int2String(int aValue)
{
	return QString("%1").arg(int(uchar(aValue)), 2, 16, QLatin1Char(ASCII::Zero));
}

//--------------------------------------------------------------------------------
QByteArray PrimFRBase::int2ByteArray(int aValue)
{
	return int2String(aValue).toLatin1();
}

//--------------------------------------------------------------------------------
