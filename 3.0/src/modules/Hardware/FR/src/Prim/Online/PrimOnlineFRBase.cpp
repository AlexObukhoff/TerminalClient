/* @file Онлайн ФР семейства ПРИМ. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// Project
#include "PrimOnlineFRBase.h"
#include "PrimOnlineFRConstants.h"

using namespace SDK::Driver;
using namespace ProtocolUtils;

//--------------------------------------------------------------------------------
// Получить модели данной реализации.
namespace CPrimFR { inline TModels OnlineModels()
{
	return TModels();
}}

//--------------------------------------------------------------------------------
PrimOnlineFRBase::PrimOnlineFRBase()
{
	// данные устройства
	mIsOnline = true;
	mAFDFont = CPrimFR::FiscalFont::Default;

	// данные моделей
	mDeviceName = CPrimFR::DefaultOnlineModelName;
	mModels = CPrimFR::OnlineModels();
	mModel = CPrimFR::Models::OnlineUnknown;

	// типы оплаты
	mPayTypeData.data().clear();

	// команды
	mCommandTimouts.append(CPrimOnlineFR::Commands::GetFSStatus, 2 * 1000);
	mCommandTimouts.append(CPrimOnlineFR::Commands::GetFiscalTLVData, 5 * 1000);

	setConfigParameter(CHardwareSDK::CanOnline, true);

	// ошибки
	mErrorData = PErrorData(new CPrimOnlineFR::Errors::Data());
}

//--------------------------------------------------------------------------------
QStringList PrimOnlineFRBase::getModelList()
{
	return CPrimFR::getModelList(CPrimFR::OnlineModels());
}

//--------------------------------------------------------------------------------
bool PrimOnlineFRBase::updateParameters()
{
	TStatusCodes statusCodes;
	CPrimFR::TData data;

	if (!getStatusInfo(statusCodes, data) || (data.size() < 12))
	{
		return false;
	}

	mSerial = CFR::serialToString(data[7]);
	mRNM = CFR::RNMToString(data[8]);
	mINN = CFR::INNToString(data[9]);

	mFSSerialNumber = CFR::FSSerialToString(data[11]);

	QString firmware = data[10].simplified();
	setDeviceParameter(CDeviceData::Firmware, firmware);

	if (firmware.size() == 7)
	{
		QString textBuild = QString("%1.%2").arg(firmware.right(3)).arg(firmware[2]);
		double build = textBuild.toDouble();

		if (build)
		{
			mFFDFR = CPrimOnlineFR::getFFD(build);
			double actualFirmware = CPrimOnlineFR::getActualFirmware(mFFDFR);
			mOldFirmware = actualFirmware && (actualFirmware > build);

			//TODO: TLV-запрос для получения версии ФФД ФН -> mFFDFS (тег 1190), будет после 1.05
			mFFDFS = EFFD::F10;
		}

		setDeviceParameter(CDeviceData::ControllerBuild, textBuild, CDeviceData::Firmware);
		int DTDBuild = firmware.mid(1, 1).toInt(0, 16);

		if (DTDBuild)
		{
			setDeviceParameter(CDeviceData::FR::DTDBuild, DTDBuild, CDeviceData::Firmware);
		}

		QString codes = CPrimOnlineFR::getCodes(firmware);

		if (!codes.isEmpty())
		{
			setDeviceParameter(CDeviceData::Printers::Codes, codes, CDeviceData::Firmware);
		}

		setDeviceParameter(CDeviceData::Printers::PNESensor, firmware[3] != ASCII::Zero, CDeviceData::Firmware);
	}

	processDeviceData();

	if (mFFDFR > EFFD::F10)
	{
		getRegTLVData(CFR::FiscalFields::LotteryMode);
		getRegTLVData(CFR::FiscalFields::GamblingMode);
		getRegTLVData(CFR::FiscalFields::ExcisableUnitMode);
		getRegTLVData(CFR::FiscalFields::InAutomateMode);
	}

	if ((!mOperatorPresence && !checkParameters()) || !checkControlSettings())
	{
		return false;
	}

	if (!isFiscal())
	{
		return true;
	}

	CPrimFR::TData commandData = CPrimFR::TData() << CPrimFR::DontPrintFD << CPrimOnlineFR::LastRegistration;

	if (!processCommand(CPrimOnlineFR::Commands::GetRegistrationTotal, commandData, &data))
	{
		return false;
	}

	uchar taxSystemData;
	uchar operationModeData;

	if (!parseAnswerData(data, 8, "tax systems", taxSystemData) || !checkTaxSystems(char(taxSystemData)) ||
	    !parseAnswerData(data, 9, "operation modes", operationModeData) || !checkOperationModes(char(operationModeData)))
	{
		return false;
	}

	if (mFFDFR > EFFD::F10)
	{
		QString FSVersion = getDeviceParameter(CDeviceData::FS::Version).toString();
		bool canCheckAgentFlags = (mLastError != CPrimOnlineFR::Errors::NoRequiedData) && !FSVersion.contains(CPrimOnlineFR::FSNoAgentFlags);
		uchar FFData;

		if (canCheckAgentFlags && (!getRegTLVData(CFR::FiscalFields::AgentFlagsReg, FFData) || !checkAgentFlags(char(FFData))))
		{
			return false;
		}
	}

	return checkTaxes() && checkPayTypes();
}

//--------------------------------------------------------------------------------
bool PrimOnlineFRBase::checkPayTypes()
{
	mPayTypeData.data().clear();

	for (int i = 0; i < CPrimOnlineFR::PayTypeAmount; ++i)
	{
		CPrimFR::TData data;

		if (!processCommand(CPrimOnlineFR::Commands::GetPayTypeData, CPrimFR::TData() << int2String(i).toLatin1(), &data))
		{
			return false;
		}

		ushort field;

		if (parseAnswerData(data, 12, "pay type", field))
		{
			if (!mFFData.data().contains(field))
			{
				toLog(LogLevel::Warning, mDeviceName + ": Unknown fiscal field of pay type " + QString::number(field));
			}
			else if (!CPrimOnlineFR::PayTypeData.data().contains(field))
			{
				toLog(LogLevel::Warning, mDeviceName + ": Unknown fiscal field of pay type" + QString::number(field));
			}
			else
			{
				mPayTypeData.add(CPrimOnlineFR::PayTypeData[field], i);
			}
		}
	}

	if (mPayTypeData.data().isEmpty())
	{
		toLog(LogLevel::Error, mDeviceName + ": Pay type data is empty");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool PrimOnlineFRBase::getRegTLVData(int aField, uchar & aData)
{
	CPrimFR::TData commandData = CPrimFR::TData() << QByteArray::number(qToBigEndian(ushort(aField)), 16);

	return processCommand(CPrimOnlineFR::Commands::GetRegTLVData, commandData, 5, mFFData.getTextLog(aField), aData);
}

//--------------------------------------------------------------------------------
bool PrimOnlineFRBase::getRegTLVData(int aField)
{
	uchar FFData;

	if (!getRegTLVData(aField, FFData))
	{
		return false;
	}

	mFFEngine.setConfigParameter(mFFData[aField].textKey, FFData);
	toLog(LogLevel::Normal, mDeviceName + QString(": Add %1 = %2 to config data").arg(mFFData.getTextLog(aField)).arg(FFData));

	return true;
}

//--------------------------------------------------------------------------------
bool PrimOnlineFRBase::getStatus(TStatusCodes & aStatusCodes)
{
	CPrimFR::TData data;

	if (!PrimFRBase::getStatus(aStatusCodes))
	{
		return false;
	}

	ushort OFDNotSentCount;
	TResult result = processCommand(CPrimOnlineFR::Commands::GetOFDNotSentCount, 5, "OFD not sent fiscal documents count", OFDNotSentCount);

	if (!CORRECT(result))
	{
		return false;
	}
	else if (result == CommandResult::Device)
	{
		int statusCode = getErrorStatusCode(mErrorData->value(mLastError).type);
		aStatusCodes.insert(statusCode);
	}
	else if (result == CommandResult::Answer)
	{
		aStatusCodes.insert(DeviceStatusCode::Warning::OperationError);
	}
	else
	{
		checkOFDNotSentCount(OFDNotSentCount, aStatusCodes);
	}

	return true;
}

//--------------------------------------------------------------------------------
QDateTime PrimOnlineFRBase::getDateTime()
{
	CPrimFR::TData data;

	if (processCommand(CPrimOnlineFR::Commands::GetFSStatus, &data) && (data.size() > 16))
	{
		return QDateTime::fromString(data[10].insert(4, "20"), CPrimOnlineFR::FSDateTimeFormat);
	}

	return QDateTime();
}

//--------------------------------------------------------------------------------
void PrimOnlineFRBase::processDeviceData()
{
	CPrimFR::TData data;

	removeDeviceParameter(CDeviceData::FR::Session);

	if (processCommand(CPrimOnlineFR::Commands::GetFSStatus, &data))
	{
		uchar sessionStateData;

		if (parseAnswerData(data, 8, "session state", sessionStateData))
		{
			QString sessionState = sessionStateData ? CDeviceData::Values::Opened : CDeviceData::Values::Closed;
			setDeviceParameter(CDeviceData::FR::Session, sessionState);
		}

		loadDeviceData<uint>(data, CDeviceData::FR::FiscalDocuments, "fiscal documents count", 12);

		if (data.size() > 13)
		{
			if (data[13].size() == 6)
			{
				data[13] = data[13].insert(4, "20");
			}

			QDate date = QDate::fromString(data[13], CFR::DateFormat);

			if (date.isValid())
			{
				setDeviceParameter(CDeviceData::FS::ValidityData, date.toString(CFR::DateLogFormat));
			}
		}

		if (data.size() > 15)
		{
			setDeviceParameter(CDeviceData::FS::Version, clean(data[14]).data());
			setDeviceParameter(CDeviceData::Type, data[15].toUInt() ? "serial" : "debug", CDeviceData::FS::Version);
		}

		loadDeviceData<ushort>(data, CDeviceData::Count, "count", 16, CDeviceData::FR::Session);
	}

	if (processCommand(CPrimFR::Commands::GetStatus, &data))
	{
		loadDeviceData<uchar>(data, CDeviceData::FR::FreeReregistrations, "free reregistrations", 5);

		if (data.size() > 8)
		{
			QString dateTimedata = data[8] + data[7];
			QDateTime dateTime = QDateTime::fromString(dateTimedata.insert(4, "20"), CPrimOnlineFR::FSDateTimeFormat);

			if (dateTime.isValid())
			{
				setDeviceParameter(CDeviceData::FR::OpeningDate, dateTime.toString(CFR::DateTimeLogFormat), CDeviceData::FR::Session);
			}
		}
	}

	checkDateTime();

	mOFDDataError = !processCommand(CPrimOnlineFR::Commands::GetOFDData, &data) || (data.size() <= 9) ||
		!checkOFDData(data[9], getBufferFromString(data[5].right(2) + data[5].left(2)));
}

//--------------------------------------------------------------------------------
bool PrimOnlineFRBase::performFiscal(const QStringList & aReceipt, const SPaymentData & aPaymentData, quint32 * aFDNumber)
{
	if (!PrimFRBase::performFiscal(aReceipt, aPaymentData))
	{
		return false;
	}

	if (aFDNumber && !processCommand(CPrimOnlineFR::Commands::GetFSStatus, 12, "last document number in FS", *aFDNumber))
	{
		aFDNumber = 0;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool PrimOnlineFRBase::getFiscalFields(quint32 aFDNumber, TFiscalPaymentData & aFPData, TComplexFiscalPaymentData & aPSData)
{
	CPrimFR::TData commandData = CPrimFR::TData()
		<< QString("%1").arg(qToBigEndian(aFDNumber), 8, 16, QLatin1Char(ASCII::Zero)).toLatin1()
		<< int2ByteArray(CPrimOnlineFR::FiscalTLVDataFlags::Start);

	if (!processCommand(CPrimOnlineFR::Commands::GetFiscalTLVData, commandData))
	{
		return false;
	}

	commandData[1] = int2ByteArray(CPrimOnlineFR::FiscalTLVDataFlags::Get);
	CPrimFR::TData answer;

	if (!processCommand(CPrimOnlineFR::Commands::GetFiscalTLVData, commandData, &answer))
	{
		return false;
	}

	QRegExp regExp(CPrimOnlineFR::RegExpTLVData);

	for (int i = 5; i < answer.size(); ++i)
	{
		QString TLVData = mCodec->toUnicode(answer[i]);

		if (regExp.indexIn(TLVData) == -1)
		{
			toLog(LogLevel::Error, mDeviceName + QString(": Failed to parse TLV data %1 (%2)").arg(TLVData).arg(answer[i].toHex().data()));
			return false;
		}

		QStringList capturedData = regExp.capturedTexts();
		CFR::STLV TLV;

		bool OK;
		TLV.field = capturedData[1].toInt(&OK);

		if (!OK)
		{
			toLog(LogLevel::Error, mDeviceName + ": Failed to parse TLV field number " + capturedData[1]);
			return false;
		}

		TLV.data = mCodec->fromUnicode(capturedData[2]);

		if (TLV.field == CFR::FiscalFields::PayOffSubject)
		{
			aPSData << TFiscalPaymentData();
		}
		else if (mFFData.data().contains(TLV.field))
		{
			if (!CFR::FiscalFields::PayOffSubjectFields.contains(TLV.field))
			{
				setFPData(aFPData, TLV);
			}
			else if (!aPSData.isEmpty())
			{
				setFPData(aPSData.last(), TLV);
			}
		}
		else
		{
			toLog(LogLevel::Warning, QString("%1: Failed to add fiscal field %2 due to it is unknown").arg(mDeviceName).arg(TLV.field));
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
void PrimOnlineFRBase::setFPData(TFiscalPaymentData & aFPData, const CFR::STLV & aTLV)
{
	QString data = mCodec->toUnicode(aTLV.data);
	CFR::FiscalFields::SData FFData = mFFData[aTLV.field];

	if (FFData.type == CFR::FiscalFields::ETypes::UnixTime)
	{
		QDateTime dateTime = QDateTime::fromString(data.insert(6, "20"), CPrimOnlineFR::FFDateTimeFormat);
		mFFEngine.setFPData(aFPData, aTLV.field, dateTime);
	}
	else if (!FFData.isMoney)
	{
		mFFEngine.setFPData(aFPData, aTLV.field, data);
	}
	else
	{
		bool OK;
		double value = data.toDouble(&OK);

		if (OK)
		{
			mFFEngine.setFPData(aFPData, aTLV.field, qulonglong(100 * value));
		}
		else
		{
			toLog(LogLevel::Warning, mDeviceName + QString(": Failed to parse money data %1 for field %2").arg(data).arg(aTLV.field));
		}
	}
}

//--------------------------------------------------------------------------------
void PrimOnlineFRBase::setFiscalData(CPrimFR::TData & aCommandData, CPrimFR::TDataList & aAdditionalAFDData, const SPaymentData & aPaymentData, int aReceiptSize)
{
	QString serialNumber   = getConfigParameter(CHardware::FR::Strings::SerialNumber).toString();
	QString documentNumber = getConfigParameter(CHardware::FR::Strings::DocumentNumber).toString();
	QString INN            = getConfigParameter(CHardware::FR::Strings::INN).toString();
	QString cashier        = getConfigParameter(CHardware::FR::Strings::Cashier).toString();
	QString session        = getConfigParameter(CHardware::FR::Strings::Session).toString();
	QString receiptNumber  = getConfigParameter(CHardware::FR::Strings::ReceiptNumber).toString();
	QString total          = getConfigParameter(CHardware::FR::Strings::Total).toString();

	auto getAmountData = [&] (const TSum & aSum) -> QByteArray { return QString::number(qRound64(aSum * 100.0) / 100.0, ASCII::Zero, 2).toLatin1(); };
	auto getDataY = [&] (int aSize) -> int { return CPrimOnlineFR::AFD::LineSize::GField - aSize - 1; };
	auto getAmountY = [&] (const TSum & aSum) -> int { return getDataY(getAmountData(aSum).size()); };

	QByteArray sumData = getAmountData(getTotalAmount(aPaymentData));
	QByteArray FDType = aPaymentData.back ? CPrimFR::FDTypes::SaleBack : CPrimFR::FDTypes::Sale;

	QString cashierValue = mFFEngine.getConfigParameter(CFiscalSDK::Cashier).toString();
	QString cashierINN = CFR::INNToString(mFFEngine.getConfigParameter(CFiscalSDK::CashierINN).toByteArray());
	QString operatorId = CPrimFR::OperatorID;

	if (!cashierValue.isEmpty())
	{
		operatorId = cashierValue;

		if (!cashierINN.isEmpty() && (mFFDFR > EFFD::F10))
		{
			operatorId += "|" + cashierINN;
		}
	}

	int serialNumberY = serialNumber.size() + 2;
	int documentNumberY = getDataY(5);
	int timeY = getDataY(5);
	bool cashierExist = operatorId != CPrimFR::OperatorID;
	int cashierX = cashierExist ? 3 : 2;

	int startX = aReceiptSize;
	int lastX = startX + cashierX;

	int INNY = INN.size() + 2;
	int cashierY = cashierExist ? (cashier.size() + 2) : (INNY + 16);
	int totalY = getAmountY(getTotalAmount(aPaymentData));
	int receiptNumberY = getDataY(5);

	aCommandData
		<< addGFieldToBuffer(startX + 1, serialNumberY, mAFDFont)      // серийный номер
		<< addGFieldToBuffer(startX + 1, documentNumberY, mAFDFont)    // номер документа
		<< addGFieldToBuffer(startX + 2, 1, mAFDFont)                  // дата
		<< addGFieldToBuffer(startX + 2, timeY, mAFDFont)              // время
		<< addGFieldToBuffer(lastX  + 2, INNY, mAFDFont)               // ИНН
		<< addGFieldToBuffer(lastX  + 0, cashierY, mAFDFont) << mCodec->fromUnicode(operatorId)    // ID оператора
		<< addGFieldToBuffer(lastX  + 2, totalY, mAFDFont)   << sumData;    // сумма

	aAdditionalAFDData
		<< addArbitraryFieldToBuffer(startX + 1, 1, serialNumber, mAFDFont)
		<< addArbitraryFieldToBuffer(startX + 1, documentNumberY - (documentNumber.size() + 1), documentNumber, mAFDFont)
		<< addArbitraryFieldToBuffer(lastX  + 2, 1, INN, mAFDFont)
		<< addArbitraryFieldToBuffer(lastX  + 2, totalY - total.size() - 1, total, mAFDFont)

		// 1038 (номер смены)
		<< addArbitraryFieldToBuffer(lastX + 1, 1, session, mAFDFont)
		<<            addFiscalField(lastX + 1, 2 + session.size(), mAFDFont, CFR::FiscalFields::SessionNumber)

		// 1042 (номер чека)
		<< addArbitraryFieldToBuffer(lastX + 1, receiptNumberY - receiptNumber.size() - 1, receiptNumber, mAFDFont)
		<<            addFiscalField(lastX + 1, receiptNumberY, mAFDFont, CFR::FiscalFields::DocumentNumber);

	if (cashierExist)
	{
		aAdditionalAFDData << addArbitraryFieldToBuffer(aReceiptSize + cashierX, 1, cashier, mAFDFont);
	}

	// продажи
	lastX += 2;

	for (int i = 0; i < aPaymentData.unitDataList.size(); ++i)
	{
		SUnitData unitData = aPaymentData.unitDataList.value(i);
		int section = (unitData.section != -1) ? unitData.section : 1;
		QStringList data = QStringList()
			<< unitData.name
			<< getAmountData(unitData.sum)
			<< "1"
			<< int2String(section) +
			   int2String(mTaxData[unitData.VAT].group) +
			   int2String(CFR::PayOffSubjectMethodType) +    // 1214 (признак способа расчета)
			   int2String(unitData.payOffSubjectType)        // 1212 (признак предмета расчета)
			<< "";

		int addAFDDataIndex = aAdditionalAFDData.size();

		#define ADD_AFD_TAG(aX, aY, aField, ...) aAdditionalAFDData << addFiscalField(lastX + aX, aY, mAFDFont, CFR::FiscalFields::aField * 100 + i + 1, __VA_ARGS__);
		#define ADD_AFD_TAG_MULTI(aX, aY, aField, aData) ADD_AFD_TAG(aX, aY, aField, aData); lastX += int(newLine);

		if (mFFDFR > EFFD::F10)
		{
			TSum sum = unitData.sum;
			int amountY = getAmountY(sum);
			int VATY = getAmountY(sum * unitData.VAT / 100.0);

			int unitNameSize = unitData.name.size();
			int unitRest = (unitNameSize % CPrimOnlineFR::AFD::LineSize::GField) + int(unitNameSize > CPrimOnlineFR::AFD::LineSize::Unit);
			bool newLine = (amountY - unitRest) < 4;

			ADD_AFD_TAG_MULTI(1, 1, PayOffSubject, data.join("|"));    // 1059 (товар)
			ADD_AFD_TAG(1, amountY, PayOffSubjectUnitPrice);           // 1079 (цена)
			ADD_AFD_TAG(2,       1, PayOffSubjectQuantity);            // 1023 (количество)
			ADD_AFD_TAG(2, amountY, PayOffSubjectAmount);              // 1043 (стоимость)
			ADD_AFD_TAG(3,       1, VATRate);                          // 1199 (НДС, %)
			ADD_AFD_TAG(3,    VATY, PayOffSubjectTaxAmount);           // 1200 (НДС, сумма)
		}
		else
		{
			aAdditionalAFDData << addFiscalField(lastX + 1, 1, mAFDFont, CFR::FiscalFields::UnitName, data.join("|"));    // 1030 (наименование товара)
		}

		foreach (auto AFDData, aAdditionalAFDData.mid(addAFDDataIndex))
		{
			int newLastX = qToBigEndian(AFDData[0].toUShort(0, 16));
			lastX = qMax(lastX, newLastX);
		}
	}

	// 1057 (флаг агента)
	if ((mFFDFR > EFFD::F10) && (aPaymentData.agentFlag != EAgentFlags::None))
	{
		aAdditionalAFDData << addFiscalField(lastX + 1, 1, mAFDFont, CFR::FiscalFields::AgentFlagsReg, int2String(uchar(aPaymentData.agentFlag)));
	}

	// 1055 (СНО)
	if (aPaymentData.taxSystem != ETaxSystems::None)
	{
		int index = 0;

		while (~aPaymentData.taxSystem & (1 << index++)) {}

		aAdditionalAFDData << addFiscalField(lastX + 1, 25, mAFDFont, CFR::FiscalFields::TaxSystem, int2String(index));
	}

	QString userContact = mFFEngine.getConfigParameter(CFiscalSDK::UserContact).toString();

	if (!userContact.isEmpty())
	{
		if (mFFDFR >= EFFD::F105)
		{
			aAdditionalAFDData << addFiscalField(lastX + 2, 1, mAFDFont, CFR::FiscalFields::UserContact, userContact);
		}
		else
		{
			toLog(LogLevel::Warning, mDeviceName + QString(": Failed to transmit %1 due to FFD version = %2, need %3 min")
				.arg(mFFData.getTextLog(CFR::FiscalFields::UserContact)).arg(CFR::FFD[mFFDFR].description).arg(CFR::FFD[EFFD::F105].description));
		}
	}
}

//--------------------------------------------------------------------------------
bool PrimOnlineFRBase::performZReport(bool /*aPrintDeferredReports*/)
{
	return execZReport(false);
}

