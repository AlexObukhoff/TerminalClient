/* @file Онлайн ФР семейства ПРИМ. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// Project
#include "PrimOnlineFRBase.h"
#include "PrimOnlineFRConstants.h"

using namespace SDK::Driver;

//--------------------------------------------------------------------------------
PrimOnlineFRBase::PrimOnlineFRBase()
{
	// данные устройства
	mErrorData = PErrorData(new CPrimOnlineFR::Errors::Specification);
	mIsOnline = true;

	// типы оплаты
	mPayTypeData.add(EPayTypes::Cash,         0);
	mPayTypeData.add(EPayTypes::EMoney,       1);
	mPayTypeData.add(EPayTypes::PostPayment,  2);
	mPayTypeData.add(EPayTypes::Credit,       3);
	mPayTypeData.add(EPayTypes::CounterOffer, 4);

	// данные команд
	mCommandTimouts.append(CPrimOnlineFR::Commands::GetFSStatus, 2 * 1000);

	setConfigParameter(CHardwareSDK::CanOnline, true);
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

	int firmware = data[10].simplified().toInt();
	setDeviceParameter(CDeviceData::Firmware, firmware);

	mFSSerialNumber = CFR::FSSerialToString(data[11]);

	mFFDFR = EFFD::Enum((firmware % 1000) / 100 + 1);
	int actualFirmware = CPrimOnlineFR::getActualFirmware(mFFDFR);
	mOldFirmware = actualFirmware && (actualFirmware > (firmware % 100));

	if (mFFDFR > EFFD::F10)
	{
		//TODO: TLV-запрос для получения версии ФФД ФН -> mFFDFS (тег 1190)
	}
	else
	{
		mFFDFS = mFFDFR;
	}

	if (!processCommand(CPrimOnlineFR::Commands::GetRegistrationTotal, CPrimFR::TData() << CPrimFR::DontPrintFD, &data) || (data.size() <= 9))
	{
		return false;
	}

	char taxationData   = char(data[8].toInt(0, 16));
	char operationModes = char(data[9].toInt(0, 16));

	if (!checkTaxationData(taxationData) || !checkOperationModes(operationModes))
	{
		return false;
	}

	if (mFFDFR > EFFD::F10)
	{
		// TODO: запрос тега 1057
		/*
		if (!checkAgentFlags(agentFlagsData))
		{
			return false;
		}
		*/
	}

	if (!mOperatorPresence && isFiscal() && !checkParameters())
	{
		return false;
	}

	if (!checkControlSettings() || !checkTaxes())
	{
		return false;
	}

	processDeviceData();

	return true;
}

//--------------------------------------------------------------------------------
QDateTime PrimOnlineFRBase::getDateTime()
{
	CPrimFR::TData data;

	if (processCommand(CPrimOnlineFR::Commands::GetFSStatus, &data) && (data.size() > 16))
	{
		return QDateTime::fromString(data[10].insert(4, "20"), CPrimOnlineFR::DateTimeFormat);
	}

	return QDateTime();
}

