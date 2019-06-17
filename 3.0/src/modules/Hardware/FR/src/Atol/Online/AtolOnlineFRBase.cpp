/* @file Онлайн ФР семейства АТОЛ. */

#include "AtolOnlineFRBase.h"

using namespace SDK::Driver;
using namespace ProtocolUtils;

template class AtolOnlineFRBase<Atol2FRBase>;
template class AtolOnlineFRBase<Atol3FRBase>;

//--------------------------------------------------------------------------------
template<class T>
AtolOnlineFRBase<T>::AtolOnlineFRBase()
{
	// параметры семейства ФР
	mSupportedModels = getModelList();
	mDeviceName = "ATOL online FR";
	mNextReceiptProcessing = false;
	mIsOnline = true;
	mFRBuildUnifiedTaxes = 3689;
	setConfigParameter(CHardwareSDK::CanOnline, true);

	mOFDFiscalParametersOnSale
		<< CFR::FiscalFields::PayOffSubjectMethodType;

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
	mErrorData = PErrorData(new CAtolOnlineFR::Errors::Data());
	mUnprocessedErrorData.add(CAtolOnlineFR::Commands::FS::GetFiscalTLVData, CAtolOnlineFR::Errors::NoRequiedDataInFS);
}

//--------------------------------------------------------------------------------
template<class T>
QStringList AtolOnlineFRBase<T>::getModelList()
{
	return CAtolFR::CModelData().getModelList(EFRType::FS, false);
}

//--------------------------------------------------------------------------------
template<class T>
char AtolOnlineFRBase<T>::getPrinterId()
{
	return CAtolOnlinePrinters::Trade;
}

//--------------------------------------------------------------------------------
template<class T>
bool AtolOnlineFRBase<T>::updateParameters()
{
	QByteArray data;

	if (processCommand(CAtolOnlineFR::Commands::GetInternalFirmware, &data) && (data.size() >= 2))
	{
		mFRBuild = data.right(2).toHex().toInt();
		setDeviceParameter(CDeviceData::InternalFirmware, mFRBuild);
	}

	if (!T::updateParameters())
	{
		return false;
	}

	#define SET_LCONFIG_FISCAL_FIELD(aName) if (getTLV(CFR::FiscalFields::aName, data)) { mFFEngine.setLConfigParameter(CFiscalSDK::aName, data); \
		QString value = mFFEngine.getConfigParameter(CFiscalSDK::aName, data).toString(); toLog(LogLevel::Normal, mDeviceName + \
			QString(": Add %1 = \"%2\" to config data").arg(mFFData.getTextLog(CFR::FiscalFields::aName)).arg(value)); }

	#define SET_BCONFIG_FISCAL_FIELD(aName) if (getTLV(CFR::FiscalFields::aName, data)) { char value = data[0]; mFFEngine.setConfigParameter(CFiscalSDK::aName, value); \
		toLog(LogLevel::Normal, mDeviceName + QString(": Add %1 = %2 to config data").arg(mFFData.getTextLog(CFR::FiscalFields::aName)).arg(uchar(value))); }

	SET_LCONFIG_FISCAL_FIELD(FTSURL);
	SET_LCONFIG_FISCAL_FIELD(OFDName);
	SET_LCONFIG_FISCAL_FIELD(LegalOwner);
	SET_LCONFIG_FISCAL_FIELD(PayOffAddress);
	SET_LCONFIG_FISCAL_FIELD(PayOffPlace);

	SET_BCONFIG_FISCAL_FIELD(LotteryMode);
	SET_BCONFIG_FISCAL_FIELD(GamblingMode);
	SET_BCONFIG_FISCAL_FIELD(ExcisableUnitMode);
	SET_BCONFIG_FISCAL_FIELD(InAutomateMode);

	if (!isFiscal())
	{
		return true;
	}

	int reregistrationNumber = getDeviceParameter(CDeviceData::FR::ReregistrationNumber).toInt();

	if (!processCommand(CAtolOnlineFR::Commands::FS::GetFiscalizationTotal, QByteArray(1, char(reregistrationNumber)), &data) || (data.size() <= 40))
	{
		toLog(LogLevel::Normal, mDeviceName + ": Failed to get fiscalization total");
		return false;
	}

	if (!checkTaxSystems(data[39]) || !checkOperationModes(data[40]))
	{
		return false;
	}

	return getTLV(CFR::FiscalFields::AgentFlagsReg, data) && !data.isEmpty() && checkAgentFlags(data[0]);
}

//--------------------------------------------------------------------------------
template<class T>
CAtolFR::TModelKey AtolOnlineFRBase<T>::getModelKey(const QByteArray & aAnswer)
{
	int modelNumber = uchar(aAnswer[3]);
	toLog(LogLevel::Normal, QString("AtolFR: model number = %1").arg(modelNumber));

	return CAtolFR::TModelKey(modelNumber, EFRType::FS);
}

