/* @file Онлайн ФР семейства АТОЛ. */

#include "AtolOnlineFRBase.h"

using namespace SDK::Driver;
using namespace ProtocolUtils;

//--------------------------------------------------------------------------------
AtolOnlineFRBase::AtolOnlineFRBase()
{
	// параметры семейства ФР
	mSupportedModels = getModelList();
	mDeviceName = "ATOL online FR";
	mNextReceiptProcessing = false;
	mIsOnline = true;
	mFRBuildUnifiedTaxes = 3689;

	mOFDFiscalParameters
		<< FiscalFields::TaxSystem
		<< FiscalFields::AgentFlag;

	// регистры
	      mRegisterData.add(CAtolFR::Registers::SerialNumber,      '\x16', 7);
	      mRegisterData.add(CAtolFR::Registers::NonNullableAmount, '\x28', 7);
	mRegisterData.add(CAtolOnlineFR::Registers::OFDNotSentCount,   '\x2C', 3);
	mRegisterData.add(CAtolOnlineFR::Registers::FFD,               '\x36', 6);
	mRegisterData.add(CAtolOnlineFR::Registers::SessionData,       '\x35', 6);
	mRegisterData.add(CAtolOnlineFR::Registers::ExtendedErrorData, '\x37', 8);

	// типы оплаты
	mPayTypeData.add(EPayTypes::Cash,         1);
	mPayTypeData.add(EPayTypes::EMoney,       2);
	mPayTypeData.add(EPayTypes::PostPayment,  3);
	mPayTypeData.add(EPayTypes::Credit,       4);
	mPayTypeData.add(EPayTypes::CounterOffer, 5);

	// команды
	setConfigParameter(CHardware::FR::Commands::PrintingDeferredZReports, CAtolOnlineFR::Commands::PrintDeferredZReports);

	// ошибки
	mErrorData = PErrorData(new CAtolOnlineFR::Errors::CData());
	setConfigParameter(CHardwareSDK::CanOnline, true);
}

//--------------------------------------------------------------------------------
QStringList AtolOnlineFRBase::getModelList()
{
	return CAtolFR::CModelData().getModelList(EFRType::FS, false);
}

//--------------------------------------------------------------------------------
bool AtolOnlineFRBase::updateParameters()
{
	QByteArray data;

	if (processCommand(CAtolOnlineFR::Commands::GetInternalFirmware, &data) && (data.size() >= 2))
	{
		mFRBuild = data.right(2).toHex().toInt();
		setDeviceParameter(CDeviceData::InternalFirmware, mFRBuild);
	}

	if (!AtolFRBase::updateParameters())
	{
		return false;
	}

	if (!processCommand(CAtolOnlineFR::Commands::FS::GetFiscalizationResume, &data) || (data.size() <= 40))
	{
		return false;
	}

	if (!checkTaxationData(data[39]) || !checkOperationModes(data[40]))
	{
		return false;
	}

	using namespace CFR::FiscalFields;

	ERequired::Enum required = (mTaxations.size() > 1) ? ERequired::Yes : ERequired::No;
	mFiscalFieldData.data()[FiscalFields::TaxSystem].required = required;

	#define SET_LCONFIG_FISCAL_FIELD(aName) if (getTLV(FiscalFields::aName, data)) { setLConfigParameter(CHardware::FiscalFields::aName, data); \
		toLog(LogLevel::Normal, QString("%1: Set fiscal tag %2 (%3) = \"%4\" to config data") \
			.arg(mDeviceName).arg(FiscalFields::aName).arg(CHardware::FiscalFields::aName).arg(getConfigParameter(CHardware::FiscalFields::aName, data).toString())); }

	SET_LCONFIG_FISCAL_FIELD(FTSURL);
	SET_LCONFIG_FISCAL_FIELD(OFDURL);
	SET_LCONFIG_FISCAL_FIELD(OFDName);
	SET_LCONFIG_FISCAL_FIELD(LegalOwner);
	SET_LCONFIG_FISCAL_FIELD(PayOffAddress);
	SET_LCONFIG_FISCAL_FIELD(PayOffPlace);

	if (getTLV(FiscalFields::AgentFlagsRegistered, data))
	{
		return checkAgentFlags(data[0]);
	}

	return true;
}

