#ifndef LIBIDT_COMM_H_
#define LIBIDT_COMM_H_

#include "IDTDef.h"

enum COMM_RESULT {
    COMM_SUCCESS,
    COMM_CURL_COULD_NOT_BE_INITIALIZED,
    COMM_USER_NOT_VALIDATED_TO_PERFORM_THIS_FUNCTION,
    COMM_SERVER_NOT_RESPONDING,
    COMM_INVALID_ADDRESS,
    COMM_LOGIN_DENIED,
    COMM_INVALID_TLS_OPTION,
    COMM_UNABLE_TO_VALIDATE_WITH_SERVER,
    COMM_SENDER_PARAMETER_ERROR,
    COMM_RECIPIENTS_PARAMETER_ERROR,
    COMM_CC_PARAMETER_ERROR,
    COMM_BODY_PARAMETER_ERROR,
    COMM_INVALID_URL,
    COMM_FILE_NOT_FOUND_ON_SERVER,
    COMM_FILE_NOT_FOUND_ON_CLIENT,
    COMM_INVALID_HTTP_METHOD,
    COMM_INVALID_HEADERS,
    COMM_INVALID_ASYNC_VALUE,
    COMM_TIMEOUT,
    COMM_ASYNC_FAIL,

    COMM_FAILED // Generic error - should try to return another result code if possible
};

/**
 * Define the comm callback function to get FTP file transfer status <br/>
 * It should be passed as a parameter in a FTP request,
 * Signature (int, int, int) = response code, current block, total blocks
 * RESPONSE CODES:
 *		100 = FILE DOWNLOAD STARTED
 *		101 = FILE BLOCK XX OF XX RECEIVED
 *		102 = FILE DOWNLOAD COMPLETED
 *		103 = FILE DOWNLOAD TERMINATED PREMATURELY

 */
typedef void (*ftpComm_callBack)(int, int, int);

/**
 * Define the comm callback function to get the async url data <br/>
 * It should be registered using the comm_registerHTTPCallback
 */
typedef void (*httpComm_callBack)(BYTE*, int);

/**
 * Define the comm callback function to receive the V4 Protocol packets
 * received by the device from an external source (IP/USB/RS-232)
 * It should be registered using the comm_registerV4Callback,
 * Data callback will contain command, sub-command, and data from V4 packet
 */
typedef void (*v4Comm_callBack)( BYTE, BYTE, BYTE*, int);

/**
 * Register Comm HTTP Async Callback
 *
 * @param cBack – HTTP Comm callback
 */
void comm_registerHTTPCallback(httpComm_callBack cBack);

/**
 * Register External V4 Protocol commands Callback
 *
 * @param cBack – V4 Protocol Comm callback
 */
void comm_registerV4Callback(v4Comm_callBack cBack);

/**
 * Set the path to the certificates for running on VP8800 internally
 *
 * @param path – The directory that include all the certificate files including "cacert.pem".
 */
void comm_setCAPath(const char *path);

///**
// * Get the path to the certificates for running on VP8800 internally
// *
// * @return
// * 		The directory that include all the certificate files including "cacert.pem".
// */
//char* comm_getCAPath(char *path);

/**
 * Performs a manual ftp file download from a supplied URL.
 * Saved to specified path.  If no path is specified, saved to temp location and path returned
 *
 * BROADCAST NOTIFICATIONS:
 *		FILE DOWNLOAD STARTED
 *		FILE BLOCK XX OF XX RECEIVED
 *		FILE DOWNLOAD COMPLETED
 *		FILE DOWNLOAD TERMINATED PREMATURELY
 *
 *  @param url The FTP url, must start with ftp://, ftps:// and include filename
 *  @param login Login credential as null terminated string. If anonymous, “ANONYMOUS”
 *  @param password Password, if required, as a null terminated string
 *  @param path Path to save the file, if null path sent, saves to temp location.
 *  @param callback Callback to receive FTP status messages
 *
 *  @return 	SUCCESS
 * 		ERROR, USER NOT VALIDATED TO PERFORM THIS FUNCTION
 * 		ERROR, SERVER NOT RESPONDING
 * 		ERROR, INVALID URL
 * 		ERROR, UNABLE TO VALIDATE WITH SERVER
 * 		ERROR, FILE NOT FOUND ON SERVER
 */
int comm_downloadFTP(IN char *url, IN char *login, IN char *password, IN_OUT char *path, ftpComm_callBack callback);

