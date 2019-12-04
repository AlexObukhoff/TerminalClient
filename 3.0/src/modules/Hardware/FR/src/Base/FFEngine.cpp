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
	mCodec = CodecByName[CHardware::Codepages::CP866];
}

//--------------------------------------------------------------------------------
void FFEngine::setConfigParameter(const QString & aName, const QVariant & aValue)
{
	int field = mFFData.getKey(aName);
	QVariant value(aValue);

	if (value.isValid() && field)
	{
		CFR::FiscalFields::SData & data = mFFData.data()[field];

		if (data.isString())
		{
			value = clean(value.toString());
		}

		if (data.isINN())
		{
			value = value.toString().simplified().leftJustified(CFR::INN::Person::Natural, QChar(ASCII::Space));
		}
	}

	DeviceConfigManager::setConfigParameter(aName, value);

	mOperatorPresence = getConfigParameter(CHardwareSDK::OperatorPresence, mOperatorPresence).toBool();
}

//--------------------------------------------------------------------------------
bool FFEngine::parseTLV(const QByteArray & aData, CFR::STLV & aTLV)
{
	auto getInt = [&aData] (int aIndex, int aShift) -> int { int result = uchar(aData[aIndex]); return result << (8 * aShift); };
	int fullDataSize = aData.size();

	if (fullDataSize < CFR::MinTLVSize)
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to parse TLV data due to full data size = %1, need %2 min").arg(fullDataSize).arg(CFR::MinTLVSize));
		return false;
	}

	int size = getInt(2, 0) | getInt(3, 1);
	int dataSize = fullDataSize - 4;

	if (dataSize < size)
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to parse TLV data due to data size = %1, need %2 min").arg(dataSize).arg(size));
		return false;
	}

	aTLV.field = getInt(0, 0) | getInt(1, 1);
	aTLV.data  = aData.mid(4, size);

	return checkTLVData(aTLV);
}

//--------------------------------------------------------------------------------
bool FFEngine::checkTLVData(CFR::STLV & aTLV)
{
	using namespace CFR::FiscalFields;

	if (!mFFData.data().contains(aTLV.field))
	{
		toLog(LogLevel::Warning, mDeviceName + QString(": Cannot check TLV-data for field %2 due to no data in specification").arg(aTLV.field));
		return true;
	}

	SData FFData = mFFData[aTLV.field];
	Types::SData typeData = Types::Data[FFData.type];

	int FFMinSize = typeData.minSize;
	int FFDataSize = aTLV.data.size();

	if (FFDataSize < FFMinSize)
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to parse %1 %2 due to size = %3, need %4 min")
			.arg(typeData.description).arg(mFFData.getTextLog(aTLV.field)).arg(FFDataSize).arg(FFMinSize));
		return false;
	}

	if (typeData.fixSize)
	{
		aTLV.data = aTLV.data.left(FFMinSize);
	}

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
void FFEngine::parseTLVDataList(const CFR::TTLVList & aTLVs, SDK::Driver::TFiscalPaymentData & aFPData)
{
	for (auto it = aTLVs.begin(); it != aTLVs.end(); ++it)
	{
		parseTLVData(CFR::STLV(it.key(), it.value()), aFPData);
	}
}

//--------------------------------------------------------------------------------
void FFEngine::parseTLVData(const CFR::STLV & aTLV, TFiscalPaymentData & aFPData)
{
	if (!mFFData.data().contains(aTLV.field))
	{
		toLog(LogLevel::Error, QString("%1: Failed to parse TLV-data for field %2 due to no data in specification").arg(mDeviceName).arg(aTLV.field));
		return;
	}

	using namespace CFR::FiscalFields;

	SData FFData = mFFData[aTLV.field];
	QVariant result;

	if (FFData.isSTLV())
	{
		return;
	}

	switch (FFData.type)
	{
		case ETypes::String:
		{
			result = mCodec->toUnicode(aTLV.data);

			break;
		}
		case ETypes::FVLN:
		{
			qulonglong digitData = revert(aTLV.data.mid(1)).toHex().toULongLong(0, 16);
			QString textData = QString::number(digitData);
			textData = textData.insert(textData.size() - int(uchar(aTLV.data[0])), QChar(ASCII::Dot));
			result = textData.toDouble();

			break;
		}
		case ETypes::Byte:
		case ETypes::VLN:
		case ETypes::UINT32:
		{
			result = revert(aTLV.data).toHex().toUInt(0, 16);

			break;
		}
		case ETypes::UnixTime:
		{
			QDateTime dateTime;
			dateTime.setTimeSpec(Qt::UTC);
			uint seconds = revert(aTLV.data).toHex().toUInt(0, 16);
			dateTime.setTime_t(seconds);

			result = dateTime;

			break;
		}
		case ETypes::ByteArray:
		{
			result = aTLV.data;    // формат априори неизвестен

			break;
		}
		default:
		{
			toLog(LogLevel::Error, QString("%1: Failed to parse TLV-data for field %2 due to data type to set = %3 is unknown").arg(mDeviceName).arg(aTLV.field).arg(int(FFData.type)));

			break;
		}
	}

	if (result.isValid())
	{
		setFPData(aFPData, aTLV.field, result);
	}
}

