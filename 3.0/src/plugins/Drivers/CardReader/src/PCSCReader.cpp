/* @file Реализация интерфейса работы с PC/SC API. */

// Modules
#include "SysUtils/ISysUtils.h"
#include "Hardware/Protocols/Common/ProtocolUtils.h"

// Project
#include "PCSCReader.h"
#include "PCSCReaderStatusCodes.h"

#define CHECK_SCARD_ERROR(aFunctionName, ...) handleResult(#aFunctionName, aFunctionName(__VA_ARGS__))

//--------------------------------------------------------------------------------
PCSCReader::PCSCReader():
	mContext(0),
	mCard(0),
	mActiveProtocol(SCARD_PROTOCOL_UNDEFINED)
{
	CHECK_SCARD_ERROR(SCardEstablishContext, SCARD_SCOPE_SYSTEM, NULL, NULL, &mContext);
}

//--------------------------------------------------------------------------------
PCSCReader::~PCSCReader()
{
	disconnect(false);

	if (mContext)
	{
		CHECK_SCARD_ERROR(SCardReleaseContext, mContext);
		mContext = 0;
	}
}

//--------------------------------------------------------------------------------
bool PCSCReader::reset(QByteArray & /*aAnswer*/)
{
	if (!mCard)
	{
		return false;
	}

	return CHECK_SCARD_ERROR(SCardReconnect, mCard, SCARD_SHARE_SHARED, SCARD_PROTOCOL_Tx, SCARD_UNPOWER_CARD, (LPDWORD)&mActiveProtocol);
}

//--------------------------------------------------------------------------------
QStringList PCSCReader::getReaderList()
{
	QStringList readers;
	wchar_t * mszReaders = nullptr;
	DWORD dwReaders = SCARD_AUTOALLOCATE;

	if (!CHECK_SCARD_ERROR(SCardListReadersW, mContext, SCARD_ALL_READERS, (LPTSTR)&mszReaders, &dwReaders))
	{
		return readers;
	}

	wchar_t * pReader = mszReaders;

	while ('\0' != *pReader)
	{
		readers << QString::fromWCharArray(pReader);
		// Advance to the next value.
		pReader = pReader + wcslen(pReader) + 1;
	}

	CHECK_SCARD_ERROR(SCardFreeMemory, mContext, mszReaders);

	return readers;
}

//--------------------------------------------------------------------------------
bool PCSCReader::handleResult(const QString & aFunctionName, HRESULT aResultCode)
{
	// Complete list of APDU responses
	// http://www.eftlab.co.uk/index.php/site-map/knowledge-base/118-apdu-response-list

	if (aResultCode == SCARD_S_SUCCESS)
	{
		return true;
	}

	SDeviceCodeSpecification specification = CPCSCReader::DeviceCodeSpecification[aResultCode];
	mStatusCodes.insert(specification.statusCode);

	QString log = ISysUtils::getErrorMessage(aResultCode, false);

	if (log.isEmpty())
	{
		log = specification.description;
	}

	toLog(LogLevel::Normal, aFunctionName + " returns " + log);

	return false;
}

//--------------------------------------------------------------------------------
TStatusCodes PCSCReader::getStatusCodes()
{
	TStatusCodes statusCodes(mStatusCodes);
	mStatusCodes.clear();

	return statusCodes;
}

//--------------------------------------------------------------------------------
bool PCSCReader::connect(const QString & aReaderName)
{
	// TOSO PORT_QT5 (const wchar_t *)aReaderName.utf16()???
	if (!CHECK_SCARD_ERROR(SCardConnect, mContext, (const wchar_t *)aReaderName.utf16(), SCARD_SHARE_SHARED, SCARD_PROTOCOL_Tx, &mCard, (LPDWORD)&mActiveProtocol))
	{
		return false;
	}

	if (mActiveProtocol == SCARD_PROTOCOL_T0) mPioSendPci = *SCARD_PCI_T0;
	if (mActiveProtocol == SCARD_PROTOCOL_T1) mPioSendPci = *SCARD_PCI_T1;

	return true;
}

//--------------------------------------------------------------------------------
void PCSCReader::disconnect(bool aEject)
{
	if (mCard)
	{
		CHECK_SCARD_ERROR(SCardDisconnect, mCard, aEject ? SCARD_EJECT_CARD : SCARD_LEAVE_CARD);
		mCard = 0;
	}
}

//--------------------------------------------------------------------------------
bool PCSCReader::communicate(const QByteArray & aRequest, QByteArray & aResponse)
{
	char outBuffer[4096] = {0};
	DWORD dwRecvLength = 4096; // достаточно ;)

	if (!CHECK_SCARD_ERROR(SCardTransmit, mCard, &mPioSendPci, (const byte *)aRequest.constData(), aRequest.size(), NULL, (LPBYTE)outBuffer, &dwRecvLength))
	{
		return false;
	}

	aResponse.clear();
	aResponse.append(outBuffer, dwRecvLength);

	return true;
}

//--------------------------------------------------------------------------------
bool PCSCReader::isConnected() const
{
	return mCard != 0;
}

//--------------------------------------------------------------------------------
