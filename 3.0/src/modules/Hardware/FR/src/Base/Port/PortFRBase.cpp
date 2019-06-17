/* @file Базовый ФР с портовой реализацией протокола. */

#include "PortFRBase.h"

//--------------------------------------------------------------------------------
template class PortFRBase<SerialPrinterBase<PrinterBase<SerialDeviceBase<PortPollingDeviceBase<ProtoFR>>>>>;
template class PortFRBase<PortPrinterBase<PrinterBase<TCPDeviceBase<PortPollingDeviceBase<ProtoFR>>>>>;

template QByteArray PortFRBase<SerialPrinterBase<PrinterBase<SerialDeviceBase<PortPollingDeviceBase<ProtoFR>>>>>::performStatus(TStatusCodes &, char, int);
template QByteArray PortFRBase<SerialPrinterBase<PrinterBase<SerialDeviceBase<PortPollingDeviceBase<ProtoFR>>>>>::performStatus(TStatusCodes &, const char *, int);
template QByteArray PortFRBase<PortPrinterBase<PrinterBase<TCPDeviceBase<PortPollingDeviceBase<ProtoFR>>>>>::performStatus(TStatusCodes &, char, int);
template QByteArray PortFRBase<PortPrinterBase<PrinterBase<TCPDeviceBase<PortPollingDeviceBase<ProtoFR>>>>>::performStatus(TStatusCodes &, const char *, int);

using namespace SDK::Driver;

//--------------------------------------------------------------------------------
template <class T>
PortFRBase<T>::PortFRBase() : mLastError('\x00'), mLastCommandResult(CommandResult::OK)
{
	setInitialData();

	// ошибки
	mErrorData = PErrorData(new FRError::Data());
}

//--------------------------------------------------------------------------------
template <class T>
void PortFRBase<T>::setInitialData()
{
	FRBase<T>::setInitialData();

	mProcessingErrors.clear();
	mIOMessageLogging = ELoggingType::None;
}

//---------------------------------------------------------------------------
template <class T>
bool PortFRBase<T>::checkExistence()
{
	mProcessingErrors.clear();

	return FRBase<T>::checkExistence();
}

//---------------------------------------------------------------------------
template <class T>
bool PortFRBase<T>::loadSectionNames(const TLoadSectionName & aLoadSectionName)
{
	if (!aLoadSectionName)
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to get section names due to no functional logic");
	}

	TSectionNames sectionNames;
	QByteArray data;
	int group = 0;

	while (aLoadSectionName(++group, data))
	{
		sectionNames.insert(group, mCodec->toUnicode(data.replace(ASCII::NUL, "").simplified()));
	}

	LogLevel::Enum  logLevel = sectionNames.isEmpty() ? LogLevel::Error : LogLevel::Warning;
	toLog(logLevel, mDeviceName + QString(": Failed to get name for %1 section").arg(group));

	if (!mLastCommandResult || !mLastError || !isErrorUnprocessed(mLastCommand, mLastError))
	{
		return false;
	}

	if (sectionNames.isEmpty())
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to get section names due to they are not exist");
		return false;
	}

	mProcessingErrors.pop_back();
	mLastError = 0;

	setConfigParameter(CHardwareSDK::FR::SectionNames, QVariant::fromValue<TSectionNames>(sectionNames));

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
template <class T2>
QByteArray PortFRBase<T>::performStatus(TStatusCodes & aStatusCodes, T2 aCommand, int aIndex)
{
	QByteArray data;
	TResult result = processCommand(aCommand, &data);

	if (result == CommandResult::Device)
	{
		int statusCode = getErrorStatusCode(mErrorData->value(mLastError).type);
		aStatusCodes.insert(statusCode);
	}
	else if ((result == CommandResult::Answer) || (data.size() <= aIndex))
	{
		aStatusCodes.insert(DeviceStatusCode::Warning::OperationError);
	}
	else if (result)
	{
		return data;
	}

	return !CORRECT(result) ? CFR::Result::Fail : CFR::Result::Error;
}