/**
 * Performs a manual ftp file upload to a supplied URL.
 *
 * BROADCAST NOTIFICATIONS:
 *		FILE UPLOAD STARTED
 *		FILE BLOCK XX OF XX SEND
 *		FILE UPLOAD COMPLETED
 *		FILE UPLOAD TERMINATED PREMATURELY
 *
 *  @param url The FTP url, must start with ftp://, ftps:// and include filename
 *  @param login Login credential as null terminated string. If anonymous, “ANONYMOUS”
 *  @param password Password, if required, as a null terminated string
 *  @param path Path to retrieve the file
 *  @param callback Callback to receive FTP status messages
 *
 *  @return 	SUCCESS
 * 		ERROR, USER NOT VALIDATED TO PERFORM THIS FUNCTION
 * 		ERROR, SERVER NOT RESPONDING
 * 		ERROR, INVALID URL
 * 		ERROR, UNABLE TO VALIDATE WITH SERVER
 * 		ERROR, FILE NOT FOUND ON PATH
 */
int comm_uploadFTP(IN char *url, IN char *login, IN char *password, IN char *path, ftpComm_callBack callback);

/**
 * Connect to a SMTP server to send an email message.
 *
 * Supports TLS on / off and authentication either Password or None.
 * No authentication support for MD5, NTLM or Kerberos.
 *
 * @param address - The SMTP address / port, example: smtp.mail.com:25
 * @param login Login credential as null terminated string. If none, pass null string
 * @param password Password, if required, as a null terminated string. If none, pass null string
 * @param useTLS 0 = no TLS, 1 = use TLS
 * @param from - Email address of the sender as null terminated string
 * @param to - Email address of the recipients, separated by semicolon
 * @param cc - Email addresses of the cc recipients, separated by semicolon
 * @param subject - Email subject as nulled terminated string
 * @param body - Body as null terminated string
 *
 * @return	SUCCESS,
 * 			ERROR, USER NOT VALIDATED TO PERFORM THIS FUNCTION
 * 			ERROR, SERVER NOT RESPONDING
 * 			ERROR, INVALID ADDRESS
 * 			ERROR, LOGIN DENIED
 * 			ERROR, INVALID TLS OPTION
 * 			ERROR, UNABLE TO VALIDATE WITH SERVER
 * 			ERROR, SENDER PARAMETER ERROR
 * 			ERROR, RECIPIENTS PARAMETER ERROR
 * 			ERROR, CC PARAMETER ERROR -
 * 			ERROR, BODY PARAMETER ERROR -
 */
int comm_sendEmail(IN const char *address, IN const char *login, IN const char *password, IN int useTLS, IN const char *from, IN const char *to, IN const char *cc, IN const char *subject, IN const char *body);

/**
 * HTTP Request
 *
 *
 * @param url – valid URL as null terminated string. Must begin with http:// or https://
 * @param method – Valid Values: "GET", "POST" or "HEAD", as null terminated strings.
 * @param timeout - timeout, in seconds
 * @param headers - Null terminated string pairs for header, each separated by 0x1C FS
 *                  example "Content-Type=application/json<FS>Content-Length=250\0"
 * @param headerCount = Number of header entries
 * @param body - data body
 * @param bodyLen - Length of body
 * @param async - 1 = send async, 0 = send synchronous
 * @param callback - Callback function for async request
 *
 * @return COMM_SUCCESS
 *  ERROR, COMM_INVALID_URL
 *  ERROR, COMM_INVALID_HTTP_METHOD
 *  ERROR, COMM_INVALID_HEADERS
 *  ERROR, COMM_BODY_PARAMETER_ERROR
 *  ERROR, COMM_INVALID_ASYNC_VALUE
 *  ERROR, COMM_TIMEOUT
 *  ERROR, COMM_UNABLE_TO_VALIDATE_WITH_SERVER
 *  ERROR, COMM_FAILED
 *  ERROR, COMM_ASYNC_FAIL
 *  ERROR, COMM_CURL_COULD_NOT_BE_INITIALIZED
 *
 */
int comm_httpRequest(IN char *url, IN char *method, IN int timeout, IN char *headers, IN int headerCount,  IN BYTE *body, IN int bodyLen, IN int async, httpComm_callBack callback);

/**
 * Sends a V4 packet to the connected OS / host.
 *
 * @param command - The command byte
 * @param subCommand - The sub command byte
 * @param data - The data to send
 * @param dataLen - The data's length
 */
void comm_sendV4Packet(IN BYTE command, IN BYTE subCommand, IN BYTE *data, IN int bodyLen);

