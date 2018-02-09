/* @file Прото-ФР семейства Штрих на COM-порту. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// Project
#include "ProtoShtrihFR.h"

using namespace SDK::Driver;
using namespace ProtocolUtils;

//TODO: реализовать тег Bold командой Печать жирной строки (12h)

//--------------------------------------------------------------------------------
template class ProtoShtrihFR<ShtrihSerialFRBase>;
template class ProtoShtrihFR<ShtrihTCPFRBase>;

//--------------------------------------------------------------------------------
template<class T>
ProtoShtrihFR<T>::ProtoShtrihFR()
{
	// кодек
	mCodec = CodecByName[CHardware::Codepages::Win1251];

	// параметры семейства ФР
	mNextReceiptProcessing = false;
	mMode = CShtrihFR::InnerModes::Work;
	mSubmode = CShtrihFR::InnerSubmodes::PaperOn;
	mType = CShtrihFR::Types::NoType;
	mModel = CShtrihFR::Models::ID::NoModel;
	mNonNullableAmount = 0;
	mFontNumber = CShtrihFR::Fonts::Default;

	// налоги
	mTaxData.add(18, 1, "НДС 18%");
	mTaxData.add(10, 2, "НДС 10%");
	mTaxData.add( 0, 0, "БЕЗ НАЛОГА");

	// данные команд
	mCommandData.add(CShtrihFR::Commands::GetModelInfo, CShtrihFR::Timeouts::Default, false);

	// ошибки
	mErrorData = PErrorData(new FRError::CData);
}

//--------------------------------------------------------------------------------
template<class T>
bool ProtoShtrihFR<T>::getPrintingSettings()
{
	QByteArray commandData(1, mFontNumber);
	QByteArray answer;

	if (!processCommand(CShtrihFR::Commands::GetFontSettings, commandData, &answer) || (answer.size() <= 4))
	{
		toLog(LogLevel::Error, "ShtrihFR: Failed to getting font settings");
		return false;
	}

	ushort paperWidth = revert(answer.mid(2, 2)).toHex().toUShort(0, 16);
	ushort letterSize = ushort(answer[4]);
	mLineSize = paperWidth / letterSize;

	return true;
}

//--------------------------------------------------------------------------------
template<class T>
void ProtoShtrihFR<T>::getSectionNames()
{
	TSectionNames sectionNames;
	int group = 1;
	char lastError = 0;
	TResult lastCommandResult = CommandResult::OK;

	do
	{
		QByteArray data;

		if (!getFRParameter(CShtrihFR::FRParameters::SectionName, data, char(group++)))
		{
			toLog(LogLevel::Error, mDeviceName + QString(": Failed to get name for %1 section").arg(group - 1));
		}
		else
		{
			sectionNames.insert(group - 1, mCodec->toUnicode(data.replace(ASCII::NUL, "").simplified()));
		}

		if (!mLastCommandResult && (mLastCommandResult == lastCommandResult) && CommandResult::ProtocolErrors.contains(mLastCommandResult))
		{
			break;
		}

		lastCommandResult = mLastCommandResult;

		if (mLastError && (lastError == mLastError))
		{
			break;
		}

		lastError = mLastError;
	}
	while (mLastError != CShtrihFR::Errors::WrongParametersInCommand);

	if (!sectionNames.isEmpty())
	{
		setConfigParameter(CHardwareSDK::FR::SectionNames, QVariant::fromValue<TSectionNames>(sectionNames));
	}
}

//--------------------------------------------------------------------------------
template<class T>
bool ProtoShtrihFR<T>::updateParameters()
{
	processDeviceData();
	setFRParameters();
	getSectionNames();

	return checkTaxes() && getPrintingSettings();
}

//---------------------------------------------------------------------------
template <class T>
QDateTime ProtoShtrihFR<T>::getDateTime()
{
	QByteArray data;

	if (getLongStatus(data))
	{
		QString dateTime = hexToBCD(data.mid(25, 6)).insert(4, "20");

		return QDateTime::fromString(dateTime, CShtrihFR::DateTimeFormat);
	}

	return QDateTime();
}

//--------------------------------------------------------------------------------
template<class T>
bool ProtoShtrihFR<T>::setTaxValue(TVAT aVAT, int aGroup)
{
	if (!setFRParameter(CShtrihFR::FRParameters::Taxes::Value, getHexReverted(int(aVAT * 100), 2), char(aGroup)))
	{
		toLog(LogLevel::Error, QString("ShtrihFR: Failed to set tax value %1% (%2 tax group)").arg(aVAT, 5, 'f', 2, ASCII::Zero).arg(aGroup));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
template<class T>
bool ProtoShtrihFR<T>::checkTax(TVAT aVAT, const CFR::Taxes::SData & aData)
{
	if (!aData.group)
	{
		return true;
	}

	char group = char(aData.group);
	QByteArray rawValue;
	QByteArray rawDescription;

	if (!getFRParameter(CShtrihFR::FRParameters::Taxes::Value, rawValue, group) ||
	    !getFRParameter(CShtrihFR::FRParameters::Taxes::Description, rawDescription, group))
	{
		return false;
	}

	int value = int(aVAT * 100);
	int FRValue = revert(rawValue).toHex().toUShort(0, 16);

	if (value != FRValue)
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Wrong tax value = %1% (%2 tax group), need %3%").arg(FRValue/100.0, 5, 'f', 2, ASCII::Zero).arg(aData.group).arg(aVAT, 5, 'f', 2, ASCII::Zero));

		if (!setTaxValue(aVAT, group))
		{
			return false;
		}
	}

	QString description = aData.description;
	QString FRDescription = mCodec->toUnicode(rawDescription.replace(ASCII::NUL, ""));

	if ((description != FRDescription) && !setFRParameter(CShtrihFR::FRParameters::Taxes::Description, description.leftJustified(57, QChar(ASCII::NUL)), group))
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to set description for tax value %1% (%2 tax group)").arg(aVAT, 5, 'f', 2, ASCII::Zero).arg(aData.group));
	}

	return true;
}

//--------------------------------------------------------------------------------
template<class T>
bool ProtoShtrihFR<T>::prepareFiscal()
{
	//TODO: поддержать базовый функционал работы с незабранным чеком
	if (!getLongStatus())
	{
		return false;
	}

	if (mMode == CShtrihFR::InnerModes::NeedCloseSession)
	{
		return execZReport(true);
	}
	else if (mMode == CShtrihFR::InnerModes::DocumentOpened)
	{
		return processCommand(CShtrihFR::Commands::CancelDocument);
	}

	return true;
}

//--------------------------------------------------------------------------------
template<class T>
bool ProtoShtrihFR<T>::performFiscal(const QStringList & aReceipt, const SPaymentData & aPaymentData, TFiscalPaymentData & /*aFPData*/, TComplexFiscalPaymentData & /*aPSData*/)
{
	if (!prepareFiscal() || !processReceipt(aReceipt, false) || !openDocument(aPaymentData.back))
	{
		return false;
	}

	bool result = true;

	foreach (auto amountData, aPaymentData.amountDataList)
	{
		result = result && sale(amountData, aPaymentData.back);
	}

	result = result && setOFDParameters() && closeDocument(getTotalAmount(aPaymentData), aPaymentData.payType);

	waitForPrintingEnd(true);
	//isFiscalDocumentOpened();

	if (!result && aPaymentData.back && (mLastError == CShtrihFR::Errors::NotEnoughMoney))
	{
		emitStatusCode(FRStatusCode::Error::NoMoney, EFRStatus::NoMoneyForSellingBack);
	}

	if (!result)
	{
		cancelFiscal();
	}

	return result;
}

