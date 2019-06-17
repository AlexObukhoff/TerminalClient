/* @file ФР на базе Custom VKP-80 на протоколе АТОЛ. */

#include "AtolVKP80BasedFR.h"

//--------------------------------------------------------------------------------
template class AtolVKP80BasedFR<AtolSerialFR>;
template class AtolVKP80BasedFR<Atol2OnlineFRBase>;
template class AtolVKP80BasedFR<Atol3OnlineFRBase>;

using namespace SDK::Driver;

//--------------------------------------------------------------------------------
template <class T>
AtolVKP80BasedFR<T>::AtolVKP80BasedFR() : mEjectorMode(0)
{
	mEjectorSettings = CEjectorAtolFR::SData(CEjectorAtolFR::InitReceipt, '\x20', '\xE0', '\x30');
}

//--------------------------------------------------------------------------------
template <class T>
void AtolVKP80BasedFR<T>::setDeviceConfiguration(const QVariantMap & aConfiguration)
{
	AtolEjectorFR::setDeviceConfiguration(aConfiguration);

	mEjectorSettings.receipt = CEjectorAtolFR::InitReceipt;
	mEjectorSettings.ZReport &= CEjectorAtolFR::SpecialSettingMask;
	mEjectorSettings.autoZReport &= CEjectorAtolFR::SpecialSettingMask;

	using namespace CHardware::Printer::Settings;

	int presentationLength = getConfigParameter(PresentationLength).toInt();

	if (containsConfigParameter(PresentationLength))
	{
		if (presentationLength < CAtolVKP80BasedFR::MinPresentationLength) presentationLength = CAtolVKP80BasedFR::MinPresentationLength;
		if (presentationLength > CAtolVKP80BasedFR::MaxPresentationLength) presentationLength = CAtolVKP80BasedFR::MaxPresentationLength;

		setConfigParameter(PresentationLength, presentationLength);
	}

	bool loopEnable = getConfigParameter(Loop).toString() == CHardwareSDK::Values::Use;
	char previousPush = char(getConfigParameter(PreviousReceipt).toString() == CHardware::Printer::Values::Push);
	char presentation = char(presentationLength) % ~CEjectorAtolFR::SpecialSettingMask;

	mEjectorSettings.receipt |= (0x40 * previousPush) | (loopEnable ? presentation : 0x80);
}

//--------------------------------------------------------------------------------
template <class T>
bool AtolVKP80BasedFR<T>::setEjectorMode(char aEjectorMode)
{
	if (aEjectorMode == mEjectorMode)
	{
		return true;
	}

	if (AtolEjectorFR::setEjectorMode(aEjectorMode))
	{
		mEjectorMode = aEjectorMode;

		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------
template <class T>
bool AtolVKP80BasedFR<T>::getCommonStatus(TStatusCodes & aStatusCodes)
{
	int count = 0;

	do
	{
		aStatusCodes.clear();

		if (!AtolEjectorFR<T>::getCommonStatus(aStatusCodes))
		{
			return false;
		}

		// если есть перегрев и есть другие мех. ошибки - обнуляем перегрев
		if (aStatusCodes.contains(PrinterStatusCode::Error::Temperature) &&
		   (aStatusCodes.contains(PrinterStatusCode::Error::PaperEnd) ||
		    aStatusCodes.contains(PrinterStatusCode::Error::PrintingHead) ||
		    aStatusCodes.contains(PrinterStatusCode::Error::Cutter)))
		{
			aStatusCodes.remove(PrinterStatusCode::Error::Temperature);
		}

		// переинициализируем принтер, если:
		// 1. Принтер не находится в состоянии коллапса
		// 2. Не превышен максимум попыток переинициализации (ошибка еще не пролезла наверх)
		// 3. В ответе на запрос статуса - перегрев и при этом нет сопутствующих мех. ошибок - нет бумаги, мех. ошибка и ошибка отрезчика
		if (!mPrinterCollapse && !mProcessingErrors.contains(CAtolFR::Errors::NoPaper) &&
			!mStatusCollection.contains(PrinterStatusCode::Error::Temperature) &&
			      aStatusCodes.contains(PrinterStatusCode::Error::Temperature))
		{
			toLog(LogLevel::Normal, "Trying to reinitializing printer #" + QString::number(count));
			reInitPrinter();
		}
		else
		{
			break;
		}
	}
	while(++count < CAtolVKP80BasedFR::MaxReInitPrinterCount);

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool AtolVKP80BasedFR<T>::reInitPrinter()
{
	bool result = true;

	if (!processCommand(CAtolFR::Commands::PrinterAccess, CAtolVKP80BasedFR::ReInitialization::Data))
	{
		toLog(LogLevel::Error, "Failed to reinitialize printer");
		result = false;
	}

	SleepHelper::msleep(CAtolVKP80BasedFR::ReInitialization::Timeout);  // необходима при любом результате переинициализации

	return result;
}

//--------------------------------------------------------------------------------
template <class T>
bool AtolVKP80BasedFR<T>::processReceipt(const QStringList & aReceipt, bool aProcessing)
{
	//TODO: проверить необходимость реализации печати картинки
	char ejectorMode = mEjectorSettings.receipt | (char(mNextDocument) * mEjectorSettings.nextMask);
	setEjectorMode(ejectorMode);

	return AtolEjectorFR<T>::processReceipt(aReceipt, aProcessing);
}

//--------------------------------------------------------------------------------
template <class T>
bool AtolVKP80BasedFR<T>::performFiscal(const QStringList & aReceipt, const SPaymentData & aPaymentData, quint32 * aFDNumber)
{
	char ejectorMode = mEjectorSettings.receipt | (char(mNextDocument) * mEjectorSettings.nextMask);
	setEjectorMode(ejectorMode);

	return AtolEjectorFR<T>::performFiscal(aReceipt, aPaymentData, aFDNumber);
}

//--------------------------------------------------------------------------------
template <class T>
bool AtolVKP80BasedFR<T>::processPayout(double aAmount)
{
	setEjectorMode(mEjectorSettings.receipt);

	return AtolEjectorFR<T>::processPayout(aAmount);
}

//--------------------------------------------------------------------------------
template <class T>
bool AtolVKP80BasedFR<T>::processXReport()
{
	setEjectorMode(mEjectorSettings.receipt);

	return AtolEjectorFR<T>::processXReport();
}

//--------------------------------------------------------------------------------
template <class T>
bool AtolVKP80BasedFR<T>::performZReport(bool aPrintDeferredReports)
{
	setEjectorMode(mEjectorSettings.ZReport);

	return AtolEjectorFR<T>::performZReport(aPrintDeferredReports);
}

//--------------------------------------------------------------------------------
template <class T>
bool AtolVKP80BasedFR<T>::openDocument(EPayOffTypes::Enum aPayOffType)
{
	bool result = AtolEjectorFR<T>::openDocument(aPayOffType);
	char ejectorMode = mEjectorSettings.receipt | CEjectorAtolFR::PushLastDocument;

	if (mLocked && !setEjectorMode(ejectorMode))
	{
		toLog(LogLevel::Error, "AtolEjectorFR: Failed to set receipt processing parameters for printing fiscal document on locked FR");
	}

	return result;
}

//--------------------------------------------------------------------------------
template <class T>
void AtolVKP80BasedFR<T>::cancelDocument(bool aDocumentIsOpened)
{
	setEjectorMode(mEjectorSettings.receipt);

	AtolEjectorFR<T>::cancelDocument(aDocumentIsOpened);
}

//--------------------------------------------------------------------------------