//--------------------------------------------------------------------------------
CAtolFR::TModelKey AtolOnlineFRBase::getModelKey(const QByteArray & aAnswer)
{
	int modelNumber = uchar(aAnswer[3]);
	toLog(LogLevel::Normal, QString("AtolFR: model number = %1").arg(modelNumber));

	return CAtolFR::TModelKey(modelNumber, EFRType::FS);
}

//--------------------------------------------------------------------------------
bool AtolOnlineFRBase::getPrintingSettings()
{
	QByteArray data;

	if (!getRegister(CAtolFR::Registers::PrintingSettings, data))
	{
		return false;
	}

	mLineSize = data.left(1).toHex().toInt();

	return true;
}

//--------------------------------------------------------------------------------
void AtolOnlineFRBase::processDeviceData()
{
	AtolFRBase::processDeviceData();

	QByteArray data;
	mNonNullableAmount = 0;

	if (getRegister(CAtolFR::Registers::NonNullableAmount, data, CAtolOnlineFR::SalingFiscalDocument))
	{
		mNonNullableAmount = qlonglong(data.toHex().toULongLong()) / 100.0;
	}

	if (processCommand(CAtolOnlineFR::Commands::FS::GetVersion, &data) && (data.size() > 18))
	{
		setDeviceParameter(CDeviceData::FS::Version, data.mid(2, 16));
	}

	if (processCommand(CAtolOnlineFR::Commands::FS::GetFiscalizationResume, &data))
	{
		mINN = CFR::INNToString(data.mid(7, 12));
		mRNM = CFR::RNMToString(data.mid(19, 20));
	}

	auto getData = [&data] (int aIndex) -> QString { return QString("%1").arg(int(uchar(data[aIndex])), 2, 10, QChar(ASCII::Zero)); };

	if (processCommand(CAtolOnlineFR::Commands::FS::GetValidity, &data) && (data.size() >= 5))
	{
		QDate date = QDate::fromString("20" + hexToBCD(data.mid(2, 3)), CAtolOnlineFR::DateFormat);

		if (date.isValid())
		{
			setDeviceParameter(CDeviceData::FS::ValidityData, date.toString(CFR::DateLogFormat));
		}

		if (data.size() > 6)
		{
			setDeviceParameter(CDeviceData::FR::ReregistrationNumber, uchar(data[6]));
			setDeviceParameter(CDeviceData::FR::FreeReregistrations,  uchar(data[5]));
		}
	}

	if (processCommand(CAtolOnlineFR::Commands::FS::GetStatus, &data) && (data.size() >= 32))
	{
		mFSSerialNumber = CFR::FSSerialToString(data.mid(12, 16));
		int FDCount = revert(data.mid(28, 4)).toHex().toUInt(0, 16);

		setDeviceParameter(CDeviceData::FR::FiscalDocuments, FDCount);
	}

	checkDateTime();

	if (getRegister(CAtolOnlineFR::Registers::FFD, data) && (data.size() > 1))
	{
		mFFDFR = EFFD::Enum(int(data[0]));
		mFFDFS = EFFD::Enum(int(data[1]));
	}

	char mode = mMode;

	if (enterInnerMode(CAtolFR::InnerModes::Programming))
	{
		QByteArray addressData;
		QByteArray portData;
		mOFDDataError = !getFRParameter(CAtolOnlineFR::FRParameters::OFDAddress, addressData) ||
		                !getFRParameter(CAtolOnlineFR::FRParameters::OFDPort, portData);
		mOFDDataError = mOFDDataError || !checkOFDData(addressData, portData);
	}

	enterInnerMode(mode);
}

//---------------------------------------------------------------------------
int AtolOnlineFRBase::getSessionNumber()
{
	QByteArray data;

	if (getRegister(CAtolOnlineFR::Registers::SessionData, data))
	{
		return data.mid(3, 3).toHex().toInt();
	}

	return 0;
}

//--------------------------------------------------------------------------------
bool AtolOnlineFRBase::setFRParameters()
{
	if (!AtolFRBase::setFRParameters())
	{
		return false;
	}

	// налоги
	if (!setFRParameter(CAtolOnlineFR::FRParameters::Taxes, CAtolOnlineFR::TaxParameters))
	{
		return false;
	}

	// печать РНМ перед ИНН - всегда
	setFRParameter(CAtolOnlineFR::FRParameters::PrintRNM, false);

	// Z-отчет
	setFRParameter(CAtolOnlineFR::FRParameters::ZReport, CAtolOnlineFR::ZReportParameters);

	return true;
}

