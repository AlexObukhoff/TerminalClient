/* @file ФР семейства Штрих на COM-порту. */

#pragma once

#include "ShtrihSerialFR.h"
#include "ShtrihFRBaseConstants.h"

//--------------------------------------------------------------------------------
ShtrihSerialFR::ShtrihSerialFR()
{
	// данные семейства ФР
	mSupportedModels = getModelList();
	mDeviceName = CShtrihFR::Models::Default;

	// данные команд
	mCommandData.add(CShtrihFRBase::Commands::GetFMTotalSum, 30 * 1000);

	// ошибки
	mErrorData = PErrorData(new CShtrihFRBase::Errors::CData);

	// данные налогов
	mTaxData.add(0, 3);
}

//--------------------------------------------------------------------------------
QStringList ShtrihSerialFR::getModelList()
{
	return CShtrihFR::Models::CData().getNonEjectorModels(false);
}

//--------------------------------------------------------------------------------
bool ShtrihSerialFR::updateParameters()
{
	if (!TShtrihSerialFRBase::updateParameters())
	{
		return false;
	}

	mNonNullableAmount = 0;
	QByteArray commandData(1, CShtrihFRBase::TotalFMSumType);
	QByteArray answer;

	if (mFiscalized && processCommand(CShtrihFRBase::Commands::GetFMTotalSum, commandData, &answer))
	{
		mNonNullableAmount = ProtocolUtils::revert(answer.mid(3, 8)).toHex().toULongLong(0, 16) / 100.0;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool ShtrihSerialFR::isNotError(char aCommand)
{
	if (TShtrihSerialFRBase::isNotError(aCommand))
	{
		return true;
	}

	if ((mLastError == CShtrihFRBase::Errors::FMInDataEntryMode) && getLongStatus() && (mMode != CShtrihFR::InnerModes::DataEjecting))
	{
		toLog(LogLevel::Normal, "ShtrihFR: mode is not data ejecting, it isn`t error");
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------
bool ShtrihSerialFR::processAnswer(const QByteArray & aCommand)
{
	if (mLastError == CShtrihFRBase::Errors::FMInDataEntryMode)
	{
		mProcessingErrors.push_back(mLastError);

		return processCommand(CShtrihFR::Commands::BreakDataEjecting);
	}
	else if (mLastError == CShtrihFRBase::Errors::CannotSetModeTable)
	{
		mProcessingErrors.push_back(mLastError);

		if (!getLongStatus())
		{
			toLog(LogLevel::Error, "ShtrihFR: Failed to get status, exit from answer handler");
			return false;
		}

		if ((mMode != CShtrihFR::InnerModes::SessionOpened) && (mMode != CShtrihFR::InnerModes::NeedCloseSession))
		{
			toLog(LogLevel::Error, "ShtrihFR: mode is not session opened or need close session, so the error cannot be processed");
			return false;
		}

		return execZReport(true);
	}

	return TShtrihSerialFRBase::processAnswer(aCommand);
}

//--------------------------------------------------------------------------------
void ShtrihSerialFR::parseDeviceData(const QByteArray & aData)
{
	TShtrihSerialFRBase::parseDeviceData(aData);

	// заводской номер
	mSerial = CFR::serialToString(ProtocolUtils::revert(aData.mid(32, 4)).toHex(), 16);

	// ИНН
	mINN = CFR::INNToString(ProtocolUtils::revert(aData.mid(42, 6)).toHex(), 16);

	// данные прошивки ФП
	CShtrihFR::SSoftInfo FMInfo;
	FMInfo.version = aData.mid(18, 2).insert(1, ASCII::Dot);
	FMInfo.build   = ProtocolUtils::revert(aData.mid(20, 2)).toHex().toUShort(0, 16);
	QString FMDate = ProtocolUtils::hexToBCD(aData.mid(22, 3)).insert(4, "20");
	FMInfo.date    = QDate::fromString(FMDate, CFR::DateFormat);

	setDeviceParameter(CDeviceData::Version, FMInfo.version, CDeviceData::FM::Firmware, true);
	setDeviceParameter(CDeviceData::Build, FMInfo.build, CDeviceData::FM::Firmware);
	setDeviceParameter(CDeviceData::Date, FMInfo.date.toString(CFR::DateLogFormat), CDeviceData::FM::Firmware);

	// количество свободных записей в ФП
	ushort freeFMSessions = ProtocolUtils::revert(aData.mid(38, 2)).toHex().toUShort(0, 16);
	setDeviceParameter(CDeviceData::FM::FreeSessions, freeFMSessions);

	// признак фискализированности ККМ
	mFiscalized = aData[40];

	// наличие ЭКЛЗ
	ushort flags = ProtocolUtils::revert(aData.mid(13, 2)).toHex().toUShort(0, 16);
	mEKLZ = flags & CShtrihFRBase::Statuses::EKLZExists;
	setDeviceParameter(CDeviceData::FR::EKLZ, mEKLZ);

	// данные ЭКЛЗ
	if (mEKLZ && mFiscalized && processCommand(CShtrihFRBase::Commands::EKLZActivizationTotal))
	{
		QTextCodec * codec = CodecByName[CHardware::Codepages::Win1251];
		QStringList totalData;
		QByteArray data;

		while (processCommand(CShtrihFRBase::Commands::GetEKLZReportData, &data))
		{
			totalData << codec->toUnicode(data.mid(2));
		}

		totalData = totalData.mid(0, totalData.size() - 1);
		QString EKLZData = totalData.join(" ").replace(QRegExp("[\r\n\t]+"), " ").replace(QRegExp(" +"), " ").toUpper().simplified();

		QRegExp regexp(QString::fromWCharArray(L"([A-Я ]+)([^A-Я]+)"));
		typedef QPair<QString, QString> TEKLZItem;
		typedef QList<TEKLZItem> TEKLZData;
		TEKLZData totalEKLZData;
		int offset = 0;

		while (regexp.indexIn(EKLZData, offset) != -1)
		{
			QStringList itemData = regexp.capturedTexts();
			totalEKLZData << TEKLZItem(itemData[1].simplified(), itemData[2].simplified());
			offset += itemData[0].size();
		}

		auto getData = [&] (const QString & aLexeme) -> QString { auto it = std::find_if(totalEKLZData.begin(), totalEKLZData.end(),
			[&](const TEKLZItem & aItem) -> bool { return aItem.first.contains(aLexeme.toUpper()); }); return (it == totalEKLZData.end()) ? "" : it->second; };

		mRNM = CFR::RNMToString(getData(QString::fromWCharArray(L"рег")).toLatin1());

		QString EKLZActivationData = getData(QString::fromWCharArray(L"актив")).split(" ")[0].replace("/", "").insert(4, "20");
		QDate EKLZActivizationDate = QDate::fromString(EKLZActivationData, CFR::DateFormat);

		setDeviceParameter(CDeviceData::EKLZ::ActivizationDate, EKLZActivizationDate.toString(CFR::DateLogFormat));
	}
}

//--------------------------------------------------------------------------------
void ShtrihSerialFR::setErrorFlags(const QByteArray & /*aCommand*/)
{
	if (isEKLZErrorCritical(mLastError))
	{
		mEKLZError = true;
	}

	if (isFMErrorCritical(mLastError))
	{
		mFMError = true;
	}
}

//--------------------------------------------------------------------------------
void ShtrihSerialFR::appendStatusCodes(ushort aFlags, TStatusCodes & aStatusCodes)
{
	TShtrihSerialFRBase::appendStatusCodes(aFlags, aStatusCodes);

	using namespace PrinterStatusCode;

	// ошибки контрольной ленты, выявленные весовым и оптическим датчиками соответственно
	bool controlWeightSensor  = (~aFlags & CShtrihFR::Statuses::WeightSensor::NoControlPaper)  && isControlWeightSensor();
	bool controlOpticalSensor = (~aFlags & CShtrihFR::Statuses::OpticalSensor::NoControlPaper) && isControlOpticalSensor();

	if (controlWeightSensor || controlOpticalSensor)
	{
		aStatusCodes.insert(Error::ControlPaperEnd);
		toLog(LogLevel::Error, QString("ShtrihFR: Control tape error, report %1")
			.arg((controlWeightSensor && controlOpticalSensor) ? "both optical & weight sensors" :
				 (controlWeightSensor ? "weight sensor" : "optical sensor")));
	}

	// рычаг контрольной ленты
	if ((~aFlags & CShtrihFR::Statuses::ControlLeverNotDropped) && isControlLeverExist())
	{
		aStatusCodes.insert(DeviceStatusCode::Error::MechanismPosition);
		toLog(LogLevel::Error, "ShtrihFR: Control lever error");
	}

	// отказы левого и правого датчиков печатающей МАТРИЧНОЙ головы принтера
	bool leftHeadSensor  = (aFlags & CShtrihFRBase::Statuses::LeftHeadSensor)  && isHeadSensorsExist();
	bool rightHeadSensor = (aFlags & CShtrihFRBase::Statuses::RightHeadSensor) && isHeadSensorsExist();

	if (leftHeadSensor || rightHeadSensor)
	{
		aStatusCodes.insert(Error::PrintingHead);
		toLog(LogLevel::Error, QString("ShtrihFR: %1 failure")
			.arg((leftHeadSensor && rightHeadSensor) ? "Left & right head sensors" :
					(leftHeadSensor ? "Left head sensor" : "Right head sensor")));
	}

	// ЭКЛЗ близка к заполнению
	if (aFlags & CShtrihFRBase::Statuses::EKLZNearEnd)
	{
		aStatusCodes.insert(FRStatusCode::Warning::EKLZNearEnd);
	}

	mEKLZ = aFlags & CShtrihFRBase::Statuses::EKLZExists;

	//TODO: надо существенно расширить перечень ошибок, учесть наличие презентера и ретрактора в принтере, ошибки ЭКЛЗ и ФП;
	//TODO: надо добавить ошибки комплектации (презентер, ретрактор, термоюнит)
}

//--------------------------------------------------------------------------------
bool ShtrihSerialFR::isHeadSensorsExist() const
{
	return (mType == CShtrihFR::Types::KKM) && (mModel == CShtrihFR::Models::ID::ATOLElvesMiniFRF);
}

//--------------------------------------------------------------------------------
/*
Если модель оснащена чековой лентой и если нельзя отключить печать на оной ленте установкой 36 поля,
то, если какой-то из датчиков операционной ленты (весовой, оптический и датчик рычага термоголовки принтера
контрольной ленты) сообщит об ошибке контрольной ленты (например, закончилась),
это придется учитывать, т.к. ФР НИЧЕГО печатать не будет. Поэтому это ошибка.
Только! для Штрих-ФР-К можно выставить это поле (что мы и делаем на инициализации), после этого
контроллер ФР не будет обращать внимание на показания датчиков. Поэтому это даже не ворнинг.
И мы поэтому будем не обращать внимание на эти датчики, в соотв. Функциях модели Штрих-ФР-К не будет.
*/
bool ShtrihSerialFR::isControlWeightSensor() const
{
	return (mType == CShtrihFR::Types::KKM) &&
	       ((mModel == CShtrihFR::Models::ID::ShtrihFRF)      ||
	        (mModel == CShtrihFR::Models::ID::ATOLFelixRF)    ||
	      //(mModel == CShtrihFR::Models::ID::ShtrihFRK)      ||
	        (mModel == CShtrihFR::Models::ID::Shtrih950K)     ||
	        (mModel == CShtrihFR::Models::ID::ShtrihFRFKazah) ||
	        (mModel == CShtrihFR::Models::ID::ShtrihFRFBelorus));
}

//--------------------------------------------------------------------------------
bool ShtrihSerialFR::isControlOpticalSensor() const
{
	return (mType == CShtrihFR::Types::KKM) &&
	       ((mModel == CShtrihFR::Models::ID::ShtrihFRF)      ||
	      //(mModel == CShtrihFR::Models::ID::ShtrihFRK)      ||
	        (mModel == CShtrihFR::Models::ID::Shtrih950K)     ||
	        (mModel == CShtrihFR::Models::ID::ShtrihFRFKazah) ||
	        (mModel == CShtrihFR::Models::ID::ShtrihFRFBelorus));
}

//--------------------------------------------------------------------------------
bool ShtrihSerialFR::isControlLeverExist() const
{
	return (mType == CShtrihFR::Types::KKM) &&
	       ((mModel == CShtrihFR::Models::ID::ShtrihFRF)      ||
	      //(mModel == CShtrihFR::Models::ID::ShtrihFRK)      ||
	        (mModel == CShtrihFR::Models::ID::ShtrihFRFKazah) ||
	        (mModel == CShtrihFR::Models::ID::ShtrihFRFBelorus));
}

//--------------------------------------------------------------------------------
bool ShtrihSerialFR::getStatus(TStatusCodes & aStatusCodes)
{
	if (!TShtrihSerialFRBase::getStatus(aStatusCodes))
	{
		return false;
	}

	QByteArray answer;
	TResult result = processCommand(CShtrihFR::Commands::GetShortStatus, &answer);

	if (!CORRECT(result))
	{
		return false;
	}
	else if (result && (answer.size() > 11))
	{
		int statusCode = CShtrihFRBase::EKLZStatusDescription[answer[11]];
		aStatusCodes.insert(statusCode);

		if (statusCode == FRStatusCode::Error::EKLZ)
		{
			mEKLZError = true;
		}

		if (isFMErrorCritical(answer[10]))
		{
			aStatusCodes.insert(FRStatusCode::Error::FiscalMemory);
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
bool ShtrihSerialFR::isEKLZErrorCritical(char aError) const
{
	return mErrorData->value(aError).type == FRError::EType::EKLZ;
}

//--------------------------------------------------------------------------------
bool ShtrihSerialFR::isFMErrorCritical(char aError) const
{
	return mErrorData->value(aError).type == FRError::EType::FM;
}

//--------------------------------------------------------------------------------
