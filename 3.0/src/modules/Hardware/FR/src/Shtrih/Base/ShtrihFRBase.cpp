/* @file ФР семейства Штрих на порту. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// Project
#include "ShtrihFRBase.h"

using namespace SDK::Driver;

//TODO: реализовать тег Bold командой Печать жирной строки (12h)

//--------------------------------------------------------------------------------
template class ShtrihFRBase<ShtrihSerialFRBase>;
template class ShtrihFRBase<ShtrihTCPFRBase>;

//--------------------------------------------------------------------------------
template<class T>
ShtrihFRBase<T>::ShtrihFRBase()
{
	// параметры семейства ФР
	mLineFeed = false;
	setConfigParameter(CHardware::Printer::FeedingAmount, 6);

	// данные команд
	using namespace CShtrihFR::Commands;

	mCommandData.add(GetLongStatus, 6 * 1000);
	mCommandData.add(ExtentionPrinting, 6 * 1000);
	mCommandData.add(CancelDocument, 6 * 1000);
	mCommandData.add(ZReport, 30 * 1000);
	mCommandData.add(ZReportInBuffer, 30 * 1000);
	mCommandData.add(PrintDeferredZReports, 6 * 1000);
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihFRBase<T>::updateParameters()
{
	if (!ProtoShtrihFR<T>::updateParameters())
	{
		return false;
	}

	getZReportQuantity();

	return true;
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihFRBase<T>::isConnected()
{
	EPortTypes::Enum portType = mIOPort->getType();

	if (portType == EPortTypes::COMEmulator)
	{
		toLog(LogLevel::Error, mDeviceName + ": Port type is COM-emulator");
		return false;
	}

	QByteArray answer;
	TResult result = processCommand(CShtrihFR::Commands::GetModelInfo, &answer);

	if (!CORRECT(result))
	{
		return false;
	}

	mType = CShtrihFR::Types::NoType;
	mModel = CShtrihFR::Models::ID::NoModel;
	CShtrihFR::Models::CData modeData;

	bool isLoading = !isAutoDetecting();
	QString modelName = getConfigParameter(CHardwareSDK::ModelName).toString();
	auto modelDataIt = std::find_if(modeData.data().begin(), modeData.data().end(), [&modelName] (const CShtrihFR::SModelData & data) -> bool { return data.name == modelName; });

	if (result && (answer.size() > 6))
	{
		mType = uchar(answer[2]);
		mModel = uchar(answer[6]);
	}
	else if (!result && isLoading && !modelName.isEmpty() && (modelDataIt != modeData.data().end()))
	{
		mType = CShtrihFR::Types::KKM;
		mModel = modelDataIt.key();
	}

	answer = answer.mid(7).replace(ASCII::NUL, "");

	QString modelId = mCodec->toUnicode(answer);
	modelDataIt = std::find_if(modeData.data().begin(), modeData.data().end(), [&modelName, &modelId] (const CShtrihFR::SModelData & data) -> bool
		{ return !modelId.isEmpty() && !data.id.isEmpty() && modelId.contains(data.id, Qt::CaseInsensitive); });

	if (modelDataIt != modeData.data().end())
	{
		mType = CShtrihFR::Types::KKM;
		mModel = modelDataIt.key();
	}

	mVerified = false;
	mDeviceName = CShtrihFR::Models::Default;
	mModelData = modeData[mModel];
	mCanProcessZBuffer = mModelData.ZBufferSize;

	if (mType == CShtrihFR::Types::KKM)
	{
		mVerified = mModelData.verified;
		mDeviceName = mModelData.name;
		setConfigParameter(CHardware::Printer::FeedingAmount, mModelData.feed);
	}
	else if ((mType == CShtrihFR::Types::Printer) && (mModel == CShtrihFR::Models::ID::Shtrih500))
	{
		mDeviceName = "Shtrih-M Shtrih-500";
		mFontNumber = CShtrihFR::Fonts::Shtrih500;
	}

	if (mDeviceName == CShtrihFR::Models::Default)
	{
		toLog(LogLevel::Error, QString("ShtrihFR: Unknown model number = %1 or type = %2").arg(mModel).arg(mType));
	}

	mModelCompatibility = mSupportedModels.contains(mDeviceName);

	if (CShtrihFR::FRParameters::Fields.data().contains(mModel))
	{
		mParameters = CShtrihFR::FRParameters::Fields[mModel];
	}

	return true;
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihFRBase<T>::getStatus(TStatusCodes & aStatusCodes)
{
	if (!ProtoShtrihFR<T>::getStatus(aStatusCodes))
	{
		return false;
	}

	if (canGetZReportQuantity() && !mWhiteSpaceZBuffer)
	{
		aStatusCodes.insert(FRStatusCode::Warning::ZBufferFull);
	}

	return true;
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihFRBase<T>::performZReport(bool aPrintDeferredReports)
{
	toLog(LogLevel::Normal, "ShtrihFR: Begin processing Z-report");
	bool printDeferredZReportsOK = true;

	bool ZBufferOverflow = mZBufferOverflow;
	bool printZReportOK = execZReport(false);

	if (printZReportOK && ZBufferOverflow)
	{
		mZBufferFull = true;
	}

	if (aPrintDeferredReports && mCanProcessZBuffer)
	{
		printDeferredZReportsOK = printDeferredZReports();
		getZReportQuantity();

		if (printDeferredZReportsOK || (mWhiteSpaceZBuffer > 0))
		{
			mZBufferFull = false;
		}
	}

	return (printDeferredZReportsOK && aPrintDeferredReports) || printZReportOK;
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihFRBase<T>::printDeferredZReports()
{
	toLog(LogLevel::Normal, "ShtrihFR: Begin printing deferred Z-reports");

	bool printDeferredZReportSuccess = processCommand(CShtrihFR::Commands::PrintDeferredZReports);
	SleepHelper::msleep(CShtrihFR::Pause::ZReportPrintingEnd);

	if (!printDeferredZReportSuccess)
	{
		toLog(LogLevel::Error, "ShtrihFR: Failed to print deferred Z-reports");
		return false;
	}

	SleepHelper::msleep(CShtrihFR::Pause::ZReportPrintingEnd);
	waitForPrintingEnd();

	return true;
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihFRBase<T>::prepareZReport(bool aAuto, QVariantMap & aData)
{
	bool needCloseSession = mMode == CShtrihFR::InnerModes::NeedCloseSession;

	if (aAuto)
	{
		if (mOperatorPresence)
		{
			toLog(LogLevel::Error, mDeviceName + ": Failed to process auto-Z-report due to presence of the operator.");
			mNeedCloseSession = mNeedCloseSession || needCloseSession;

			return false;
		}

		if (!mIsOnline && !mCanProcessZBuffer)
		{
			toLog(LogLevel::Normal, mDeviceName + ": FR has no buffer, auto-Z-report is not available");
			mNeedCloseSession = mNeedCloseSession || needCloseSession;

			return false;
		}
	}

	return ProtoShtrihFR<T>::prepareZReport(aAuto, aData);
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihFRBase<T>::execZReport(bool aAuto)
{
	QVariantMap outData;

	if (!prepareZReport(aAuto, outData))
	{
		return false;
	}

	char command = aAuto ? CShtrihFR::Commands::ZReportInBuffer : CShtrihFR::Commands::ZReport;
	mNeedCloseSession = false;
	bool success = processCommand(command);

	if (success)
	{
		mZBufferOverflow = false;
		SleepHelper::msleep(CShtrihFR::Pause::ZReportPrintingEnd);
		success = waitForChangeZReportMode();
	}

	if (getLongStatus())
	{
		mNeedCloseSession = mMode == CShtrihFR::InnerModes::NeedCloseSession;
	}

	if (success)
	{
		emit FRSessionClosed(outData);
	}

	if (command == CShtrihFR::Commands::ZReportInBuffer)
	{
		getZReportQuantity();
	}

	toLog(success ? LogLevel::Normal : LogLevel::Error, success ?
		"ShtrihFR: Z-report is successfully processed" :
		"ShtrihFR: error in processing Z-report");

	return success;
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihFRBase<T>::canGetZReportQuantity()
{
	return mFiscalized && (mModel == CShtrihFR::Models::ID::ShtrihComboFRK) && isFiscalReady(false, EFiscalPrinterCommand::ZReport);
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihFRBase<T>::getZReportQuantity()
{
	if (!canGetZReportQuantity())
	{
		return false;
	}

	QByteArray ZReportQuantity;

	if (!getRegister(CShtrihFR::Registers::ZReportsQuantity, ZReportQuantity))
	{
		toLog(LogLevel::Error, "ShtrihFR: Failed to get Z-report quantity");
		return false;
	}

	int count = ZReportQuantity.toInt();
	toLog(LogLevel::Error, "ShtrihFR: Z-report count = " + QString::number(count));

	mWhiteSpaceZBuffer = mModelData.ZBufferSize - count;

	if (mWhiteSpaceZBuffer < 0)
	{
		mWhiteSpaceZBuffer = 0;
	}

	return true;
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihFRBase<T>::waitForChangeZReportMode()
{
	QTime clockTimer;
	clockTimer.start();

	TStatusCodes errorStatusCodes = getErrorFRStatusCodes();

	do
	{
		QTime clock = QTime::currentTime();

		// 3.1. запрашиваем статус
		TStatusCodes statusCodes;

		if (getStatus(statusCodes))
		{
			// 3.2. анализируем режим и подрежим, если печать Z-отчета окончена - выходим
			if (!statusCodes.intersect(errorStatusCodes).isEmpty())
			{
				toLog(LogLevel::Error, "ShtrihFR: Failed to print Z-report, exit!");
				return false;
			}

			if ((mSubmode == CShtrihFR::InnerSubmodes::PaperEndPassive) ||
			    (mSubmode == CShtrihFR::InnerSubmodes::PaperEndActive))
			{
				// 3.3. подрежим - закончилась бумага
				return false;
			}
			//если режим или подрежим - печать или печать отчета или Z-отчета, то
			else if ((mMode == CShtrihFR::InnerModes::DataEjecting) ||
			         (mMode == CShtrihFR::InnerModes::PrintFullZReport) ||
			         (mMode == CShtrihFR::InnerModes::PrintEKLZReport) ||
			         (mSubmode == CShtrihFR::InnerSubmodes::PrintingFullReports) ||
			         (mSubmode == CShtrihFR::InnerSubmodes::Printing))
			{
				toLog(LogLevel::Normal, "ShtrihFR: service Z-report process, wait...");
			}
			else if ((mMode == CShtrihFR::InnerModes::SessionClosed) ||
			         (mSubmode == CShtrihFR::InnerSubmodes::PaperOn))
			{
				// 3.3. режим - тот, который ожидаем, если Z-отчет допечатался, все хорошо
				return true;
			}
			else
			{
				// 3.4. режим не тот, который ожидаем в соответствии с протоколом, выходим с ошибкой
				toLog(LogLevel::Error, QString("ShtrihFR: Z-report, unknown mode.submode = %1.%2").arg(int(mMode)).arg(int(mSubmode)));
				return false;
			}

			//спим до периода опроса
			int sleepTime = CShtrihFR::Interval::ReportPoll - abs(clock.msecsTo(QTime::currentTime()));

			if (sleepTime > 0)
			{
				SleepHelper::msleep(sleepTime);
			}
		}
	}
	while(clockTimer.elapsed() < CShtrihFR::Timeouts::MaxZReportNoAnswer);

	toLog(LogLevel::Normal, "ShtrihFR: Timeout for Z-report.");

	//вышли по таймауту, значит, не смогли дождаться нужного режима/подрежима
	return false;
}

//--------------------------------------------------------------------------------
template<class T>
bool ShtrihFRBase<T>::processAnswer(const QByteArray & aCommand)
{
	if (ProtoShtrihFR<T>::processAnswer(aCommand))
	{
		return true;
	}

	if (mLastError == CShtrihFR::Errors::ChequeBufferOverflow)
	{
		mZBufferOverflow = mCanProcessZBuffer;
	}

	return false;
}

//--------------------------------------------------------------------------------