//--------------------------------------------------------------------------------
void FFEngine::parseSTLVData(const CFR::STLV & aTLV, TComplexFiscalPaymentData & aPSData)
{
	if (!mFFData[aTLV.field].isSTLV())
	{
		return;
	}

	CFR::TTLVList complexFPData = parseSTLV(aTLV.data);
	TFiscalPaymentData FPData;

	for (auto it = complexFPData.begin(); it != complexFPData.end(); ++it)
	{
		parseTLVData(CFR::STLV(it.key(), it.value()), FPData);
	}

	if (!FPData.isEmpty())
	{
		aPSData << FPData;
	}
}

//--------------------------------------------------------------------------------
QByteArray FFEngine::getTLVData(int aField, QString * aLog)
{
	QString textKey = mFFData[aField].textKey;
	QVariant value = getConfigParameter(textKey);

	return getTLVData(aField, value, aLog);
}

//--------------------------------------------------------------------------------
QByteArray FFEngine::getTLVData(const QString & aTextKey, const QVariant & aValue, QString * aLog)
{
	if (!mFFData.getTextKeys().contains(aTextKey))
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to get TLV-data for field with text key %1 due to no data in specification").arg(aTextKey));
		return "";
	}

	int field = mFFData.getKey(aTextKey);

	return getTLVData(field, aValue, aLog);
}

//--------------------------------------------------------------------------------
QByteArray FFEngine::getTLVData(int aField, const QVariant & aValue, QString * aLog)
{
	if (!mFFData.data().contains(aField))
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to get TLV-data for field %1 due to no data in specification").arg(aField));
		return "";
	}

	using namespace CFR::FiscalFields;

	SData FFData = mFFData[aField];
	QString log = mFFData.getTextLog(aField) + " = ";

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

			if (data.isEmpty())
			{
				data = " ";
				toLog(LogLevel::Warning, mDeviceName + QString(": No data in %1, changing to \"%2\"").arg(mFFData.getTextLog(aField)).arg(data));
			}

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
	CFR::FiscalFields::SData data = mFFData[aField];
	aFPData.insert(data.textKey, aValue.isValid() ? aValue : QString());
}

//--------------------------------------------------------------------------------
void FFEngine::checkFPData(TFiscalPaymentData & aFPData, int aField)
{
	QString textKey = mFFData[aField].textKey;
	QVariant value = getConfigParameter(textKey);

	checkFPData(aFPData, aField, value);
}

//--------------------------------------------------------------------------------
void FFEngine::checkFPData(TFiscalPaymentData & aFPData, int aField, const QVariant & aValue)
{
	QString textKey = mFFData[aField].textKey;

	if (!aFPData.contains(textKey))
	{
		if (!mFFData[aField].isString())
		{
			aFPData.insert(textKey, aValue);
		}
		else
		{
			QString value = clean(aValue.toString());
			aFPData.insert(textKey, value);
		}
	}
}