//--------------------------------------------------------------------------------
template<class T>
bool ProtoShtrihFR<T>::cancelFiscal()
{
	return processCommand(CShtrihFR::Commands::CancelDocument);
}

//--------------------------------------------------------------------------------
template<class T>
bool ProtoShtrihFR<T>::getStatus(TStatusCodes & aStatusCodes)
{
	QByteArray data;
	TResult result = getLongStatus(data);

	if (!CORRECT(result))
	{
		return false;
	}
	else if (result)
	{
		ushort flags = revert(data.mid(13, 2)).toHex().toUShort(0, 16);
		appendStatusCodes(flags, aStatusCodes);
	}
	else if (mLastError)
	{
		int statusCode = CShtrihFR::Errors::Cutter ? PrinterStatusCode::Error::Cutter : DeviceStatusCode::Error::Unknown;
		aStatusCodes.insert(statusCode);
	}

	return true;
}

//--------------------------------------------------------------------------------
template<class T>
void ProtoShtrihFR<T>::appendStatusCodes(ushort aFlags, TStatusCodes & aStatusCodes)
{
	using namespace PrinterStatusCode;

	// ошибки чековой ленты, выявленные весовым и оптическим датчиками соответственно
	bool paperWeightSensor  = (~aFlags & CShtrihFR::Statuses::WeightSensor::NoChequePaper)  && isPaperWeightSensor();
	bool paperOpticalSensor = (~aFlags & CShtrihFR::Statuses::OpticalSensor::NoChequePaper) && isPaperOpticalSensor();

	if (paperWeightSensor || paperOpticalSensor)
	{
		aStatusCodes.insert(Error::PaperEnd);
		toLog(LogLevel::Error, QString("ShtrihFR: Paper tape error, report %1")
			.arg((paperWeightSensor && paperOpticalSensor) ? "both optical & weight sensors" :
				 (paperWeightSensor ? "weight sensor" : "optical sensor")));
	}

	// рычаг чековой ленты
	if ((~aFlags & CShtrihFR::Statuses::PaperLeverNotDropped)  && isPaperLeverExist())
	{
		aStatusCodes.insert(DeviceStatusCode::Error::MechanismPosition);
		toLog(LogLevel::Error, "ShtrihFR: Paper lever error");
	}

	// крышка корпуса
	if ((aFlags & CShtrihFR::Statuses::CoverNotClosed) && isCoverSensor())
	{
		aStatusCodes.insert(DeviceStatusCode::Error::CoverIsOpened);
	}
}

