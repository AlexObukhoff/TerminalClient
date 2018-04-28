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
	return TModels()
		<< CPrimFR::Models::PRIM_06F;
}}

//--------------------------------------------------------------------------------
PrimOnlineFRBase::PrimOnlineFRBase()
{
	// данные устройства
	mErrorData = PErrorData(new CPrimOnlineFR::Errors::Specification);
	mIsOnline = true;

	// данные моделей
	mDeviceName = CPrimFR::DefaultOnlineModelName;
	mModels = CPrimFR::OnlineModels();
	mModel = CPrimFR::Models::OnlineUnknown;

	// типы оплаты
	mPayTypeData.data().clear();

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

	mFSSerialNumber = CFR::FSSerialToString(data[11]);
	mAFDFont = CPrimFR::AFD::Font::Default;

	QString firmware = data[10].simplified();
	setDeviceParameter(CDeviceData::Firmware, firmware);
	int build = firmware.right(3).toInt();

	if (build)
	{
		mFFDFR = CPrimOnlineFR::getFFD(build);
		int actualFirmware = CPrimOnlineFR::getActualFirmware(mFFDFR);
		mOldFirmware = actualFirmware && (actualFirmware > build);

		//TODO: TLV-запрос для получения версии ФФД ФН -> mFFDFS (тег 1190), будет после 1.05
		mFFDFS = EFFD::F10;
	}

	if (firmware.size() == 7)
	{
		setDeviceParameter(CDeviceData::ControllerBuild, QString("%1.%2").arg(firmware.right(3)).arg(firmware[2]), CDeviceData::Firmware);
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
		mAFDFont = CPrimFR::AFD::Font::Narrow;

		commandData = CPrimFR::TData() << QByteArray::number(qToBigEndian(ushort(CFR::FiscalFields::AgentFlagsReg)), 16);
		uchar agentFlagsData;

		if (!processCommand(CPrimOnlineFR::Commands::GetRegTLVData, commandData, 5, "agent flags", agentFlagsData) || !checkAgentFlags(char(agentFlagsData)))
		{
			return false;
		}
	}

	if (!mOperatorPresence && isFiscal() && !checkParameters())
	{
		return false;
	}

	if (!checkControlSettings() || !checkTaxes())
	{
		return false;
	}

	mPayTypeData.data().clear();

	for (int i = 0; i < CPrimOnlineFR::PayTypeAmount; ++i)
	{
		if (!processCommand(CPrimOnlineFR::Commands::GetPayTypeData, CPrimFR::TData() << int2String(i).toLatin1(), &data))
		{
			return false;
		}

		ushort field;

		if (parseAnswerData(data, 12, "pay type", field))
		{
			if (!mFiscalFieldData.data().contains(field))
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
			setDeviceParameter(CDeviceData::FS::Version, QString("%1, type %2").arg(clean(data[14]).data()).arg(data[15].toUInt() ? "serial" : "debug"));
		}

		loadDeviceData<ushort>(data, CDeviceData::Count, "count", 16, CDeviceData::FR::Session);
	}

	if (processCommand(CPrimFR::Commands::GetStatus, &data))
	{
		loadDeviceData<uchar>(data, CDeviceData::FR::FreeReregistrations, "free reregistrations", 5);

		if (data.size() > 8)
		{
			QString dateTimedata = data[8] + data[7];
			QDateTime dateTime = QDateTime::fromString(dateTimedata.insert(4, "20"), CPrimOnlineFR::DateTimeFormat);

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
		<< addGFieldToBuffer(startX + 1, serialNumberY)      // серийный номер
		<< addGFieldToBuffer(startX + 1, documentNumberY)    // номер документа
		<< addGFieldToBuffer(startX + 2, 1)                  // дата
		<< addGFieldToBuffer(startX + 2, timeY)              // время
		<< addGFieldToBuffer(lastX  + 2, INNY)               // ИНН
		<< addGFieldToBuffer(lastX  + 0, cashierY) << mCodec->fromUnicode(operatorId)    // ID оператора
		<< addGFieldToBuffer(lastX  + 2, totalY)   << sumData;    // сумма

	aAdditionalAFDData
		<< addArbitraryFieldToBuffer(startX + 1, 1, serialNumber)
		<< addArbitraryFieldToBuffer(startX + 1, documentNumberY - (documentNumber.size() + 1), documentNumber)
		<< addArbitraryFieldToBuffer(lastX  + 2, 1, INN)
		<< addArbitraryFieldToBuffer(lastX  + 2, totalY - total.size() - 1, total)

		// 1038 (номер смены)
		<< addArbitraryFieldToBuffer(lastX + 1, 1, session)
		<<            addFiscalField(lastX + 1, 2 + session.size(), CFR::FiscalFields::SessionNumber)

		// 1042 (номер чека)
		<< addArbitraryFieldToBuffer(lastX + 1, receiptNumberY - receiptNumber.size() - 1, receiptNumber)
		<<            addFiscalField(lastX + 1, receiptNumberY, CFR::FiscalFields::DocumentNumber);

	if (cashierExist)
	{
		aAdditionalAFDData << addArbitraryFieldToBuffer(aReceiptSize + cashierX, 1, cashier);
	}

	// продажи
	lastX += 2;

	for (int i = 0; i < aPaymentData.unitDataList.size(); ++i)
	{
		SUnitData & unitData = aPaymentData.unitDataList.value(i);
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
		int addX = 0;

		#define ADD_AFD_TAG(aX, aY, aField, ...) aAdditionalAFDData << addFiscalField(lastX + aX,  aY, CFR::FiscalFields::aField * 100 + i + 1, __VA_ARGS__);
		#define ADD_AFD_TAG_MULTI(aX, aY, aField, aData) ADD_AFD_TAG(aX, aY, aField, aData); lastX += addX;

		if (mFFDFR > EFFD::F10)
		{
			TSum sum = unitData.sum;
			int amountY = getAmountY(sum);
			int VATY = getAmountY(sum * unitData.VAT / 100.0);

			int unitNameSize = unitData.name.size();
			addX = 0;//unitNameSize / CPrimOnlineFR::UnitLineSize;
			addX += int((amountY - (unitNameSize - addX * CPrimOnlineFR::AFD::LineSize::Unit)) < 4);

			ADD_AFD_TAG_MULTI(1, 1, PayOffSubject, data.join("|"));    // 1059 (товар)
			ADD_AFD_TAG(1, amountY, PayOffSubjectUnitPrice);           // 1079 (цена)
			ADD_AFD_TAG(2,       1, PayOffSubjectQuantity);            // 1023 (количество)
			ADD_AFD_TAG(2, amountY, PayOffSubjectAmount);              // 1043 (стоимость)
			ADD_AFD_TAG(3,       1, VATRate);                          // 1199 (НДС, %)
			ADD_AFD_TAG(3,    VATY, PayOffSubjectTaxAmount);           // 1200 (НДС, сумма)
		}
		else
		{
			aAdditionalAFDData << addFiscalField(lastX + 1, 1, CFR::FiscalFields::UnitName, data.join("|"));    // 1030 (наименование товара)
		}

		foreach (auto data, aAdditionalAFDData.mid(addAFDDataIndex))
		{
			int newLastX = qToBigEndian(data[0].toUShort(0, 16));
			lastX = qMax(lastX, newLastX);
		}
	}

	// 1057 (флаг агента)
	if ((mFFDFR > EFFD::F10) && (aPaymentData.agentFlag != EAgentFlags::None))
	{
		int index = 0;

		while (~aPaymentData.agentFlag & (1 << index++)) {}

		aAdditionalAFDData << addFiscalField(lastX + 1, 1, CFR::FiscalFields::AgentFlagsReg, int2String(index));
	}

	// 1055 (СНО)
	if (aPaymentData.taxSystem != ETaxSystems::None)
	{
		int index = 0;

		while (~aPaymentData.taxSystem & (1 << index++)) {}

		aAdditionalAFDData << addFiscalField(lastX + 1, 25, CFR::FiscalFields::TaxSystem, int2String(index));
	}

	QString userContact = getConfigParameter(CFiscalSDK::UserContact).toString();

	if ((mFFDFR > EFFD::F10) && !userContact.isEmpty())
	{
		aAdditionalAFDData << addFiscalField(lastX + 2, 1, CFR::FiscalFields::UserContact, userContact);
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
CPrimFR::TData PrimOnlineFRBase::addFiscalField(int aX, int aY, int aFiscalField, const QString & aData)
{
	return addArbitraryFieldToBuffer(aX, aY, QString("<%1>").arg(aFiscalField) + aData);
}

//--------------------------------------------------------------------------------