//--------------------------------------------------------------------------------
bool AtolOnlineFRBase::getStatus(TStatusCodes & aStatusCodes)
{
	QByteArray data;

	if (!AtolFRBase::getStatus(aStatusCodes) || !getRegister(CAtolOnlineFR::Registers::OFDNotSentCount, data))
	{
		return false;
	}

	int OFDNotSentCount = data.isEmpty() ? -1 : data.toHex().toInt();
	checkOFDNotSentCount(OFDNotSentCount, aStatusCodes);

	return true;
}

//--------------------------------------------------------------------------------
bool AtolOnlineFRBase::performFiscal(const QStringList & aReceipt, const SPaymentData & aPaymentData, TFiscalPaymentData & aFPData, TComplexFiscalPaymentData & aPSData)
{
	openFRSession();

	if (mTaxations.size() > 1)
	{
		setConfigParameter(CHardware::FiscalFields::TaxSystem, aPaymentData.taxation);
	}

	bool result = AtolFRBase::performFiscal(aReceipt, aPaymentData, aFPData, aPSData);

	if (result)
	{
		QByteArray data;

		if (processCommand(CAtolOnlineFR::Commands::FS::GetStatus, &data) && (data.size() >= 32))
		{
			uint FDNumber = revert(data.mid(28, 4)).toHex().toUInt(0, 16);
			aFPData.insert(FiscalFields::FDNumber, FDNumber);

			if (processCommand(CAtolOnlineFR::Commands::FS::StartFiscalTLVData, getHexReverted(FDNumber, 4)))
			{
				CFR::STLV TLV;

				while (processCommand(CAtolOnlineFR::Commands::FS::GetFiscalTLVData, &data))
				{
					if (parseTLV(data.mid(2), TLV))
					{
						if (CFR::FiscalFields::FSRequired.contains(TLV.field))
						{
							parseSTLVData(TLV, aPSData);
							parseTLVData(TLV.field, TLV.data, aFPData);
						}
					}
				}
			}
		}
	}

	QByteArray data;

	if (getRegister(CAtolOnlineFR::Registers::SessionData, data))
	{
		aFPData.insert(FiscalFields::DocumentNumber, data.left(3).toHex().toInt());
	}

	return result;
}

//--------------------------------------------------------------------------------
bool AtolOnlineFRBase::performEncashment(const QStringList & aReceipt, double aAmount)
{
	openFRSession();

	return AtolFRBase::performEncashment(aReceipt, aAmount);
}

//--------------------------------------------------------------------------------
bool AtolOnlineFRBase::processAnswer(const QByteArray & aCommand, char aError)
{
	switch (aError)
	{
		case CAtolOnlineFR::Errors::NeedExtendedErrorCode:
		{
			QByteArray data;

			if (getRegister(CAtolOnlineFR::Registers::ExtendedErrorData, data))
			{
				char command = char(data.left(2).toHex().toInt(0, 16));

				if (command == aCommand[0])
				{
					ushort error = data.mid(2, 2).toHex().toUShort(0, 16);

					toLog(LogLevel::Error, "AtolFR: Extended error: " + CAtolOnlineFR::Errors::ExtendedData[error]);
				}
			}

			break;
		}
		case CAtolFR::Errors::WrongFieldNumber:
		{
			mOldFirmware = mOldFirmware || (aCommand[0] == CAtolOnlineFR::Commands::SetOFDParameter);

			break;
		}
	}

	return AtolFRBase::processAnswer(aCommand, aError);
}

//---------------------------------------------------------------------------
bool AtolOnlineFRBase::checkTaxes()
{
	if (mFRBuild >= mFRBuildUnifiedTaxes)
	{
		mTaxData.add(18, 1, "НДС 18%");
		mTaxData.add(10, 2, "НДС 10%");
		mTaxData.add( 0, 6, "БЕЗ НАЛОГА");
	}

	return AtolFRBase::checkTaxes();
}

//--------------------------------------------------------------------------------
bool AtolOnlineFRBase::checkTax(TVAT aVAT, const CFR::Taxes::SData & aData)
{
	return checkTaxValue(aVAT, aData, CAtolOnlineFR::FRParameters::Tax, false);
}

