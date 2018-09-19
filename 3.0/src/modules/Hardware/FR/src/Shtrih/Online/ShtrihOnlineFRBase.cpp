/* @file Онлайн ФР семейства Штрих. */

#include "ShtrihOnlineFRBase.h"

//--------------------------------------------------------------------------------
template class ShtrihOnlineFRBase<ShtrihTCPFRBase>;
template class ShtrihOnlineFRBase<ShtrihSerialFRBase>;

using namespace SDK::Driver;
using namespace ProtocolUtils;

//--------------------------------------------------------------------------------
template<class T>
ShtrihOnlineFRBase<T>::ShtrihOnlineFRBase()
{
	// данные семейства ФР
	mSupportedModels = getModelList();
	mDeviceName = CShtrihFR::Models::OnlineDefault;
	setConfigParameter(CHardwareSDK::CanOnline, true);
	mNotEnableFirmwareUpdating = false;
	mPrinterStatusEnabled = true;
	mIsOnline = true;
	mOFDFiscalParameters.remove(CFR::FiscalFields::Cashier);
	mOFDFiscalParameters.remove(CFR::FiscalFields::TaxSystem);

	setConfigParameter(CHardwareSDK::FR::CanWithoutPrinting, true);

	// типы оплаты
	mPayTypeData.add(EPayTypes::Cash,          1);
	mPayTypeData.add(EPayTypes::EMoney,        2);
	mPayTypeData.add(EPayTypes::PrePayment,   14);
	mPayTypeData.add(EPayTypes::PostPayment,  15);
	mPayTypeData.add(EPayTypes::CounterOffer, 16);

	// данные команд
	mCommandData.add(CShtrihOnlineFR::Commands::Reboot, CShtrihFR::Timeouts::Default, false);

	// ошибки
	mErrorData = PErrorData(new CShtrihOnlineFR::Errors::CData);
}

