/* @file Онлайн ФР семейства MStar на протоколе AFP. */

#include "AFPFR.h"
#include "ModelData.h"

using namespace SDK::Driver;
using namespace ProtocolUtils;

//--------------------------------------------------------------------------------
AFPFR::AFPFR()
{
	using namespace SDK::Driver::IOPort::COM;

	// данные порта
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR115200);   // USB
	mPortParameters[EParameters::Parity].append(EParity::No);

	// параметры семейства ФР
	mDeviceName = CAFPFR::Models::Default;
	mIsOnline = true;
	mLineFeed = false;
	mNextReceiptProcessing = false;

	setConfigParameter(CHardwareSDK::CanOnline, true);
	setConfigParameter(CHardwareSDK::FR::CanWithoutPrinting, true);

	// налоги. Таблица налогов программно недоступна.
	mTaxData.add(10, 1);
	mTaxData.add(18, 0);
	mTaxData.add( 0, 5);

	// ошибки
	mErrorData = PErrorData(new CAFPFR::Errors::Data());
}

//--------------------------------------------------------------------------------
QStringList AFPFR::getModelList()
{
	QList<CAFPFR::Models::SData> modelData = CAFPFR::Models::CData().data().values();
	QStringList result;

	foreach (auto data, modelData)
	{
		result << data.name;
	}

	return result;
}

//--------------------------------------------------------------------------------
bool AFPFR::getFRData(const CAFPFR::FRInfo::SData & aInfo, CAFPFR::TData & aData)
{
	QString log = QString("data by index %1 (%2)").arg(aInfo.index).arg(aInfo.name);
	CAFPFR::TAnswerTypes answerTypes = CAFPFR::Requests::Data[CAFPFR::Commands::GetFRData].answerTypes;
	answerTypes.removeLast();

	if (!processCommand(CAFPFR::Commands::GetFRData, aInfo.index, &aData, answerTypes + aInfo.answerTypes))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to get " + log);
		return false;
	}
	else if (aInfo.index != aData[0].toInt())
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Invalid data index in answer = %1 at the request of %2").arg(aData[0].toInt()).arg(log));
		return false;
	}

	aData = aData.mid(1);

	return true;
}

//--------------------------------------------------------------------------------
bool AFPFR::updateParameters()
{
	processDeviceData();

	if (!mIsOnline)
	{
		return true;
	}

	setFRParameter(CAFPFR::FRParameters::PrintingOnClose, true);
	CAFPFR::TData data;

	if (!processCommand(CAFPFR::Commands::GetFiscalizationTotal, 1, &data))
	{
		return false;
	}

	if (!checkTaxSystems(char(data[4].toInt())) || !checkOperationModes(char(data[5].toInt())))
	{
		return false;
	}

	if (!processCommand(CAFPFR::Commands::GetLastFiscalizationData, CFR::FiscalFields::AgentFlagsReg, &data, CAFPFR::EAnswerTypes::FInt) ||
	    !checkAgentFlags(char(data[0].toInt())))
	{
		return false;
	}

	return !mOperatorPresence || loadSectionNames();
}

//--------------------------------------------------------------------------------
bool AFPFR::loadSectionNames()
{
	TSectionNames sectionNames;

	for (int i = 1; i <= CAFPFR::SectionAmount; ++i)
	{
		QVariant data;

		if (!getFRParameter(CAFPFR::FRParameters::SectionName(i), data))
		{
			toLog(LogLevel::Error, mDeviceName + QString(": Failed to get name for %1 section").arg(i));
			return false;
		}

		sectionNames.insert(i, data.toString());
	}

	if (sectionNames.isEmpty())
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to get section names due to they are not exist");
		return false;
	}

	setConfigParameter(CHardwareSDK::FR::SectionNames, QVariant::fromValue<TSectionNames>(sectionNames));

	return true;
}