//--------------------------------------------------------------------------------
template<class T>
bool ProtoShtrihFR<T>::isPaperWeightSensor() const
{
	bool result = !((mType == CShtrihFR::Types::Printer) ||
	                (mType == CShtrihFR::Types::KKM &&
	              ((mModel == CShtrihFR::Models::ID::ShtrihElvesFRK)   ||
	               (mModel == CShtrihFR::Models::ID::Yarus01K)         ||
	               (mModel == CShtrihFR::Models::ID::Yarus02K)         ||
	               (mModel == CShtrihFR::Models::ID::ShtrihKioskFRK_2) ||
	               (mModel == CShtrihFR::Models::ID::NeoService)       ||

	               (mModel == CShtrihFR::Models::ID::PayOnline01FA)    ||
	               (mModel == CShtrihFR::Models::ID::PayVKP80KFA))));

	bool weightSensorsEnabled = !containsConfigParameter(CHardware::Printer::Settings::PaperWeightSensors) ||
		(getConfigParameter(CHardware::Printer::Settings::PaperWeightSensors).toString() == CHardware::Values::Use);

	return result && weightSensorsEnabled;
}

//--------------------------------------------------------------------------------
template<class T>
bool ProtoShtrihFR<T>::isPaperOpticalSensor() const
{
	return !((mType == CShtrihFR::Types::KKM) &&
	         ((mModel == CShtrihFR::Models::ID::ATOLElvesMiniFRF) ||
	          (mModel == CShtrihFR::Models::ID::ATOLFelixRF)));
}

//--------------------------------------------------------------------------------
template<class T>
bool ProtoShtrihFR<T>::isPaperLeverExist() const
{
	return  (mType == CShtrihFR::Types::Printer) ||
	       ((mType == CShtrihFR::Types::KKM) &&
	        ((mModel == CShtrihFR::Models::ID::ShtrihFRF)        ||
	         (mModel == CShtrihFR::Models::ID::ShtrihFRK)        ||
	         (mModel == CShtrihFR::Models::ID::ShtrihElvesFRK)   ||
	         (mModel == CShtrihFR::Models::ID::ShtrihFRFKazah)   ||
	         (mModel == CShtrihFR::Models::ID::ShtrihFRFBelorus) ||
	         (mModel == CShtrihFR::Models::ID::ShtrihComboFRK)   ||
	         (mModel == CShtrihFR::Models::ID::ShtrihKioskFRK)   ||
	         (mModel == CShtrihFR::Models::ID::ShtrihKioskFRK_2) ||
	         (mModel == CShtrihFR::Models::ID::Yarus01K)         ||
	         (mModel == CShtrihFR::Models::ID::NeoService)       ||

	         (mModel == CShtrihFR::Models::ID::PayOnline01FA)    ||
	         (mModel == CShtrihFR::Models::ID::PayVKP80KFA)      ||
	         (mModel == CShtrihFR::Models::ID::ShtrihFR01F)));
}

//--------------------------------------------------------------------------------
template<class T>
bool ProtoShtrihFR<T>::isCoverSensor() const
{
	return (mType == CShtrihFR::Types::KKM) &&
	       ((mModel == CShtrihFR::Models::ID::ShtrihFRF)        ||
	        (mModel == CShtrihFR::Models::ID::ShtrihFRK)        ||
	        (mModel == CShtrihFR::Models::ID::ShtrihFRFKazah)   ||
	        (mModel == CShtrihFR::Models::ID::ShtrihFRFBelorus) ||
	        (mModel == CShtrihFR::Models::ID::Shtrih950K)       ||
	        (mModel == CShtrihFR::Models::ID::ShtrihMiniFRK)    ||
	        (mModel == CShtrihFR::Models::ID::ShtrihMini01F)    ||
	        (mModel == CShtrihFR::Models::ID::NeoService)       ||

	        (mModel == CShtrihFR::Models::ID::ShtrihFR01F));
}

//--------------------------------------------------------------------------------
template<class T>
TResult ProtoShtrihFR<T>::execCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer)
{
	mProtocol.setPort(mIOPort);
	mProtocol.setLog(mLog);

	QByteArray commandData = aCommand;

	if (mCommandData[aCommand].password)
	{
		QByteArray passwordData = QByteArray(1, CShtrihFR::AdminPassword).leftJustified(4, ASCII::NUL);
		commandData.append(passwordData);
	};

	mLastError = 0;

	commandData += aCommandData;
	QByteArray answer;

	int repeatCount = 0;

	do
	{
		if (repeatCount)
		{
			toLog(LogLevel::Normal, mDeviceName + QString(": iteration %1").arg(repeatCount + 1));
		}

		mLastCommandResult = mProtocol.processCommand(commandData, answer, mCommandData[aCommand].timeout);

		if (!mLastCommandResult)
		{
			return mLastCommandResult;
		}

		if (aAnswer)
		{
			*aAnswer = answer;
		}

		if (answer.size() < CShtrihFR::MinAnswerDataSize)
		{
			toLog(LogLevel::Error, QString("ShtrihFR: Answer data size = %1, need min %2").arg(answer.size()).arg(CShtrihFR::MinAnswerDataSize));
			mLastCommandResult = CommandResult::Answer;
		}

		QByteArray answerCommand = answer.left(aCommand.size());

		if (aCommand != answerCommand)
		{
			toLog(LogLevel::Error, QString("ShtrihFR: Invalid answer command = 0x%1, need = 0x%2.")
				.arg(answerCommand.toHex().toUpper().data())
				.arg(aCommand.toHex().toUpper().data()));
			mLastCommandResult = CommandResult::Answer;
		}
	}
	while (!mLastCommandResult && aAnswer && (repeatCount++ < CShtrihFR::MaxRepeatPacket));

	if (!mLastCommandResult)
	{
		return aAnswer ? mLastCommandResult : CommandResult::OK;
	}

	int errorPosition = aCommand.size();
	mLastError = answer[errorPosition];

	if (!mLastError)
	{
		return CommandResult::OK;
	}

	toLog(LogLevel::Error, "ShtrihFR: Error: " + mErrorData->value(mLastError).description);
	setErrorFlags(aCommand);

	if (isNotError(aCommand[0]))
	{
		mLastError = 0;

		return CommandResult::OK;
	}

	if (!mProcessingErrors.isEmpty() && (mProcessingErrors.last() == mLastError))
	{
		return CommandResult::Device;
	}

	if (!processAnswer(aCommand))
	{
		return CommandResult::Device;
	}

	mProcessingErrors.pop_back();

	return processCommand(aCommand, aCommandData, aAnswer);
}