//--------------------------------------------------------------------------------
bool AtolOnlineFRBase::sale(const SAmountData & aAmountData)
{
	if (!processCommand(CAtolOnlineFR::Commands::StartSale, CAtolOnlineFR::StartSailingData))
	{
		return false;
	}

	int taxGroup = mTaxData[aAmountData.VAT].group;
	QByteArray sum = getBCD(aAmountData.sum / 10.0, 7, 2, 3);
	QByteArray name = mCodec->fromUnicode(aAmountData.name);
	char section = (aAmountData.section == -1) ? ASCII::NUL : char(aAmountData.section);

	QByteArray commandData;
	commandData.append(CAtolOnlineFR::SaleFlags);     // флаги
	commandData.append(sum);                          // сумма
	commandData.append(getBCD(10, 5, 2));             // количество = 1 штука
	commandData.append(sum);                          // стоимость
	commandData.append(uchar(taxGroup));              // налог (тип)
	commandData.append(getBCD(0, 7, 2, 3));           // налог (сумма) - ФР считает налог сам
	commandData.append(uchar(section));               // секция
	commandData.append(QByteArray(3, ASCII::NUL));    // признак предмета расчета, признак способа расчета и знак скидки
	commandData.append(getBCD(0, 7, 2, 3));           // информация о скидке
	commandData.append(QByteArray(2, ASCII::NUL));    // зарезервировано
	commandData.append(name);                         // товар

	return processCommand(CAtolOnlineFR::Commands::EndSale, commandData);
}

//--------------------------------------------------------------------------------
bool AtolOnlineFRBase::setTLV(int aField)
{
	bool result;

	if (!checkFiscalField(aField, result))
	{
		return result;
	}

	QString fieldLog;
	QByteArray commandData = getTLVData(aField, getConfigParameter(mFiscalFieldData[aField].description), &fieldLog);
	commandData.prepend('\x00');    // номер блока
	commandData.prepend('\x01');    // количество блоков
	commandData.prepend(CAtolOnlineFR::PrintOFDParameter);    /// печатать ОФД-реквизит.

	if (!processCommand(CAtolOnlineFR::Commands::SetOFDParameter, commandData))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to set " + fieldLog);
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool AtolOnlineFRBase::getTLV(int aField, QByteArray & aData, uchar aBlockNumber)
{
	aData.clear();

	QByteArray commandData;
	commandData.append(char(aField >> 0));
	commandData.append(char(aField >> 8));
	commandData.append(aBlockNumber);

	QByteArray data;
	CFR::STLV TLV;
	QString errorLog = QString("%1: Failed to get fiscal tag %2 (%3)").arg(mDeviceName).arg(aField).arg(mFiscalFieldData[aField].description);

	if (aBlockNumber)
	{
		errorLog += ", block " + QString::number(int(aBlockNumber));
	}

	if (!processCommand(CAtolOnlineFR::Commands::GetOFDParameter, commandData, &data))
	{
		toLog(LogLevel::Error, errorLog);
		return false;
	}

	if (data.size() < 3)
	{
		toLog(LogLevel::Error, QString("%1 due to invalid answer length = %4, need 3 minimum").arg(errorLog).arg(data.size()));
		return false;
	}

	if (aBlockNumber)
	{
		aData = data.mid(3);

		return true;
	}

	uchar blocks = uchar(data[2]);

	for (uchar i = 1; i < blocks; ++i)
	{
		if (!getTLV(aField, aData, i))
		{
			return false;
		}

		data += aData;
	}

	if (!parseTLV(data.mid(3), TLV))
	{
		toLog(LogLevel::Error, errorLog + " due to parse error");
		return false;
	}

	aData = TLV.data;

	return true;
}

//--------------------------------------------------------------------------------
void AtolOnlineFRBase::setErrorFlags(char aError, const QByteArray & aCommand)
{
	AtolFRBase::setErrorFlags(aError, aCommand);
	bool noError = (aCommand == CAtolOnlineFR::Commands::FS::GetFiscalTLVData) && (aError == CAtolOnlineFR::Errors::NoRequiedDataInFS);

	if ((mErrorData->value(aError).type == FRError::EType::FS) && !noError)
	{
		mFSError = true;
	}
}

//--------------------------------------------------------------------------------
