/* @file Онлайн ФР на платформе АТОЛ5. */

// Project
#include "Atol5OnlineFRBase.h"
#include "Atol5OnlineFRConstants.h"

using namespace SDK::Driver;
using namespace ProtocolUtils;

template class Atol5OnlineFRBase<CInteractionTypes::ItExternalCOM>;
template class Atol5OnlineFRBase<CInteractionTypes::ItExternalVCOM>;

//--------------------------------------------------------------------------------
template <class T>
Atol5OnlineFRBase<T>::Atol5OnlineFRBase(): mThreadProxy(&mThread), mTriedToConnect(false)
{
	mThreadProxy.moveToThread(&mThread);

	// данные устройства
	mIsOnline = true;
	mSupportedModels = getModelList();
	mDeviceName = "ATOL5 online FR";

	// параметры устройства
	setConfigParameter(CHardwareSDK::FR::CanWithoutPrinting, true);

	// эти теги ставим логином оператора на платеже, если касса не в автомате
	mOFDFiscalFields.remove(CFR::FiscalFields::Cashier);
	mOFDFiscalFields.remove(CFR::FiscalFields::CashierINN);

	// налоги
	mTaxData.add(20, 1);
	mTaxData.add(10, 2);
	mTaxData.add( 0, 6);
}

//--------------------------------------------------------------------------------
template <class T>
QStringList Atol5OnlineFRBase<T>::getModelList()
{
	return CAtolFR::CModelData().getModelList(EFRType::FS, false);
}

