/* @file Движок фискальных тегов. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/CodecDescriptions.h"
#include "Hardware/Common/HardwareConstants.h"
#include "Hardware/Common/DeviceDataConstants.h"
#include "Hardware/Protocols/Common/ProtocolUtils.h"

// Project
#include "FFEngine.h"

using namespace SDK::Driver;
using namespace ProtocolUtils;

//--------------------------------------------------------------------------------
FFEngine::FFEngine(ILog * aLog): DeviceLogManager(aLog), mOperatorPresence(false)
{
}

//--------------------------------------------------------------------------------
void FFEngine::setConfigParameter(const QString & aName, const QVariant & aValue)
{
	DeviceConfigManager::setConfigParameter(aName, aValue);

	mOperatorPresence = getConfigParameter(CHardwareSDK::OperatorPresence, mOperatorPresence).toBool();
}

//--------------------------------------------------------------------------------
bool FFEngine::parseTLV(const QByteArray & aData, CFR::STLV & aTLV)
{
	auto getInt = [&aData] (int aIndex, int aShift) -> int { int result = uchar(aData[aIndex]); return result << (8 * aShift); };
	int fullDataSize = aData.size();

	if (fullDataSize < CFR::MinTLVSize)
	{
		toLog(LogLevel::Error, QString("%1: Failed to parse TLV data due to data size = %2, need %3 min").arg(mDeviceName).arg(fullDataSize).arg(CFR::MinTLVSize));
		return false;
	}

	int size = getInt(2, 0) | getInt(3, 1);
	int dataSize = fullDataSize - 4;

	if (dataSize < size)
	{
		toLog(LogLevel::Error, QString("%1: Failed to parse TLV data due to data size = %2, need %3 min").arg(mDeviceName).arg(dataSize).arg(size));
		return false;
	}

	int field = getInt(0, 0) | getInt(1, 1);
	QByteArray data  = aData.mid(4, size);

	using namespace CFR::FiscalFields;

	SData FFData = mFiscalFieldData[field];
	Types::SData typeData = Types::Data[FFData.type];

	int FFMinSize = typeData.minSize;
	int FFDataSize = data.size();

	if (FFDataSize < FFMinSize)
	{
		toLog(LogLevel::Error, QString("%1: Failed to parse %2 field %3 (%4) due to size = %5, need %6 min")
			.arg(mDeviceName).arg(typeData.description).arg(field).arg(FFData.textKey).arg(FFDataSize).arg(FFMinSize));
		return false;
	}

	if (typeData.fixSize)
	{
		data = data.left(FFMinSize);
	}

	aTLV.field = field;
	aTLV.data  = data;

	return true;
}

//--------------------------------------------------------------------------------
CFR::TTLVList FFEngine::parseSTLV(const QByteArray & aData)
{
	CFR::TTLVList result;
	CFR::STLV TLV;
	int index = 0;

	while ((index < aData.size()) && parseTLV(aData.mid(index), TLV))
	{
		result.insert(TLV.field, TLV.data);
		index += TLV.data.size() + 4;
	}

	return result;
}

//--------------------------------------------------------------------------------
void FFEngine::parseTLVData(int aField, const QByteArray & aData, TFiscalPaymentData & aFPData)
{
	if (!mFiscalFieldData.data().contains(aField))
	{
		toLog(LogLevel::Error, QString("%1: Failed to parse TLV-data for field %2 due to no data in specification").arg(mDeviceName).arg(aField));
		return;
	}

	using namespace CFR::FiscalFields;

	SData FFData = mFiscalFieldData[aField];
	QVariant result;

	if (FFData.type == ETypes::STLV)
	{
		return;
	}

	switch (FFData.type)
	{
	case ETypes::String:
	{
		if (mCodec)
		{
			QTextCodec * codec = CodecByName[CHardware::Codepages::CP866];
			result = codec->toUnicode(aData);
		}

		break;
	}
	case ETypes::FVLN:
	{
		qulonglong digitData = revert(aData.mid(1)).toHex().toULongLong(0, 16);
		QString textData = QString::number(digitData);
		textData = textData.insert(textData.size() - int(uchar(aData[0])), QChar(ASCII::Dot));
		result = textData.toDouble();

		break;
	}
	case ETypes::Byte:
	case ETypes::VLN:
	case ETypes::UINT32:
	{
		result = revert(aData).toHex().toUInt(0, 16);

		break;
	}
	case ETypes::UnixTime:
	{
		QDateTime dateTime;
		dateTime.setTimeSpec(Qt::UTC);
		uint seconds = revert(aData).toHex().toUInt(0, 16);
		dateTime.setTime_t(seconds);

		result = dateTime;

		break;
	}
	case ETypes::ByteArray:
	{
		result = aData;    // формат априори неизвестен

		break;
	}
	default:
	{
		toLog(LogLevel::Error, QString("%1: Failed to parse TLV-data for field %2 due to data type to set = %3 is unknown").arg(mDeviceName).arg(aField).arg(int(FFData.type)));

		break;
	}
	}

	if (result.isValid())
	{
		setFPData(aFPData, aField, result);
	}
}

//--------------------------------------------------------------------------------
void FFEngine::parseSTLVData(const CFR::STLV & aTLV, TComplexFiscalPaymentData & aPSData)
{
	if (mFiscalFieldData[aTLV.field].type == CFR::FiscalFields::ETypes::STLV)
	{
		CFR::TTLVList complexFPData = parseSTLV(aTLV.data);
		TFiscalPaymentData FPData;

		for (auto it = complexFPData.begin(); it != complexFPData.end(); ++it)
		{
			parseTLVData(it.key(), it.value(), FPData);
		}

		if (!FPData.isEmpty())
		{
			aPSData << FPData;
		}
	}
}

//--------------------------------------------------------------------------------
QByteArray FFEngine::getTLVData(int aField, QString * aLog)
{
	QString textKey = mFiscalFieldData[aField].textKey;
	QVariant value = getConfigParameter(textKey);

	return getTLVData(aField, value, aLog);
}

//--------------------------------------------------------------------------------
QByteArray FFEngine::getTLVData(int aField, const QVariant & aValue, QString * aLog)
{
	if (!mFiscalFieldData.data().contains(aField))
	{
		toLog(LogLevel::Error, QString("%1: Failed to get TLV-data for field %2 due to no data in specification").arg(mDeviceName).arg(aField));
		return "";
	}

	using namespace CFR::FiscalFields;

	SData FFData = mFiscalFieldData[aField];
	QString log = QString("field %1 (%2) = ").arg(aField).arg(FFData.textKey);

	QByteArray result = getHexReverted(aField, 2);

	switch (FFData.type)
	{
		case ETypes::VLN:
		case ETypes::UINT32:
		{
			qulonglong data = aValue.toULongLong();
			result += getDigitTLVData(data);
			log += QString::number(data);

			break;
		}
		case ETypes::FVLN:
		{
			double data = aValue.toDouble();
			int dotNumber = QString::number(data).indexOf(ASCII::Dot) + 1;
			result.append(uchar(dotNumber));

			//TODO: проверить на Касби - qPow(data, data)

			result.append(data ? getDigitTLVData(qulonglong(qPow(data, data))) : ASCII::NUL);
			log += QString::number(data, 'f', 2);

			break;
		}
		case ETypes::String:
		{
			QString data = aValue.toString();
			result += mCodec->fromUnicode(data);
			log += data;

			break;
		}
		case ETypes::Byte:
		{
			char data = char(aValue.toUInt());
			result += data;
			log += toHexLog(data);

			break;
		}
		//TODO: доделать лог для STLV - для Касби
		case ETypes::STLV:
		case ETypes::ByteArray:
		{
			QByteArray data = aValue.toByteArray();
			result += data;
			log += "0x" + data.toHex().toUpper();

			break;
		}
		default:
		{
			result.clear();    // чтобы не прошел запрос установки TLV-параметра из-за неверных данных, с которыми неясно что делать
			log += "0x" + aValue.toByteArray();
			toLog(LogLevel::Error, QString("%1: Failed to get TLV-data for field %2 due to data type to set = %3 is unknown").arg(mDeviceName).arg(aField).arg(int(FFData.type)));

			break;
		}
	}

	result.insert(2, getHexReverted(result.size() - 2, 2));

	if (aLog)
	{
		*aLog = log;
	}

	return result;
}

//--------------------------------------------------------------------------------
QByteArray FFEngine::getDigitTLVData(qulonglong aValue)
{
	int size = sizeof(aValue);
	QByteArray result;

	for (int i = 0; i < size; ++i)
	{
		char data = char(aValue >> (8 * (size - i - 1)));

		if (!result.isEmpty() || data)
		{
			result.append(data);
		}
	}

	if (result.isEmpty())
	{
		result.append(ASCII::NUL);
	}

	return revert(result);
}

//--------------------------------------------------------------------------------
void FFEngine::setFPData(TFiscalPaymentData & aFPData, int aField, const QVariant & aValue)
{
	CFR::FiscalFields::SData data = mFiscalFieldData[aField];
	aFPData.insert(data.textKey, aValue.isValid() ? aValue : QString());
}

//--------------------------------------------------------------------------------
void FFEngine::checkFPData(TFiscalPaymentData & aFPData, int aField)
{
	QString textKey = mFiscalFieldData[aField].textKey;
	QVariant value = getConfigParameter(textKey);

	checkFPData(aFPData, aField, value);
}

//--------------------------------------------------------------------------------
void FFEngine::checkFPData(TFiscalPaymentData & aFPData, int aField, const QVariant & aValue)
{
	QString textKey = mFiscalFieldData[aField].textKey;

	if (!aFPData.contains(textKey))
	{
		aFPData.insert(textKey, aValue);
	}
}

//--------------------------------------------------------------------------------
void FFEngine::checkSimpleFPData(TFiscalPaymentData & aFPData, int aField)
{
	CFR::FiscalFields::SData data = mFiscalFieldData[aField];

	if (aFPData.contains(data.textKey) || getConfigParameter(data.textKey, 0).toInt())
	{
		aFPData.insert(data.textKey, QString());
	}
}

//--------------------------------------------------------------------------------
QString FFEngine::getFPDataLog(const TFiscalPaymentData & aFPData) const
{
	if (aFPData.isEmpty())
	{
		return "";
	}

	QList<int> fields;
	int maxKeySize = 0;
	int maxTextKeySize = 0;

	foreach (const QString & textKey, aFPData.keys())
	{
		int field = mFiscalFieldData.getKey(textKey);
		CFR::FiscalFields::SData data = mFiscalFieldData[field];

		maxKeySize = qMax(maxKeySize, data.translationPF.size());
		maxTextKeySize = qMax(maxTextKeySize, data.textKey.size());
		fields << field;
	}

	qSort(fields);

	QStringList logData;

	foreach(int field, fields)
	{
		CFR::FiscalFields::SData data = mFiscalFieldData[field];

		QString textKey = data.textKey;
		QString textKeyLog = QString("(%1)").arg(textKey).leftJustified(maxTextKeySize + 2);
		QString log = QString("%1 %2").arg(field).arg(textKeyLog);

		QString key = data.translationPF;
		QString value = aFPData[textKey].toString();

		if (!key.isEmpty() && !value.isEmpty()) log += QString(" : %1 = %2").arg(key.leftJustified(maxKeySize)).arg(value);
		else if (!key.isEmpty())                     log += QString(" : %1").arg(key);
		else if (!value.isEmpty())                   log += QString(" = %1").arg(value);

		logData << log;
	}

	return "\n" + logData.join("\n") + "\n";
}

//---------------------------------------------------------------------------
bool FFEngine::checkTaxSystems(char aData, TTaxSystems & aTaxSystems)
{
	QStringList errorLog;

	for (int i = 0; i < (sizeof(aData) * 8); ++i)
	{
		char taxSystem = 1 << i;

		if (aData & taxSystem)
		{
			if (!CFR::TaxSystems.data().contains(taxSystem))
			{
				errorLog << toHexLog(taxSystem);
			}

			aTaxSystems << taxSystem;
		}
	}

	mTaxSystems = aTaxSystems;

	if (!errorLog.isEmpty())
	{
		toLog(LogLevel::Error, mDeviceName + ": Unknown taxation system(s): " + errorLog.join(", "));
		return false;
	}

	using namespace CFR::FiscalFields;

	ERequired::Enum required = (aTaxSystems.size() > 1) ? ERequired::Yes : ERequired::No;
	mFiscalFieldData.data()[CFR::FiscalFields::TaxSystem].required = required;

	return true;
}

//---------------------------------------------------------------------------
bool FFEngine::checkAgentFlags(char aData, TAgentFlags & aAgentFlags)
{
	QStringList errorLog;

	for (int i = 0; i < (sizeof(aData) * 8); ++i)
	{
		char agentFlag = 1 << i;

		if (aData & agentFlag)
		{
			if (!CFR::AgentFlags.data().contains(agentFlag))
			{
				errorLog << toHexLog(agentFlag);
			}

			aAgentFlags << agentFlag;
		}
	}

	mAgentFlags = aAgentFlags;

	if (!errorLog.isEmpty())
	{
		toLog(LogLevel::Error, mDeviceName + ": Unknown agent flag(s): " + errorLog.join(", "));
		return false;
	}

	using namespace CFR::FiscalFields;

	ERequired::Enum required = (aAgentFlags.size() > 1) ? ERequired::Yes : ERequired::No;
	mFiscalFieldData.data()[CFR::FiscalFields::AgentFlagsReg].required = required;

	return true;
}

//---------------------------------------------------------------------------
bool FFEngine::checkOperationModes(char aData, TOperationModes & aOperationModes)
{
	QStringList errorLog;

	for (int i = 0; i < (sizeof(aData) * 8); ++i)
	{
		char operationMode = 1 << i;

		if (aData & operationMode)
		{
			if (!CFR::OperationModeData.data().contains(operationMode))
			{
				errorLog << toHexLog(operationMode);
			}

			aOperationModes << operationMode;
		}
	}

	mOperationModes = aOperationModes;

	if (!errorLog.isEmpty())
	{
		toLog(LogLevel::Error, mDeviceName + ": Unknown operation mode(s): " + errorLog.join(", "));
		return false;
	}

	return true;
}

//---------------------------------------------------------------------------
bool FFEngine::checkFiscalField(int aField, bool & aResult)
{
	if (!mFiscalFieldData.data().contains(aField))
	{
		aResult = false;
		toLog(LogLevel::Error, QString("%1: Failed to set %2 fiscal field due to it is unknown").arg(mDeviceName).arg(aField));

		return false;
	}

	using namespace CFR::FiscalFields;

	SData data = mFiscalFieldData[aField];

	if (containsConfigParameter(data.textKey))
	{
		return true;
	}

	if (data.required == ERequired::No)
	{
		aResult = true;
		toLog(LogLevel::Debug, QString("%1: don`t set field %2 (%3) due to the field is not required").arg(mDeviceName).arg(aField).arg(data.textKey));

		return false;
	}
	else if ((data.required == ERequired::PM) && !(mOperatorPresence))
	{
		aResult = true;
		toLog(LogLevel::Debug, QString("%1: don`t set field %2 (%3) due to the operator is not present").arg(mDeviceName).arg(aField).arg(data.textKey));

		return false;
	}

	aResult = false;
	toLog(LogLevel::Error, QString("%1: Failed to set field %2 (%3) due due to it is absent").arg(mDeviceName).arg(aField).arg(data.textKey));

	return false;
}

//--------------------------------------------------------------------------------
bool FFEngine::checkDealerTaxSystem(ERequestStatus::Enum aInitialized, bool aCanLog)
{
	CFR::DealerDataManager DDManager(this, CHardwareSDK::FR::DealerTaxSystem);

	if (mOperatorPresence)
	{
		return true;
	}

	if (!containsConfigParameter(CHardwareSDK::FR::DealerTaxSystem))
	{
		if (aCanLog)
		{
			toLog(LogLevel::Normal, mDeviceName + ": No dealer taxation system");
		}

		return false;
	}

	QVariant taxSystemData = getConfigParameter(CHardwareSDK::FR::DealerTaxSystem);

	if (taxSystemData.toString().isEmpty())
	{
		if (aCanLog)
		{
			toLog(LogLevel::Warning, mDeviceName + ": Dealer taxation system is empty");
		}

		return false;
	}

	char taxSystem = char(taxSystemData.toInt());

	if (!CFR::TaxSystems.data().keys().contains(taxSystem))
	{
		if (aCanLog)
		{
			toLog(LogLevel::Warning, QString("%1: Wrong dealer taxation system = %2").arg(mDeviceName).arg(int(taxSystem)));
		}

		return false;
	}

	DDManager.setValue(CFR::TaxSystems[taxSystem]);

	if ((aInitialized == ERequestStatus::Success) && !mTaxSystems.isEmpty() && !mTaxSystems.contains(taxSystem))
	{
		if (aCanLog)
		{
			toLog(LogLevel::Warning, QString("%1: The actual taxation system(s) %2 don`t contain dealer taxation system %3 (%4)")
				.arg(mDeviceName).arg(toHexLog(CFR::joinData(mTaxSystems))).arg(toHexLog(taxSystem)).arg(CFR::TaxSystems[taxSystem]));
		}

		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool FFEngine::checkDealerAgentFlag(ERequestStatus::Enum aInitialized, bool aCanLog)
{
	CFR::DealerDataManager DDManager(this, CHardwareSDK::FR::DealerAgentFlag);

	if (mOperatorPresence)
	{
		return true;
	}

	if (!containsConfigParameter(CHardwareSDK::FR::DealerAgentFlag))
	{
		if (aCanLog)
		{
			toLog(LogLevel::Normal, mDeviceName + ": No dealer agent flag");
		}

		return false;
	}

	QVariant agentFlagData = getConfigParameter(CHardwareSDK::FR::DealerAgentFlag);

	if (agentFlagData.toString().isEmpty())
	{
		if (aCanLog)
		{
			toLog(LogLevel::Warning, mDeviceName + ": Dealer agent flag is empty");
		}

		return false;
	}

	char agentFlag = char(agentFlagData.toInt());

	if (!CFR::AgentFlags.data().keys().contains(agentFlag))
	{
		if (aCanLog)
		{
			toLog(LogLevel::Warning, QString("%1: Wrong dealer agent flag = %2").arg(mDeviceName).arg(int(agentFlag)));
		}

		return false;
	}

	DDManager.setValue(CFR::AgentFlags[agentFlag]);

	if ((aInitialized == ERequestStatus::Success) && !mAgentFlags.isEmpty() && !mAgentFlags.contains(agentFlag))
	{
		if (aCanLog)
		{
			toLog(LogLevel::Warning, QString("%1: The actual agent flag(s) %2 don`t contain dealer agent flag %3 (%4)")
				.arg(mDeviceName).arg(toHexLog(CFR::joinData(mAgentFlags))).arg(toHexLog(agentFlag)).arg(CFR::AgentFlags[agentFlag]));
		}

		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool FFEngine::checkTaxSystemOnPayment(SPaymentData & aPaymentData)
{
	char taxSystem = char(aPaymentData.taxSystem);
	char taxSystemData = char(getConfigParameter(CHardwareSDK::FR::DealerTaxSystem).toInt());
	ETaxSystems::Enum dealerTaxSystem = ETaxSystems::Enum(taxSystemData);

	if (dealerTaxSystem != ETaxSystems::None)
	{
		if (mTaxSystems.contains(taxSystemData))
		{
			aPaymentData.taxSystem = dealerTaxSystem;
		}
		else if (mTaxSystems.size() == 1)
		{
			aPaymentData.taxSystem = ETaxSystems::Enum(mTaxSystems[0]);
		}
	}
	else if (!mTaxSystems.isEmpty())
	{
		char joinedTaxSystems = CFR::joinData(mTaxSystems);

		if (aPaymentData.taxSystem == ETaxSystems::None)
		{
			if (mTaxSystems.size() == 1)
			{
				aPaymentData.taxSystem = ETaxSystems::Enum(mTaxSystems[0]);
			}
			else
			{
				toLog(LogLevel::Error, QString("%1: Failed to determine the required taxation system from the several ones (%2)")
					.arg(mDeviceName).arg(toHexLog(joinedTaxSystems)));
				return false;
			}
		}
		else if (!mTaxSystems.contains(taxSystem))
		{
			toLog(LogLevel::Error, QString("%1: The actual taxation system(s) %2 don`t contain system %3 (%4)")
				.arg(mDeviceName).arg(toHexLog(joinedTaxSystems)).arg(toHexLog(taxSystem)).arg(CFR::TaxSystems[taxSystem]));
			return false;
		}
	}

	removeConfigParameter(CFiscalSDK::TaxSystem);

	if (aPaymentData.taxSystem != ETaxSystems::None)
	{
		setConfigParameter(CFiscalSDK::TaxSystem, char(aPaymentData.taxSystem));
	}

	return true;
}

//--------------------------------------------------------------------------------
bool FFEngine::checkAgentFlagOnPayment(SPaymentData & aPaymentData)
{
	/*
	if (mAgentFlags.size() > 1)
	{
		foreach (const SUnitData & unitData, aPaymentData.unitDataList)
		{
			QString providerINN = unitData.providerINN;
			QString log = QString("%1: Failed to sale%2 for %3 (sum = %4) due to ").arg(mDeviceName).arg(aPaymentData.back ? " back" : "").arg(unitData.name).arg(unitData.sum, 0, 'f', 2);
			int size = providerINN.size();

			if (!size)
			{
				toLog(LogLevel::Error, log + "provider INN is empty");
				return false;
			}
			else if (((size != CFR::INNSize::LegalPerson) && (size != CFR::INNSize::NaturalPerson)) || (QRegExp("[0-9]+").indexIn(providerINN) == -1))
			{
				toLog(LogLevel::Error, log + "wrong provider INN = " + providerINN);
				return false;
			}
		}
	}
	*/

	char agentFlag = char(aPaymentData.agentFlag);
	char agentFlagData = char(getConfigParameter(CHardwareSDK::FR::DealerAgentFlag).toInt());
	EAgentFlags::Enum dealerAgentFlag = EAgentFlags::Enum(agentFlagData);

	if (dealerAgentFlag != EAgentFlags::None)
	{
		if (mAgentFlags.contains(agentFlagData))
		{
			aPaymentData.agentFlag = dealerAgentFlag;
		}
		else if (mAgentFlags.size() == 1)
		{
			aPaymentData.agentFlag = EAgentFlags::Enum(mAgentFlags[0]);
		}
	}
	else if (!mAgentFlags.isEmpty())
	{
		char joinedAgentFlags = CFR::joinData(mAgentFlags);

		if (aPaymentData.agentFlag == EAgentFlags::None)
		{
			if (mAgentFlags.size() == 1)
			{
				aPaymentData.agentFlag = EAgentFlags::Enum(mAgentFlags[0]);
			}
			else
			{
				toLog(LogLevel::Error, QString("%1: Failed to determine the required agent flag from the several ones (%2)")
					.arg(mDeviceName).arg(toHexLog(joinedAgentFlags)));
				return false;
			}
		}
		else if (!mAgentFlags.contains(agentFlag))
		{
			toLog(LogLevel::Error, QString("%1: The actual agent flag(s) %2 don`t contain flag %3 (%4)")
				.arg(mDeviceName).arg(toHexLog(joinedAgentFlags)).arg(toHexLog(agentFlag)).arg(CFR::AgentFlags[agentFlag]));
			return false;
		}
	}

	removeConfigParameter(CFiscalSDK::AgentFlag);
	removeConfigParameter(CFiscalSDK::AgentFlagsReg);

	if (aPaymentData.agentFlag != EAgentFlags::None)
	{
		agentFlag = char(aPaymentData.agentFlag);
		setConfigParameter(CFiscalSDK::AgentFlag,            agentFlag);
		setConfigParameter(CFiscalSDK::AgentFlagsReg, agentFlag);
	}

	return true;
}