//--------------------------------------------------------------------------------
template<class T>
bool ProtoShtrihFR<T>::isNotError(char aCommand)
{
	if ((mLastError == CShtrihFR::Errors::BadModeForCommand) &&
	    (aCommand == CShtrihFR::Commands::CancelDocument) &&
	    getLongStatus() && (mMode != CShtrihFR::InnerModes::DocumentOpened))
	{
		toLog(LogLevel::Normal, "ShtrihFR: Fiscal document already closed");
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------
template<class T>
double ProtoShtrihFR<T>::getAmountInCash()
{
	QByteArray data;

	if (!getRegister(CShtrihFR::Registers::TotalCashSum, data))
	{
		return -1;
	}

	bool OK;
	double result = revert(data).toHex().toULongLong(&OK, 16) / 100.0;

	return OK ? result : -1;
}

//--------------------------------------------------------------------------------
template<class T>
bool ProtoShtrihFR<T>::processPayout(double aAmount)
{
	return processCommand(CShtrihFR::Commands::Encashment, getHexReverted(aAmount, 5, 2));
}

//--------------------------------------------------------------------------------
template<class T>
bool ProtoShtrihFR<T>::printLine(const QByteArray & aString)
{
	QDateTime beginning = QDateTime::currentDateTime();

	QByteArray commandData;
	commandData.append(CShtrihFR::PrintOnChequeTape);
	commandData.append(mFontNumber);
	commandData.append(aString.leftJustified(CShtrihFR::FixedStringSize, ASCII::Space));

	TResult result = processCommand(CShtrihFR::Commands::PrintString, commandData);

	if ((result != CommandResult::Port) && (result != CommandResult::NoAnswer) && mModelData.linePrintingTimeout)
	{
		int pause = mModelData.linePrintingTimeout - int(beginning.msecsTo(QDateTime::currentDateTime()));

		if (pause > 0)
		{
			toLog(LogLevel::Debug, mDeviceName + QString(": Pause after printing line = %1 ms").arg(pause));
			SleepHelper::msleep(pause);
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
template<class T>
bool ProtoShtrihFR<T>::cut()
{
	QByteArray commandData(1, CShtrihFR::FullCutting);

	if (!processCommand(CShtrihFR::Commands::Cut, commandData))
	{
		toLog(LogLevel::Error, "ShtrihFR: Failed to cut");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
template<class T>
bool ProtoShtrihFR<T>::isFiscalDocumentOpened()
{
	return getLongStatus() && (mMode == CShtrihFR::InnerModes::DocumentOpened);
}

//--------------------------------------------------------------------------------
template<class T>
bool ProtoShtrihFR<T>::openDocument(bool aBack)
{
	char documentType = aBack ? CShtrihFR::DocumentTypes::SaleBack : CShtrihFR::DocumentTypes::Sale;

	if (!processCommand(CShtrihFR::Commands::OpenDocument, QByteArray(1, documentType)))
	{
		toLog(LogLevel::Error, "ShtrihFR: Failed to open document, feed, cut and exit");
		return false;
	}

	return isFiscalDocumentOpened();
}

//--------------------------------------------------------------------------------
template<class T>
bool ProtoShtrihFR<T>::closeDocument(double aSum, EPayTypes::Enum aPayType)
{
	QByteArray commandData;

	for (int i = 1; i <= CShtrihFR::PayTypeQuantity; ++i)
	{
		double sum = (i == mPayTypeData[aPayType].value) ? aSum : 0;
		commandData.append(getHexReverted(sum, 5, 2));    // сумма
	}

	commandData.append(getHexReverted(0, 2, 2));          // скидка
	commandData.append(CShtrihFR::ClosingFiscalTaxes);    // налоги
	commandData.append(CShtrihFR::UnitName);              // текст продажи

	if (!processCommand(CShtrihFR::Commands::CloseDocument, commandData))
	{
		toLog(LogLevel::Error, "ShtrihFR: Failed to close document, feed, cut and exit");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
template<class T>
void ProtoShtrihFR<T>::checkSalesName(QString & aName)
{
	if ((aName.size() > mLineSize) && processReceipt(QStringList() << aName, false))
	{
		aName.clear();
	}

	aName = aName.leftJustified(CShtrihFR::FixedStringSize, QChar(ASCII::Space), true);
}

//--------------------------------------------------------------------------------
template<class T>
bool ProtoShtrihFR<T>::sale(const SAmountData & aAmountData, bool aBack)
{
	int taxIndex = mTaxData[aAmountData.VAT].group;
	QString name = aAmountData.name;
	char section = (aAmountData.section == -1) ? CShtrihFR::SectionNumber : char(aAmountData.section);
	checkSalesName(name);

	QByteArray commandData;
	commandData.append(getHexReverted(1, 5, 3));                  // количество
	commandData.append(getHexReverted(aAmountData.sum, 5, 2));    // сумма
	commandData.append(section);                                  // отдел
	commandData.append(getHexReverted(taxIndex, 4));              // налоги
	commandData.append(mCodec->fromUnicode(name));                // текст продажи

	char command = aBack ? CShtrihFR::Commands::SaleBack : CShtrihFR::Commands::Sale;

	if (!processCommand(command, commandData))
	{
		toLog(LogLevel::Error, QString("%1: Failed to sale for %2 (%3, VAT = %4), feed, cut and exit").arg(mDeviceName).arg(aAmountData.sum, 0, 'f', 2).arg(name).arg(aAmountData.VAT));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
template<class T>
bool ProtoShtrihFR<T>::performZReport(bool /*aPrintDeferredReports*/)
{
	return execZReport(false);
}

//--------------------------------------------------------------------------------
template<class T>
void ProtoShtrihFR<T>::parseDeviceData(const QByteArray & aData)
{
	// данные прошивки ФР
	CShtrihFR::SSoftInfo FRInfo;
	FRInfo.version = aData.mid(3, 2).insert(1, ASCII::Dot);
	FRInfo.build   = revert(aData.mid(5, 2)).toHex().toUShort(0, 16);
	QString FRDate = hexToBCD(aData.mid(7, 3)).insert(4, "20");
	FRInfo.date    = QDate::fromString(FRDate, CFR::DateFormat);

	mOldFirmware = (mModelData.build && (FRInfo.build != mModelData.build)) || ((mModelData.date < QDate::currentDate()) && (FRInfo.date < mModelData.date));

	setDeviceParameter(CDeviceData::Version, FRInfo.version, CDeviceData::Firmware);
	setDeviceParameter(CDeviceData::Build, FRInfo.build, CDeviceData::Firmware);
	setDeviceParameter(CDeviceData::Date, FRInfo.date.toString(CFR::DateLogFormat), CDeviceData::Firmware);
	setDeviceParameter(CDeviceData::FR::FreeReregistrations, uchar(aData[41]));
}

//--------------------------------------------------------------------------------
template<class T>
void ProtoShtrihFR<T>::processDeviceData()
{
	QByteArray data;

	if (getLongStatus(data))
	{
		parseDeviceData(data);
	}

	QByteArray answer;

	if (processCommand(CShtrihFR::Commands::GetModelInfo, &answer))
	{
		QString protocol = hexToBCD(answer.mid(4, 2), ASCII::LF).simplified().insert(1, ASCII::Dot);
		uchar languageId = uchar(answer[7]);

		if (mModel >= 0)
		{
			setDeviceParameter(CDeviceData::ModelNumber, mModel);
		}

		setDeviceParameter(CDeviceData::ProtocolVersion, protocol);
		setDeviceParameter(CDeviceData::FR::Language, CShtrihFR::Languages[languageId]);
	}
}

//--------------------------------------------------------------------------------
template<class T>
void ProtoShtrihFR<T>::setFRParameters()
{
	if (!CShtrihFR::FRParameters::Fields.data().contains(mModel))
	{
		toLog(LogLevel::Normal, QString("ShtrihFR: Cannot set any fields for the device with model Id %1 as no data of system tables").arg(mModel));
		return;
	}

	//0. автообнуление денежной наличности при закрытии смены - нет
	setFRParameter(mParameters.autoNulling, false);

	//1. Печать рекламного текста (шапка чека, клише) - да
	//   на самом деле это доступность всей таблицы 4 - шапка чека + реклама в конце
	//setFRParameter(mParameters.documentCapEnable, true);

	//2. Печать текстовых строк на ленте операционного журнала (контрольной) - нет
	setFRParameter(mParameters.printOnControlTape, false);

	//3. Печать только на чековой ленте - да
	setFRParameter(mParameters.printOnChequeTapeOnly, true);

	//4. Печать необнуляемой суммы (на Z-отчете) - да
	setFRParameter(mParameters.printNotNullingSum, true);

	//5. Отрезка чека после завершения печати - да
	QByteArray data;

	if (getFRParameter(mParameters.cutting, data))
	{
		char cutting = (mModel == CShtrihFR::Models::ID::PayVKP80KFA) ? CShtrihFR::FRParameters::PartialCutting : CShtrihFR::FRParameters::FullCutting;

		if (data[0] != cutting)
		{
			setFRParameter(mParameters.cutting, cutting);
			mNeedReboot = mModel == CShtrihFR::Models::ID::PayVKP80KFA;
		}
	}

	//6. Печать заголовка чека (шапка чека, клише) - в конце чека
	//   на самом деле это фискальные параметры чека - серийник ККМ, ИНН дилера и др.
	setFRParameter(mParameters.documentCapPlace, true);

	//7. Печать единичного количества - нет
	setFRParameter(mParameters.printOneAmount, false);

	//8. Промотка ленты перед отрезкой чека - нет, проматываем сами
	setFRParameter(mParameters.feedBeforeCutting, false);

	//9. Использование весовых датчиков при отсутствии контроля бумаги - да
	setFRParameter(mParameters.weightSensorEnable, isPaperWeightSensor());

	//10. Длинный Z-отчет с гашением - да
	setFRParameter(mParameters.ZReportType, true);

	//11. Начисление налогов - на каждую операцию в чеке/налог вычисляется в ФР.
	setFRParameter(mParameters.taxesCalculation, CShtrihFR::TaxForEachOperationInFR);

	//12. Печать налогов - налоговые ставки, оборот, названия, накопления.
	setFRParameter(mParameters.taxesPrinting, CShtrihFR::PrintAllTaxData);

	//13. Количество строк рекламного текста - 0, не нужно оно.
	//    тут надо было бы поставить 0, но это даст ошибку 33h - значение вне диапазона.
	//    пишем 2, это также может вызвать ошибку, если прошивка старая.
	setFRParameter(mParameters.documentCapAmount, CShtrihFR::MaxAdvertStringsSize);

	//14. Печать клише (шапка чека) - да
	setFRParameter(mParameters.printDocumentCap, true);

	//15. Отрезка чека при открытом чеке - да
	//    если фискальный чек на каком-то этапе обломился, надо выдать юзеру то, что есть
	setFRParameter(mParameters.cuttingWithOpenedDocument, true);
}

//--------------------------------------------------------------------------------
template<class T>
bool ProtoShtrihFR<T>::setFRParameter(const CShtrihFR::FRParameters::SData & aData, const QVariant & aValue, char aSeries)
{
	if (mProcessingErrors.contains(CShtrihFR::Errors::RAM))
	{
		return false;
	}

	if (!CShtrihFR::FRParameters::Fields.data().contains(mModel))
	{
		toLog(LogLevel::Normal, QString("ShtrihFR: Cannot set field for the device with model Id %1 as no data of system tables").arg(mModel));
		return true;
	}

	if (aData.field == CShtrihFR::FRParameters::NA)
	{
		toLog(LogLevel::Normal, QString("ShtrihFR: The field %1 for table %2 for the device with model Id %3 is not available")
			.arg(aData.description, mParameters.getMaxNADescriptionSize()).arg(aData.table).arg(mModel));
		return true;
	}

	QByteArray commandData;
	commandData.append(char(aData.table));
	commandData.append(getHexReverted(aSeries, 2));
	commandData.append(char(aData.field));

	     if (aValue.type() == QVariant::ByteArray) commandData.append(aValue.toByteArray());
	else if (aValue.type() == QVariant::String) commandData.append(mCodec->fromUnicode(aValue.toString()));
	else commandData.append(char(aValue.toInt()));

	return processCommand(CShtrihFR::Commands::SetFRParameter, commandData);
}

//--------------------------------------------------------------------------------
template<class T>
bool ProtoShtrihFR<T>::getFRParameter(const CShtrihFR::FRParameters::SData & aData, QByteArray & aValue, char aSeries)
{
	if (mProcessingErrors.contains(CShtrihFR::Errors::RAM))
	{
		return false;
	}

	QByteArray commandData;
	commandData.append(char(aData.table));
	commandData.append(getHexReverted(aSeries, 2));
	commandData.append(char(aData.field));
	QByteArray data;

	if (!processCommand(CShtrihFR::Commands::GetFRParameter, commandData, &data))
	{
		toLog(LogLevel::Error, QString("ShtrihFR: Failed to get field %1 (series %2) of system table %3").arg(aData.field).arg(uint(aSeries)).arg(aData.table));
		return false;
	}

	aValue = data.mid(2);

	return !aValue.isEmpty();
}

//--------------------------------------------------------------------------------
template<class T>
bool ProtoShtrihFR<T>::processXReport()
{
	if ((mModel == CShtrihFR::Models::ID::NeoService) && getLongStatus() && (mMode == CShtrihFR::InnerModes::SessionClosed))
	{
		toLog(LogLevel::Error, "ShtrihFR: Session is closed, exit!");
		return true;
	}

	if (processCommand(CShtrihFR::Commands::XReport))
	{
		return waitForChangeXReportMode();
	}

	return false;
}

//--------------------------------------------------------------------------------
template<class T>
bool ProtoShtrihFR<T>::waitForChangeXReportMode()
{
	QTime clockTimer;
	clockTimer.start();

	do
	{
		QTime clock = QTime::currentTime();

		// 3.1. запрашиваем статус
		TStatusCodes statusCodes;

		if (getStatus(statusCodes))
		{
			// 3.2. анализируем режим и подрежим, если печать X-отчета окончена - выходим
			if (!statusCodes.intersect(CFR::XReportFiscalErrors).isEmpty())
			{
				toLog(LogLevel::Error, "ShtrihFR: Failed to print X-Report, exit!");
				return false;
			}

			if ((mSubmode == CShtrihFR::InnerSubmodes::ActivePaperOff) ||
			    (mSubmode == CShtrihFR::InnerSubmodes::PassivePaperOff))
			{
				// 3.3. подрежим - закончилась бумага
				return false;
			}
			//если режим или подрежим - печать или печать отчета или X-отчета, то
			else if ((mMode == CShtrihFR::InnerModes::DataEjecting) ||
			         (mMode == CShtrihFR::InnerModes::PrintFullZReport) ||
			         (mMode == CShtrihFR::InnerModes::PrintEKLZReport) ||
			         (mSubmode == CShtrihFR::InnerSubmodes::PrintingFullReports) ||
			         (mSubmode == CShtrihFR::InnerSubmodes::Printing))
			{
				toLog(LogLevel::Normal, "ShtrihFR: service X-report process, wait...");
			}
			else if ((mMode == CShtrihFR::InnerModes::SessionOpened)    ||
			         (mMode == CShtrihFR::InnerModes::SessionClosed)    ||
			         (mMode == CShtrihFR::InnerModes::NeedCloseSession) ||
			         (mSubmode == CShtrihFR::InnerSubmodes::PaperOn))
			{
				// 3.3. режим - тот, который ожидаем, если X-отчет допечатался, все хорошо
				return true;
			}
			else
			{
				// 3.4. режим не тот, который ожидаем в соответствии с протоколом, выходим с ошибкой
				toLog(LogLevel::Error, QString("ShtrihFR: X-report, unknown mode.submode = %1.%2").arg(int(mMode)).arg(int(mSubmode)));
				return false;
			}

			//спим до периода опроса
			int sleepTime = CShtrihFR::Interval::ReportPoll - abs(clock.msecsTo(QTime::currentTime()));

			if (sleepTime > 0)
			{
				SleepHelper::msleep(sleepTime);
			}
		}
	}
	while(clockTimer.elapsed() < CShtrihFR::Timeouts::MaxXReportNoAnswer);

	toLog(LogLevel::Normal, "ShtrihFR: Timeout for X report.");

	//вышли по таймауту, значит, не смогли дождаться нужного режима/подрежима
	return false;
}

//--------------------------------------------------------------------------------
template<class T>
QVariantMap ProtoShtrihFR<T>::getSessionOutData(const QByteArray & aAnswer)
{
	QVariantMap result;
	result.insert(CFiscalPrinter::Serial, mSerial);
	result.insert(CFiscalPrinter::RNM, mRNM);

	result.insert(CFiscalPrinter::FRDateTime, getDateTime().toString(CFR::DateTimeLogFormat));

	QDateTime systemDateTime = QDateTime::currentDateTime();
	result.insert(CFiscalPrinter::SystemDateTime, systemDateTime.toString(CFR::DateTimeLogFormat));

	ushort lastClosedSession = 1 + revert(aAnswer.mid(36, 2)).toHex().toUShort(0, 16);
	result.insert(CFiscalPrinter::ZReportNumber, lastClosedSession);

	double paymentAmount = 0;
	QByteArray data;

	if (getRegister(CShtrihFR::Registers::PaymentAmount, data))
	{
		paymentAmount = revert(data).toHex().toInt(0, 16) / 100.0;
		result.insert(CFiscalPrinter::PaymentAmount, paymentAmount);
	}

	if (getRegister(CShtrihFR::Registers::PaymentCount, data))
	{
		int paymentCount = revert(data).toHex().toInt(0, 16);
		result.insert(CFiscalPrinter::PaymentCount, paymentCount);
	}

	if (mFiscalized)
	{
		mNonNullableAmount += paymentAmount;
		result.insert(CFiscalPrinter::NonNullableAmount, mNonNullableAmount);
	}

	return result;
}

//--------------------------------------------------------------------------------
template<class T>
bool ProtoShtrihFR<T>::execZReport(bool aAuto)
{
	toLog(LogLevel::Normal, QString("ShtrihFR: Begin processing %1Z-report").arg(aAuto ? "auto-" : ""));

	bool needCloseSession = mMode == CShtrihFR::InnerModes::NeedCloseSession;

	// проверяем, нормальный ли режим, делаем запрос статуса
	QByteArray answer;

	if (!getLongStatus(answer))
	{
		toLog(LogLevel::Error, QString("ShtrihFR: Failed to get status therefore failed to process %1Z-report.").arg(aAuto ? "auto-" : ""));
		mNeedCloseSession = mNeedCloseSession || needCloseSession;

		return false;
	}

	QVariantMap outData = getSessionOutData(answer);

	bool success = processCommand(CShtrihFR::Commands::ZReport);

	if (getLongStatus())
	{
		mNeedCloseSession = mMode == CShtrihFR::InnerModes::NeedCloseSession;
	}

	if (success)
	{
		emit FRSessionClosed(outData);
	}

	toLog(success ? LogLevel::Normal : LogLevel::Error, success ?
		"ShtrihFR: Z-report is successfully processed" :
		"ShtrihFR: error in processing Z-report");

	return success;
}

//--------------------------------------------------------------------------------
template<class T>
TResult ProtoShtrihFR<T>::getLongStatus()
{
	QByteArray answer;

	return getLongStatus(answer);
}

//--------------------------------------------------------------------------------
template<class T>
TResult ProtoShtrihFR<T>::getLongStatus(QByteArray & aAnswer)
{
	TResult result = processCommand(CShtrihFR::Commands::GetLongStatus, &aAnswer);

	if (CORRECT(result) && result && (aAnswer.size() > 16))
	{
		mMode    = aAnswer[15] & CShtrihFR::InnerModes::Mask;
		mSubmode = aAnswer[16];
	}

	return result;
}

//--------------------------------------------------------------------------------
template<class T>
ESessionState::Enum ProtoShtrihFR<T>::getSessionState()
{
	QByteArray data;

	if (!getLongStatus(data))
	{
		return ESessionState::Error;
	}

	bool result = (mMode == CShtrihFR::InnerModes::SessionOpened) || (mMode == CShtrihFR::InnerModes::NeedCloseSession);

	return result ? ESessionState::Opened : ESessionState::Closed;
}

//--------------------------------------------------------------------------------
template<class T>
void ProtoShtrihFR<T>::postPollingAction(const TStatusCollection & aNewStatusCollection, const TStatusCollection & aOldStatusCollection)
{
	// если нет ошибок и нужно продолжать печать - продолжаем
	if ((mSubmode == CShtrihFR::InnerSubmodes::NeedContinuePrinting) && isDeviceReady(false))
	{
		toLog(LogLevel::Normal, "ShtrihFR: The paper is in the printer, continue printing...");
		processCommand(CShtrihFR::Commands::ExtentionPrinting);
	}

	T::postPollingAction(aNewStatusCollection, aOldStatusCollection);
}

//--------------------------------------------------------------------------------
template<class T>
bool ProtoShtrihFR<T>::getRegister(const CShtrihFR::TRegisterId & aRegister, QByteArray & aFRRegister)
{
	CShtrihFR::Registers::SData data = CShtrihFR::Registers::Data.getInfo(aRegister);

	toLog(LogLevel::Normal, QString("ShtrihFR: Begin to get FR register %1, type %2 (%3)")
		.arg(aRegister.first).arg(data.typeDescription).arg(data.description));

	char command = (aRegister.second == CShtrihFR::ERegisterType::Money) ? CShtrihFR::Commands::GetMoneyRegister : CShtrihFR::Commands::GetOperationalRegister;
	QByteArray answer;
	bool processSuccess = processCommand(command, QByteArray(1, aRegister.first), &answer);

	aFRRegister = answer.mid(3);

	return processSuccess;
}

//--------------------------------------------------------------------------------
template<class T>
bool ProtoShtrihFR<T>::processAnswer(const QByteArray & aCommand)
{
	switch (mLastError)
	{
		case CShtrihFR::Errors::DocumentIsOpened :
		{
			mProcessingErrors.push_back(mLastError);

			return processCommand(CShtrihFR::Commands::CancelDocument);
		}
		//--------------------------------------------------------------------------------
		case CShtrihFR::Errors::BadModeForCommand :
		{
			mProcessingErrors.push_back(mLastError);

			if (getLongStatus())
			{
				switch (mMode)
				{
					case CShtrihFR::InnerModes::NeedCloseSession :
					{
						return execZReport(true);
					}
					case CShtrihFR::InnerModes::SessionOpened :
					{
						bool rightCommand = (aCommand[0] == CShtrihFR::Commands::Encashment) || (aCommand[0] == CShtrihFR::Commands::SetFRParameter);

						return rightCommand && execZReport(true);
					}
					case CShtrihFR::InnerModes::DocumentOpened :
					{
						return processCommand(CShtrihFR::Commands::CancelDocument);
					}
				}
			}

			return false;
		}
		//--------------------------------------------------------------------------------
		case CShtrihFR::Errors::NeedZReport :
		{
			mProcessingErrors.push_back(mLastError);

			return execZReport(true);
		}
		//--------------------------------------------------------------------------------
		case CShtrihFR::Errors::NeedExtentionPrinting :
		{
			mProcessingErrors.push_back(mLastError);

			return processCommand(CShtrihFR::Commands::ExtentionPrinting);
		}
		//--------------------------------------------------------------------------------
		case CShtrihFR::Errors::NeedWaitForPrinting :
		{
			mProcessingErrors.push_back(mLastError);

			return waitForPrintingEnd();
		}
	}

	return false;
}

//--------------------------------------------------------------------------------
template<class T>
bool ProtoShtrihFR<T>::waitForPrintingEnd(bool aCanBeOff)
{
	QTime clockTimer;
	clockTimer.start();

	do
	{
		QTime clock = QTime::currentTime();

		// 1. запрашиваем статус
		if (!getLongStatus())
		{
			if (aCanBeOff)
			{
				continue;
			}
			else
			{
				return false;
			}
		}

		if ((mSubmode == CShtrihFR::InnerSubmodes::ActivePaperOff) ||
			(mSubmode == CShtrihFR::InnerSubmodes::PassivePaperOff))
		{
			// закончилась бумага
			return false;
		}
		//если режим или подрежим - печать или печать отчета или Z-отчета или режим - исходный, то
		else if ((mMode    == CShtrihFR::InnerModes::Work) ||
		         (mSubmode == CShtrihFR::InnerSubmodes::Printing) ||
		         (mSubmode == CShtrihFR::InnerSubmodes::PrintingFullReports))
		{
			// 2. подрежим - идет печать. Ждем
			toLog(LogLevel::Normal, "ShtrihFR: printing, wait...");
		}
		else
		{
			// 4. подрежим - документ допечатался, все хорошо
			return true;
		}

		//спим до периода опроса
		int sleepTime = CShtrihFR::Interval::WaitForPrintingEnd - abs(clock.msecsTo(QTime::currentTime()));

		if (sleepTime > 0)
		{
			SleepHelper::msleep(sleepTime);
		}
	}
	while(clockTimer.elapsed() < CShtrihFR::Timeouts::MaxWaitForPrintingEnd);

	// 6. вышли по таймауту, значит, не смогли дождаться нужного режима/подрежима
	return false;
}

//--------------------------------------------------------------------------------