//--------------------------------------------------------------------------------
template <class T>
void Atol5OnlineFRBase<T>::initialize()
{
	START_IN_WORKING_THREAD(initialize)

	initializeResources();
	PollingDeviceBase::initialize();

	if (mInitialized == ERequestStatus::Fail)
	{
		if (!checkConnectionAbility())
		{
			processStatusCode(DeviceStatusCode::Error::ThirdPartyDriverFail);
		}

		releaseExternalResource();
	}
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::updateParameters()
{
	processDeviceData();
	setConfigParameter(CHardware::Printer::FeedingAmount, mModelData.feedingAmount);

	if (!setFRParameters() || !getPrintingSettings())
	{
		return false;
	}

	setConfigParameter(CHardwareSDK::FR::SectionNames, QVariant::fromValue<TSectionNames>(CAtol5OnlineFR::getSectionNames()));

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::getFiscalizationData(int aField, QString & aValue, const CFR::TFFFormatDataMethod & aMethod)
{
	mDriver.setDriverParameter(LIBFPTR_PARAM_FN_DATA_TYPE, int(LIBFPTR_FNDT_TAG_VALUE));
	mDriver.setDriverParameter(LIBFPTR_PARAM_TAG_NUMBER, aField);

	if (mDriver()->fnQueryData())
	{
		return mDriver.logLastError("get fiscalization data by " + mFFData.getTextLog(aField));
	}

	aValue = aMethod(mDriver.getDriverParameter<QString>(LIBFPTR_PARAM_TAG_VALUE).toLatin1(), 10);
	mFFEngine.addConfigParameter<QString>(aField, aValue);

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::setFRParameters()
{
	mDriver.setDriverParameter(LIBFPTR_PARAM_FN_DATA_TYPE, int(LIBFPTR_FNDT_REG_INFO));

	if (mDriver()->fnQueryData())
	{
		return mDriver.logLastError("get fiscalization data");
	}

	int taxSystemsData = mDriver.getDriverParameter<int>(CFR::FiscalFields::TaxSystemsReg);
	int agentFlagsData = mDriver.getDriverParameter<int>(CFR::FiscalFields::AgentFlagsReg);

	QString FFDData = QString::number(mDriver.getDriverParameter<int>(CFR::FiscalFields::FFD));
	EFFD::Enum FFD = CFR::FFD.getVersion(FFDData);

	if (FFD != EFFD::Unknown)
	{
		mFFDFR = FFD;
		mFFDFS = mFFDFR;
	}

	if (!checkTaxSystems(char(taxSystemsData)) || !checkAgentFlags(char(agentFlagsData)))
	{
		return false;
	}

	CFR::FiscalFields::TFields operationModeFields = CFR::OperationModeData.data().values();
	char operationModes = ASCII::NUL;

	foreach (int field, CFR::FiscalFields::ModeFields)
	{
		bool mode = mDriver.getDriverParameter<bool>(field);

		if (mode)
		{
			mFFEngine.addConfigParameter<int>(field, int(mode));

			if (operationModeFields.contains(field))
			{
				operationModes |= CFR::OperationModeData.key(field);
			}
		}
	}

	checkOperationModes(operationModes);

	auto addConfigParameter = [&] (int aField) { mFFEngine.addConfigParameter<QString>(aField, mDriver.getDriverParameter<QString>(aField), nullptr); };

	addConfigParameter(CFR::FiscalFields::FTSURL);
	addConfigParameter(CFR::FiscalFields::PayOffAddress);
	addConfigParameter(CFR::FiscalFields::LegalOwner);
	addConfigParameter(CFR::FiscalFields::PayOffPlace);
	addConfigParameter(CFR::FiscalFields::AutomaticNumber);
	addConfigParameter(CFR::FiscalFields::OFDINN);
	addConfigParameter(CFR::FiscalFields::OFDName);

	mINN = CFR::INNToString(mDriver.getDriverParameter<QString>(CFR::FiscalFields::INN).toLatin1());
	mRNM = CFR::RNMToString(mDriver.getDriverParameter<QString>(CFR::FiscalFields::RNM).toLatin1());

	mFFEngine.addConfigParameter<QString>(CFiscalSDK::INN, mINN);
	mFFEngine.addConfigParameter<QString>(CFiscalSDK::RNM, mRNM);

	auto checkData = [&] (int aFiscalField, QString & aData) -> bool { return !aData.isEmpty() || getFiscalizationData(aFiscalField, aData, CFR::FSSerialToString); };

	if (!checkData(CFR::FiscalFields::SerialFSNumber, mFSSerialNumber) ||
		!checkData(CFR::FiscalFields::SerialFRNumber, mSerial))
	{
		return false;
	}

	// обнулять сумму в кассе при закрытии смены
	QString nullingSumInCash = getConfigParameter(CHardwareSDK::FR::NullingSumInCash).toString();
	bool useNullingSumInCash = nullingSumInCash == CHardwareSDK::Values::Use;

	if ((nullingSumInCash != CHardwareSDK::Values::Auto) &&
		!mDriver.setFRParameter<bool>(CAtol5OnlineFR::FRParameters::NullingSumInCash, useNullingSumInCash))
	{
		return false;
	}

	// межстрочный интервал
	if (mModelData.lineSpacing && containsConfigParameter(CHardware::Printer::Settings::LineSpacing))
	{
		int linespacing = getConfigParameter(CHardware::Printer::Settings::LineSpacing).toInt();
		mDriver.setFRParameter<int>(CAtol5OnlineFR::FRParameters::LineSpacing, linespacing);
	}

	// печатать номер секции - нет
	mDriver.setFRParameter<bool>(CAtol5OnlineFR::FRParameters::PrintSectionNumber, false);

	// звуковой сигнал при ошибке - нет
	mDriver.setFRParameter<bool>(CAtol5OnlineFR::FRParameters::BeepOnError, false);

	// звуковой сигнал при включении - да
	mDriver.setFRParameter<bool>(CAtol5OnlineFR::FRParameters::BeepOnPowerOn, true);

	// СНО по умолчанию - никакая. Если не будет установлена, то будет лучше ошибка
	mDriver.setFRParameter<int>(CAtol5OnlineFR::FRParameters::DefaultTaxSystem, ETaxSystems::None);

	// Печатать признак способа расчета (1214) - да
	mDriver.setFRParameter<bool>(CAtol5OnlineFR::FRParameters::PrintPayOffSubjectMethodType, true);

	// Печатать признак предмета расчета (1212) - да
	mDriver.setFRParameter<bool>(CAtol5OnlineFR::FRParameters::PrintPayOffSubjectType, true);

	// включить отрезчик - да
	mDriver.setFRParameter<bool>(CAtol5OnlineFR::FRParameters::EnableCutter, true);

	// полная отрезка - да
	mDriver.setFRParameter<bool>(CAtol5OnlineFR::FRParameters::FullCutting, true);

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
void Atol5OnlineFRBase<T>::getVersionData(libfptr_unit_type aType, const QString & aKey)
{
	mDriver.setDriverParameter<int>(LIBFPTR_PARAM_DATA_TYPE, LIBFPTR_DT_UNIT_VERSION);
	mDriver.setDriverParameter<int>(LIBFPTR_PARAM_UNIT_TYPE, aType);

	if (mDriver()->queryData())
	{
		mDriver.logLastError("get version of " + QString(aKey).replace("_", " "));
	}
	else
	{
		QString data = mDriver.getDriverParameter<QString>(LIBFPTR_PARAM_UNIT_VERSION);
		setDeviceParameter(aKey, data);
	}
}

//--------------------------------------------------------------------------------
template <class T>
void Atol5OnlineFRBase<T>::processDeviceData()
{
	getVersionData(LIBFPTR_UT_FIRMWARE,      CDeviceData::InternalFirmware);
	getVersionData(LIBFPTR_UT_CONFIGURATION, CDeviceData::DriverConfig);
	getVersionData(LIBFPTR_UT_TEMPLATES,     CDeviceData::DriverTemplate);
	getVersionData(LIBFPTR_UT_CONTROL_UNIT,  CDeviceData::DriverControlUnit);
	getVersionData(LIBFPTR_UT_BOOT,          CDeviceData::DriverLoader);

	mFRBuild = getDeviceParameter(CDeviceData::InternalFirmware).toInt();

	QString addressData;
	int portData = 0;
	mOFDDataError = !mDriver.getFRParameter<QString>(CAtol5OnlineFR::FRParameters::OFDAddress, addressData) ||
	                !mDriver.getFRParameter<int>(CAtol5OnlineFR::FRParameters::OFDPort, portData);
	mOFDDataError = mOFDDataError || !checkOFDData(addressData.toLatin1(), ProtocolUtils::toBCD(portData));

	// данные ФР - серийный номер
	mDriver.setDriverParameter<int>(LIBFPTR_PARAM_DATA_TYPE, LIBFPTR_DT_STATUS);

	if (!mDriver()->queryData())
	{
		mSerial = CFR::serialToString(mDriver.getDriverParameter<QString>(LIBFPTR_PARAM_SERIAL_NUMBER).toLatin1());
	}
	else
	{
		mDriver.logLastError("get serial number");
	}

	// данные ФН
	mDriver.setDriverParameter(LIBFPTR_PARAM_FN_DATA_TYPE, int(LIBFPTR_FNDT_FN_INFO));

	if (!mDriver()->fnQueryData())
	{
		mFSSerialNumber = CFR::FSSerialToString(mDriver.getDriverParameter<QString>(LIBFPTR_PARAM_SERIAL_NUMBER).toLatin1());

		int FSTypeData = mDriver.getDriverParameter<int>(LIBFPTR_PARAM_FN_TYPE);
		QString FSType = "unknown";

		     if (FSTypeData == LIBFPTR_FNT_DEBUG)   FSType = "debug";
		else if (FSTypeData == LIBFPTR_FNT_RELEASE) FSType = "release";

		QString FSTypeVersion = QString("%1, type %2").arg(mDriver.getDriverParameter<QString>(LIBFPTR_PARAM_FN_VERSION)).arg(FSType);
		setDeviceParameter(CDeviceData::FS::Version, FSTypeVersion);
	}
	else
	{
		mDriver.logLastError("get FS data");
	}

	// ресурс ФН
	mDriver.setDriverParameter(LIBFPTR_PARAM_FN_DATA_TYPE, int(LIBFPTR_FNDT_VALIDITY));

	if (!mDriver()->fnQueryData())
	{
		QString FSValidityData = mDriver.getDriverParameter<QDateTime>(LIBFPTR_PARAM_DATE_TIME).toString(CFR::DateLogFormat);
		setDeviceParameter(CDeviceData::FS::ValidityData, FSValidityData);

		setDeviceParameter(CDeviceData::FR::ReregistrationNumber, mDriver.getDriverParameter<int>(LIBFPTR_PARAM_REGISTRATIONS_COUNT));
		setDeviceParameter(CDeviceData::FR::FreeReregistrations,  mDriver.getDriverParameter<int>(LIBFPTR_PARAM_REGISTRATIONS_REMAIN));
	}
	else
	{
		mDriver.logLastError("get FS resource data");
	}

	// количество регистраций
	mDriver.setDriverParameter(LIBFPTR_PARAM_DATA_TYPE, int(LIBFPTR_DT_RECEIPT_COUNT));
	mDriver.setDriverParameter(LIBFPTR_PARAM_RECEIPT_TYPE, int(LIBFPTR_RT_SELL));

	if (!mDriver()->queryData())
	{
		setDeviceParameter(CDeviceData::FR::FiscalDocuments, mDriver.getDriverParameter<int>(LIBFPTR_PARAM_DOCUMENTS_COUNT));
	}
	else
	{
		mDriver.logLastError("get documents data");
	}

	// необнуляемая сумма
	mNonNullableAmount = 0;
	mDriver.setDriverParameter(LIBFPTR_PARAM_DATA_TYPE, int(LIBFPTR_DT_NON_NULLABLE_SUM));
	mDriver.setDriverParameter(LIBFPTR_PARAM_RECEIPT_TYPE, int(LIBFPTR_RT_SELL));

	if (!mDriver()->queryData())
	{
		mNonNullableAmount = mDriver.getDriverParameter<double>(LIBFPTR_PARAM_SUM);
	}
	else
	{
		mDriver.logLastError("get non nullable amount");
	}

	checkDateTime();

	// ресурс отрезчика
	mDriver.setDriverParameter(LIBFPTR_PARAM_DATA_TYPE, int(LIBFPTR_DT_CUTTER_RESOURCE ));
	mDriver.setDriverParameter(LIBFPTR_PARAM_COUNTER_TYPE, int(LIBFPTR_CT_ROLLUP));

	if (!mDriver()->queryData())
	{
		setDeviceParameter(CDeviceData::Printers::CutterResource,  mDriver.getDriverParameter<int>(LIBFPTR_PARAM_COUNT));
	}
	else
	{
		mDriver.logLastError("get cutter resource");
		removeDeviceParameter(CDeviceData::Printers::CutterResource);
	}

	// ресурс двигателя
	mDriver.setDriverParameter(LIBFPTR_PARAM_DATA_TYPE, int(LIBFPTR_DT_STEP_RESOURCE));
	mDriver.setDriverParameter(LIBFPTR_PARAM_COUNTER_TYPE, int(LIBFPTR_CT_ROLLUP));
	mDriver.setDriverParameter(LIBFPTR_PARAM_STEP_COUNTER_TYPE, int(LIBFPTR_SCT_OVERALL));

	if (!mDriver()->queryData())
	{
		setDeviceParameter(CDeviceData::Printers::EngineResource,  mDriver.getDriverParameter<int>(LIBFPTR_PARAM_COUNT));
	}
	else
	{
		mDriver.logLastError("get engine resource");
		removeDeviceParameter(CDeviceData::Printers::EngineResource);
	}

	// ресурс термоголовки
	mDriver.setDriverParameter(LIBFPTR_PARAM_DATA_TYPE, int(LIBFPTR_DT_TERMAL_RESOURCE));
	mDriver.setDriverParameter(LIBFPTR_PARAM_COUNTER_TYPE, int(LIBFPTR_CT_ROLLUP));

	if (!mDriver()->queryData())
	{
		setDeviceParameter(CDeviceData::Printers::HeadResource,  mDriver.getDriverParameter<int>(LIBFPTR_PARAM_COUNT));
	}
	else
	{
		mDriver.logLastError("get head resource");
		removeDeviceParameter(CDeviceData::Printers::HeadResource);
	}

	ESessionState::Enum sessionState = getSessionState();

	if (sessionState != ESessionState::Error)
	{
		setDeviceParameter(CDeviceData::FR::Session, CFR::SessionState[sessionState]);

		mDriver.setDriverParameter<int>(LIBFPTR_PARAM_DATA_TYPE, LIBFPTR_DT_SHIFT_STATE);

		if (mDriver()->queryData())
		{
			mDriver.logLastError("get session full state");
		}
		else
		{
			int sessionNumber = mDriver.getDriverParameter<int>(LIBFPTR_PARAM_SHIFT_NUMBER);

			if (sessionState == ESessionState::Closed)
			{
				setDeviceParameter(CDeviceData::LastNumber, sessionNumber, CDeviceData::FR::Session);
			}
			else
			{
				QDateTime openingSessionDT = mDriver.getDriverParameter<QDateTime>(LIBFPTR_PARAM_DATE_TIME);

				setDeviceParameter(CDeviceData::Number, sessionNumber, CDeviceData::FR::Session);
				setDeviceParameter(CDeviceData::FR::OpeningDate, openingSessionDT.toString(CFR::DateTimeLogFormat), CDeviceData::FR::Session);
			}
		}
	}

	//TODO: для TCP-реализации запрашивать шлюз и маску
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::getPrintingSettings()
{
	mDriver.setDriverParameter(LIBFPTR_PARAM_DATA_TYPE, int(LIBFPTR_DT_RECEIPT_LINE_LENGTH));

	if (mDriver()->queryData())
	{
		return mDriver.logLastError("get line size");
	}

	mLineSize = mDriver.getDriverParameter<int>(LIBFPTR_PARAM_RECEIPT_LINE_LENGTH);

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::checkTaxes()
{
	// TODO: дотестить и удалить, если надо
	return ProtoAtolFR<FRBase<PrinterBase<ProtoAtol5FR<T>>>>::checkTaxes();
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::checkConnectionAbility()
{
	if (!mDriver.isInitialized())
	{
		return false;
	}

	//TODO: проверять имя порта по списку имеющихся виртуальных USB-портов
	CAtol5OnlineFR::TConnectionParameters changed = CAtol5OnlineFR::TConnectionParameters()
		<< mDriver.checkSetting<bool>(LIBFPTR_SETTING_AUTO_RECONNECT, true)
		<< mDriver.checkSetting<int>(LIBFPTR_SETTING_MODEL, LIBFPTR_MODEL_ATOL_AUTO);

	if (!checkConnectionParameters(&mDriver, changed))
	{
		return false;
	}

	if (std::find_if(changed.begin(), changed.end(), [&] (bool aChanged) -> bool { return aChanged; }) != changed.end())
	{
		mDriver()->applySingleSettings();
	}

	return mDriver.connect();
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::releaseExternalResource()
{
	return !mOperatorPresence || mDriver.disconnect();
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::checkExistence()
{
	mTriedToConnect = false;

	if (!isWorkingThread())
	{
		if (mThread.isRunning())
		{
			QMetaObject::invokeMethod(this, "checkExistence", Qt::QueuedConnection);
		}
		else
		{
			connect(&mThread, SIGNAL(started()), this, SLOT(checkExistence()), Qt::UniqueConnection);
			mThread.start();
		}

		QMutexLocker locker(&mStartMutex);

		if (!mTriedToConnect)
		{
			mStartCondition.wait(&mStartMutex);
		}

		return mConnected;
	}

	initializeResources();

	bool result = false;

	if (checkConnectionAbility())
	{
		logConnectionParameters();
		bool result = ProtoAtolFR<FRBase<PrinterBase<ProtoAtol5FR<T>>>>::checkExistence();
	}

	mTriedToConnect = true;
	mStartCondition.wakeAll();

	return result;
}

//--------------------------------------------------------------------------------
template <class T>
void Atol5OnlineFRBase<T>::initializeResources()
{
	QString libraryPath = getConfigParameter(CHardwareSDK::PluginDirectory).toString();
	QString    userPath = getConfigParameter(CPluginParameters::ConfigurationDirectory).toString();
	QString     logPath = getConfigParameter(CHardwareSDK::LogDirectory).toString();
	bool debug = mLog->getLevel() >= LogLevel::Debug;

	toLog(LogLevel::Debug, "Library path: " + libraryPath);
	toLog(LogLevel::Debug, "   User path: " + userPath);
	toLog(LogLevel::Debug, "    Log path: " + logPath);

	if (mDriver.initializeLogData(userPath, logPath, debug) && !mDriver.isInitialized())
	{
		mDriver.initialize(libraryPath);
	}
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::performRelease()
{
	releasePolling();

	mDriver.disconnect();
	mDriver.release();

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::release()
{
	if (!mDriver.isInitialized() || !mThread.isRunning())
	{
		return true;
	}

	bool result;

	if (!isWorkingThread())
	{
		QMetaObject::invokeMethod(this, "performRelease", Qt::BlockingQueuedConnection, QReturnArgument<bool>("bool", result));
	}
	else
	{
		result = performRelease();
	}

	if (!DeviceBase::release())
	{
		result = false;
	}

	return result;
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::getStatus(TStatusCodes & aStatusCodes)
{
	mDriver.setDriverParameter<int>(LIBFPTR_PARAM_DATA_TYPE, LIBFPTR_DT_STATUS);

	if (mDriver()->queryData())
	{
		return mDriver.logLastError("get main status");
	}

	mFiscalized = mDriver.getDriverParameter<bool>(LIBFPTR_PARAM_FN_FISCAL);

	if (!mDriver.getDriverParameter<bool>(LIBFPTR_PARAM_FISCAL))                aStatusCodes.insert(FRStatusCode::Warning::FRNotRegistered);
	if (!mDriver.getDriverParameter<bool>(LIBFPTR_PARAM_FN_PRESENT))            aStatusCodes.insert(FRStatusCode::Error::NoFS);
	if ( mDriver.getDriverParameter<bool>(LIBFPTR_PARAM_INVALID_FN))            aStatusCodes.insert(FRStatusCode::Error::FS);
	if (!mDriver.getDriverParameter<bool>(LIBFPTR_PARAM_RECEIPT_PAPER_PRESENT)) aStatusCodes.insert(PrinterStatusCode::Error::PaperEnd);
	if ( mDriver.getDriverParameter<bool>(LIBFPTR_PARAM_PAPER_NEAR_END))        aStatusCodes.insert(PrinterStatusCode::Warning::PaperNearEnd);
	if ( mDriver.getDriverParameter<bool>(LIBFPTR_PARAM_COVER_OPENED))          aStatusCodes.insert(DeviceStatusCode::Error::CoverIsOpened);

	if ( mDriver.getDriverParameter<bool>(LIBFPTR_PARAM_PRINTER_CONNECTION_LOST) ||
	     mDriver.getDriverParameter<bool>(LIBFPTR_PARAM_PRINTER_ERROR))         aStatusCodes.insert(PrinterStatusCode::Error::PrinterFR);
	if ( mDriver.getDriverParameter<bool>(LIBFPTR_PARAM_CUT_ERROR))             aStatusCodes.insert(PrinterStatusCode::Error::Cutter);
	if ( mDriver.getDriverParameter<bool>(LIBFPTR_PARAM_PRINTER_OVERHEAT))      aStatusCodes.insert(PrinterStatusCode::Error::PrintingHead);
	if ( mDriver.getDriverParameter<bool>(LIBFPTR_PARAM_BLOCKED))               aStatusCodes.insert(DeviceStatusCode::Error::Unknown);

	mDriver.setDriverParameter(LIBFPTR_PARAM_FN_DATA_TYPE, int(LIBFPTR_FNDT_FN_INFO));

	if (mDriver()->fnQueryData())
	{
		aStatusCodes.insert(FRStatusCode::Error::FS);
	}
	else
	{
		if ( mDriver.getDriverParameter<bool>(LIBFPTR_PARAM_FN_RESOURCE_EXHAUSTED)) aStatusCodes.insert(FRStatusCode::Error::FSEnd);
		if ( mDriver.getDriverParameter<bool>(LIBFPTR_PARAM_FN_MEMORY_OVERFLOW))    aStatusCodes.insert(FRStatusCode::Error::FSOverflow);
		if ( mDriver.getDriverParameter<bool>(LIBFPTR_PARAM_FN_CRITICAL_ERROR))     aStatusCodes.insert(FRStatusCode::Error::FS);

		// TODO: выяснить смысл параметра
		//if ( mDriver.getDriverParameter<bool>(LIBFPTR_PARAM_FN_OFD_TIMEOUT))        aStatusCodes.insert(FRStatusCode::Error::NeedOFDConnection);
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::getModeStatus()
{
	mDriver.setDriverParameter<int>(LIBFPTR_PARAM_DATA_TYPE, LIBFPTR_DT_STATUS);

	if (mDriver()->queryData())
	{
		return mDriver.logLastError("get mode status");
	}

	mMode    = char(mDriver.getDriverParameter<int>(LIBFPTR_PARAM_MODE));
	mSubmode = char(mDriver.getDriverParameter<int>(LIBFPTR_PARAM_SUBMODE));

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
void Atol5OnlineFRBase<T>::doPoll(TStatusCodes & aStatusCodes)
{
	MutexLocker locker(&mExternalMutex);

	QDate currentDate = QDate::currentDate();

	if (mLogDate.day() != currentDate.day())
	{
		mDriver.disconnect();
		mDriver.connect();
	}

	ProtoAtolFR<FRBase<PrinterBase<ProtoAtol5FR<T>>>>::doPoll(aStatusCodes);
}

//--------------------------------------------------------------------------------
template <class T>
TResult Atol5OnlineFRBase<T>::processIntMethod(TIntMethod aMethod, const QString & aLog, const QString & aMethodData)
{
	mThreadProxy.invokeMethod<int>(aMethod);
	TResult result = mDriver(false)->errorCode();

	if (result)
	{
		foreach (const CAtol5OnlineFR::SErrorData & errorData, mErrors)
		{
			if (errorData.method == aMethodData)
			{
				mErrors.removeAll(errorData);
			}
		}
	}
	else
	{
		mDriver.logLastError(aLog);

		CAtol5OnlineFR::SErrorData errorData = CAtol5OnlineFR::SErrorData(aMethodData, result, mDriver.getFRMethodParameters());

		if (!mErrors.contains(errorData))
		{
			auto repeatMethod = [&] () -> TResult { mErrors.removeLast(); toLog(LogLevel::Normal, QString("repeat previous method %1.").arg(aMethodData));
				mDriver.setFRMethodParameters(errorData.methodParameters); return processIntMethod(aMethod, aLog, aMethodData); };

			mErrors.append(errorData);
			int error = result;

			switch (error)
			{
				case LIBFPTR_ERROR_SHIFT_EXPIRED:
				{
					if (execZReport(true))
					{
						result = repeatMethod();
					}

					break;
				}
				//--------------------------------------------------------------------------------
				case LIBFPTR_ERROR_DENIED_IN_OPENED_RECEIPT:
				{
					if (cancelDocument())
					{
						result = repeatMethod();
					}

					break;
				}
			}
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
template <class T>
QDateTime Atol5OnlineFRBase<T>::getDateTime()
{
	mDriver.setDriverParameter<int>(LIBFPTR_PARAM_DATA_TYPE, LIBFPTR_DT_STATUS);

	if (mDriver()->queryData())
	{
		mDriver.logLastError("get date and time");
		return QDateTime();
	}

	return mDriver.getDriverParameter<QDateTime>(LIBFPTR_PARAM_DATE_TIME);
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::processModelKey(CAtolFR::TModelKey & aModeKey)
{
	mDriver.setDriverParameter<int>(LIBFPTR_PARAM_DATA_TYPE, LIBFPTR_DT_STATUS);

	if (mDriver()->queryData())
	{
		return mDriver.logLastError("get model key");
	}

	int model = mDriver.getDriverParameter<int>(LIBFPTR_PARAM_MODEL);
	aModeKey = CAtolFR::TModelKey(model, EFRType::FS);

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::processReceipt(const QStringList & aReceipt, bool aProcessing)
{
	if (!isPrintingNeed(aReceipt))
	{
		return true;
	}

	setConfigParameter(CHardware::Printer::NeedProcessing, aProcessing);

	if (aProcessing && mDriver()->beginNonfiscalDocument())
	{
		return mDriver.logLastError("begin nonfiscal document");
	}

	if (!ProtoAtolFR<FRBase<PrinterBase<ProtoAtol5FR<T>>>>::processReceipt(aReceipt, aProcessing))
	{
		if (aProcessing)
		{
			cancelDocument();
		}

		return false;
	}

	if (aProcessing && mDriver()->endNonfiscalDocument())
	{
		mDriver.logLastError("end nonfiscal document");
		cancelDocument();

		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::printReceipt(const Tags::TLexemeReceipt & aLexemeReceipt)
{
	if (mModelData.lineSpacing && containsConfigParameter(CHardware::Printer::Settings::LineSpacing))
	{
		int linespacing = getConfigParameter(CHardware::Printer::Settings::LineSpacing).toInt();
		mDriver.setDriverParameter<int>(LIBFPTR_PARAM_LINESPACING, linespacing);
	}

	mDriver.setDriverParameter<int>(LIBFPTR_PARAM_TEXT_WRAP, LIBFPTR_TW_CHARS);

	foreach (auto lexemeCollection, aLexemeReceipt)
	{
		QString line;
		Tags::TTypes tags;

		foreach (auto lexemes, lexemeCollection)
		{
			foreach (auto lexeme, lexemes)
			{
				if (lexeme.tags.contains(Tags::Type::Image))
				{
					if (!line.isEmpty() && !printLine(line))
					{
						return false;
					}

					// печать картинки

					line.clear();
				}

				line += lexeme.data;
				tags += lexeme.tags;
			}
		}

		mDriver.setDriverParameter<bool>(LIBFPTR_PARAM_FONT_DOUBLE_WIDTH,  tags.contains(Tags::Type::DoubleWidth));
		mDriver.setDriverParameter<bool>(LIBFPTR_PARAM_FONT_DOUBLE_HEIGHT, tags.contains(Tags::Type::DoubleHeight));

		libfptr_alignment alignment = tags.contains(Tags::Type::Center) ? LIBFPTR_ALIGNMENT_CENTER : LIBFPTR_ALIGNMENT_LEFT;
		mDriver.setDriverParameter<int>(LIBFPTR_PARAM_ALIGNMENT, alignment);

		if (!line.isEmpty() && !printLine(line))
		{
			return false;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::printLine(const QVariant & aLine)
{
	if (!getConfigParameter(CHardware::Printer::NeedProcessing).toBool())
	{
		mDriver.setDriverParameter<int>(LIBFPTR_PARAM_DEFER, LIBFPTR_DEFER_PRE);
	}

	mDriver.setDriverParameter<QString>(LIBFPTR_PARAM_TEXT, aLine.toString());

	if (mDriver()->printText())
	{
		return mDriver.logLastError("print line");
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::receiptProcessing()
{
	// процессинг чека делается выше и только после одиночной печати нефискального чека.
	// при печати комплексного документа открытия нефискального чека не требуется.
	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::cancelDocument()
{
	if (mDriver()->cancelReceipt())
	{
		return mDriver.logLastError("cancel nonfiscal document");
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
ESessionState::Enum Atol5OnlineFRBase<T>::getSessionState()
{
	mDriver.setDriverParameter<int>(LIBFPTR_PARAM_DATA_TYPE, LIBFPTR_DT_SHIFT_STATE);

	if (mDriver()->queryData())
	{
		mDriver.logLastError("get session state");
		return ESessionState::Error;
	}

	int sessionData = mDriver.getDriverParameter<int>(LIBFPTR_PARAM_SHIFT_STATE);

	if (sessionData == LIBFPTR_SS_CLOSED)  return ESessionState::Closed;
	if (sessionData == LIBFPTR_SS_EXPIRED) return ESessionState::Expired;

	return ESessionState::Opened;
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::setNotPrintDocument(bool /*aEnabled*/, bool /*aZReport*/)
{
	// включаем параметром в месте использования.
	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::operatorLogin()
{
	if (!mOperatorPresence)
	{
		return true;
	}

	QString cashier;
	QString cashierINN;

	mFFEngine.checkData(CFiscalSDK::Cashier, cashier);
	mFFEngine.checkData(CFiscalSDK::CashierINN, cashierINN);
	mFFEngine.checkINN(cashierINN, CFR::INN::Person::Natural);

	mDriver.setDriverParameter<QString>(CFR::FiscalFields::Cashier, cashier);
	mDriver.setDriverParameter<QString>(CFR::FiscalFields::CashierINN, cashierINN);

	if (mDriver()->operatorLogin())
	{
		return mDriver.logLastError("login the operator");
	}

	toLog(LogLevel::Normal, "Operator has logged in successfully.");

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::complexFiscalDocument(TBoolMethod aMethod, const QString & aLog)
{
	if (aMethod())
	{
		return true;
	}

	if (!mNextReceiptProcessing)
	{
		onPoll();

		TStatusCodes nonFiscalErrors = mStatusCollection.value(EWarningLevel::Error) - getFiscalStatusCodes(EWarningLevel::Error);

		if (nonFiscalErrors.isEmpty())
		{
			//TODO: если не будет дополнительного функционала - объединить с аналогичным методом FRBase.
			// отличие только в данной реакции на ошибку.
			cancelDocument();
		}
	}

	toLog(LogLevel::Error, mDeviceName + ": Failed to process " + aLog);

	return false;
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::performFiscal(const QStringList & aReceipt, const SPaymentData & aPaymentData, quint32 * aFDNumber)
{
	bool result = processReceipt(aReceipt, false);

	if (result && checkNotPrinting())
	{
		result = complexFiscalDocument(std::bind(&Atol5OnlineFRBase<T>::execFiscal, this, std::ref(aReceipt), std::ref(aPaymentData), aFDNumber), "fiscal document");
	}

	return result;
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::execFiscal(const QStringList & aReceipt, const SPaymentData & aPaymentData, quint32 * aFDNumber)
{
	if (!operatorLogin())
	{
		return false;
	}

	openFRSession();

	if (!openDocument(aPaymentData.payOffType))
	{
		return false;
	}

	bool result = true;

	foreach (auto unitData, aPaymentData.unitDataList)
	{
		result = result && sale(unitData);
	}

	if (result)
	{
		result = closeDocument(aPaymentData.payType);
	}
	else if (aPaymentData.back() && mErrors.codes().contains(LIBFPTR_ERROR_NO_CASH))
	{
		emitStatusCode(FRStatusCode::Error::NoMoney, EFRStatus::NoMoneyForSellingBack);
	}

	if (result)
	{
		toLog(LogLevel::Normal, mDeviceName + QString(": Fiscal document %1 is processed successfully").arg(*aFDNumber));
		return true;
	}

	cancelDocument();

	return false;
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::openDocument(EPayOffTypes::Enum aPayOffType)
{
	if (!mOperationModes.contains(EOperationModes::Automatic))
	{
		operatorLogin();
	}

	mDriver.setDriverParameter<int>(LIBFPTR_PARAM_RECEIPT_TYPE, CAtol5OnlineFR::PayOffTypeData[aPayOffType]);

	if (!setFiscalFieldsOnPayment())
	{
		return false;
	}

	return INT_CALL_ATOL(openReceipt, "open fiscal document");
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::setTLV(int aField, bool /*aOnSale*/)
{
	bool result;

	if (!mFFEngine.checkFiscalField(aField, result))
	{
		return result;
	}

	QString textKey = mFFData[aField].textKey;
	QVariant value = mFFEngine.getConfigParameter(textKey);
	mDriver.setDriverParameter<QVariant>(aField, value);

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::sale(const SDK::Driver::SUnitData & aUnitData)
{
	mDriver.setDriverParameter<QString>(LIBFPTR_PARAM_COMMODITY_NAME, aUnitData.name);
	mDriver.setDriverParameter<TSum>(LIBFPTR_PARAM_PRICE, aUnitData.sum);
	mDriver.setDriverParameter<double>(LIBFPTR_PARAM_QUANTITY, 1);
	mDriver.setDriverParameter<TVAT>(LIBFPTR_PARAM_TAX_TYPE, CAtol5OnlineFR::TaxData[aUnitData.VAT]);

	if (!setFiscalFieldsOnSale(aUnitData))
	{
		return false;
	}

	if (aUnitData.section != -1)
	{
		mDriver.setDriverParameter<int>(LIBFPTR_PARAM_DEPARTMENT, aUnitData.section);
	}

	if (aUnitData.payOffSubjectType != EPayOffSubjectTypes::None)
	{
		mDriver.setDriverParameter<int>(CFR::FiscalFields::PayOffSubjectType, aUnitData.payOffSubjectType);
	}

	if (aUnitData.payOffSubjectMethodType != EPayOffSubjectMethodTypes::None)
	{
		mDriver.setDriverParameter<int>(CFR::FiscalFields::PayOffSubjectMethodType, aUnitData.payOffSubjectMethodType);
	}

	if (mDriver()->registration())
	{
		return mDriver.logLastError(QString("sale for %1 (%2, VAT = %3)").arg(aUnitData.sum, 0, 'f', 2).arg(aUnitData.name).arg(aUnitData.VAT));
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::closeDocument(EPayTypes::Enum aPayType)
{
	mDriver.setDriverParameter<int>(LIBFPTR_PARAM_PAYMENT_TYPE, CAtol5OnlineFR::PayTypeData[aPayType]);

	if (mDriver()->closeReceipt())
	{
		return mDriver.logLastError("close fiscal document");
	}
	else if (mDriver()->checkDocumentClosed())
	{
		return mDriver.logLastError("check document closed on opening session");
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::openSession()
{
	if (!operatorLogin())
	{
		return false;
	}

	mDriver.setDriverParameter<bool>(LIBFPTR_PARAM_REPORT_ELECTRONICALLY, true);

	if (mDriver()->openShift())
	{
		return mDriver.logLastError("open session");
	}

	if (mDriver()->checkDocumentClosed())
	{
		return mDriver.logLastError("check document closed on opening session");
	}

	toLog(LogLevel::Normal, mDeviceName + ": Session is successfully opened");

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::performZReport(bool /*aPrintDeferredReports*/)
{
	return execZReport(false);
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::execZReport(bool aAuto)
{
	toLog(LogLevel::Normal, mDeviceName + QString(": Begin processing %1Z-report").arg(aAuto ? "auto-" : ""));
	ESessionState::Enum sessionState = getSessionState();

	     if (sessionState == ESessionState::Error)  return false;
	else if (sessionState == ESessionState::Closed) return true;

	bool needCloseSession = sessionState == ESessionState::Expired;
	bool notPrinting = aAuto || isNotPrinting();

	if (aAuto)
	{
		if (mOperatorPresence)
		{
			toLog(LogLevel::Error, mDeviceName + ": Failed to process auto-Z-report due to presence of the operator.");
			mNeedCloseSession = mNeedCloseSession || needCloseSession;

			return false;
		}
		else if (!notPrinting)
		{
			mNeedCloseSession = mNeedCloseSession || needCloseSession;

			return false;
		}
	}

	bool result = false;

	if (operatorLogin())
	{
		mDriver.setDriverParameter<bool>(LIBFPTR_PARAM_REPORT_ELECTRONICALLY, notPrinting);
		mDriver.setDriverParameter<int>(LIBFPTR_PARAM_REPORT_TYPE, LIBFPTR_RT_CLOSE_SHIFT);

		result = !mDriver()->report();

		if (!result)
		{
			return mDriver.logLastError("Z-report");
		}
	}

	mNeedCloseSession = getSessionState() == ESessionState::Expired;

	if (!result)
	{
		toLog(LogLevel::Error, mDeviceName + ": error in processing Z-report");
		return false;
	}

	toLog(LogLevel::Normal, mDeviceName + ": Z-report is successfully processed");

	if (mDriver()->checkDocumentClosed())
	{
		return mDriver.logLastError("check document closed on Z-report");
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::performXReport(const QStringList & aReceipt)
{
	mDriver.setFRParameter<bool>(CAtol5OnlineFR::FRParameters::EnableCutter, false);

	return ProtoAtolFR<FRBase<PrinterBase<ProtoAtol5FR<T>>>>::performXReport(aReceipt);
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::processXReport()
{
	mDriver.setFRParameter<bool>(CAtol5OnlineFR::FRParameters::EnableCutter, true);

	if (!operatorLogin())
	{
		return false;
	}

	mDriver.setDriverParameter<bool>(LIBFPTR_PARAM_REPORT_ELECTRONICALLY, false);
	mDriver.setDriverParameter<int>(LIBFPTR_PARAM_REPORT_TYPE, LIBFPTR_RT_X);

	if (mDriver()->report())
	{
		return mDriver.logLastError("X-report");
	}

	toLog(LogLevel::Normal, mDeviceName + ": X-report is successfully processed");

	if (mDriver()->checkDocumentClosed())
	{
		return mDriver.logLastError("check document closed on X-report");
	}

	return true;
}

//--------------------------------------------------------------------------------
template<class T>
bool Atol5OnlineFRBase<T>::performEncashment(const QStringList & aReceipt, double aAmount)
{
	mDriver.setFRParameter<bool>(CAtol5OnlineFR::FRParameters::EnableCutter, false);

	return ProtoAtolFR<FRBase<PrinterBase<ProtoAtol5FR<T>>>>::performEncashment(aReceipt, aAmount);
}

//--------------------------------------------------------------------------------
template <class T>
double Atol5OnlineFRBase<T>::getAmountInCash()
{
	mDriver.setDriverParameter<int>(LIBFPTR_PARAM_DATA_TYPE, LIBFPTR_DT_CASH_SUM);

	if (mDriver()->queryData())
	{
		return mDriver.logLastError("get sum in cash");
	}

	return mDriver.getDriverParameter<double>(LIBFPTR_PARAM_SUM);
}

//--------------------------------------------------------------------------------
template <class T>
bool Atol5OnlineFRBase<T>::processPayout(double aAmount)
{
	mDriver.setFRParameter<bool>(CAtol5OnlineFR::FRParameters::EnableCutter, true);
	mDriver.setDriverParameter<double>(LIBFPTR_PARAM_SUM, aAmount);

	if (mDriver()->cashOutcome())
	{
		return mDriver.logLastError("payout");
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
void Atol5OnlineFRBase<T>::setLog(ILog * aLog)
{
	ProtoAtolFR<FRBase<PrinterBase<ProtoAtol5FR<T>>>>::setLog(aLog);

	mDriver.setLog(aLog);
}

//--------------------------------------------------------------------------------
