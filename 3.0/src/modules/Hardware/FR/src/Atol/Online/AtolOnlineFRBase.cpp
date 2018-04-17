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

	mOFDFiscalParametersOnSale
		<< CFR::FiscalFields::PayOffSubjectMethodType;

	setConfigParameter(CHardwareSDK::FR::CanWithoutPrinting, true);

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
	mPayTypeData.add(EPayTypes::PrePayment,   3);
	mPayTypeData.add(EPayTypes::PostPayment,  4);
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
char AtolOnlineFRBase::getPrinterId()
{
	return CAtolOnlinePrinters::Trade;
}

//--------------------------------------------------------------------------------
bool AtolOnlineFRBase::setNotPrintDocument(bool aEnabled)
{
	char printingValue = aEnabled ? CAtolOnlinePrinters::Memory : getPrinterId();

	if (!printingValue)
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to set printer model due to printer model Id is invalid");
		return false;
	}

	char mode = mMode;

	if (!enterInnerMode(CAtolFR::InnerModes::Programming))
	{
		return false;
	}

	QByteArray data;
	bool result = getFRParameter(CAtolOnlineFR::FRParameters::PrinterModel, data) && !data.isEmpty() && (data[0] == printingValue);

	if (!result)
	{
		result = setFRParameter(CAtolOnlineFR::FRParameters::PrinterModel, printingValue) && reboot();
	}

	enterInnerMode(mode);

	return result;
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

	int reregistrationNumber = getDeviceParameter(CDeviceData::FR::ReregistrationNumber).toInt();

	if (!processCommand(CAtolOnlineFR::Commands::FS::GetFiscalizationResume, QByteArray(1, char(reregistrationNumber)), &data) || (data.size() <= 40))
	{
		return false;
	}

	if (!checkTaxSystems(data[39]) || !checkOperationModes(data[40]))
	{
		return false;
	}

	#define SET_LCONFIG_FISCAL_FIELD(aName) QString aName##Log = QString("fiscal tag %1 (%2)").arg(CFR::FiscalFields::aName).arg(CFiscalSDK::aName); \
		if (getTLV(CFR::FiscalFields::aName, data)) { mFFEngine.setLConfigParameter(CFiscalSDK::aName, data); \
		     QString value = mFFEngine.getConfigParameter(CFiscalSDK::aName, data).toString(); \
		     toLog(LogLevel::Normal, QString("%1: Add %2 = \"%3\" to config data").arg(mDeviceName).arg(aName##Log).arg(value)); }

	#define SET_BCONFIG_FISCAL_FIELD(aName) QString aName##Log = QString("fiscal tag %1 (%2)").arg(CFR::FiscalFields::aName).arg(CFiscalSDK::aName); \
		if (getTLV(CFR::FiscalFields::aName, data)) { char value = data[0]; mFFEngine.setConfigParameter(CFiscalSDK::aName, value); \
		     toLog(LogLevel::Normal, QString("%1: Add %2 = %3 to config data").arg(mDeviceName).arg(aName##Log).arg(int(value))); }

	SET_LCONFIG_FISCAL_FIELD(FTSURL);
	SET_LCONFIG_FISCAL_FIELD(OFDURL);
	SET_LCONFIG_FISCAL_FIELD(OFDName);
	SET_LCONFIG_FISCAL_FIELD(LegalOwner);
	SET_LCONFIG_FISCAL_FIELD(PayOffAddress);
	SET_LCONFIG_FISCAL_FIELD(PayOffPlace);

	SET_BCONFIG_FISCAL_FIELD(LotteryMode);
	SET_BCONFIG_FISCAL_FIELD(GamblingMode);
	SET_BCONFIG_FISCAL_FIELD(ExcisableUnitMode);

	return getTLV(CFR::FiscalFields::AgentFlagsReg, data) && !data.isEmpty() && checkAgentFlags(data[0]);
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
		setDeviceParameter(CDeviceData::FS::Version, QString("%1, type %2").arg(clean(data.mid(2, 16)).data()).arg(data[18] ? "serial" : "debug"));
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
bool AtolOnlineFRBase::reboot()
{
	bool mode = mMode;

	if (!enterInnerMode(CAtolFR::InnerModes::Cancel))
	{
		return false;
	}

	bool result = processCommand(CAtolOnlineFR::Commands::Reboot, QByteArray(1, ASCII::NUL));

	if (!result)
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to reboot FR properly");
	}

	SleepHelper::msleep(CAtolOnlineFR::RebootPause);
	result = waitReady(CAtolOnlineFR::RebootWaiting);

	enterInnerMode(mode);

	return result;
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

	bool result = AtolFRBase::performFiscal(aReceipt, aPaymentData, aFPData, aPSData);

	if (result)
	{
		auto getDataWaiting = [&] (const std::function<TResult()> & aCommand) -> TResult { TResult result; int i = 0; do { result = aCommand(); }
			while ((result == CommandResult::Transport) && waitReady(CAtolOnlineFR::GetFiscalWaiting) && (++i < CAtolOnlineFR::MaxRepeatingFiscalData)); return result; };
		#define GET_FS_DATA_WAITING(aCommand, ...) getDataWaiting([&] () -> TResult { return processCommand(CAtolOnlineFR::Commands::FS::aCommand, __VA_ARGS__); })

		QByteArray data;

		if (GET_FS_DATA_WAITING(GetStatus, &data) && (data.size() >= 32))
		{
			uint FDNumber = revert(data.mid(28, 4)).toHex().toUInt(0, 16);
			mFFEngine.checkFPData(aFPData, CFR::FiscalFields::FDNumber, FDNumber);

			if (GET_FS_DATA_WAITING(StartFiscalTLVData, getHexReverted(FDNumber, 4)))
			{
				CFR::STLV TLV;

				while (GET_FS_DATA_WAITING(GetFiscalTLVData, &data))
				{
					if (mFFEngine.parseTLV(data.mid(2), TLV))
					{
						if (mFiscalFieldData.data().contains(TLV.field))
						{
							mFFEngine.parseSTLVData(TLV, aPSData);
							mFFEngine.parseTLVData(TLV.field, TLV.data, aFPData);
						}
						else
						{
							toLog(LogLevel::Warning, QString("%1: Failed to add fiscal field %2 due to it is unknown").arg(mDeviceName).arg(TLV.field));
						}
					}
				}
			}
		}
	}

	QByteArray data;

	if (getRegister(CAtolOnlineFR::Registers::SessionData, data))
	{
		uint documentNumber = data.left(3).toHex().toUInt();
		mFFEngine.checkFPData(aFPData, CFR::FiscalFields::DocumentNumber, documentNumber);
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

	bool result = AtolFRBase::processAnswer(aCommand, aError);

	if ((aCommand == CAtolOnlineFR::Commands::FS::GetFiscalTLVData) && !mProcessingErrors.isEmpty() && (mProcessingErrors.last() == CAtolOnlineFR::Errors::NoRequiedDataInFS))
	{
		mProcessingErrors.pop_back();
		mLastError = 0;
	}

	return result;
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
bool AtolOnlineFRBase::sale(const SUnitData & aUnitData)
{
	if (!processCommand(CAtolOnlineFR::Commands::StartSale, CAtolOnlineFR::StartSailingData))
	{
		return false;
	}

	if (!setOFDParametersOnSale(aUnitData))
	{
		return false;
	}

	int taxGroup = mTaxData[aUnitData.VAT].group;
	QByteArray sum = getBCD(aUnitData.sum / 10.0, 7, 2, 3);
	QByteArray name = mCodec->fromUnicode(aUnitData.name);
	char section = (aUnitData.section == -1) ? ASCII::NUL : char(aUnitData.section);

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
bool AtolOnlineFRBase::setTLV(int aField, bool /*aForSale*/)
{
	bool result;

	if (!mFFEngine.checkFiscalField(aField, result))
	{
		return result;
	}

	QString fieldLog;
	QByteArray commandData = mFFEngine.getTLVData(aField, &fieldLog);
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
	QString errorLog = QString("%1: Failed to get fiscal tag %2 (%3)").arg(mDeviceName).arg(aField).arg(mFiscalFieldData[aField].textKey);

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

	if (!mFFEngine.parseTLV(data.mid(3), TLV))
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