//--------------------------------------------------------------------------------
template <class T>
void PortFRBase<T>::makeReceipt(const QStringList & aReceipt, QStringList & aBuffer)
{
	Tags::TLexemeReceipt lexemeReceipt;
	makeLexemeReceipt(aReceipt, lexemeReceipt);

	foreach(auto collection, lexemeReceipt)
	{
		foreach(auto buffer, collection)
		{
			QString line;

			foreach(auto tagLexeme, buffer)
			{
				if (!tagLexeme.tags.contains(Tags::Type::Image))
				{
					QString data = tagLexeme.data;

					if (tagLexeme.tags.contains(Tags::Type::BarCode))
					{
						data = "Barcode <" + data + ">";
					}

					line += data;
				}
			}

			aBuffer << line;
		}
	}
}

//--------------------------------------------------------------------------------
template <class T>
void PortFRBase<T>::makeReceipt(const QStringList & aReceipt, TReceiptBuffer & aBuffer)
{
	QStringList buffer;
	makeReceipt(aReceipt, buffer);

	foreach(const QString & line, buffer)
	{
		aBuffer << mCodec->fromUnicode(line);
	}
}

//--------------------------------------------------------------------------------
template <class T>
bool PortFRBase<T>::printFiscal(const QStringList & aReceipt, const SPaymentData & aPaymentData, quint32 * aFDNumber)
{
	bool result = FRBase<T>::printFiscal(aReceipt, aPaymentData, aFDNumber); 

	if (mOperatorPresence)
	{
		mIOPort->close();
	}

	return result;
}

//--------------------------------------------------------------------------------
template <class T>
bool PortFRBase<T>::processFiscalTLVData(const TGetFiscalTLVData & aGetFiscalTLVData, TFiscalPaymentData * aFPData, TComplexFiscalPaymentData * aPSData)
{
	TProcessTLVAction fiscalTLVAction = [&] (const CFR::STLV aTLV) -> bool
	{
		if (aPSData) mFFEngine.parseSTLVData(aTLV, *aPSData);
		if (aFPData) mFFEngine.parseTLVData (aTLV, *aFPData);

		return true;
	};

	return processTLVData(aGetFiscalTLVData, fiscalTLVAction);
}

//--------------------------------------------------------------------------------
template <class T>
bool PortFRBase<T>::processTLVData(const TGetFiscalTLVData & aGetFiscalTLVData, TProcessTLVAction aAction)
{
	TResult commandResult;
	bool result = true;

	do
	{
		QByteArray data;
		CFR::STLV TLV;
		commandResult = aGetFiscalTLVData(data);

		if ((commandResult == CommandResult::Device) && isErrorUnprocessed(mLastCommand, mLastError))
		{
			mProcessingErrors.pop_back();
			mLastError = 0;

			return result;
		}
		else if (!commandResult)
		{
			return false;
		}
		else if (data.isEmpty())
		{
			toLog(LogLevel::Warning, mDeviceName + ": Failed to add fiscal field due to no data");

			continue;
		}
		else if (!mFFEngine.parseTLV(data, TLV))
		{
			result = false;
		}
		else if (!mFFData.data().contains(TLV.field))
		{
			toLog(LogLevel::Warning, mDeviceName + QString(": Failed to add fiscal field %1 due to it is unknown").arg(TLV.field));
		}
		else if (aAction && !aAction(TLV))
		{
			result = false;
		}
	}
	while (commandResult);

	return false;
}

//--------------------------------------------------------------------------------
template <class T>
bool PortFRBase<T>::printZReport(bool aPrintDeferredReports)
{
	bool result = FRBase<T>::printZReport(aPrintDeferredReports); 

	if (mOperatorPresence)
	{
		mIOPort->close();
	}

	return result;
}

//--------------------------------------------------------------------------------
template <class T>
bool PortFRBase<T>::printXReport(const QStringList & aReceipt)
{
	bool result = FRBase<T>::printXReport(aReceipt); 

	if (mOperatorPresence)
	{
		mIOPort->close();
	}

	return result;
}

//--------------------------------------------------------------------------------
template <class T>
bool PortFRBase<T>::isErrorUnprocessed(char aCommand, char aError)
{
	return mUnprocessedErrorData.data().value(QByteArray(1, aCommand)) == aError;
}

//--------------------------------------------------------------------------------
template <class T>
bool PortFRBase<T>::isErrorUnprocessed(const QByteArray & aCommand, char aError)
{
	return mUnprocessedErrorData.data().value(aCommand) == aError;
}

//--------------------------------------------------------------------------------