//--------------------------------------------------------------------------------
void FFEngine::filterAfterPayment(const SPaymentData & aPaymentData, TFiscalPaymentData & aFPData, TComplexFiscalPaymentData & aPSData)
{
	QStringList removedFields;

	foreach(int field, CFR::FiscalFields::FiscalTotals)
	{
		QString key = mFiscalFieldData[field].textKey;
		removedFields << QString("%1 (%2)").arg(field).arg(key);

		if (!aFPData.value(key).toInt())
		{
			aFPData.remove(key);
		}
	}

	toLog(LogLevel::Normal, QString("%1: %2 have been removed from the fiscal payment data").arg(mDeviceName).arg(removedFields.join(", ")));

	EPayOffTypes::Enum payOffType = aPaymentData.back ? EPayOffTypes::DebitBack : EPayOffTypes::Debit;

	  setFPData(aFPData, CFR::FiscalFields::FDName);
	  setFPData(aFPData, CFR::FiscalFields::PayOffType, CFR::PayOffTypes[payOffType]);
	checkFPData(aFPData, CFR::FiscalFields::SerialFSNumber);
	checkFPData(aFPData, CFR::FiscalFields::SerialFRNumber);
	checkFPData(aFPData, CFR::FiscalFields::INN);
	checkFPData(aFPData, CFR::FiscalFields::RNM);
	checkFPData(aFPData, CFR::FiscalFields::FTSURL);
	checkFPData(aFPData, CFR::FiscalFields::OFDURL);
	checkFPData(aFPData, CFR::FiscalFields::OFDName);
	checkFPData(aFPData, CFR::FiscalFields::LegalOwner);
	checkFPData(aFPData, CFR::FiscalFields::PayOffAddress);
	checkFPData(aFPData, CFR::FiscalFields::PayOffPlace);

	if (aFPData.contains(CFiscalSDK::FDSign))
	{
		QByteArray FDSignData = aFPData[CFiscalSDK::FDSign].toByteArray();
		qulonglong FDSign = FDSignData.right(4).toHex().toULongLong(0, 16);
		QString FDSignTextData = QString("%1").arg(FDSign, CFR::FDSignSize, 10, QChar(ASCII::Zero));
		setFPData(aFPData, CFR::FiscalFields::FDSign, FDSignTextData);
	}

	if (aFPData.contains(CFiscalSDK::FDDateTime))
	{
		QDateTime dateTime = aFPData.value(CFiscalSDK::FDDateTime).toDateTime();
		setFPData(aFPData, CFR::FiscalFields::FDDateTime, dateTime.toString(CFR::DateTimeShortLogFormat));
	}

	for (int i = 0; i < mOperationModes.size(); ++i)
	{
		int field = CFR::OperationModeData[mOperationModes[i]];

		if (field)
		{
			setFPData(aFPData, field);
		}
	}

	checkSimpleFPData(aFPData, CFR::FiscalFields::LotteryMode);
	checkSimpleFPData(aFPData, CFR::FiscalFields::GamblingMode);
	checkSimpleFPData(aFPData, CFR::FiscalFields::ExcisableUnitMode);

	#define ADD_SPEC_DFIELD(aData, aName, aSpecification) if (aData.contains(CFiscalSDK::aName)) { int value = aData[CFiscalSDK::aName].toInt(); \
		aData[CFiscalSDK::aName] = CFR::aSpecification[char(value)]; }
	#define ADD_SPEC_DFIELD_SAME(aData, aName) ADD_SPEC_DFIELD(aData, aName, aName##s)

	ADD_SPEC_DFIELD(aFPData, AgentFlagsReg, AgentFlags);
	ADD_SPEC_DFIELD_SAME(aFPData, TaxSystem);

	for (int i = 0; i < aPSData.size(); ++i)
	{
		ADD_SPEC_DFIELD_SAME(aPSData[i], PayOffSubjectType);
		ADD_SPEC_DFIELD_SAME(aPSData[i], PayOffSubjectMethodType);
		ADD_SPEC_DFIELD_SAME(aPSData[i], VATRate);
		ADD_SPEC_DFIELD_SAME(aPSData[i], AgentFlag);
	}
}

//--------------------------------------------------------------------------------
void FFEngine::setDeviceName(const QString & aDeviceName)
{
	QWriteLocker lock(&mConfigurationGuard);

	mDeviceName = aDeviceName;
}

//--------------------------------------------------------------------------------
void FFEngine::setCodec(QTextCodec * aCodec)
{
	mCodec = aCodec;
}

//--------------------------------------------------------------------------------
