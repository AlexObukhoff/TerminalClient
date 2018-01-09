/* @file Описатель кодов состояний PC-SC ридера. */
#pragma once

// windows
#ifdef __APPLE__
#include <PCSC/winscard.h>
#include <PCSC/wintypes.h>
#else
#include <winscard.h>
#pragma comment (lib, "Winscard.lib")
#endif

// Modules
#include "Hardware/Common/DeviceCodeSpecificationBase.h"
#include "Hardware/CardReaders/CardReaderStatusCodes.h"

//------------------------------------------------------------------------------
namespace CPCSCReader
{
	/// Статусы.
	class CDeviceCodeSpecification : public DeviceCodeSpecificationBase<HRESULT>
	{
	public:
		CDeviceCodeSpecification()
		{
			appendStatus(SCARD_S_SUCCESS, DeviceStatusCode::OK::OK);
			appendStatus(SCARD_F_INTERNAL_ERROR, DeviceStatusCode::Error::Unknown, "An internal consistency check failed");
			appendStatus(SCARD_E_CANCELLED, DeviceStatusCode::OK::OK, "The action was cancelled by a SCardCancel request");
			appendStatus(SCARD_E_INVALID_HANDLE, DeviceStatusCode::Warning::OperationError, "The supplied handle was invalid");
			appendStatus(SCARD_E_INVALID_PARAMETER, DeviceStatusCode::Warning::OperationError, "One or more of the supplied parameters could not be properly interpreted");
			appendStatus(SCARD_E_INVALID_TARGET, DeviceStatusCode::Warning::OperationError, "Registry startup information is missing or invalid");
			appendStatus(SCARD_E_NO_MEMORY, CardReaderStatusCode::SCOperarionError::Memory, "Not enough memory available to complete this command");
			appendStatus(SCARD_F_WAITED_TOO_LONG, DeviceStatusCode::Warning::OperationError, "An internal consistency timer has expired");
			appendStatus(SCARD_E_INSUFFICIENT_BUFFER, DeviceStatusCode::Warning::Developing, "The data buffer to receive returned data is too small for the returned data");
			appendStatus(SCARD_E_UNKNOWN_READER, DeviceStatusCode::Warning::OperationError, "The specified reader name is not recognized");
			appendStatus(SCARD_E_TIMEOUT, DeviceStatusCode::Warning::OperationError, "The user-specified timeout value has expired");
			appendStatus(SCARD_E_SHARING_VIOLATION, CardReaderStatusCode::SCOperarionError::Unknown, "The smart card cannot be accessed because of other connections outstanding");
			appendStatus(SCARD_E_NO_SMARTCARD, CardReaderStatusCode::Error::SAM, "The operation requires a smart card, but no smart card is currently in the device");
			appendStatus(SCARD_E_UNKNOWN_CARD, CardReaderStatusCode::SCOperarionError::Unknown, "The specified smart card name is not recognized");
			appendStatus(SCARD_E_CANT_DISPOSE, DeviceStatusCode::Warning::ThirdPartyDriver, "The system could not dispose of the media in the requested manner");
			appendStatus(SCARD_E_PROTO_MISMATCH, CardReaderStatusCode::SCOperarionError::Unknown, "The requested protocols are incompatible with the protocol currently in use with the smart card");
			appendStatus(SCARD_E_NOT_READY, DeviceStatusCode::Error::Unknown, "The reader or smart card is not ready to accept commands");
			appendStatus(SCARD_E_INVALID_VALUE, CardReaderStatusCode::SCOperarionError::Unknown, "One or more of the supplied parameters values could not be properly interpreted");
			appendStatus(SCARD_E_SYSTEM_CANCELLED, DeviceStatusCode::Warning::ThirdPartyDriver, "The action was cancelled by the system, presumably to log off or shut down");
			appendStatus(SCARD_F_COMM_ERROR, DeviceStatusCode::Warning::ThirdPartyDriver, "An internal communications error has been detected");
			appendStatus(SCARD_F_UNKNOWN_ERROR, DeviceStatusCode::Warning::ThirdPartyDriver, "An internal error has been detected, but the source is unknown");
			appendStatus(SCARD_E_INVALID_ATR, DeviceStatusCode::Warning::OperationError, "An ATR obtained from the registry is not a valid ATR string");
			appendStatus(SCARD_E_NOT_TRANSACTED, DeviceStatusCode::Warning::ThirdPartyDriver, "An attempt was made to end a non-existent transaction");
			appendStatus(SCARD_E_READER_UNAVAILABLE, DeviceStatusCode::Error::Unknown, "The specified reader is not currently available for use");
			appendStatus(SCARD_P_SHUTDOWN, DeviceStatusCode::Warning::ThirdPartyDriver, "The operation has been aborted to allow the server application to exit");
			appendStatus(SCARD_E_PCI_TOO_SMALL, DeviceStatusCode::Error::ThirdPartyDriver, "The PCI Receive buffer was too small");
			appendStatus(SCARD_E_READER_UNSUPPORTED, DeviceStatusCode::Warning::ThirdPartyDriver, "The reader driver does not meet minimal requirements for support");
			appendStatus(SCARD_E_DUPLICATE_READER, DeviceStatusCode::Warning::ThirdPartyDriver, "The reader driver did not produce a unique reader name");
			appendStatus(SCARD_E_CARD_UNSUPPORTED, CardReaderStatusCode::SCOperarionError::Unknown, "The smart card does not meet minimal requirements for support");
			appendStatus(SCARD_E_NO_SERVICE, CardReaderStatusCode::SCOperarionError::Unknown, "The smart card resource manager is not running");
			appendStatus(SCARD_E_SERVICE_STOPPED, CardReaderStatusCode::SCOperarionError::Unknown, "The smart card resource manager has shut down");
			appendStatus(SCARD_E_UNEXPECTED, CardReaderStatusCode::SCOperarionError::Unknown, "An unexpected card error has occurred");
			appendStatus(SCARD_E_ICC_INSTALLATION, CardReaderStatusCode::SCOperarionError::Unknown, "No primary provider can be found for the smart card");
			appendStatus(SCARD_E_ICC_CREATEORDER, CardReaderStatusCode::SCOperarionError::Unknown, "The requested order of object creation is not supported");
			appendStatus(SCARD_E_UNSUPPORTED_FEATURE, CardReaderStatusCode::SCOperarionError::Unknown, "This smart card does not support the requested feature");
			appendStatus(SCARD_E_DIR_NOT_FOUND, CardReaderStatusCode::SCOperarionError::Unknown, "The identified directory does not exist in the smart card");
			appendStatus(SCARD_E_FILE_NOT_FOUND, CardReaderStatusCode::SCOperarionError::Unknown, "The identified file does not exist in the smart card");
			appendStatus(SCARD_E_NO_DIR, CardReaderStatusCode::SCOperarionError::Unknown, "The supplied path does not represent a smart card directory");
			appendStatus(SCARD_E_NO_FILE, CardReaderStatusCode::SCOperarionError::Unknown, "The supplied path does not represent a smart card file");
			appendStatus(SCARD_E_NO_ACCESS, CardReaderStatusCode::SCOperarionError::Unknown, "Access is denied to this file");
			appendStatus(SCARD_E_WRITE_TOO_MANY, CardReaderStatusCode::SCOperarionError::Memory, "The smart card does not have enough memory to store the information");
			appendStatus(SCARD_E_BAD_SEEK, CardReaderStatusCode::SCOperarionError::Unknown, "There was an error trying to set the smart card file object pointer");
			appendStatus(SCARD_E_INVALID_CHV, CardReaderStatusCode::SCOperarionError::Unknown, "The supplied PIN is incorrect");
			appendStatus(SCARD_E_UNKNOWN_RES_MNG, DeviceStatusCode::Warning::ThirdPartyDriver, "An unrecognized error code was returned from a layered component");
			appendStatus(SCARD_E_NO_SUCH_CERTIFICATE, CardReaderStatusCode::SCOperarionError::Sertificate, "The requested certificate does not exist");
			appendStatus(SCARD_E_CERTIFICATE_UNAVAILABLE, CardReaderStatusCode::SCOperarionError::Sertificate, "The requested certificate could not be obtained");
			appendStatus(SCARD_E_NO_READERS_AVAILABLE, DeviceStatusCode::Error::NotAvailable, "Cannot find a smart card reader");
			appendStatus(SCARD_E_COMM_DATA_LOST, CardReaderStatusCode::SCOperarionError::Unknown, "A communications error with the smart card has been detected. Retry the operation");
			appendStatus(SCARD_E_NO_KEY_CONTAINER, CardReaderStatusCode::SCOperarionError::Unknown, "The requested key container does not exist on the smart card");
			appendStatus(SCARD_E_SERVER_TOO_BUSY, CardReaderStatusCode::Error::SAM, "The smart card resource manager is too busy to complete this operation");
			appendStatus(SCARD_W_UNSUPPORTED_CARD, CardReaderStatusCode::Error::SAM, "The reader cannot communicate with the card, due to ATR string configuration conflicts");
			appendStatus(SCARD_W_UNRESPONSIVE_CARD, CardReaderStatusCode::Error::SAM, "The smart card is not responding to a reset");
			appendStatus(SCARD_W_UNPOWERED_CARD, CardReaderStatusCode::Error::SAM, "Power has been removed from the smart card, so that further communication is not possible");
			appendStatus(SCARD_W_RESET_CARD, CardReaderStatusCode::Error::SAM, "The smart card has been reset, so any shared state information is invalid");
			appendStatus(SCARD_W_REMOVED_CARD, CardReaderStatusCode::Error::SAM, "The smart card has been removed, so further communication is not possible");
			appendStatus(SCARD_W_SECURITY_VIOLATION, CardReaderStatusCode::SCOperarionError::Security, "Access was denied because of a security violation");
			appendStatus(SCARD_W_WRONG_CHV, DeviceStatusCode::Warning::OperationError, "The card cannot be accessed because the wrong PIN was presented");
			appendStatus(SCARD_W_CHV_BLOCKED, CardReaderStatusCode::SCOperarionError::Security, "The card cannot be accessed because the maximum number of PIN entry attempts has been reached");
			appendStatus(SCARD_W_EOF, CardReaderStatusCode::Error::SAM, "The end of the smart card file has been reached");
			appendStatus(SCARD_W_CANCELLED_BY_USER, DeviceStatusCode::Warning::OperationError, "The action was cancelled by the user");
			appendStatus(SCARD_W_CARD_NOT_AUTHENTICATED, CardReaderStatusCode::Error::SAM, "No PIN was presented to the smart card");
		}
	};

	static CDeviceCodeSpecification DeviceCodeSpecification;
}

//--------------------------------------------------------------------------------
