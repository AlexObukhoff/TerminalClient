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
	mOFDFiscalParameters -= FiscalFields::Cashier;
	mNotEnableFirmwareUpdating = false;
	mPrinterStatusEnabled = true;
	mIsOnline = true;

	// типы оплаты
	mPayTypeData.add(EPayTypes::Cash,          1);
	mPayTypeData.add(EPayTypes::EMoney,        2);
	mPayTypeData.add(EPayTypes::PostPayment,  14);
	mPayTypeData.add(EPayTypes::Credit,       15);
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

	if (mOperatorPresence)
	{
		if (!containsConfigParameter(CHardware::FiscalFields::Cashier))
		{
			toLog(LogLevel::Error, mDeviceName + ": Failed to set cashier fiscal field due to it is absent");
			return false;
		}

		QString cashier = getConfigParameter(CHardware::FiscalFields::Cashier).toString();

		if (!setFRParameter(CShtrihOnlineFR::FRParameters::Cashier, cashier, CShtrihOnlineFR::FRParameters::CashierSeries))
		{
			toLog(LogLevel::Error, QString("%1: Failed to set cashier fiscal field (%2)").arg(mDeviceName).arg(cashier));
			return false;
		}
	}

	QByteArray data;

	if (!processCommand(CShtrihOnlineFR::Commands::FS::GetFiscalizationResume, &data) || (data.size() <= 41))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to get fiscalization resume");
		return false;
	}

	return checkTaxationData(data[40]) && checkOperationModes(data[41]);
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
		int FDCount = ProtocolUtils::revert(data.right(4)).toHex().toUInt(0, 16);

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
		setDeviceParameter(CDeviceData::FS::Type, data[19] ? "serial" : "debug");
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

	#define SET_LCONFIG_FISCAL_FIELD(aName) QString aName##Log = QString("fiscal tag %1 (%2)").arg(FiscalFields::aName).arg(CHardware::FiscalFields::aName); \
		if (getFRParameter(aName, data)) { setLConfigParameter(CHardware::FiscalFields::aName, data); \
		     QString value = getConfigParameter(CHardware::FiscalFields::aName, data).toString(); \
		     toLog(LogLevel::Normal, QString("%1: Add %2 = \"%3\" to config data").arg(mDeviceName).arg(aName##Log).arg(value)); } \
		else toLog(LogLevel::Error, QString("%1: Failed to add %2 to config data").arg(mDeviceName).arg(aName##Log));

	SET_LCONFIG_FISCAL_FIELD(FTSURL);
	SET_LCONFIG_FISCAL_FIELD(OFDURL);
	SET_LCONFIG_FISCAL_FIELD(OFDName);
	SET_LCONFIG_FISCAL_FIELD(LegalOwner);
	SET_LCONFIG_FISCAL_FIELD(PayOffAddress);
	SET_LCONFIG_FISCAL_FIELD(PayOffPlace);

	if (getFRParameter(SD::Status, data))
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
				return ProtocolUtils::revert(data).toHex().toUInt(0, 16); };

			QString SDCardData = QString("cluster %1 KB, space %2 MB (%3 MB free), io errors %4, retry count %5")
				.arg(getSDData(SD::ClusterSize) / 1024)
				.arg(getSDData(SD::TotalSize)   / CShtrihOnlineFR::SectorsInMB)
				.arg(getSDData(SD::FreeSize)    / CShtrihOnlineFR::SectorsInMB)
				.arg(getSDData(SD::IOErrors))
				.arg(getSDData(SD::RetryCount));

			setDeviceParameter(CDeviceData::SDCard, SDCardData);
		}
	}

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

	int value = ProtocolUtils::revert(data).toHex().toUShort(0, 16);

	if (value != aValue)
	{
		toLog(LogLevel::Normal, QString("%1: Need reboot due to %2 updating data").arg(mDeviceName).arg(aLogData));
		aNeedReboot = true;

		if (!setFRParameter(aData, value))
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
		!checkFirmwareUpdatingData(FirmwareUpdating::Enabling, Enabling, "enabling", needReboot) ||
		!checkFirmwareUpdatingData(FirmwareUpdating::Interval, Interval, "interval", needReboot) ||
		!checkFirmwareUpdatingData(FirmwareUpdating::Single,   Single,   "single",   needReboot))
	{
		return false;
	}

	return true;
	// до выяснения причин невозможности установки параметров обновления прошивки
	/*
	mNeedReboot = needReboot && (mModel == CShtrihFR::Models::ID::PayVKP80KFA);

	if (!needReboot || mNeedReboot)
	{
		return true;
	}

	QVariantMap portConfig = mIOPort->getDeviceConfiguration();
	TResult result = processCommand(CShtrihOnlineFR::Commands::Reboot, CShtrihOnlineFR::Reboot);

	if (result != CommandResult::Device)
	{
		mIOPort->close();
		SleepHelper::msleep(CShtrihOnlineFR::RebootPause);

		if (mIOPort->getType() == EPortTypes::TCP)
		{
			portConfig.insert(CHardware::Port::OpeningTimeout, CShtrihOnlineFR::TCPReopeningTimeout);
			mIOPort->setDeviceConfiguration(portConfig);
		}

		mIOPort->open();

		mIOPort->setDeviceConfiguration(portConfig);
	}

	return result;
	*/
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
bool ShtrihOnlineFRBase<T>::sale(const SAmountData & aAmountData, bool aBack)
{
	if (mOldFirmware)
	{
		return ShtrihFRBase<T>::sale(aAmountData, aBack);
	}

	char documentType = aBack ? CShtrihFR::DocumentTypes::SaleBack : CShtrihFR::DocumentTypes::Sale;
	char taxIndex = char(mTaxData[aAmountData.VAT].group);
	char section = (aAmountData.section == -1) ? CShtrihFR::SectionNumber : char(aAmountData.section);
	QByteArray sum = getHexReverted(aAmountData.sum, 5, 2);
	char payOffSubjectType = char(aAmountData.payOffSubjectType);
	QString name = aAmountData.name;

	QByteArray commandData;
	commandData.append(documentType);                         // тип операции
	commandData.append(getHexReverted(1, 6, 6));              // количество
	commandData.append(sum);                                  // цена
	commandData.append(sum);                                  // сумма операций
	commandData.append(CShtrihOnlineFR::FiscalTaxData);       // налог
	commandData.append(taxIndex);                             // налоговая ставка
	commandData.append(section);                              // отдел
	commandData.append(EPayOffMethodTypes::Prepayment100);    // признак способа расчета
	commandData.append(payOffSubjectType);                    // признак предмета расчета
	commandData.append(mCodec->fromUnicode(name));            // наименование товара

	if (!processCommand(CShtrihOnlineFR::Commands::FS::Sale, commandData))
	{
		toLog(LogLevel::Error, QString("%1: Failed to sale for %2 (%3, VAT = %4), feed, cut and exit").arg(mDeviceName).arg(aAmountData.sum, 0, 'f', 2).arg(name).arg(aAmountData.VAT));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihOnlineFRBase<T>::performFiscal(const QStringList & aReceipt, const SPaymentData & aPaymentData, TFiscalPaymentData & aFPData, TComplexFiscalPaymentData & aPSData)
{
	char taxation = char(aPaymentData.taxation);

	if ((taxation != ETaxations::None) && (mTaxations.size() != 1) && !setFRParameter(CShtrihOnlineFR::FRParameters::Taxation, taxation))
	{
		toLog(LogLevel::Error, QString("%1: Failed to set taxation system %2 (%3)").arg(mDeviceName).arg(ProtocolUtils::toHexLog(taxation)).arg(CFR::Taxations[taxation]));
		return false;
	}

	bool result = ShtrihFRBase<T>::performFiscal(aReceipt, aPaymentData, aFPData, aPSData);

	if (result)
	{
		QByteArray data;

		if (processCommand(CShtrihOnlineFR::Commands::FS::GetStatus, &data) && (data.size() >= 33))
		{
			uint FDNumber = ProtocolUtils::revert(data.mid(29, 4)).toHex().toUInt(0, 16);
			aFPData.insert(FiscalFields::FDNumber, FDNumber);

			if (processCommand(CShtrihOnlineFR::Commands::FS::StartFiscalTLVData, getHexReverted(FDNumber, 4)))
			{
				CFR::STLV TLV;

				while (processCommand(CShtrihOnlineFR::Commands::FS::GetFiscalTLVData, &data))
				{
					if (parseTLV(data.mid(3), TLV))
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

		CShtrihFR::TRegisterId registerId = aPaymentData.back ? CShtrihFR::Registers::SalesBackCount : CShtrihFR::Registers::SalesCount;

		if (getRegister(registerId, data))
		{
			aFPData.insert(FiscalFields::DocumentNumber, revert(data).toHex().toUInt(0, 16));
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
	bool needCloseSession = mMode == CShtrihFR::InnerModes::NeedCloseSession;

	if (aAuto && mOperatorPresence)
	{
		toLog(LogLevel::Error, "ShtrihFR: Failed to process auto-Z-report due to presence of the operator.");
		mNeedCloseSession = mNeedCloseSession || needCloseSession;

		return false;
	}

	toLog(LogLevel::Normal, QString("ShtrihFR: Begin processing %1Z-report").arg(aAuto ? "auto-" : ""));

	// проверяем, нормальный ли режим, делаем запрос статуса
	QByteArray answer;

	if (!getLongStatus(answer))
	{
		toLog(LogLevel::Error, QString("ShtrihFR: Failed to get status therefore failed to process %1Z-report.").arg(aAuto ? "auto-" : ""));
		mNeedCloseSession = mNeedCloseSession || needCloseSession;

		return false;
	}

	QVariantMap outData = getSessionOutData(answer);
	bool result = setFRParameter(CShtrihOnlineFR::FRParameters::NotPrintDocument, aAuto);

	if (!result)
	{
		if (aAuto)
		{
			toLog(LogLevel::Error, "ShtrihFR: Failed to disable printing next document.");
			mNeedCloseSession = mNeedCloseSession || needCloseSession;

			return false;
		}

		toLog(LogLevel::Warning, "ShtrihFR: Failed to enable printing next document.");
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

	return result;
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihOnlineFRBase<T>::processAnswer(const QByteArray & aCommand)
{
	switch (mLastError)
	{
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

				return (mMode == CShtrihFR::InnerModes::SessionClosed) && openSession();
			}
		}
	}

	return ShtrihFRBase<T>::processAnswer(aCommand);
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
		return openSession();
	}

	return true;
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihOnlineFRBase<T>::setTLV(int aField)
{
	bool result;

	if (!checkFiscalField(aField, result))
	{
		return result;
	}

	QString fieldLog;
	QByteArray commandData = getTLVData(aField, getConfigParameter(mFiscalFieldData[aField].description), &fieldLog);

	if (!processCommand(CShtrihOnlineFR::Commands::FS::SetOFDParameter, commandData))
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

	setFRParameter(CShtrihOnlineFR::FRParameters::NotPrintDocument, true);

	bool result = processCommand(CShtrihFR::Commands::OpenFRSession);
	waitForPrintingEnd();

	return result;
}

//--------------------------------------------------------------------------------
