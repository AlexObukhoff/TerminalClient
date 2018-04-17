/* @file Базовый ФР с портовой реализацией протокола. */

#include "PortFRBase.h"

//--------------------------------------------------------------------------------
template class PortFRBase<SerialPrinterBase<PrinterBase<SerialDeviceBase<PortPollingDeviceBase<ProtoFR>>>>>;
template class PortFRBase<PortPrinterBase<PrinterBase<TCPDeviceBase<PortPollingDeviceBase<ProtoFR>>>>>;

//--------------------------------------------------------------------------------
template <class T>
PortFRBase<T>::PortFRBase() : mLastError('\x00'), mLastCommandResult(CommandResult::OK)
{
	setInitialData();
}

//--------------------------------------------------------------------------------
template <class T>
void PortFRBase<T>::setInitialData()
{
	mProcessingErrors.clear();
	mIOMessageLogging = ELoggingType::None;

	FRBase<T>::setInitialData();
}

//---------------------------------------------------------------------------
template <class T>
bool PortFRBase<T>::checkExistence()
{
	mProcessingErrors.clear();

	return FRBase<T>::checkExistence();
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
bool PortFRBase<T>::printFiscal(const QStringList & aReceipt, const SDK::Driver::SPaymentData & aPaymentData, SDK::Driver::TFiscalPaymentData & aFPData, SDK::Driver::TComplexFiscalPaymentData & aPSData)
{
	bool result = FRBase<T>::printFiscal(aReceipt, aPaymentData, aFPData, aPSData); 

	if (mOperatorPresence)
	{
		mIOPort->close();
	}

	return result;
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