//--------------------------------------------------------------------------------
TResult PrimOnlineFRBase::doZReport(bool aAuto)
{
	CPrimFR::TData commandData = CPrimFR::TData() << CPrimFR::OperatorID << "" << "";    // + сообщение для ОФД и доп. реквизит
	commandData << (aAuto ? CPrimOnlineFR::ZReportInBuffer : CPrimOnlineFR::ZReportOut);

	return processCommand(CPrimFR::Commands::ZReport, commandData);
}

//--------------------------------------------------------------------------------
bool PrimOnlineFRBase::openSession()
{
	CPrimFR::TData commandData = CPrimFR::TData() << CPrimFR::OperatorID << "" << "" << "";    // + сообщение оператору, доп. реквизит и реквизиты смены

	return processCommand(CPrimFR::Commands::OpenFRSession, commandData);
}

//--------------------------------------------------------------------------------
bool PrimOnlineFRBase::processAnswer(char aError)
{
	if (aError == CPrimOnlineFR::Errors::FSOfflineEnd)
	{
		mProcessingErrors.push_back(mLastError);

		mFSOfflineEnd = true;
	}

	return PrimFRBase::processAnswer(aError);
}


//--------------------------------------------------------------------------------
int PrimOnlineFRBase::getVerificationCode()
{
	int result;

	if (!processCommand(CPrimOnlineFR::Commands::GetFSStatus, 12, "last document number in FS", result))
	{
		return 0;
	}

	return result;
}

//--------------------------------------------------------------------------------
CPrimFR::TData PrimOnlineFRBase::addFiscalField(int aX, int aY, int aFont, int aFiscalField, const QString & aData)
{
	return addArbitraryFieldToBuffer(aX, aY, QString("<%1>").arg(aFiscalField) + aData, aFont);
}

//--------------------------------------------------------------------------------