//--------------------------------------------------------------------------------
void FFEngine::checkSimpleFPData(TFiscalPaymentData & aFPData, int aField)
{
	CFR::FiscalFields::SData data = mFFData[aField];

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

	CFR::FiscalFields::TFields fields;
	int maxKeySize = 0;
	int maxTextKeySize = 0;

	foreach (const QString & textKey, aFPData.keys())
	{
		int field = mFFData.getKey(textKey);
		CFR::FiscalFields::SData data = mFFData[field];

		maxKeySize = qMax(maxKeySize, data.translationPF.size());
		maxTextKeySize = qMax(maxTextKeySize, data.textKey.size());
		fields << field;
	}

	qSort(fields);

	QStringList logData;

	foreach(int field, fields)
	{
		CFR::FiscalFields::SData data = mFFData[field];

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
	mFFData.data()[CFR::FiscalFields::TaxSystem].required = required;

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
	mFFData.data()[CFR::FiscalFields::AgentFlagsReg].required = required;

	return true;
}

//---------------------------------------------------------------------------
bool FFEngine::checkOperationModes(char aData, TOperationModes & aOperationModes)
{
	QStringList errorLog;
	aData &= ~CFR::OperationModeData.TrashMask;

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
	if (!mFFData.data().contains(aField))
	{
		aResult = false;
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to set %2 fiscal field due to it is unknown").arg(aField));

		return false;
	}

	using namespace CFR::FiscalFields;

	SData data = mFFData[aField];
	QString log = mFFData.getTextLog(aField);

	auto makeResult = [&] (const QString & aAddLog) -> bool
	{
		QString addLog = aAddLog.isEmpty() ? "" : aAddLog + ", but ";

		if (data.required == CFR::FiscalFields::ERequired::No)
		{
			toLog(LogLevel::Debug, mDeviceName + QString(": Don`t set %1 due to %2the field is not required").arg(log).arg(addLog));
			aResult = true;
		}
		else if ((data.required == CFR::FiscalFields::ERequired::PM) && !mOperatorPresence)
		{
			toLog(LogLevel::Debug, mDeviceName + QString(": Don`t set %1 due to %2the operator is not present").arg(log).arg(addLog));
			aResult = true;
		}
		else
		{
			toLog(LogLevel::Error, mDeviceName + QString(": Failed to set required %1 due to %2").arg(log).arg(aAddLog));
			aResult = false;
		}

		return false;
	};

	if (!containsConfigParameter(data.textKey))
	{
		return makeResult("the field is absent");
	}

	QVariant value = getConfigParameter(data.textKey);

	if (!value.isValid())
	{
		return makeResult("the field is not valid");
	}
	else if (value.toString().simplified().isEmpty())
	{
		return makeResult("the field is empty");
	}

	aResult = true;

	return true;
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

		return true;
	}

	QVariant agentFlagData = getConfigParameter(CHardwareSDK::FR::DealerAgentFlag);

	if (agentFlagData.toString().isEmpty())
	{
		if (aCanLog)
		{
			toLog(LogLevel::Warning, mDeviceName + ": Dealer agent flag is empty");
		}

		return true;
	}

	char agentFlag = char(agentFlagData.toInt());

	if (!agentFlag)
	{
		if (aCanLog)
		{
			toLog(LogLevel::Debug, mDeviceName + ": Dealer agent flag is zero");
		}

		return true;
	}

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
bool FFEngine::checkCashier(QString & aCashier)
{
	if (!containsConfigParameter(CFiscalSDK::Cashier))
	{
		toLog(LogLevel::Warning, mDeviceName + ": Failed to set cashier due to it is absent");
		return false;
	}

	aCashier = getConfigParameter(CFiscalSDK::Cashier).toString().simplified();

	if (aCashier.isEmpty())
	{
		toLog(LogLevel::Warning, mDeviceName + ": Failed to set cashier due to it is empty");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool FFEngine::checkTaxSystemOnPayment(SPaymentData & aPaymentData)
{
	char paymentTaxSystemData = char(aPaymentData.taxSystem);
	char dealerTaxSystemData  = char(getConfigParameter(CHardwareSDK::FR::DealerTaxSystem).toInt());
	ETaxSystems::Enum dealerTaxSystem = ETaxSystems::Enum(dealerTaxSystemData);

	if (dealerTaxSystem != ETaxSystems::None)
	{
		if (mTaxSystems.contains(dealerTaxSystemData))
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
				toLog(LogLevel::Error, mDeviceName + QString(": Failed to determine the required taxation system from the several ones (%1)").arg(toHexLog(joinedTaxSystems)));
				return false;
			}
		}
		else if (!mTaxSystems.contains(paymentTaxSystemData))
		{
			toLog(LogLevel::Error, mDeviceName + QString(": The actual taxation system(s) %1 don`t contain system %2 (%3)")
				.arg(toHexLog(joinedTaxSystems)).arg(toHexLog(paymentTaxSystemData)).arg(CFR::TaxSystems[paymentTaxSystemData]));
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
			if (!checkINN(unitData.providerINN))
			{
				return false;
			}
		}
	}
	*/

	char paymentAgentFlagData = char(aPaymentData.agentFlag);
	char dealerAgentFlagData  = char(getConfigParameter(CHardwareSDK::FR::DealerAgentFlag).toInt());
	EAgentFlags::Enum dealerAgentFlag = EAgentFlags::Enum(dealerAgentFlagData);

	if (dealerAgentFlag != EAgentFlags::None)
	{
		if (mAgentFlags.isEmpty() || mAgentFlags.contains(dealerAgentFlagData))
		{
			aPaymentData.agentFlag = dealerAgentFlag;
		}
		else if (mAgentFlags.size() == 1)
		{
			aPaymentData.agentFlag = EAgentFlags::Enum(mAgentFlags[0]);
		}
	}
	else if ((aPaymentData.agentFlag != EAgentFlags::None) && !mAgentFlags.isEmpty() && !mAgentFlags.contains(paymentAgentFlagData))
	{
		toLog(LogLevel::Error, mDeviceName + QString(": The actual agent flag(s) %1 don`t contain flag %2 (%3)")
			.arg(toHexLog(CFR::joinData(mAgentFlags))).arg(toHexLog(paymentAgentFlagData)).arg(CFR::AgentFlags[paymentAgentFlagData]));
		return false;
	}

	removeConfigParameter(CFiscalSDK::AgentFlag);
	removeConfigParameter(CFiscalSDK::AgentFlagsReg);

	if (aPaymentData.agentFlag != EAgentFlags::None)
	{
		paymentAgentFlagData = char(aPaymentData.agentFlag);
		setConfigParameter(CFiscalSDK::AgentFlag,     paymentAgentFlagData);
		setConfigParameter(CFiscalSDK::AgentFlagsReg, paymentAgentFlagData);
	}

	return true;
}

//--------------------------------------------------------------------------------
void FFEngine::filterAfterPayment(TFiscalPaymentData & aFPData, TComplexFiscalPaymentData & aPSData)
{
	CFR::FiscalFields::TFields removedFields;

	foreach(int field, CFR::FiscalFields::FiscalTotals)
	{
		QString textKey = mFFData[field].textKey;

		if (!aFPData.value(textKey).toInt())
		{
			removedFields << field;
			aFPData.remove(textKey);
		}
	}

	toLog(LogLevel::Normal, mDeviceName + QString(": fiscal fields %1 have been removed from the fiscal payment data").arg(mFFData.getLogFromList(removedFields)));

	  setFPData(aFPData, CFR::FiscalFields::FDName);
	checkFPData(aFPData, CFR::FiscalFields::PayOffType);
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
	checkSimpleFPData(aFPData, CFR::FiscalFields::InAutomateMode);

	#define ADD_SPEC_DFIELD(aData, aName, aSpecification) if (aData.contains(CFiscalSDK::aName)) { int value = aData[CFiscalSDK::aName].toInt(); \
		aData[CFiscalSDK::aName] = CFR::aSpecification[char(value)]; }
	#define ADD_SPEC_DFIELD_SAME(aData, aName) ADD_SPEC_DFIELD(aData, aName, aName##s)

	ADD_SPEC_DFIELD(aFPData, AgentFlagsReg, AgentFlags);
	ADD_SPEC_DFIELD_SAME(aFPData, TaxSystem);
	ADD_SPEC_DFIELD_SAME(aFPData, PayOffType);

	for (int i = 0; i < aPSData.size(); ++i)
	{
		ADD_SPEC_DFIELD_SAME(aPSData[i], PayOffSubjectType);
		ADD_SPEC_DFIELD_SAME(aPSData[i], PayOffSubjectMethodType);
		ADD_SPEC_DFIELD_SAME(aPSData[i], VATRate);
		ADD_SPEC_DFIELD_SAME(aPSData[i], AgentFlag);
	}
}