//--------------------------------------------------------------------------------
void PrimOnlineFRBase::processDeviceData()
{
	CPrimFR::TData data;

	if (processCommand(CPrimOnlineFR::Commands::GetFSStatus, &data) && (data.size() > 16))
	{
		QString sessionState = data[8].toInt() ? CDeviceData::Values::Opened : CDeviceData::Values::Closed;
		setDeviceParameter(CDeviceData::FR::Session, sessionState);

		bool OK;
		uint fiscalDocuments = qToBigEndian(data[12].toUInt(&OK, 16));

		if (OK)
		{
			setDeviceParameter(CDeviceData::FR::FiscalDocuments, fiscalDocuments);
		}

		QDate date = QDate::fromString(data[13].insert(4, "20"), CFR::DateFormat);

		if (date.isValid())
		{
			setDeviceParameter(CDeviceData::FS::ValidityData, date.toString(CFR::DateLogFormat));
		}

		setDeviceParameter(CDeviceData::FS::Version, data[14]);

		uint sessionCount = qToBigEndian(data[16].toUInt(&OK, 16));

		if (OK)
		{
			setDeviceParameter(CDeviceData::FR::SessionCount, sessionCount);
		}
	}

	checkDateTime();

	mOFDDataError = !processCommand(CPrimOnlineFR::Commands::GetOFDData, &data) || (data.size() < 9) ||
		!checkOFDData(data[9], ProtocolUtils::getBufferFromString(data[5].right(2) + data[5].left(2)));
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

	QByteArray sum = QString::number(qRound64(getTotalAmount(aPaymentData) * 100.0) / 100.0, '0', 2).toLatin1();
	QByteArray FDType = aPaymentData.back ? CPrimFR::FDTypes::SaleBack : CPrimFR::FDTypes::Sale;

	QString cashierValue = getConfigParameter(CHardware::FiscalFields::Cashier).toString();
	QString cashierINN = CFR::INNToString(getConfigParameter(CHardware::FiscalFields::CashierINN).toByteArray());
	QString operatorId = CPrimFR::OperatorID;

	if (!cashierValue.isEmpty())
	{
		operatorId = cashierValue;

		if (!cashierINN.isEmpty() && (mFFDFS > EFFD::F10))
		{
			operatorId += "|" + cashierINN;
		}
	}

	int taxData = int(aPaymentData.taxation);

	if (taxData)
	{
		taxData = qRound(qLn(taxData) / qLn(2)) + 1;
	}

	int serialNumberY = serialNumber.size() + 2;
	int documentNumberY = CPrimOnlineFR::FiscalLineSize - 6;
	int timeY = CPrimOnlineFR::FiscalLineSize - 6;
	int INNY = INN.size() + 2;
	bool cashierExist = operatorId != CPrimFR::OperatorID;

	int cashierX = cashierExist ? 3 : 2;
	int cashierY = cashierExist ? (cashier.size() + 2) : (INNY + 16);

	int totalX = cashierX + aPaymentData.amountDataList.size() + 1;
	int totalY = CPrimOnlineFR::UnitLineSize - sum.size() - 1;

	int receiptNumberY = CPrimOnlineFR::FiscalLineSize - 6;

	aCommandData
		<< addGFieldToBuffer(aReceiptSize + 1, serialNumberY)      // серийный номер
		<< addGFieldToBuffer(aReceiptSize + 1, documentNumberY)    // номер документа
		<< addGFieldToBuffer(aReceiptSize + 2, 1)                  // дата
		<< addGFieldToBuffer(aReceiptSize + 2, timeY)              // время
		<< addGFieldToBuffer(aReceiptSize + totalX, INNY)          // ИНН
		<< addGFieldToBuffer(aReceiptSize + cashierX, cashierY) << mCodec->fromUnicode(operatorId)    // ID оператора
		<< addGFieldToBuffer(aReceiptSize + totalX, totalY) << sum;    // сумма

	aAdditionalAFDData
		<< addArbitraryFieldToBuffer(aReceiptSize + 1, 1, serialNumber)
		<< addArbitraryFieldToBuffer(aReceiptSize + 1, documentNumberY - (documentNumber.size() + 1), documentNumber)
		<< addArbitraryFieldToBuffer(aReceiptSize + totalX, 1, INN)
		<< addArbitraryFieldToBuffer(aReceiptSize + totalX, totalY - total.size() - 1, total)

		// СНО (1055)
		<< addFiscalField(aReceiptSize + 1, 1, FiscalFields::TaxSystem, int2String(taxData))

		// номер смены (1038)
		<< addArbitraryFieldToBuffer(aReceiptSize + totalX + 1, 1, session)
		<< addFiscalField(aReceiptSize + totalX + 1, 2 + session.size(), FiscalFields::SessionNumber)

		// номер чека (1042)
		<< addArbitraryFieldToBuffer(aReceiptSize + totalX + 1, receiptNumberY - receiptNumber.size() - 1, receiptNumber)
		<< addFiscalField(aReceiptSize + totalX + 1, receiptNumberY, FiscalFields::DocumentNumber);

	// продажи
	for (int i = 0; i < aPaymentData.amountDataList.size(); ++i)
	{
		SAmountData amountData = aPaymentData.amountDataList[i];
		int section = (amountData.section != -1) ? amountData.section : 1;
		QStringList data = QStringList()
			<< amountData.name
			<< QString::number(qRound64(amountData.sum * 100.0) / 100.0, ASCII::Zero, 2) << "1"
			<< int2String(section) + int2String(mTaxData[amountData.VAT].group)  << "";

		//TODO: ФФД 1.05 -> изменение формата налога

		int x = aReceiptSize + cashierX + i + 1;
		aAdditionalAFDData << addFiscalField(x,  1, FiscalFields::UnitName, data.join("|"));
	}

	if (cashierExist)
	{
		aAdditionalAFDData << addArbitraryFieldToBuffer(aReceiptSize + cashierX, 1, cashier);
	}

	// флаги агента
	if ((mFFDFS > EFFD::F10) && (aPaymentData.agentFlag != EAgentFlags::None))
	{
		int agentFlagsData = int(aPaymentData.agentFlag);

		if (agentFlagsData)
		{
			agentFlagsData = qRound(qLn(agentFlagsData) / qLn(2));
		}

		aAdditionalAFDData << addFiscalField(10, 1, FiscalFields::AgentFlag, int2String(agentFlagsData));
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
TResult PrimOnlineFRBase::openFRSession()
{
	CPrimFR::TData commandData = CPrimFR::TData() << CPrimFR::OperatorID << "" << "" << "";    // + сообщение оператору, доп. реквизит и реквизиты смены

	return processCommand(CPrimFR::Commands::OpenFRSession, commandData);
}

//--------------------------------------------------------------------------------
int PrimOnlineFRBase::getVerificationCode()
{
	CPrimFR::TData answer;

	if (!processCommand(CPrimOnlineFR::Commands::GetFSStatus, &answer) || (answer.size() < 13))
	{
		toLog(LogLevel::Error, "PrimPrinters: Failed to get last document number in FS");
		return 0;
	}

	return qToBigEndian(answer[12].toInt(0, 16));
}

//--------------------------------------------------------------------------------
CPrimFR::TData PrimOnlineFRBase::addFiscalField(int aX, int aY, int aFiscalField, const QString & aData)
{
	return addArbitraryFieldToBuffer(aX, aY, QString("<%1>").arg(aFiscalField) + aData);
}

//--------------------------------------------------------------------------------