//--------------------------------------------------------------------------------
template<class T>
bool AtolOnlineFRBase<T>::getPrintingSettings()
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
template<class T>
void AtolOnlineFRBase<T>::processDeviceData()
{
	T::processDeviceData();

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

	if (processCommand(CAtolOnlineFR::Commands::FS::GetFiscalizationTotal, &data))
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
template<class T>
int AtolOnlineFRBase<T>::getSessionNumber()
{
	QByteArray data;

	if (getRegister(CAtolOnlineFR::Registers::SessionData, data))
	{
		return data.mid(3, 3).toHex().toInt();
	}

	return 0;
}

//--------------------------------------------------------------------------------
template<class T>
bool AtolOnlineFRBase<T>::setFRParameters()
{
	if (!T::setFRParameters())
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
template<class T>
bool AtolOnlineFRBase<T>::reboot()
{
	char mode = mMode;

	if (!enterInnerMode(CAtolFR::InnerModes::Cancel))
	{
		return false;
	}

	bool result = processCommand(CAtolOnlineFR::Commands::Reboot);

	if (!result)
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to reboot FR properly");
	}

	SleepHelper::msleep(CAtolOnlineFR::RebootPause);
	result = waitReady(CAtolOnlineFR::RebootWaiting);

	exitInnerMode(true);
	enterInnerMode(mode);

	return result;
}

//--------------------------------------------------------------------------------
template<class T>
bool AtolOnlineFRBase<T>::getStatus(TStatusCodes & aStatusCodes)
{
	if (!T::getStatus(aStatusCodes))
	{
		return false;
	}

	QByteArray data = performStatus(aStatusCodes, CAtolOnlineFR::Commands::FS::GetStatus, 6);

	if (data == CFR::Result::Fail)
	{
		return false;
	}
	else if (data != CFR::Result::Error)
	{
		checkFSFlags(data[6], aStatusCodes);
	}

	TResult result = getRegister(CAtolOnlineFR::Registers::OFDNotSentCount, data);

	if (!CORRECT(result))
	{
		return false;
	}
	else if (result == CommandResult::Answer)
	{
		aStatusCodes.insert(DeviceStatusCode::Warning::OperationError);
	}
	else if (result == CommandResult::Device)
	{
		int statusCode = getErrorStatusCode(mErrorData->value(mLastError).type);
		aStatusCodes.insert(statusCode);
	}
	else
	{
		checkOFDNotSentCount(data.toHex().toInt(), aStatusCodes);
	}

	return true;
}

//--------------------------------------------------------------------------------
template<class T>
bool AtolOnlineFRBase<T>::performFiscal(const QStringList & aReceipt, const SPaymentData & aPaymentData, quint32 * aFDNumber)
{
	if (!T::performFiscal(aReceipt, aPaymentData))
	{
		return false;
	}

	QByteArray data;

	if (aFDNumber && PROCESS_ATOL_FD_DATA(GetStatus, &data) && (data.size() >= 32))
	{
		*aFDNumber = revert(data.mid(28, 4)).toHex().toUInt(0, 16);
	}

	return true;
}

//--------------------------------------------------------------------------------
template<class T>
TResult AtolOnlineFRBase<T>::getFiscalTLVData(QByteArray & aData)
{
	TResult result = CommandResult::OK;

	do
	{
		QByteArray data;
		result = PROCESS_ATOL_FD_DATA(GetFiscalTLVData, &data);

		aData += data.mid(2);

		auto getInt = [&aData] (int aIndex, int aShift) -> int { int result = uchar(aData[aIndex]); return result << (8 * aShift); };
		int size = getInt(2, 0) | getInt(3, 1);
		int dataSize = aData.size() - 4;

		if (dataSize >= size)
		{
			break;
		}
	}
	while (result && (aData.size() >= 4));

	return result;
}

//--------------------------------------------------------------------------------
template<class T>
bool AtolOnlineFRBase<T>::getFiscalFields(quint32 aFDNumber, TFiscalPaymentData & aFPData, TComplexFiscalPaymentData & aPSData)
{
	if (!PROCESS_ATOL_FD_DATA(StartFiscalTLVData, getHexReverted(aFDNumber, 4)))
	{
		return false;
	}

	TGetFiscalTLVData getFiscalTLVData = std::bind(&AtolOnlineFRBase::getFiscalTLVData, this, std::placeholders::_1);

	return processFiscalTLVData(getFiscalTLVData, &aFPData, &aPSData);
}

//--------------------------------------------------------------------------------
template<class T>
TResult AtolOnlineFRBase<T>::processDataWaiting(const std::function<TResult()> & aCommand)
{
	TResult result;
	int i = 0;

	do
	{
		result = aCommand();
	}
	while ((result == CommandResult::Transport) && waitReady(CAtolOnlineFR::GetFiscalWaiting) && (++i < CAtolOnlineFR::MaxRepeatingFiscalData));

	return result;
}

//--------------------------------------------------------------------------------
template<class T>
bool AtolOnlineFRBase<T>::performEncashment(const QStringList & aReceipt, double aAmount)
{
	openFRSession();

	return T::performEncashment(aReceipt, aAmount);
}

//--------------------------------------------------------------------------------
template<class T>
bool AtolOnlineFRBase<T>::processAnswer(const QByteArray & aCommand, char aError)
{
	switch (aError)
	{
		case CAtolOnlineFR::Errors::FSOfflineEnd:
		{
			mProcessingErrors.push_back(aError);

			mFSOfflineEnd = true;

			break;
		}
		case CAtolOnlineFR::Errors::NeedExtendedErrorCode:
		{
			QByteArray data;

			if (getRegister(CAtolOnlineFR::Registers::ExtendedErrorData, data))
			{
				char command = char(data.left(2).toHex().toInt(0, 16));

				if (command == aCommand[0])
				{
					ushort error = data.mid(2, 2).toHex().toUShort(0, 16);

					toLog(LogLevel::Error, mDeviceName + ": Extended error: " + CAtolOnlineFR::Errors::ExtraData[error]);
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

	return T::processAnswer(aCommand, aError);
}

//---------------------------------------------------------------------------
template<class T>
bool AtolOnlineFRBase<T>::checkTaxes()
{
	if (mFRBuild >= mFRBuildUnifiedTaxes)
	{
		mTaxData.add(18, 1);
		mTaxData.add(10, 2);
		mTaxData.add( 0, 6);
	}

	return T::checkTaxes();
}

//--------------------------------------------------------------------------------
template<class T>
bool AtolOnlineFRBase<T>::checkTax(TVAT aVAT, CFR::Taxes::SData & aData)
{
	return checkTaxValue(aVAT, aData, CAtolOnlineFR::FRParameters::Tax, false);
}

//--------------------------------------------------------------------------------
template<class T>
bool AtolOnlineFRBase<T>::sale(const SUnitData & aUnitData)
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
	char section = (aUnitData.section == -1) ? ASCII::NUL : char(aUnitData.section);
	QByteArray sum = getBCD(aUnitData.sum / 10.0, 7, 2, 3);
	QByteArray payOffSubjectType = getBCD(aUnitData.payOffSubjectType, 1);
	QByteArray payOffSubjectMethodType = getBCD(CFR::PayOffSubjectMethodType, 1);

	QByteArray commandData;
	commandData.append(CAtolOnlineFR::SaleFlags);               // флаги
	commandData.append(sum);                                    // сумма
	commandData.append(getBCD(10, 5, 2));                       // количество = 1 штука
	commandData.append(sum);                                    // стоимость
	commandData.append(uchar(taxGroup));                        // налог (тип)
	commandData.append(getBCD(0, 7, 2, 3));                     // налог (сумма) - ФР считает налог сам
	commandData.append(uchar(section));                         // секция
	commandData.append(payOffSubjectType);                      // признак предмета расчета
	commandData.append(payOffSubjectMethodType);                // признак способа расчета
	commandData.append(CAtolOnlineFR::DiscountSign);            // знак скидки
	commandData.append(getBCD(0, 7, 2, 3));                     // информация о скидке
	commandData.append(QByteArray(2, ASCII::NUL));              // зарезервировано
	commandData.append(mCodec->fromUnicode(aUnitData.name));    // товар

	return processCommand(CAtolOnlineFR::Commands::EndSale, commandData);
}

//--------------------------------------------------------------------------------
template<class T>
bool AtolOnlineFRBase<T>::setTLV(int aField, bool /*aForSale*/)
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
template<class T>
bool AtolOnlineFRBase<T>::getTLV(int aField, QByteArray & aData, uchar aBlockNumber)
{
	aData.clear();

	QByteArray commandData;
	commandData.append(char(aField >> 0));
	commandData.append(char(aField >> 8));
	commandData.append(aBlockNumber);

	QByteArray data;
	CFR::STLV TLV;
	QString errorLog = mDeviceName + ": Failed to get " + mFFData.getTextLog(aField);

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
template<class T>
void AtolOnlineFRBase<T>::setErrorFlags(const QByteArray & aCommand, char aError)
{
	T::setErrorFlags(aCommand, aError);

	if (mErrorData->value(aError).type == FRError::EType::FS)
	{
		mFSError = true;
	}
}

//--------------------------------------------------------------------------------