//--------------------------------------------------------------------------------
bool FFEngine::checkINN(const QString & aINN, int aType) const
{
	int size = aINN.size();
	bool wrongSize = (size != CFR::INN::Person::Legal) && (size != CFR::INN::Person::Natural);

	if (wrongSize || (aType && (aType != size)))
	{
		QString log = QString("%1: Wrong INN size = %2, need ").arg(mDeviceName).arg(size);
		QString legalLog   = QString("%1 for legal person").arg(CFR::INN::Person::Legal);
		QString naturalLog = QString("%1 for natural person").arg(CFR::INN::Person::Natural);

		     if (aType == CFR::INN::Person::Unknown) log += legalLog + " or " + naturalLog;
		else if (aType == CFR::INN::Person::Legal)   log += legalLog;
		else if (aType == CFR::INN::Person::Natural) log += naturalLog;

		toLog(LogLevel::Error, log);

		return false;
	}

	if (QRegExp("^[0-9]+$").indexIn(aINN) == -1)
	{
		toLog(LogLevel::Error, mDeviceName + ": Wrong INN = " + aINN);
		return false;
	}

	if (size == CFR::INN::Person::Legal)
	{
		qlonglong data = 0;

		for (int i = 0; i < aINN.size() - 1; ++i)
		{
			int digit = aINN[i].digitValue();
			data += digit * CFR::INN::Factors::Legal[i];
		}

		int digit = (data % CFR::INN::Divider) % 10;
		int dataDigit = aINN[CFR::INN::Person::Legal - 1].digitValue();

		if (digit != dataDigit)
		{
			toLog(LogLevel::Error, mDeviceName + ": Control numbers are not equal for INN " + aINN);
			return false;
		}
	}
	else if (size == CFR::INN::Person::Natural)
	{
		qlonglong data1 = 0;
		qlonglong data2 = 0;

		for (int i = 0; i < aINN.size() - 1; ++i)
		{
			int digit = aINN[i].digitValue();
			data2 += digit * CFR::INN::Factors::Natural2[i];

			if (i < aINN.size() - 2)
			{
				data1 += digit * CFR::INN::Factors::Natural1[i];
			}
		}

		int digit1 = (data1 % CFR::INN::Divider) % 10;
		int digit2 = (data2 % CFR::INN::Divider) % 10;
		int dataDigit1 = aINN[CFR::INN::Person::Natural - 2].digitValue();
		int dataDigit2 = aINN[CFR::INN::Person::Natural - 1].digitValue();

		if ((digit1 != dataDigit1) || (digit2 != dataDigit2))
		{
			toLog(LogLevel::Error, mDeviceName + ": Control numbers are not equal for INN " + aINN);
			return false;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
QString FFEngine::filterPhone(const QString & aData) const
{
	if (!aData.contains(QRegExp("[0-9]+")))
	{
		return "";
	}

	QString result = revert(aData).remove(QRegExp("\\n\\r\\t")).remove(QRegExp("^[^0-9]+"));
	int index = 1 + result.lastIndexOf(QRegExp("[0-9]+"));
	int last  = 1 + result.indexOf(QRegExp("[\\+\\(]"), index);
	result = revert(result.left(last ? last : index));

	if (result.startsWith("("))
	{
		result.prepend("+7");
	}
	else if (!result.indexOf(QRegExp("8[^0-9]")))
	{
		result.replace(0, 1, "+7");
	}

	index = result.indexOf("(");

	if (index != -1)
	{
		int index2 = result.indexOf("(", index + 1);

		if (index2 != -1)
		{
			QString data = result.left(index2);
			int index3 = data.lastIndexOf(ASCII::Space);

			if (index3 < index)
			{
				index3 = data.lastIndexOf(QRegExp("[0-9]"));
			}

			if ((index3 < index2) && (index3 > index))
			{
				result = result.left(index3 + 1);
			}
		}
	}

	index = result.indexOf(QRegExp(QString::fromUtf8("[a-zA-Zа-яА-Я\\.\\,\\;\\+]+")), 1);
	result = result.left(index).remove(QRegExp("[^0-9\\+]+"));

	if (result.size() == 10)
	{
		result.prepend("+7");
	}

	return result;
}

//--------------------------------------------------------------------------------
void FFEngine::addData(const CFR::FiscalFields::TData & aData)
{
	mFFData.add(aData);
}

//--------------------------------------------------------------------------------
void FFEngine::setDeviceName(const QString & aDeviceName)
{
	QWriteLocker lock(&mConfigurationGuard);

	mDeviceName = aDeviceName;
}

//--------------------------------------------------------------------------------