//--------------------------------------------------------------------------------
void AFPFR::processDeviceData()
{
	using namespace CAFPFR::EAnswerTypes;

	CAFPFR::TData FRData;

	if (getFRData(CAFPFR::FRInfo::SerialNumber, FRData)) mSerial = CFR::serialToString(FRData[0].toByteArray());
	if (getFRData(CAFPFR::FRInfo::INN, FRData)) mINN = CFR::INNToString(FRData[0].toByteArray());
	if (getFRData(CAFPFR::FRInfo::RNM, FRData)) mRNM = CFR::RNMToString(FRData[0].toByteArray());
	if (getFRData(CAFPFR::FRInfo::FFDFR, FRData)) mFFDFR = EFFD::Enum(FRData[0].toInt());
	if (getFRData(CAFPFR::FRInfo::FFDFS, FRData)) mFFDFS = EFFD::Enum(FRData[1].toInt());

	if (getFRData(CAFPFR::FRInfo::Firmware, FRData))
	{
		QString data = FRData[0].toString();

		if (data.size() >= 4)
		{
			data = data.insert(1, ASCII::Dot).insert(3, ASCII::Dot).insert(5, ASCII::Dot);
		}

		setDeviceParameter(CDeviceData::Firmware, data);

		mOldFirmware = DeviceUtils::isComplexFirmwareOld(data, mModelData.firmware);
	}

	if (getFRData(CAFPFR::FRInfo::FirmwareDate, FRData))
	{
		setDeviceParameter(CDeviceData::Date, FRData[0].toDate().toString(CFR::DateLogFormat), CDeviceData::Firmware);
	}

	if (getFRData(CAFPFR::FRInfo::TotalPaySum, FRData))
	{
		setDeviceParameter(CDeviceData::FR::TotalPaySum, FRData[0].toDouble());
	}

	if (getFRData(CAFPFR::FRInfo::LastRegDate, FRData))
	{
		setDeviceParameter(CDeviceData::FR::LastRegistrationDate, FRData[0].toDate().toString(CFR::DateLogFormat));
	}

	CAFPFR::TData answerData;

	if (processCommand(CAFPFR::Commands::GetFSStatus, &answerData))
	{
		mFiscalized = answerData[0].toInt() == CAFPFR::FSData::FiscalMode;
		setDeviceParameter(CDeviceData::FR::Session, answerData[0].toInt() ? CDeviceData::Values::Opened : CDeviceData::Values::Closed);
		setDeviceParameter(CDeviceData::FR::FiscalDocuments, answerData[8].toInt());

		mFSSerialNumber = CFR::FSSerialToString(answerData[7].toByteArray());

		setDeviceParameter(CDeviceData::FS::Version, QString("%1, type %2").arg(answerData[9].toString()).arg(answerData[10].toInt() ? "serial" : "debug"));
		setDeviceParameter(CDeviceData::FR::FreeReregistrations,  answerData[12].toInt());
		setDeviceParameter(CDeviceData::FR::ReregistrationNumber, answerData[13].toInt());

		QDate date = answerData[11].toDate();
		setDeviceParameter(CDeviceData::FS::ValidityData, CFR::FSValidityDateOff(date));
	}

	QVariant addressData;
	QVariant portData;
	mOFDDataError = !getFRParameter(CAFPFR::FRParameters::OFDAddress, addressData) ||
	                !getFRParameter(CAFPFR::FRParameters::OFDPort, portData);
	portData = getBufferFromString(QByteArray::number(portData.toInt(), 16));
	mOFDDataError = mOFDDataError || !checkOFDData(addressData.toByteArray(), portData.toByteArray());

	checkDateTime();
}

//--------------------------------------------------------------------------------
QDateTime AFPFR::getDateTime()
{
	CAFPFR::TData answerData;

	if (processCommand(CAFPFR::Commands::GetFRDateTime, &answerData))
	{
		return QDateTime(answerData[0].toDate(), answerData[1].toTime());
	}

	return QDateTime();
}

//--------------------------------------------------------------------------------
bool AFPFR::getFRParameter(const CAFPFR::FRParameters::SData & aData, QVariant & aValue)
{
	CAFPFR::TData commandData = CAFPFR::TData() << aData.number << aData.index;
	CAFPFR::TData answerData;

	if (!processCommand(CAFPFR::Commands::GetFRParameter, commandData, &answerData, aData.answerType))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to get " + aData.log());
		return false;
	}

	aValue = answerData[0];

	return true;
}

