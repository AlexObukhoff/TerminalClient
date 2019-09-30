/* @file Интерфейс драйвера PC/SC ридера. */

// Modules
#include "Hardware/Common/USBDeviceModelData.h"
#include "Hardware/CardReaders/CardReaderStatusesDescriptions.h"

// Project
#include "MifareReader.h"
#include "MifareReaderModelData.h"

using namespace SDK::Driver;

//------------------------------------------------------------------------------
MifareReader::MifareReader() : mReady(false)
{
	// данные USB-функционала
	mDetectingData->set(CUSBVendors::ACS, CMifareReader::DetectingData().getProductData());

	mPDODetecting = true;
	mPortUsing = false;

	// данные устройства
	mDeviceName = CMifareReader::UnknownModel;
	mReader.setLog(mLog);
	mStatusCodesSpecification = DeviceStatusCode::PSpecifications(new CardReaderStatusCode::CSpecifications());

	mPollingInterval = 500;
}

//--------------------------------------------------------------------------------
QStringList MifareReader::getModelList()
{
	return CMifareReader::DetectingData().getModelList(CUSBVendors::ACS);
}

//------------------------------------------------------------------------------
bool MifareReader::release()
{
	mReader.disconnect(true);

	return TMifareReader::release();
}

//------------------------------------------------------------------------------
bool MifareReader::getStatus(TStatusCodes & aStatusCodes)
{
	QStringList readerList = mReader.getReaderList();
	mReady = std::find_if(readerList.begin(), readerList.end(), [&] (const QString & systemName) -> bool
		{ return mReader.connect(systemName); }) != readerList.end();

	if (!mReady)
	{
		return false;
	}

	QByteArray answer;

	if (!mReader.communicate(CMifareReader::GetVersionRequest, answer) || (answer.size() < 30) || !answer.startsWith(CMifareReader::SAM2Header))
	{
		aStatusCodes.insert(CardReaderStatusCode::Error::SAM);
	}

	aStatusCodes.unite(mReader.getStatusCodes());
	mReady = std::find_if(aStatusCodes.begin(), aStatusCodes.end(), [&](int aStatusCode) -> bool
		{ return mStatusCodesSpecification->value(aStatusCode).warningLevel == EWarningLevel::Error; }) == aStatusCodes.end();

	return true;
}

//------------------------------------------------------------------------------
bool MifareReader::isDeviceReady() const
{
	return mReady;
}

//------------------------------------------------------------------------------
void MifareReader::eject()
{
	mReader.disconnect(true);
}

//------------------------------------------------------------------------------
bool MifareReader::reset(QByteArray & aAnswer)
{
	return mReader.reset(aAnswer);
}

//------------------------------------------------------------------------------
bool MifareReader::communicate(const QByteArray & aSendMessage, QByteArray & aReceiveMessage)
{
	return mReader.communicate(aSendMessage, aReceiveMessage);
}

//------------------------------------------------------------------------------