//--------------------------------------------------------------------------------
template<class T>
QStringList ShtrihOnlineFRBase<T>::getModelList()
{
	return CShtrihFR::Models::CData().getNonEjectorModels(true);
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihOnlineFRBase<T>::setNotPrintDocument(bool aEnabled, bool /*aZReport*/)
{
	return !aEnabled || setFRParameter(CShtrihOnlineFR::FRParameters::NotPrintDocument, true);
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihOnlineFRBase<T>::updateParameters()
{
	if (!ShtrihFRBase<T>::updateParameters())
	{
		return false;
	}

	mNotEnableFirmwareUpdating = containsDeviceParameter(CDeviceData::SDCard) && !enableFirmwareUpdating();

	QByteArray addressData;
	QByteArray portData;
	getFRParameter(CShtrihOnlineFR::FRParameters::OFDAddress, addressData);
	getFRParameter(CShtrihOnlineFR::FRParameters::OFDPort, portData);
	mOFDDataError = !checkOFDData(addressData, revert(portData));

	// Формировать QR-код средствами ФР
	setFRParameter(CShtrihOnlineFR::FRParameters::QRCode, true);

	// Печатать сквозной номер документа
	setFRParameter(CShtrihOnlineFR::FRParameters::PrintEndToEndNumber, true);

	// Печатать данные ОФД в чеках
	setFRParameter(CShtrihOnlineFR::FRParameters::PrintOFDData, true);

	// Печатать все реквизиты пользователя в чеках
	setFRParameter(CShtrihOnlineFR::FRParameters::PrintUserData, CShtrihOnlineFR::PrintFullUserData);

	// Печатать фискальные теги, вводимые на платеже
	setFRParameter(CShtrihOnlineFR::FRParameters::PrintCustomFields, true);

	if (!isFiscal())
	{
		return true;
	}

	if (mOperatorPresence && !setCashier())
	{
		QByteArray data;

		if (!getFRParameter(CShtrihOnlineFR::FRParameters::Cashier, data, CShtrihOnlineFR::CashierSeries) || data.simplified().isEmpty())
		{
			toLog(LogLevel::Error, mDeviceName + ": Cannot work with invalid cashier");
			return false;
		}
	}

	QByteArray data;

	if (!processCommand(CShtrihOnlineFR::Commands::FS::GetFiscalizationTotal, &data) || (data.size() <= 46))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to get fiscalization total");
		return false;
	}

	if (!checkTaxSystems(data[40]) || !checkOperationModes(data[41]))
	{
		return false;
	}

	bool result = true;
	bool FSError = mFSError;
	bool needWaitReady = getLongStatus() && (mMode == CShtrihFR::InnerModes::SessionClosed) || (mMode == CShtrihFR::InnerModes::DataEjecting);

	TResult commandResult = processCommand(CShtrihOnlineFR::Commands::FS::StartFiscalTLVData, data.mid(43, 4));

	if (commandResult)
	{
		if (needWaitReady && !waitReady(CShtrihOnlineFR::ReadyWaiting))
		{
			return false;
		}

		CFR::STLV TLV;

		while (processCommand(CShtrihOnlineFR::Commands::FS::GetFiscalTLVData, &data))
		{
			if (mFFEngine.parseTLV(data.mid(3), TLV))
			{
				if (TLV.field == CFR::FiscalFields::AgentFlagsReg)
				{
					result = checkAgentFlags(TLV.data[0]);
				}
			}
		}
	}
	else if ((commandResult == CommandResult::Device) && (mLastError == CShtrihOnlineFR::Errors::NoRequiedDataInFS))
	{
		mFSError = FSError;
		mProcessingErrors.pop_back();
	}

	return result;
}

//---------------------------------------------------------------------------
template <class T>
bool ShtrihOnlineFRBase<T>::setCashier()
{
	if (!mFFEngine.containsConfigParameter(CFiscalSDK::Cashier))
	{
		toLog(LogLevel::Warning, mDeviceName + ": Failed to set cashier due to it is absent");
		return false;
	}

	QString cashier = mFFEngine.getConfigParameter(CFiscalSDK::Cashier).toString().simplified();

	if (cashier.isEmpty())
	{
		toLog(LogLevel::Warning, mDeviceName + ": Failed to set cashier due to it is empty");
		return false;
	}

	if (!setFRParameter(CShtrihOnlineFR::FRParameters::Cashier, cashier, CShtrihOnlineFR::CashierSeries))
	{
		toLog(LogLevel::Warning, mDeviceName + QString(": Failed to set cashier fiscal field (%2)").arg(cashier));
		return false;
	}

	return true;
}

//---------------------------------------------------------------------------
template <class T>
int ShtrihOnlineFRBase<T>::getSessionNumber()
{
	QByteArray data;

	if (processCommand(CShtrihOnlineFR::Commands::FS::GetSessionParameters, &data) && (data.size() > 5))
	{
		return int(uchar(data[4])) | (int(uchar(data[5])) << 8);
	}

	return 0;
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihOnlineFRBase<T>::setTaxValue(TVAT /*aVAT*/, int /*aGroup*/)
{
	return false;
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihOnlineFRBase<T>::getPrinterStatus(TStatusCodes & aStatusCodes)
{
	QByteArray answer;
	TResult result = processCommand(CShtrihOnlineFR::Commands::GetPrinterStatus, &answer);

	if (result == CommandResult::Device)
	{
		aStatusCodes.insert(PrinterStatusCode::Error::PrinterFR);

		return true;
	}

	return result;
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihOnlineFRBase<T>::getStatus(TStatusCodes & aStatusCodes)
{
	if (!ShtrihFRBase<T>::getStatus(aStatusCodes) || (mPrinterStatusEnabled && !getPrinterStatus(aStatusCodes)))
	{
		return false;
	}

	if (mNotEnableFirmwareUpdating)
	{
		aStatusCodes.insert(FRStatusCode::Warning::FirmwareUpdating);
	}

	QByteArray data;

	if (!processCommand(CShtrihOnlineFR::Commands::FS::GetOFDInterchangeStatus, &data))
	{
		return false;
	}

	int OFDNotSentCount = (data.size() < 7) ? -1 : revert(data.mid(5, 2)).toHex().toUShort(0, 16);
	checkOFDNotSentCount(OFDNotSentCount, aStatusCodes);

	if (!processCommand(CShtrihOnlineFR::Commands::FS::GetStatus, &data))
	{
		return false;
	}

	if (data.size() < 7)
	{
		aStatusCodes.insert(DeviceStatusCode::Error::Unknown);
	}
	else
	{
		checkFSFlags(data[7], aStatusCodes);
	}

	return true;
}

//--------------------------------------------------------------------------------
template<class T>
void ShtrihOnlineFRBase<T>::processDeviceData()
{
	ShtrihFRBase<T>::processDeviceData();

	QByteArray data;

	if (processCommand(CShtrihOnlineFR::Commands::FS::GetStatus, &data))
	{
		mFSSerialNumber = CFR::FSSerialToString(data.mid(13, 16));
		int FDCount = revert(data.right(4)).toHex().toUInt(0, 16);

		setDeviceParameter(CDeviceData::FR::FiscalDocuments, FDCount);
	}

	if (processCommand(CShtrihOnlineFR::Commands::FS::GetValidity, &data))
	{
		QDate date = QDate::fromString(hexToBCD(data.mid(3, 3)).prepend("20"), CShtrihOnlineFR::DateFormat);

		if (date.isValid())
		{
			setDeviceParameter(CDeviceData::FS::ValidityData, date.toString(CFR::DateLogFormat));
		}

		// признак фискализированности ККМ
		mFiscalized = data[7];
	}

	if (processCommand(CShtrihOnlineFR::Commands::FS::GetVersion, &data) && (data.size() > 19))
	{
		setDeviceParameter(CDeviceData::FS::Version, QString("%1, type %2").arg(clean(data.mid(3, 16)).data()).arg(data[19] ? "serial" : "debug"));
	}

	using namespace CShtrihOnlineFR::FRParameters;

	if (getFRParameter(FFDFR, data))
	{
		mFFDFR = EFFD::Enum(int(data[0]));
		mFFDFS = mFFDFR;
	}

	if (getFRParameter(SerialNumber, data))
	{
		mSerial = CFR::serialToString(data);
	}

	if (getFRParameter(INN, data)) mINN = CFR::INNToString(data);
	if (getFRParameter(RNM, data)) mRNM = CFR::RNMToString(data);

	#define SET_LCONFIG_FISCAL_FIELD(aName) QString aName##Log = mFFData.getTextLog(CFR::FiscalFields::aName); \
		if (getFRParameter(aName, data)) { mFFEngine.setLConfigParameter(CFiscalSDK::aName, data); \
		     QString value = mFFEngine.getConfigParameter(CFiscalSDK::aName, data).toString(); \
		     toLog(LogLevel::Normal, mDeviceName + QString(": Add %1 = \"%2\" to config data").arg(aName##Log).arg(value)); } \
		else toLog(LogLevel::Error,  mDeviceName + QString(": Failed to add %1 to config data").arg(aName##Log));

	#define SET_BCONFIG_FISCAL_FIELD(aName) if (value & CShtrihOnlineFR::OperationModeMask::aName) { mFFEngine.setConfigParameter(CFiscalSDK::aName, 1); \
		toLog(LogLevel::Normal, mDeviceName + QString(": Add %1 = 1 to config data").arg(mFFData.getTextLog(CFR::FiscalFields::aName))); }

	SET_LCONFIG_FISCAL_FIELD(FTSURL);
	SET_LCONFIG_FISCAL_FIELD(OFDURL);
	SET_LCONFIG_FISCAL_FIELD(OFDName);
	SET_LCONFIG_FISCAL_FIELD(LegalOwner);
	SET_LCONFIG_FISCAL_FIELD(PayOffAddress);
	SET_LCONFIG_FISCAL_FIELD(PayOffPlace);

	if (getFRParameter(OperationModes, data) && !data.isEmpty())
	{
		char value = data[0];

		SET_BCONFIG_FISCAL_FIELD(LotteryMode);
		SET_BCONFIG_FISCAL_FIELD(GamblingMode);
		SET_BCONFIG_FISCAL_FIELD(ExcisableUnitMode);
		SET_BCONFIG_FISCAL_FIELD(InAutomateMode);
	}

	if (mFFEngine.getConfigParameter(CFiscalSDK::InAutomateMode).toInt() && mOperatorPresence)
	{
		mWrongFiscalizationSettings = true;
		toLog(LogLevel::Error, mDeviceName + ": There is \"In automate mode\" flag and operator is present.");
	}

	removeDeviceParameter(CDeviceData::SDCard);

	if ((mModel != CShtrihFR::Models::ID::MStarTK2) && getFRParameter(SD::Status, data))
	{
		if (data[0] == CShtrihOnlineFR::SDNotConnected)
		{
			setDeviceParameter(CDeviceData::SDCard, "not connected");
		}
		else if (data[0])
		{
			setDeviceParameter(CDeviceData::Error, uchar(data[0]), CDeviceData::SDCard);
		}
		else
		{
			auto getSDData = [&] (const CShtrihFR::FRParameters::SData & aData) -> uint { if (!getFRParameter(aData, data)) return 0;
				return revert(data).toHex().toUInt(0, 16); };

			QString SDCardData = QString("cluster %1 KB, space %2 MB (%3 MB free), io errors %4, retry count %5")
				.arg(getSDData(SD::ClusterSize) / 1024)
				.arg(getSDData(SD::TotalSize)   / CShtrihOnlineFR::SectorsInMB)
				.arg(getSDData(SD::FreeSize)    / CShtrihOnlineFR::SectorsInMB)
				.arg(getSDData(SD::IOErrors))
				.arg(getSDData(SD::RetryCount));

			setDeviceParameter(CDeviceData::SDCard, SDCardData);
		}
	}

	mCanProcessZBuffer = mModelData.ZBufferSize && containsDeviceParameter(CDeviceData::SDCard);

	checkDateTime();
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihOnlineFRBase<T>::checkFirmwareUpdatingData(const CShtrihFR::FRParameters::SData & aData, int aValue, const QString & aLogData, bool & aNeedReboot)
{
	QByteArray data;

	if (!getFRParameter(aData, data))
	{
		toLog(LogLevel::Error, QString("%1: Failed to get %2 updating data").arg(mDeviceName).arg(aLogData));
		return false;
	}

	int value = revert(data).toHex().toUShort(0, 16);

	if (value != aValue)
	{
		toLog(LogLevel::Normal, QString("%1: Need reboot due to %2 updating data").arg(mDeviceName).arg(aLogData));
		aNeedReboot = true;

		if (!setFRParameter(aData, aValue))
		{
			toLog(LogLevel::Error, QString("%1: Failed to set %2 updating data").arg(mDeviceName).arg(aLogData));
			return false;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihOnlineFRBase<T>::enableFirmwareUpdating()
{
	using namespace CShtrihOnlineFR::FRParameters;
	using namespace CShtrihOnlineFR::FirmwareUpdating;

	bool needReboot = false;

	if (!checkFirmwareUpdatingData(FirmwareUpdating::Working,  Working,  "working",  needReboot) ||
	    !checkFirmwareUpdatingData(FirmwareUpdating::Interval, Interval, "interval", needReboot) ||
	    !checkFirmwareUpdatingData(FirmwareUpdating::Enabling, Enabling, "enabling", needReboot) ||
	    !checkFirmwareUpdatingData(FirmwareUpdating::Single,   Single,   "single",   needReboot))
	{
		return false;
	}

	return true;
	// до выяснения причин невозможности установки параметров обновления прошивки
	/*
	mNeedReboot = needReboot && (mModel == CShtrihFR::Models::ID::PayVKP80KFA);

	return !needReboot || mNeedReboot || reboot();
	*/
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihOnlineFRBase<T>::reboot()
{
	QVariantMap portConfig = mIOPort->getDeviceConfiguration();
	TResult result = processCommand(CShtrihOnlineFR::Commands::Reboot, CShtrihOnlineFR::Reboot);

	CommandResult::TResults errors = CommandResult::TResults()
		<< CommandResult::Port
		<< CommandResult::Transport
		<< CommandResult::Protocol
		<< CommandResult::Driver
		<< CommandResult::Device;

	if (!errors.contains(result))
	{
		mIOPort->close();
		SleepHelper::msleep(CShtrihOnlineFR::RebootPause);

		if (mIOPort->getType() == EPortTypes::TCP)
		{
			portConfig.insert(CHardware::Port::OpeningTimeout, CShtrihOnlineFR::TCPReopeningTimeout);
			mIOPort->setDeviceConfiguration(portConfig);
		}

		result = mIOPort->open();

		mIOPort->setDeviceConfiguration(portConfig);
	}

	return result;
}

//--------------------------------------------------------------------------------
template<class T>
void ShtrihOnlineFRBase<T>::setErrorFlags(const QByteArray & aCommand)
{
	bool noError = (aCommand == CShtrihOnlineFR::Commands::FS::GetFiscalTLVData) && (mLastError == CShtrihOnlineFR::Errors::NoRequiedDataInFS);

	if (!noError && (mErrorData->value(mLastError).type == FRError::EType::FS))
	{
		mFSError = true;
	}
}

//--------------------------------------------------------------------------------
template<class T>
void ShtrihOnlineFRBase<T>::checkSalesName(QString & aName)
{
	aName = aName.leftJustified(CShtrihFR::FixedStringSize, QChar(ASCII::Space), false);
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihOnlineFRBase<T>::sale(const SUnitData & aUnitData, bool aBack)
{
	if (mModelData.date < CShtrihOnlineFR::MinFWDate::V2)
	{
		return ShtrihFRBase<T>::sale(aUnitData, aBack);
	}

	char documentType = aBack ? CShtrihOnlineFR::DocumentTypes::SaleBack : CShtrihOnlineFR::DocumentTypes::Sale;
	char taxIndex = char(mTaxData[aUnitData.VAT].group);
	char section = (aUnitData.section == -1) ? CShtrihFR::SectionNumber : char(aUnitData.section);
	QByteArray sum = getHexReverted(aUnitData.sum, 5, 2);
	char payOffSubjectType = char(aUnitData.payOffSubjectType);
	QString name = aUnitData.name;

	QByteArray commandData;
	commandData.append(documentType);                      // тип операции
	commandData.append(getHexReverted(1, 6, 6));           // количество
	commandData.append(sum);                               // цена
	commandData.append(sum);                               // сумма операций
	commandData.append(CShtrihOnlineFR::FiscalTaxData);    // налог
	commandData.append(taxIndex);                          // налоговая ставка
	commandData.append(section);                           // отдел
	commandData.append(CFR::PayOffSubjectMethodType);      // признак способа расчета
	commandData.append(payOffSubjectType);                 // признак предмета расчета
	commandData.append(mCodec->fromUnicode(name));         // наименование товара

	if (!processCommand(CShtrihOnlineFR::Commands::FS::Sale, commandData))
	{
		toLog(LogLevel::Error, QString("%1: Failed to sale for %2 (%3, VAT = %4), feed, cut and exit").arg(mDeviceName).arg(aUnitData.sum, 0, 'f', 2).arg(name).arg(aUnitData.VAT));
		return false;
	}

	return setOFDParametersOnSale(aUnitData);
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihOnlineFRBase<T>::closeDocument(double aSum, EPayTypes::Enum aPayType)
{
	if (mModelData.date < CShtrihOnlineFR::MinFWDate::V2)
	{
		return ShtrihFRBase<T>::closeDocument(aSum, aPayType);
	}

	QByteArray commandData;

	for (int i = 1; i <= CShtrihOnlineFR::PayTypeQuantity; ++i)
	{
		double sum = (i == mPayTypeData[aPayType].value) ? aSum : 0;
		commandData.append(getHexReverted(sum, 5, 2));    // сумма
	}

	char taxSystem = char(mFFEngine.getConfigParameter(CFiscalSDK::TaxSystem).toInt());

	commandData.append(ASCII::NUL);                             // Округление до рубля
	commandData.append(CShtrihOnlineFR::ClosingFiscalTaxes);    // налоги
	commandData.append(taxSystem);                              // СНО

	if (!processCommand(CShtrihOnlineFR::Commands::FS::CloseDocument, commandData))
	{
		toLog(LogLevel::Error, "ShtrihFR: Failed to close document, feed, cut and exit");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihOnlineFRBase<T>::performFiscal(const QStringList & aReceipt, const SPaymentData & aPaymentData, TFiscalPaymentData & aFPData, TComplexFiscalPaymentData & aPSData)
{
	if ((mModelData.date < CShtrihOnlineFR::MinFWDate::V2) || (mModel == CShtrihFR::Models::ID::MStarTK2))
	{
		char taxSystem = char(aPaymentData.taxSystem);

		if ((taxSystem != ETaxSystems::None) && (mTaxSystems.size() != 1) && !setFRParameter(CShtrihOnlineFR::FRParameters::TaxSystem, taxSystem))
		{
			toLog(LogLevel::Error, QString("%1: Failed to set taxation system %2 (%3)").arg(mDeviceName).arg(toHexLog(taxSystem)).arg(CFR::TaxSystems[taxSystem]));
			return false;
		}
	}

	bool result = ShtrihFRBase<T>::performFiscal(aReceipt, aPaymentData, aFPData, aPSData);

	if (result)
	{
		QByteArray data;

		if (processCommand(CShtrihOnlineFR::Commands::FS::GetStatus, &data) && (data.size() >= 33))
		{
			uint FDNumber = revert(data.mid(29, 4)).toHex().toUInt(0, 16);
			mFFEngine.checkFPData(aFPData, CFR::FiscalFields::FDNumber, FDNumber);

			if (processCommand(CShtrihOnlineFR::Commands::FS::StartFiscalTLVData, getHexReverted(FDNumber, 4)))
			{
				CFR::STLV TLV;

				while (processCommand(CShtrihOnlineFR::Commands::FS::GetFiscalTLVData, &data))
				{
					if (mFFEngine.parseTLV(data.mid(3), TLV))
					{
						if (mFFData.data().contains(TLV.field))
						{
							mFFEngine.parseSTLVData(TLV, aPSData);
							mFFEngine.parseTLVData (TLV, aFPData);
						}
						else
						{
							toLog(LogLevel::Warning, QString("%1: Failed to add fiscal field %2 due to it is unknown").arg(mDeviceName).arg(TLV.field));
						}
					}
				}
			}
		}

		CShtrihFR::TRegisterId registerId = aPaymentData.back ? CShtrihFR::Registers::SalesBackCount : CShtrihFR::Registers::SalesCount;

		if (getRegister(registerId, data))
		{
			uint documentNumber = revert(data).toHex().toUInt(0, 16);
			mFFEngine.checkFPData(aFPData, CFR::FiscalFields::DocumentNumber, documentNumber);
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihOnlineFRBase<T>::cancelFiscal()
{
	return ShtrihFRBase<T>::cancelFiscal() && receiptProcessing();
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihOnlineFRBase<T>::canForceStatusBufferEnable()
{
	return (mModel == CShtrihFR::Models::ID::ShtrihM01F)    ||
	       (mModel == CShtrihFR::Models::ID::ShtrihM02F)    ||
	       (mModel == CShtrihFR::Models::ID::ShtrihMini01F) ||
	       (mModel == CShtrihFR::Models::ID::ShtrihLight02F);
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihOnlineFRBase<T>::execZReport(bool aAuto)
{
	QVariantMap outData;

	if (!prepareZReport(aAuto, outData))
	{
		return false;
	}

	bool result = checkNotPrinting(aAuto, true);

	if (!result && aAuto)
	{
		mNeedCloseSession = mNeedCloseSession || (mMode == CShtrihFR::InnerModes::NeedCloseSession);

		return false;
	}

	if (mOperatorPresence)
	{
		processCommand(CShtrihFR::Commands::SectionReport);
		processCommand(CShtrihFR::Commands::TaxReport);
		processCommand(CShtrihFR::Commands::CashierReport);
	}

	mNeedCloseSession = false;
	result = processCommand(CShtrihFR::Commands::ZReport);

	if (result)
	{
		mZBufferOverflow = false;
		SleepHelper::msleep(CShtrihFR::Pause::ZReportPrintingEnd);
		result = waitForChangeZReportMode();
	}

	if (getLongStatus())
	{
		mNeedCloseSession = mMode == CShtrihFR::InnerModes::NeedCloseSession;
	}

	if (result)
	{
		emit FRSessionClosed(outData);
	}

	toLog(result ? LogLevel::Normal : LogLevel::Error, result ?
		"ShtrihFR: Z-report is successfully processed" :
		"ShtrihFR: error in processing Z-report");

	checkNotPrinting(false, true);

	return result;
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihOnlineFRBase<T>::processAnswer(const QByteArray & aCommand)
{
	switch (mLastError)
	{
		case CShtrihOnlineFR::Errors::FSOfflineEnd:
		{
			mProcessingErrors.push_back(mLastError);

			mFSOfflineEnd = true;

			break;
		}
		case CShtrihOnlineFR::Errors::NeedZReport:
		{
			mProcessingErrors.push_back(mLastError);

			return execZReport(true);
		}
		case CShtrihOnlineFR::Errors::WrongFSState:
		{
			mProcessingErrors.push_back(mLastError);

			if (aCommand[0] != CShtrihFR::Commands::OpenFRSession)
			{
				getLongStatus();

				return (mMode == CShtrihFR::InnerModes::SessionClosed) && openFRSession();
			}

			break;
		}
	}

	bool result = ShtrihFRBase<T>::processAnswer(aCommand);

	if (!mProcessingErrors.isEmpty() && (mProcessingErrors.last() == CShtrihFR::Errors::BadModeForCommand))
	{
		bool needCleanErrors = false;
		QByteArray data;

		if (getLongStatus() && (mMode == CShtrihFR::InnerModes::DataEjecting))
		{
			while (processCommand(CShtrihOnlineFR::Commands::FS::GetFiscalTLVData, &data))
			{
			}

			result = mProcessingErrors.last() == CShtrihFR::Errors::BadModeForCommand;
			needCleanErrors = true;
		}

		if (needCleanErrors || (aCommand == CShtrihOnlineFR::Commands::FS::GetFiscalTLVData))
		{
			mProcessingErrors.pop_back();
			mLastError = 0;
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihOnlineFRBase<T>::prepareFiscal()
{
	if (!ShtrihFRBase<T>::prepareFiscal() || !getLongStatus())
	{
		return false;
	}

	if (mMode == CShtrihFR::InnerModes::SessionClosed)
	{
		return openFRSession();
	}

	return true;
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihOnlineFRBase<T>::setTLV(int aField, bool aForSale)
{
	bool result;

	if (!mFFEngine.checkFiscalField(aField, result))
	{
		return result;
	}

	QString fieldLog;
	QByteArray commandData = mFFEngine.getTLVData(aField, &fieldLog);
	QByteArray command = aForSale ? CShtrihOnlineFR::Commands::FS::SetOFDParameterLinked : CShtrihOnlineFR::Commands::FS::SetOFDParameter;

	if (!processCommand(command, commandData))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to set " + fieldLog);
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihOnlineFRBase<T>::openSession()
{
	TStatusCodes statusCodes;

	if (!ProtoShtrihFR<T>::getStatus(statusCodes) || statusCodes.contains(PrinterStatusCode::OK::PaperInPresenter))
	{
		waitForPrintingEnd();
		SleepHelper::msleep(CShtrihFR::Pause::Cutting);
	}

	checkNotPrinting(true);

	bool result = processCommand(CShtrihFR::Commands::OpenFRSession);
	waitForPrintingEnd();

	checkNotPrinting();

	return result;
}

//--------------------------------------------------------------------------------