//--------------------------------------------------------------------------------
bool AFPFR::setFRParameter(const CAFPFR::FRParameters::SData & aData, QVariant aValue)
{
	if ((aData.bit != CAFPFR::FRParameters::NoBit) && ((aData.answerType == CAFPFR::EAnswerTypes::Int) || (aData.answerType == CAFPFR::EAnswerTypes::FInt)))
	{
		QVariant value;

		if (!getFRParameter(aData, value))
		{
			return false;
		}

		int data = value.toInt();
		int mask = 1 << aData.bit;
		int newData = int(aValue.toBool()) << aData.bit;

		data &= ~mask;
		data |= newData;

		aValue = data;

		if (aValue == value)
		{
			return true;
		}
	}

	CAFPFR::TData commandData = CAFPFR::TData() << aData.number << aData.index << aValue;

	if (!processCommand(CAFPFR::Commands::SetFRParameter, commandData))
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to set %1 = %2").arg(aData.log()).arg(aValue.toString()));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool AFPFR::isConnected()
{
	CAFPFR::TData FRData;

	if (!getFRData(CAFPFR::FRInfo::ModelId, FRData))
	{
		return false;
	}

	QString modelId = FRData[0].toString();
	mModelData = CAFPFR::Models::Data[modelId];
	mVerified = mModelData.verified;
	mDeviceName = mModelData.name;

	if (mDeviceName == CAFPFR::Models::Default)
	{
		toLog(LogLevel::Error, mDeviceName + ": Unknown model Id = " + modelId);
	}

	mModelCompatibility = true;

	return true;
}

//--------------------------------------------------------------------------------
TResult AFPFR::processCommand(char aCommand, CAFPFR::TData * aAnswer)
{
	return processCommand(aCommand, CAFPFR::TData(), aAnswer);
}

//--------------------------------------------------------------------------------
TResult AFPFR::processCommand(char aCommand, const QVariant & aCommandData, CAFPFR::TData * aAnswer)
{
	return processCommand(aCommand, CAFPFR::TData() << aCommandData, aAnswer);
}

//--------------------------------------------------------------------------------
TResult AFPFR::processCommand(char aCommand, const CAFPFR::TData & aCommandData, CAFPFR::TData * aAnswer)
{
	return processCommand(aCommand, aCommandData, aAnswer, CAFPFR::TAnswerTypes());
}

//--------------------------------------------------------------------------------
TResult AFPFR::processCommand(char aCommand, const CAFPFR::TData & aCommandData, CAFPFR::TData * aAnswer, CAFPFR::EAnswerTypes::Enum aAnswerType)
{
	return processCommand(aCommand, aCommandData, aAnswer, CAFPFR::TAnswerTypes() << aAnswerType);
}

//--------------------------------------------------------------------------------
TResult AFPFR::processCommand(char aCommand, const QVariant & aCommandData, CAFPFR::TData * aAnswer, CAFPFR::EAnswerTypes::Enum aAnswerType)
{
	return processCommand(aCommand, CAFPFR::TData() << aCommandData, aAnswer, CAFPFR::TAnswerTypes() << aAnswerType);
}

//--------------------------------------------------------------------------------
TResult AFPFR::processCommand(char aCommand, const QVariant & aCommandData, CAFPFR::TData * aAnswer, const CAFPFR::TAnswerTypes & aAnswerTypes)
{
	return processCommand(aCommand, CAFPFR::TData() << aCommandData, aAnswer, aAnswerTypes);
}

//--------------------------------------------------------------------------------
TResult AFPFR::processCommand(char aCommand, const CAFPFR::TData & aCommandData, CAFPFR::TData * aAnswer, const CAFPFR::TAnswerTypes & aAnswerTypes)
{
	mProtocol.setPort(mIOPort);
	mProtocol.setLog(mLog);

	QByteArray commandData;

	foreach(auto dataItem, aCommandData)
	{
		QVariant::Type type = dataItem.type();
		QByteArray data;

		     if  (type == QVariant::String)  data = mCodec->fromUnicode(dataItem.toString());
		else if ((type == QVariant::ByteArray) ||
		         (type == QVariant::Double)) data = dataItem.toByteArray();
		else                                 data = QByteArray::number(dataItem.toULongLong());

		commandData += CAFPFR::Separator + data;
	}

	QByteArray command = QByteArray::number(uchar(aCommand), 16).rightJustified(2, ASCII::Zero).right(2).toUpper();
	commandData.replace(0, 1, command);

	QByteArray answer;
	mLastError = 0;

	mLastCommandResult = mProtocol.processCommand(commandData, answer, CAFPFR::Requests::Data[aCommand].timeout);

	if (!mLastCommandResult)
	{
		return mLastCommandResult;
	}

	bool OK;
	QByteArray errorData = answer.left(2);
	char error = char(errorData.toUShort(&OK, 16));

	if (!OK)
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to parse error = 0x%1").arg(errorData.toHex().data()));
		return false;
	}

	mLastError = error;
	mLastCommand = QByteArray(1, aCommand);
	QList<QByteArray> answerData = answer.mid(2).split(CAFPFR::Separator);

	if (answer.endsWith(CAFPFR::Separator))
	{
		answerData.removeLast();
	}

	if (!mLastError)
	{
		if (aAnswer)
		{
			aAnswer->clear();

			int answerSize = answerData.size();
			CAFPFR::Requests::SData requestData = CAFPFR::Requests::Data[aCommand];
			CAFPFR::TAnswerTypes answerTypes = requestData.answerTypes;
			int size = requestData.answerTypes.size();

			if (!aAnswerTypes.isEmpty())
			{
				answerTypes = aAnswerTypes;
				size = aAnswerTypes.size();
			}

			if (answerSize < size)
			{
				toLog(LogLevel::Error, mDeviceName + QString(": Wrong answer quantity of parts = %1, need = %2").arg(size).arg(size));
				return CommandResult::Answer;
			}

			for (int i = 0; i < size; ++i)
			{
				QByteArray part = answerData[i].simplified();
				int answerType = answerTypes[i];

				switch (answerType)
				{
					case CAFPFR::EAnswerTypes::Unknown : *aAnswer << part; break;
					case CAFPFR::EAnswerTypes::String  : *aAnswer << mCodec->toUnicode(part); break;
					case CAFPFR::EAnswerTypes::FString :
					{
						if (part.isEmpty())
						{
							toLog(LogLevel::Error, mDeviceName + QString(": Failed to parse string of answer part [%1] due to it is empty").arg(i));
							return CommandResult::Answer;
						}

						*aAnswer << mCodec->toUnicode(part);

						break;
					}
					case CAFPFR::EAnswerTypes::Date :
					{
						QDate result = QDate::fromString(part.insert(4, "20"), CAFPFR::DateFormat);

						if (!result.isValid())
						{
							toLog(LogLevel::Error, mDeviceName + QString(": Failed to parse date of answer part [%1] = %2").arg(i).arg(part.data()));
							return CommandResult::Answer;
						}

						*aAnswer << result;

						break;
					}
					case CAFPFR::EAnswerTypes::Time :
					{
						QTime result = QTime::fromString(part, CAFPFR::TimeFormat);

						if (!result.isValid())
						{
							toLog(LogLevel::Error, mDeviceName + QString(": Failed to parse time of answer part [%1] = %2").arg(i).arg(part.data()));
							return CommandResult::Answer;
						}

						*aAnswer << result;

						break;
					}
					case CAFPFR::EAnswerTypes::Int  :
					case CAFPFR::EAnswerTypes::FInt :
					{
						if (part.isEmpty())
						{
							if (answerType == CAFPFR::EAnswerTypes::FInt)
							{
								toLog(LogLevel::Error, mDeviceName + QString(": Failed to parse int of answer part [%1] due to it is empty").arg(i));
								return CommandResult::Answer;
							}

							*aAnswer << 0;

							break;
						}

						QRegExp regExp("^[0-9]+$");

						if (regExp.indexIn(part) == -1)
						{
							toLog(LogLevel::Error, mDeviceName + QString(": Failed to parse int of answer part [%1] = %2").arg(i).arg(part.data()));
							return CommandResult::Answer;
						}

						*aAnswer << part.toULongLong();

						break;
					}
					case CAFPFR::EAnswerTypes::Double :
					{
						QRegExp regExp("^[0-9\\.]+$");

						if (!part.isEmpty() && (regExp.indexIn(part) == -1))
						{
							toLog(LogLevel::Error, mDeviceName + QString(": Failed to parse double of answer part [%1] = %2").arg(i).arg(part.data()));
							return CommandResult::Answer;
						}

						*aAnswer << (part.isEmpty() ? 0 : part.toDouble());

						break;
					}
					default:
					{
						toLog(LogLevel::Error, mDeviceName + QString(": Failed to parse answer part [%1] = %2 due to unknown type %3").arg(i).arg(part.data()).arg(requestData.answerTypes[i]));
						return CommandResult::Driver;
					}
				}
			}
		}

		return CommandResult::OK;
	}

	toLog(LogLevel::Error, mDeviceName + ": Error: " + mErrorData->value(mLastError).description);

	if (!answerData.isEmpty())
	{
		QStringList logData;

		for (int i = 0; i < answerData.size(); ++i)
		{
			logData << mCodec->toUnicode(answerData[i]);
		}

		logData.removeAll("");

		if (!logData.isEmpty())
		{
			QString log = logData.join("\n1C\n").remove("\r");
			logData = log.split("\n");
			logData.removeAll("");
			toLog(LogLevel::Error, mDeviceName + ": Stack:\n" + logData.join("\n"));
		}
	}

	if (!isErrorUnprocessed(aCommand, error))
	{
		setErrorFlags();
	}

	if (!mProcessingErrors.isEmpty() && (mProcessingErrors.last() == mLastError))
	{
		return CommandResult::Device;
	}

	if (isErrorUnprocessed(aCommand, error) || !processAnswer(aCommand, error))
	{
		mLastError = error;
		mLastCommand = QByteArray(1, aCommand);

		return CommandResult::Device;
	}

	TResult result = processCommand(aCommand, aCommandData, aAnswer, aAnswerTypes);

	if (result)
	{
		mProcessingErrors.pop_back();
	}

	return result;
}