/**
 * If device is USB-HOST, check to see if USB client device exists on bus.
 *
 * @param vid - Device vendor ID
 * @param pid - Device product ID
 * @return 0: Device fonud on USB, 1: Device not found on USB
 */
int comm_usbDeviceExists(IN int vid, IN int pid);

/**
 * If device is USB-HOST, open USB channel.
 *
 * @param vid - Device vendor ID
 * @param pid - Device product ID
 * @param deviceID - Represents the assigned ID to the device
 * @return 0: Success, 1: Failed
 */
int comm_usbOpenDevice(IN int vid, IN int pid, OUT BYTE deviceID);

/**
 * If device is USB-HOST, close USB channel.
 *
 * @param deviceID - Represents the assigned ID to the device
 * @return 0: Success, 1: Failure
 */
int comm_usbCloseDevice(IN BYTE deviceID);

/**
 * If device is USB-HOST, close all USB channels.
 *
 * @return 0: Success, 1: Failed
 */
int comm_usbCloseAllDevices();

/**
 * Write data to USB device.
 *
 * @param deviceID - Represents the assigned ID to the device
 * @param data - Data to write
 * @param dataLen - Length of the data to write
 * @return 0: Success, 1: Failed
 */
int comm_usbWriteData(IN BYTE deviceID, IN BYTE *data, IN int dataLen);

/**
 * Read data from USB device. Blocking function.
 *
 * @param deviceID - Represents the assigned ID to the device
 * @param timeout - Timeout, in milliseconds, to wait for USB data
 * @param data - Will hold data read from USB
 * @param dataLen - Length of the data read
 *
 * @return 0: Success, 1: Failed
 */
int comm_usbReadData(IN BYTE deviceID, IN int timeout, OUT BYTE *data, OUT int dataLen);

/**
 * Check to see if a RS-232 port is available to establish a new connection.
 *
 * @param port - Com port
 *
 * @return 0: Com port found and available, 1: com port not available
 */
int comm_comPortExists(IN int port);

/**
 * Set RS-232 port settings
 *
 * @param port - Com port
 * @param baud - Baud rate
 * @param data - Data, 8 or 7 bits
 * @param pairty - Parity, odd, even, mark or space
 * @param stop - 1, 1.5, or 2 bit
 * @param flow - Xon / Xoff, hardware, or none
 */
int comm_comPortSettings(IN int port, IN int baud, IN int data, IN int parity, IN int stop, IN int flow);

/**
 * Open com port as host.
 *
 * @param port - Com port. Target COM port as an integer value
 * @param baud - Baud rate. Target baud rate as an integer value
 *                - 9600
 *                - 14400
 *                - 19200
 *                - 38400
 *                - 57600
 *                - 115200
 * @param data - Data as integer, values 8 or 7
 * @param parity - Parity as integer value
 *                - 0 = odd
 *                - 1 = even
 *                - 2 = mark
 *                - 3 = space
 * @param stop - Stop as integer value
 *                - 0 = 1.5 bit
 *                - 1 = 1 bit
 *                - 2 = 2 bit
 * @param flow - Flow as integer value
 *                - 0 = Xon / Xoff
 *                - 1 = hardware
 *                - 2 = none
 *
 * @return 0: Success, 1: Failed
 */
int comm_comPortOpen(IN int port, IN int baud, IN int data, IN int parity, IN int stop, IN int flow);

/**
 * Close Com port as host.
 *
 * @param port - Com port
 *
 * @return 0: Success, 1: Failed
 */
int comm_comPortClose(IN int port);

/**
 * Write data to com port as host.
 *
 * @param port - Com port
 * @param data - Data to write
 * @param dataLen - Length of data to write
 *
 * @return 0: Success, 1: Failed
 */
int comm_comPortWriteData(IN int port, IN BYTE *data, IN int dataLen);

/**
 * Read data from com port as host. Blocking function.
 *
 * @param port - Com port
 * @param timeout - Timeout to wait, in milliseconds
 * @param data - Will hold data read from USB
 * @param dataLen - Length of data read
 *
 * @return 0: Success, 1: Failed
 */
int comm_comPortReadData(IN int port, IN int timeout, OUT BYTE *data, OUT int *dataLen);

#endif /* LIBIDT_COMM_H_ */

/*! \file libIDT_Comm.h
 \brief Comm Handling Library.

 Comm Library API methods.
 */

/*! \def IN
  INPUT parameter.
 */

/*! \def OUT
  OUTPUT parameter.
 */

/*! \def IN_OUT
  INPUT / OUTPUT PARAMETER.
 */

/*! \def _DATA_BUF_LEN
 DATA BUFFER LENGTH
 */