//--------------------------------------------------------------------------------
bool AFPFR::processAnswer(char aCommand, char aError)
{
	switch (aError)
	{
		case CAFPFR::Errors::NeedZReport:
		{
			mProcessingErrors.append(aError);

			return execZReport(true) && openFRSession();
		}
		case CAFPFR::Errors::WrongState:
		{
			mProcessingErrors.append(aError);

			if (aCommand == CAFPFR::Commands::CancelDocument)
			{
				return false;
			}

			if (getDocumentState() == EDocumentState::Opened)
			{
				return processCommand(CAFPFR::Commands::CancelDocument);
			}
		}
		case CAFPFR::Errors::UnknownCommand:
		{
			mOldFirmware = mOldFirmware || (aCommand == CAFPFR::Commands::GetFRData);

			break;
		}
	}

	return false;
}

//--------------------------------------------------------------------------------
bool AFPFR::performFiscal(const QStringList & aReceipt, const SPaymentData & aPaymentData, quint32 * aFDNumber)
{
	if ((getDocumentState() == EDocumentState::Opened) && !processCommand(CAFPFR::Commands::CancelDocument))
	{
		return false;
	}

	// СНО
	char taxSystem = char(aPaymentData.taxSystem);

	if ((taxSystem != ETaxSystems::None) && (mTaxSystems.size() != 1) && !processCommand(CAFPFR::Commands::SetTaxSystem, taxSystem))
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to set taxation system %1 (%2)").arg(toHexLog(taxSystem)).arg(CFR::TaxSystems[taxSystem]));
		return false;
	}

	// флаг агента
	char agentFlag = char(aPaymentData.agentFlag);

	if ((agentFlag != EAgentFlags::None) && (mAgentFlags.size() != 1) && !processCommand(CAFPFR::Commands::SetAgentFlag, agentFlag))
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to set agent flag %1 (%2)").arg(toHexLog(agentFlag)).arg(CFR::AgentFlags[agentFlag]));
		return false;
	}

	// фискальные теги, ассоциированные с флагом агента
	bool isBankAgent    = CFR::isBankAgent(aPaymentData.agentFlag);
	bool isPaymentAgent = CFR::isPaymentAgent(aPaymentData.agentFlag);

	#define CHECK_AFP_FF(aField) setFRParameter(CAFPFR::FRParameters::aField, mFFEngine.getConfigParameter(CFiscalSDK::aField))

	if (isNotPrinting() && !CHECK_AFP_FF(SenderMail))
	{
		return false;
	}

	if (isBankAgent)
	{
		if (!CHECK_AFP_FF(TransferOperatorAddress) ||
			!CHECK_AFP_FF(TransferOperatorINN)     ||
			!CHECK_AFP_FF(TransferOperatorName)    ||
			!CHECK_AFP_FF(AgentOperation)          ||
			!CHECK_AFP_FF(TransferOperatorPhone))
		{
			return false;
		}
	}
	else if (isPaymentAgent)
	{
		if (!CHECK_AFP_FF(ProcessingPhone))
		{
			return false;
		}
	}

	if (isBankAgent || isPaymentAgent)
	{
		if (!CHECK_AFP_FF(AgentPhone) ||
			!CHECK_AFP_FF(ProviderPhone))
		{
			return false;
		}
	}

	char FDType = CAFPFR::PayOffTypeData[aPaymentData.payOffType];

	if (!processReceipt(aReceipt, false) || !openDocument(FDType))
	{
		return false;
	}

	bool result = true;
	ExitAction exitAction([&] () { processCommand(CAFPFR::Commands::CancelDocument); });

	foreach (auto unitData, aPaymentData.unitDataList)
	{
		result = result && sale(unitData);
	}

	bool needSetUserContact = isNotPrinting() || mOperationModes.contains(EOperationModes::Internet);
	QVariant userContact = getConfigParameter(CFiscalSDK::UserContact);

	if (needSetUserContact && !processCommand(CAFPFR::Commands::SetUserContact, userContact))
	{
		return false;
	}

	CAFPFR::TData data = CAFPFR::TData()
		<< (aPaymentData.payType - 1)
		<< getTotalAmount(aPaymentData)
		<< "";

	if (!result || !processCommand(CAFPFR::Commands::Total, data) || !closeDocument(true))
	{
		return false;
	}

	if (aFDNumber && processCommand(CAFPFR::Commands::GetFSStatus, &data))
	{
		*aFDNumber = data[8].toUInt();
	}

	return exitAction.reset();
}

//--------------------------------------------------------------------------------
bool AFPFR::getFiscalFields(quint32 aFDNumber, TFiscalPaymentData & aFPData, TComplexFiscalPaymentData & aPSData)
{
	CAFPFR::TData answer;

	if (!processCommand(CAFPFR::Commands::GetFiscalTLVData, QVariant(aFDNumber), &answer))
	{
		return false;
	}

	QString rawData = answer[0].toString();
	QString log;

	if (!checkBufferString(rawData, &log))
	{
		toLog(LogLevel::Error, mDeviceName + ": " + log);
		return false;
	}

	QByteArray data = getBufferFromString(rawData);
	int i = 0;

	while ((i + 4) < data.size())
	{
		CFR::STLV TLV;

		auto getInt = [&data, &i] (int aIndex, int aShift) -> int { int result = uchar(data[i + aIndex]); return result << (8 * aShift); };
		int field = getInt(0, 0) | getInt(1, 1);
		int size  = getInt(2, 0) | getInt(3, 1);

		if (!size)
		{
			toLog(LogLevel::Warning, mDeviceName + ": Mo data for " + mFFData.getTextLog(field));
		}

		if (!mFFEngine.parseTLV(data.mid(i, size + 4), TLV))
		{
			return false;
		}

		mFFEngine.parseSTLVData(TLV, aPSData);
		mFFEngine.parseTLVData (TLV, aFPData);

		i += size + 4;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool AFPFR::sale(const SUnitData & aUnitData)
{
	QVariant section;

	if (aUnitData.section != -1)
	{
		section = aUnitData.section;
	}

	CAFPFR::TData commandData = CAFPFR::TData()
		<< aUnitData.name                       // название товара
		<< ""                                   // артикул или штриховой код товара/номер ТРК
		<< 1                                    // количество
		<< aUnitData.sum                        // цена
		<< mTaxData[aUnitData.VAT].group        // номер ставки налога
		<< ""                                   // номер товарной позиции
		<< section                              // номер секции
		<< ""                                   // код товара
		<< ""                                   // ИНН поставщика
		<< aUnitData.payOffSubjectMethodType    // признак способа расчёта (1214)
		<< aUnitData.payOffSubjectType          // признак предмета расчёта (1212)
		<< ""                                   // доп. реквизит предмета расчёта
		<< ""                                   // код страны происхождения товара
		<< ""                                   // номер таможенной декларации
		<< 0.00                                 // акциз
		<< 0                                    // признак агента по предмету расчёта
		<< ""                                   // единица измерения
		<< 0.00;                                // сумма НДС за предмет расчёта

	return processCommand(CAFPFR::Commands::Sale, commandData);
}

//--------------------------------------------------------------------------------
EResult::Enum AFPFR::performCommand(TStatusCodes & aStatusCodes, char aCommand, CAFPFR::TData & aAnswerData)
{
	TResult result = processCommand(aCommand, &aAnswerData);

	if (result == CommandResult::Device)
	{
		int statusCode = getErrorStatusCode(mErrorData->value(mLastError).type);
		aStatusCodes.insert(statusCode);
	}
	else if (result == CommandResult::Answer)
	{
		aStatusCodes.insert(DeviceStatusCode::Warning::OperationError);
	}
	else if (result)
	{
		return EResult::OK;
	}

	return !CORRECT(result) ? EResult::Fail : EResult::Error;
}

//--------------------------------------------------------------------------------
bool AFPFR::getStatus(TStatusCodes & aStatusCodes)
{
	CAFPFR::TData answerData;
	EResult::Enum result = performCommand(aStatusCodes, CAFPFR::Commands::GetPrinterStatus, answerData);

	if (result == EResult::Fail)
	{
		return false;
	}
	else if (result != EResult::Error)
	{
		CAFPFR::Statuses::Printer.getSpecification(char(answerData[0].toInt()), aStatusCodes);
	}

	result = performCommand(aStatusCodes, CAFPFR::Commands::GetFRStatus, answerData);

	if (result == EResult::Fail)
	{
		return false;
	}
	else if (result != EResult::Error)
	{
		CAFPFR::Statuses::FR.getSpecification(char(answerData[0].toInt()), aStatusCodes);
	}

	result = performCommand(aStatusCodes, CAFPFR::Commands::GetOFDStatus, answerData);

	if (result == EResult::Fail)
	{
		return false;
	}
	else if (result != EResult::Error)
	{
		checkOFDNotSentCount(answerData[2].toInt(), aStatusCodes);
	}

	return true;
}

//--------------------------------------------------------------------------------
bool AFPFR::processReceipt(const QStringList & aReceipt, bool aProcessing)
{
	if (!isPrintingNeed(aReceipt))
	{
		return true;
	}

	if (!aProcessing)
	{
		return TSerialFRBase::processReceipt(aReceipt, false);
	}

	if (!openDocument(CAFPFR::DocumentTypes::NonFiscal))
	{
		return false;
	}

	bool result = TSerialFRBase::processReceipt(aReceipt, false);
	bool processing = closeDocument(true);

	return result && processing;
}

//--------------------------------------------------------------------------------
bool AFPFR::printLine(const QByteArray & aString)
{
	uint tags = ASCII::NUL;

	for (auto it = CAFPFR::Tags.data().begin(); it != CAFPFR::Tags.data().end(); ++it)
	{
		if (mLineTags.contains(it.key()))
		{
			tags |= uchar(it.value());
		}
	}

	return processCommand(CAFPFR::Commands::PrintLine, CAFPFR::TData() << aString << tags);
}

//--------------------------------------------------------------------------------
bool AFPFR::cut()
{
	return true;
}

//--------------------------------------------------------------------------------
bool AFPFR::openDocument(char aType)
{
	CAFPFR::TData commandData = CAFPFR::TData() << aType << "" << "" << "";

	if (aType != CAFPFR::DocumentTypes::NonFiscal)
	{
		QString cashier = mFFEngine.getConfigParameter(CFiscalSDK::Cashier).toString();
		commandData[2] = mCodec->fromUnicode(cashier);
	}

	return processCommand(CAFPFR::Commands::OpenDocument, commandData);
}

//--------------------------------------------------------------------------------
bool AFPFR::closeDocument(bool aProcessing)
{
	return processCommand(CAFPFR::Commands::CloseDocument, CAFPFR::TData() << !aProcessing);
}

//--------------------------------------------------------------------------------
ESessionState::Enum AFPFR::getSessionState()
{
	CAFPFR::TData answerData;

	if (!processCommand(CAFPFR::Commands::GetFRStatus, &answerData))
	{
		return ESessionState::Error;
	}

	char flags = char(answerData[0].toInt());

	     if (~flags & CAFPFR::SessionOpened ) return ESessionState::Closed;
	else if ( flags & CAFPFR::SessionExpired) return ESessionState::Expired;

	return ESessionState::Opened;
}

//--------------------------------------------------------------------------------
EDocumentState::Enum AFPFR::getDocumentState()
{
	CAFPFR::TData answerData;

	if (!processCommand(CAFPFR::Commands::GetFRStatus, &answerData))
	{
		return EDocumentState::Error;
	}

	return answerData[1].toInt() ? EDocumentState::Opened : EDocumentState::Closed;
}

//--------------------------------------------------------------------------------
bool AFPFR::setNotPrintDocument(bool aEnabled, bool /*aZReport*/)
{
	return setFRParameter(CAFPFR::FRParameters::NotPrintDocument, aEnabled);
}

//--------------------------------------------------------------------------------
bool AFPFR::openSession()
{
	QString cashier;

	if (mOperatorPresence && !mFFEngine.checkCashier(cashier))
	{
		return false;
	}

	checkNotPrinting(true);
	bool result = processCommand(CAFPFR::Commands::OpenSession, CAFPFR::TData() << cashier);
	checkNotPrinting();

	return result;
}

//--------------------------------------------------------------------------------
bool AFPFR::processXReport()
{
	return processCommand(CAFPFR::Commands::XReport);
}

//--------------------------------------------------------------------------------
bool AFPFR::performZReport(bool /*aPrintDeferredReports*/)
{
	return execZReport(false);
}

//--------------------------------------------------------------------------------
bool AFPFR::execZReport(bool aAuto)
{
	toLog(LogLevel::Normal, mDeviceName + QString(": Begin processing %1Z-report").arg(aAuto ? "auto-" : ""));
	ESessionState::Enum sessionState = getSessionState();

	     if (sessionState == ESessionState::Error)  return false;
	else if (sessionState == ESessionState::Closed) return true;

	bool needCloseSession = sessionState == ESessionState::Expired;

	if (aAuto)
	{
		if (mOperatorPresence)
		{
			toLog(LogLevel::Error, mDeviceName + ": Failed to process auto-Z-report due to presence of the operator.");
			mNeedCloseSession = mNeedCloseSession || needCloseSession;

			return false;
		}
		else if (!checkNotPrinting(aAuto, true))
		{
			mNeedCloseSession = mNeedCloseSession || needCloseSession;

			return false;
		}
	}

	QVariantMap outData = getSessionOutData();

	bool result = processCommand(CAFPFR::Commands::ZReport);
	mNeedCloseSession = getSessionState() == ESessionState::Expired;

	if (!result)
	{
		toLog(LogLevel::Error, mDeviceName + ": error in processing Z-report");
		return false;
	}

	emit FRSessionClosed(outData);

	toLog(LogLevel::Normal, mDeviceName + ": Z-report is successfully processed");

	return true;
}

//--------------------------------------------------------------------------------
double AFPFR::getAmountInCash()
{
	CAFPFR::TData data;

	return getFRData(CAFPFR::FRInfo::TotalCash, data) ? data[0].toDouble() : -1;
}

//--------------------------------------------------------------------------------
bool AFPFR::processPayout(double aAmount)
{
	if (!openDocument(CAFPFR::DocumentTypes::PayOut))
	{
		return false;
	}

	ExitAction exitAction([&] () { processCommand(CAFPFR::Commands::CancelDocument); });

	return processCommand(CAFPFR::Commands::PayIO, CAFPFR::TData() << "" << aAmount) && closeDocument(true) && exitAction.reset();
}

//--------------------------------------------------------------------------------
QVariantMap AFPFR::getSessionOutData()
{
	return QVariantMap();
}

//--------------------------------------------------------------------------------
void AFPFR::setErrorFlags()
{
	
}

//--------------------------------------------------------------------------------
