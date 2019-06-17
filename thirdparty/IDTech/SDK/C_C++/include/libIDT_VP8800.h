#ifndef __LIBIDT_VP8800_H___
#define __LIBIDT_VP8800_H___


#define IN
#define OUT
#define IN_OUT
#include <stdarg.h>
#include "IDTDef.h"

/**
 * Define the USB hot-plug callback function to monitor the info when plug in/out the reader. <br/>
 * It should be registered using the registerHotplugCallBk,
 * The first integer parameter is device type, and the second integer parameter is either 0: Device Plugged Out or 1: Device Plugged In
 */
typedef void (*pMessageHotplug)(int, int);

/**
 * Define the send command callback function to monitor the sending command into the reader. <br/>
 * It should be registered using the registerLogCallBk,
 */
typedef void (*pSendDataLog)(unsigned char *, int);

/**
 * Define the read response callback function to monitor the reading response from the reader. <br/>
 * It should be registered using the registerLogCallBk,
 */
typedef void (*pReadDataLog)(unsigned char *, int);

/**
 * Define the EMV callback function to get the transaction message/data/result. <br/>
 * It should be registered using the emv_registerCallBk,
 */
typedef void (*pEMV_callBack)(int, int,unsigned char *, int,IDTTransactionData*,EMV_Callback*,int);

/**
 * Define the MSR callback function to get the MSR card data <br/>
 * It should be registered using the msr_registerCallBk,
 */
typedef void (*pMSR_callBack)(int, IDTMSRData);

/**
 * Define the MSR callback function to get the MSR card data <br/>
 * It should be registered using the msr_registerCallBk,
 */
typedef void (*pMSR_callBackp)(int, IDTMSRData *);

/**
 * Define the PINPad callback function to get the input PIN Pad data <br/>
 * It should be registered using the pin_registerCallBk,
 */
typedef void (*pPIN_callBack)(int, IDTPINData *);

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
 * Define the log callback function to receive log messages.
 */
typedef void (*pLog_callback)(BYTE, char*);


#ifdef __cplusplus
extern "C" {
#endif

/**
 *To register the USB HID hot-plug callback function which implemented in the application to monitor the hotplug message from the SDK.
 */
void registerHotplugCallBk(pMessageHotplug pMsgHotplug);

/**
 *To register the log callback function which implemented in the application to monitor sending/reading data between application and reader.
 */
void registerLogCallBk(pSendDataLog pFSend, pReadDataLog pFRead);

/**
 *To register the emv callback function to get the EMV processing response.
 */
void emv_registerCallBk(pEMV_callBack pEMVf);

/**
 *To register the msr callback function to get the MSR card data.
 */
void msr_registerCallBk(pMSR_callBack pMSRf);

/**
 *To register the msr callback function to get the MSR card data pointer.
 */
void msr_registerCallBkp(pMSR_callBackp pMSRf);

/**
 *To register the ctls callback function to get the MSR card data.
 */
void ctls_registerCallBk(pMSR_callBack pCTLSf);

/**
 *To register the ctls callback function to get the MSR card data pointer.
 */
void ctls_registerCallBkp(pMSR_callBackp pCTLSf);

/**
 *To register the pin callback function to get the PINPad data.
 */
void pin_registerCallBk(pPIN_callBack pPINf);

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
 *To Get SDK version
 *@return return the SDK version string
 */
char *SDK_Version();

/**
 * Set the path to use when searching for ID TECH's libraries.
 * If this is not set, the libraries will be searched for with the system's default procedures.
 *
 * @param absoluteLibraryPath The absolute path to ID TECH's libraries.
 */
void setAbsoluteLibraryPath(const char *absoluteLibraryPath);

/**
 * Initial the device by USB<br/>
 * It will detect the device and trying connect. <br/>
 * The connect status can be checked by device_isConnected(). <br/>
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int device_init();
/**
  * Sets the current device to talk to <br/>
  * The connect status can be checked by device_isConnected(). <br/>
  * @param deviceType Device to connect to
  * @code
  * enum IDT_DEVICE_TYPE{
  *	IDT_DEVICE_UNKNOWN=0,
  *	IDT_DEVICE_AUGUSTA_HID,
  *	IDT_DEVICE_AUGUSTA_KB,
  *	IDT_DEVICE_AUGUSTA_S_HID,
  *	IDT_DEVICE_AUGUSTA_S_KB,
  *	IDT_DEVICE_AUGUSTA_S_TTK_HID,
  *	IDT_DEVICE_SPECTRUM_PRO,
  *	IDT_DEVICE_MINISMART_II,
  *	IDT_DEVICE_UNIPAY,
  *	IDT_DEVICE_UNIPAY_I_V,
  *	IDT_DEVICE_VP3300_AJ,
  *	IDT_DEVICE_L100,
  *	IDT_DEVICE_KIOSK_III,
  *	IDT_DEVICE_KIOSK_III_S,
  *	IDT_DEVICE_VENDI,
  *	IDT_DEVICE_VP3300_USB,
  *	IDT_DEVICE_UNIPAY_I_V_TTK,
  *	IDT_DEVICE_VP3300_BT,
  *	IDT_DEVICE_VP8800,
  *	IDT_DEVICE_NEO2,
  *	IDT_DEVICE_MINISMART_II_COM = IDT_DEVICE_NEO2+5,
  *	IDT_DEVICE_SPECTRUM_PRO_COM,
  *	IDT_DEVICE_KIOSK_III_COM,
  *	IDT_DEVICE_KIOSK_III_S_COM,
  *	IDT_DEVICE_NEO2_COM,
  *	IDT_DEVICE_MAX_DEVICES = IDT_DEVICE_NEO2_COM+5
  *	};
  *	@endcode
  * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
  */
int device_setCurrentDevice(int deviceType);
/**
 * Close the device <br/>
 */
void device_close();


/**
 * Review the return code description.<br/>
 * @param returnCode  the response result.
 * @param description
 * @retval the string for description of response result

 - 0: "no error, beginning task";
 - 1: "no response from reader";
 - 2: "invalid response data";

  - 01: " Incorrect Header Tag";
  - 02: " Unknown Command";
  - 03: " Unknown Sub-Command";
  - 04: " CRC Error in Frame";
  - 05: " Incorrect Parameter";
  - 06: " Parameter Not Supported";
  - 07: " Mal-formatted Data";
  - 08: " Timeout";
  - 0A: " Failed / NACK";
  - 0B: " Command not Allowed";
  - 0C: " Sub-Command not Allowed";
  - 0D: " Buffer Overflow (Data Length too large for reader buffer)";
  - 0E: " User Interface Event";
  - 10: " Need clear firmware(apply in boot loader only)";
  - 11: " Communication type not supported, VT-1, burst, etc. Need encrypted firmware (apply in boot loader only)";
  - 12: " Secure interface is not functional or is in an intermediate state.";
  - 13: " Data field is not mod 8";
  - 14: " Pad 0x80 not found where expected";
  - 15: " Specified key type is invalid";
  - 16: " Could not retrieve key from the SAM (InitSecureComm)";
  - 17: " Hash code problem";
  - 18: " Could not store the key into the SAM (InstallKey)";
  - 19: " Frame is too large";
  - 1A: " Unit powered up in authentication state but POS must resend the InitSecureComm command";
  - 1B: " The EEPROM may not be initialized because SecCommInterface does not make sense";
  - 1C: " Problem encoding APDU Module-Specific Status Codes ";
  - 20: " Unsupported Index (ILM) SAM Transceiver error - problem communicating with the SAM (Key Mgr)";
  - 21: " Unexpected Sequence Counter in multiple frames for single bitmap (ILM)Length error in data returned from the SAM (Key Mgr)
  - 22: " Improper bit map (ILM)";
  - 23: " Request Online Authorization";
  - 24: " ViVOCard3 raw data read successful";
  - 25: " Message index not available (ILM) ViVOcomm activate transaction card type (ViVOcomm)";
  - 26: " Version Information Mismatch (ILM)";
  - 27: " Not sending commands in correct index message index (ILM)";
  - 28: " Time out or next expected message not received (ILM)";
  - 29: " ILM languages not available for viewing (ILM)";
  - 2A: " Other language not supported (ILM)";
  - 41: " from 41 to 4F,	 Module-specific errors for Key Manager";

  - 50: " Auto-Switch OK";
  - 51: " Auto-Switch failed";
  - 70: " Antenna Error 80h Use another card";
  - 81: " Insert or swipe card";
  - 90: " Data encryption Key does not exist";
  - 91: " Data encryption Key KSN exhausted";

*/


void device_getIDGStatusCodeString(IN int returnCode, OUT char* despcrition);


/**
 * Check the device conntected status
 * @return DEVICE_DISCONNECT=0, or DEVICE_CONNECTED = 1
 */
int device_isConnected();
/**
 * Check if the device is attached to the USB port
 * The function device_init() must be called before this function.
 * @param deviceType, the device type of the USB device
 * @return 1 if the device is attached, or 0 if the device is not attached
 */
int device_isAttached(int deviceType);
/**
* Start Transaction Request
*
Authorizes the transaction for an MSR/CTLS/ICC card

The tags will be returned in the callback routine.

@param amount Transaction amount value	(tag value 9F02) - SEE IMPORTANT NOTE BELOW
@param amtOther Other amount value, if any	(tag value 9F03) - SEE IMPORTANT NOTE BELOW
@param type Transaction type (tag value 9C).
@param timeout Timeout value in seconds.
@param tags Any other tags to be included in the request.  Passed as TLV.	Example, tag 9F0C with amount 0x000000000100 would be 0x9F0C06000000000100
	If tags 9F02 (amount),9F03 (other amount), or 9C (transaction type) are included, they will take priority over these values supplied as individual parameters to this method.

@param tagsLen The length of tags data buffer.

 >>>>>IMPORTANT: parameters for amount and amtOther MUST BE PASSED AS A DOUBLE VALUE WITH DECIMAL POINT.  Example, do not pass 1, but instead pass 1.0 or 1.00. Otherwise, results will be unpredictable



* @return RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()
* Note: if auto poll is on, it will returm the error IDG_P2_STATUS_CODE_COMMAND_NOT_ALLOWED
*
* NOTE ON APPLEPAY VAS:
* To enable ApplePay VAS, first a merchant record must be defined in one of the six available index positions (1-6) using device_setMerchantRecord, then container tag FFEE06
* must be sent as part of the additional tags parameter of device_startTransaction.  Tag FFEE06 must contain tag 9F26 and 9F22, and can optionanally contain tags 9F2B and DFO1.
* Example FFEE06189F220201009F2604000000009F2B050100000000DF010101
* 9F22 = two bytes = ApplePay Terminal Applicaiton Version Number.	Hard defined as 0100 for now. (required)
* 9F26 = four bytes = ApplePay Terminal Capabilities Information (required)
*  - Byte 1 = RFU
*  - Byte 2 = Terminal Type
*  - - Bit 8 = VAS Support	(1=on, 0 = off)
*  - - Bit 7 = Touch ID Required  (1=on, 0 = off)
*  - - Bit 6 = RFU
*  - - Bit 5 = RFU
*  - - Bit 1,2,3,4
*  - - - 0 = Payment Terminal
*  - - - 1 = Transit Terminal
*  - - - 2 = Access Terminal
*  - - - 3 = Wireless Handoff Terminal
*  - - - 4 = App Handoff Terminal
*  - - - 15 = Other Terminal
*  - Byte 3 = RFU
*  - Byte 4 = Terminal Mode
*  - - 0 = ApplePay VAS OR ApplePay
*  - - 1 = ApplePay VAS AND ApplePay
*  - - 2 = ApplePay VAS ONLY
*  - - 3 = ApplePay ONLY
*  9F2B = 5 bytes = ApplePay VAS Filter.  Each byte filters for that specific merchant index  (optional)
*  DF01 = 1 byte = ApplePay VAS Protocol.  (optional)
*  - - Bit 1 : 1 = URL VAS, 0 = Full VAS
*  - - Bit 2 : 1 = VAS Beeps, 0 = No VAS Beeps
*  - - Bit 3 : 1 = Silent Comm Error, 2 = EMEA Comm Error
*  - - Bit 4-8 : RFU
*
 */

int device_startTransaction(IN double amount, IN double amtOther, IN int type, IN const int _timeout, IN BYTE *tags, IN int tagsLen);


	/**
	 * Start Transaction Request
	 *
	 Authorizes the transaction for an MSR/CTLS/ICC card

	 The tags will be returned in the callback routine.

	 @param timeout Timeout value in seconds.
	 @param tags The tags to be included in the request.  Passed as a TLV.	Example, tag 9F0C with amount 0x000000000100 would be 0x9F0C06000000000100
	 Be sure to include 9F02 (amount)and9C (transaction type).

	 @param tagsLen The length of tags data buffer.

	 >>>>>IMPORTANT: parameters for amount and amtOther MUST BE PASSED AS A DOUBLE VALUE WITH DECIMAL POINT.  Example, do not pass 1, but instead pass 1.0 or 1.00. Otherwise, results will be unpredictable



	 * @return RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()
	 * Note: if auto poll is on, it will returm the error IDG_P2_STATUS_CODE_COMMAND_NOT_ALLOWED
	 *
	 * NOTE ON APPLEPAY VAS:
	 * To enable ApplePay VAS, first a merchant record must be defined in one of the six available index positions (1-6) using device_setMerchantRecord, then container tag FFEE06
	 * must be sent as part of the additional tags parameter of device_startTransaction.  Tag FFEE06 must contain tag 9F26 and 9F22, and can optionanally contain tags 9F2B and DFO1.
	 * Example FFEE06189F220201009F2604000000009F2B050100000000DF010101
	 * 9F22 = two bytes = ApplePay Terminal Applicaiton Version Number.	Hard defined as 0100 for now. (required)
	 * 9F26 = four bytes = ApplePay Terminal Capabilities Information (required)
	 *  - Byte 1 = RFU
	 *  - Byte 2 = Terminal Type
	 *  - - Bit 8 = VAS Support	(1=on, 0 = off)
	 *  - - Bit 7 = Touch ID Required  (1=on, 0 = off)
	 *  - - Bit 6 = RFU
	 *  - - Bit 5 = RFU
	 *  - - Bit 1,2,3,4
	 *  - - - 0 = Payment Terminal
	 *  - - - 1 = Transit Terminal
	 *  - - - 2 = Access Terminal
	 *  - - - 3 = Wireless Handoff Terminal
	 *  - - - 4 = App Handoff Terminal
	 *  - - - 15 = Other Terminal
	 *  - Byte 3 = RFU
	 *  - Byte 4 = Terminal Mode
	 *  - - 0 = ApplePay VAS OR ApplePay
	 *  - - 1 = ApplePay VAS AND ApplePay
	 *  - - 2 = ApplePay VAS ONLY
	 *  - - 3 = ApplePay ONLY
	 *  9F2B = 5 bytes = ApplePay VAS Filter.  Each byte filters for that specific merchant index  (optional)
	 *  DF01 = 1 byte = ApplePay VAS Protocol.  (optional)
	 *  - - Bit 1 : 1 = URL VAS, 0 = Full VAS
	 *  - - Bit 2 : 1 = VAS Beeps, 0 = No VAS Beeps
	 *  - - Bit 3 : 1 = Silent Comm Error, 2 = EMEA Comm Error
	 *  - - Bit 4-8 : RFU
	 *
	 */

	int device_activateTransaction(IN const int _timeout, IN BYTE *tags, IN int tagsLen);

/**
* Cancel Transaction
*
Cancels the currently executing transaction.


* @return RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()

*/

int device_cancelTransaction();

/**
  * Drive Free Space
  * This command returns the free and used disk space on the flash drive.
  * @param free Free bytes available on device
  * @param used Used bytes on on device
  * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
  */
int device_getDriveFreeSpace(OUT int *free, OUT int *used);

/**
  * List Directory
  * This command retrieves a directory listing of user accessible files from the reader.
  * @param directoryName Directory Name.  If null, root directory is listed
  * @param directoryNameLen Directory Name Length.  If null, root directory is listed
  * @param recursive Included sub-directories
  * @param onSD TRUE = use flash storage
  * @directory The returned directory information
  * @directoryLen The returned directory information length
  * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
  */
int device_listDirectory(IN char *directoryName, IN int directoryNameLen, IN int recursive, IN int onSD, OUT char* directory, OUT int *directoryLen);


/**
  * Create Directory
  *  This command adds a subdirectory to the indicated path.
  * @param directoryName Directory Name.  The data for this command is a ASCII string with the
  *  complete path and directory name you want to create. You do not need to
  *  specify the root directory. Indicate subdirectories with a forward slash (/).
  * @param directoryNameLen Directory Name Length.
  * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
  */
int device_createDirectory(IN char *directoryName, IN int directoryNameLen);

/**
 * Delete Directory
 *  This command deletes an empty directory.
 * @param filename Complete path and file name of the directory you want to delete.
 *   You do not need to specify the root directory. Indicate subdirectories with a forward slash (/).
 * @param filenameLen File Name Length.
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int device_deleteDirectory(IN char *fileName, IN int fileNameLen);

/**
   * Transfer File
   *  This command transfers a data file to the reader.
   * @param fileName Filename.  The data for this command is a ASCII string with the
   *   complete path and file name you want to create. You do not need to
   *   specify the root directory. Indicate subdirectories with a forward slash (/).
   * @param filenameLen File Name Length.
   * @param file The data file.
   * @param fileLen File Length.
   * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
   */
int device_transferFile(IN char *fileName, IN int fileNameLen, IN BYTE *file, IN int fileLen);

/**
 * Delete File
 *  This command deletes a file or group of files.
 * @param filename Complete path and file name of the file you want to delete.
 *   You do not need to specify the root directory. Indicate subdirectories with a forward slash (/).
 * @param filenameLen File Name Length.
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int device_deleteFile(IN char *fileName, IN int fileNameLen);
/**
* Polls device for Firmware Version
*
* @param firmwareVersion Response returned of Firmware Version
*
* @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()
*
*/
int device_getFirmwareVersion(OUT char* firmwareVersion);

/**
* Ping Device

*
Pings the reader.  If connected, returns success.  Otherwise, returns timeout.

* @return RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()
*/
int device_pingDevice();
/**
* Control User Interface

*
Controls the User Interface:  Display, Beep, LED

	@param values Four bytes to control the user interface
	Byte[0] = LCD Message
	Messages 00-07 are normally controlled by the reader.
	- 00h: Idle Message (Welcome)
	- 01h: Present card (Please Present Card)
	- 02h: Time Out or Transaction cancel (No Card)
	- 03h: Transaction between reader and card is in the middle (Processing...)
	- 04h: Transaction Pass (Thank You)
	- 05h: Transaction Fail (Fail)
	- 06h: Amount (Amount $ 0.00 Tap Card)
	- 07h: Balance or Offline Available funds (Balance $ 0.00) Messages 08-0B are controlled by the terminal
	- 08h: Insert or Swipe card (Use Chip & PIN)
	- 09h: Try Again(Tap Again)
	- 0Ah: Tells the customer to present only one card (Present 1 card only)
	- 0Bh: Tells the customer to wait for authentication/authorization (Wait)
	- FFh: indicates the command is setting the LED/Buzzer only.
	Byte[1] = Beep Indicator
	- 00h: No beep
	- 01h: Single beep
	- 02h: Double beep
	- 03h: Three short beeps
	- 04h: Four short beeps
	- 05h: One long beep of 200 ms
	- 06h: One long beep of 400 ms
	- 07h: One long beep of 600 ms
	- 08h: One long beep of 800 ms
	Byte[2] = LED Number
	- 00h: LED 0 (Power LED) 01h: LED 1
	- 02h: LED 2
	- 03h: LED 3
	- FFh: All LEDs
	Byte[3] = LED Status
	- 00h: LED Off
	- 01h: LED On
* @return RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()
*/
int device_controlUserInterface(IN BYTE* values);
/**
  * Control Indicators
  *
  *
  * Control the reader.  If connected, returns success.  Otherwise, returns timeout.
  *
  *    @param indicator description as follows:
  *    - 00h: ICC LED
  *    - 01h: Blue MSR
  *    - 02h: Red MSR
  *    - 03h: Green MSR
  *    @param enable  TRUE = ON, FALSE = OFF
  *    @return success or error code.  Values can be parsed with device_getResponseCodeString
  *    @see ErrorCode
  */
int device_controlIndicator(IN int indicator, IN int enable);

/**
* Get current active device type
* @return :  return the device type defined as IDT_DEVICE_TYPE in the IDTDef.h
*/

int device_getCurrentDeviceType();


///**
//* Send a Command to device
//*
//* Sends a command  to the device .
//*
//* @param cmd buffer of command to execute.
//* @param cmdLen, the length of the buffer cmd.
//*
//* @param data buffer of IDG command data.
//* @param dataLen, the length of the buffer data.
//*
//* @param response Response data
//* @param respLen, the length of Response data
//
//* @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()
//*/
//
//int device_SendDataCommand(IN BYTE* cmd, IN int cmdLen, IN BYTE* data, IN int dataLen, OUT BYTE* response, IN_OUT int *respLen);

/**
* Send a Command to NEO device
*
* Sends a command  to the NEO device .
*
* @param cmd  command to execute.
* @param subCmd, sub command to execute.
*
* @param data buffer of NEO command data.
* @param dataLen, the length of the buffer data.
*
* @param response Response data
* @param respLen, the length of Response data

* @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()
*/

int device_SendDataCommandNEO(IN int cmd, IN int subCmd, IN BYTE* data, IN int dataLen, OUT BYTE* response, IN_OUT int *respLen);


//  /**
// * Start Remote Key Injection
// *
// Starts a remote key injection request with IDTech RKI servers.
// This function is reserved and not implemented.
//
// * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString
//
// */
//  int device_startRKI();


 /**
* Enable Pass Through
*
* Enables Pass Through Mode for direct communication with L1 interface (power on icc, send apdu, etc).
*
* @param enablePassThrough 1 = pass through ON, 0 = pass through OFF


* @return RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()
*/
int device_enablePassThrough(int enablePassThrough);

/**
   * Enables pass through mode for ICC. Required when direct ICC commands are required
   * (power on/off ICC, exchange APDU)
   *
   * @param data The data includes Poll Timeout, Flags, Contact Interface to Use, Beep Indicator, LED Status, and Display Strings.
   * @param dataLen length of data
   * @return success or error code.  Values can be parsed with device_getIDGStatusCodeString
   * @see ErrorCode
   */
int device_enhancedPassthrough(IN BYTE *data, IN int dataLen);

/**
* Set Merchant Record
Sets the merchant record for ApplePay VAS
*
* @param index Merchant Record index, valid values 1-6
* @param enabled Merchant Enabled/Valid flag
* @param merchantID  Merchant unique identifer registered with Apple.  Example com.idtechproducts.applePay
* @param merchantURL Merchant URL, when applicable
*
* @return RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()
*
*/

int device_setMerchantRecord(int index, int enabled, char* merchantID, char* merchantURL);

/**
 * Get Merchant Record
 *
 * Gets the merchant record for the device.
 *
 * @param index Merchant Record index, valid values 1-6
 * @param record
 *            response data from reader.
 *            Merchant Record Index: 1 byte
 *            enabled: 1 byte
 *            Merchant ID: 32 bytes
 *            Length of Merchant URL: 1 byte
 *            Merchant URL: 64 bytes
 *
 * @return success or error code.  Values can be parsed with device_getIDGStatusCodeString()
 * @see ErrorCode
 */
int device_getMerchantRecord(IN int index, OUT BYTE *record);

/**
 * Get Transaction Results
 * Gets the transaction results when the reader is functioning in "Auto Poll" mode
 *
 * @param cardData The transaction results
 *
 * @return success or error code.  Values can be parsed with device_getResponseCodeString
 * @see ErrorCode
 */
int device_getTransactionResults(IDTMSRData *cardData);

/**
  * Calibrate reference parameters
  * @param delta Delta value (0x02 standard default value)
  * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
  */
int device_calibrateParameters(BYTE delta);

/**
 * Polls device for Serial Number
 *
 * @param sNumber  Returns Serial Number
 *
 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString
 *
 */
int config_getSerialNumber(OUT char* sNumber);


/**
* Start CTLS Transaction Request
*
Authorizes the CTLS transaction for an ICC card

The tags will be returned in the callback routine.

@param amount Transaction amount value	(tag value 9F02) - SEE IMPORTANT NOTE BELOW
@param amtOther Other amount value, if any	(tag value 9F03) - SEE IMPORTANT NOTE BELOW
@param type Transaction type (tag value 9C).
@param timeout Timeout value in seconds.
@param tags Any other tags to be included in the request.  Passed as TLV stream.	Example, tag 9F0C with amount 0x000000000100 would be 0x9F0C06000000000100
	If tags 9F02 (amount),9F03 (other amount), or 9C (transaction type) are included, they will take priority over these values supplied as individual parameters to this method.

@param tagsLen The length of tags data buffer.

 >>>>>IMPORTANT: parameters for amount and amtOther MUST BE PASSED AS A DOUBLE VALUE WITH DECIMAL POINT.  Example, do not pass 1, but instead pass 1.0 or 1.00. Otherwise, results will be unpredictable



* @return RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()
* Note: if auto poll is on, it will returm the error IDG_P2_STATUS_CODE_COMMAND_NOT_ALLOWED
*
* NOTE ON APPLEPAY VAS:
* To enable ApplePay VAS, first a merchant record must be defined in one of the six available index positions (1-6) using device_setMerchantRecord, then container tag FFEE06
* must be sent as part of the additional tags parameter of ctls_startTransaction.  Tag FFEE06 must contain tag 9F26 and 9F22, and can optionanally contain tags 9F2B and DFO1.
* Example FFEE06189F220201009F2604000000009F2B050100000000DF010101
* 9F22 = two bytes = ApplePay Terminal Applicaiton Version Number.	Hard defined as 0100 for now. (required)
* 9F26 = four bytes = ApplePay Terminal Capabilities Information (required)
*  - Byte 1 = RFU
*  - Byte 2 = Terminal Type
*  - - Bit 8 = VAS Support	(1=on, 0 = off)
*  - - Bit 7 = Touch ID Required  (1=on, 0 = off)
*  - - Bit 6 = RFU
*  - - Bit 5 = RFU
*  - - Bit 1,2,3,4
*  - - - 0 = Payment Terminal
*  - - - 1 = Transit Terminal
*  - - - 2 = Access Terminal
*  - - - 3 = Wireless Handoff Terminal
*  - - - 4 = App Handoff Terminal
*  - - - 15 = Other Terminal
*  - Byte 3 = RFU
*  - Byte 4 = Terminal Mode
*  - - 0 = ApplePay VAS OR ApplePay
*  - - 1 = ApplePay VAS AND ApplePay
*  - - 2 = ApplePay VAS ONLY
*  - - 3 = ApplePay ONLY
*  9F2B = 5 bytes = ApplePay VAS Filter.  Each byte filters for that specific merchant index  (optional)
*  DF01 = 1 byte = ApplePay VAS Protocol.  (optional)
*  - - Bit 1 : 1 = URL VAS, 0 = Full VAS
*  - - Bit 2 : 1 = VAS Beeps, 0 = No VAS Beeps
*  - - Bit 3 : 1 = Silent Comm Error, 2 = EMEA Comm Error
*  - - Bit 4-8 : RFU
*
 */

int ctls_startTransaction(IN double amount, IN double amtOther, IN int type, IN const int _timeout, IN BYTE *tags, IN int tagsLen);


	/**
	 * Start CTLS Transaction Request
	 *
	 Authorizes the CTLS transaction for an ICC card

	 The tags will be returned in the callback routine.

	 @param timeout Timeout value in seconds.
	 @param tags The tags to be included in the request.  Passed as TLV stream.	Example, tag 9F0C with amount 0x000000000100 would be 0x9F0C06000000000100


	 @param tagsLen The length of tags data buffer.

	 >>>>>IMPORTANT: parameters for amount and amtOther MUST BE PASSED AS A DOUBLE VALUE WITH DECIMAL POINT.  Example, do not pass 1, but instead pass 1.0 or 1.00. Otherwise, results will be unpredictable



	 * @return RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()
	 * Note: if auto poll is on, it will returm the error IDG_P2_STATUS_CODE_COMMAND_NOT_ALLOWED
	 *
	 * NOTE ON APPLEPAY VAS:
	 * To enable ApplePay VAS, first a merchant record must be defined in one of the six available index positions (1-6) using device_setMerchantRecord, then container tag FFEE06
	 * must be sent as part of the additional tags parameter of ctls_startTransaction.  Tag FFEE06 must contain tag 9F26 and 9F22, and can optionanally contain tags 9F2B and DFO1.
	 * Example FFEE06189F220201009F2604000000009F2B050100000000DF010101
	 * 9F22 = two bytes = ApplePay Terminal Applicaiton Version Number.	Hard defined as 0100 for now. (required)
	 * 9F26 = four bytes = ApplePay Terminal Capabilities Information (required)
	 *  - Byte 1 = RFU
	 *  - Byte 2 = Terminal Type
	 *  - - Bit 8 = VAS Support	(1=on, 0 = off)
	 *  - - Bit 7 = Touch ID Required  (1=on, 0 = off)
	 *  - - Bit 6 = RFU
	 *  - - Bit 5 = RFU
	 *  - - Bit 1,2,3,4
	 *  - - - 0 = Payment Terminal
	 *  - - - 1 = Transit Terminal
	 *  - - - 2 = Access Terminal
	 *  - - - 3 = Wireless Handoff Terminal
	 *  - - - 4 = App Handoff Terminal
	 *  - - - 15 = Other Terminal
	 *  - Byte 3 = RFU
	 *  - Byte 4 = Terminal Mode
	 *  - - 0 = ApplePay VAS OR ApplePay
	 *  - - 1 = ApplePay VAS AND ApplePay
	 *  - - 2 = ApplePay VAS ONLY
	 *  - - 3 = ApplePay ONLY
	 *  9F2B = 5 bytes = ApplePay VAS Filter.  Each byte filters for that specific merchant index  (optional)
	 *  DF01 = 1 byte = ApplePay VAS Protocol.  (optional)
	 *  - - Bit 1 : 1 = URL VAS, 0 = Full VAS
	 *  - - Bit 2 : 1 = VAS Beeps, 0 = No VAS Beeps
	 *  - - Bit 3 : 1 = Silent Comm Error, 2 = EMEA Comm Error
	 *  - - Bit 4-8 : RFU
	 *
	 */

	int ctls_activateTransaction( IN const int _timeout, IN BYTE *tags, IN int tagsLen);

/**
* Cancel EMV Transaction
*
Cancels the currently executing EMV transaction.


* @return RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()

*/

int ctls_cancelTransaction();


/**
 * Retrieve Application Data by AID
 *
 Retrieves the Application Data as specified by the AID name passed as a parameter.

 * @param AID Name of ApplicationID. Must be between 5 and 16 bytes
 * @param AIDLen the length of AID data buffer.
 * @param tlv  The TLV elements of the requested AID
 * @param tlvLen the length of tlv data buffer.

 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()
 */
int ctls_retrieveApplicationData(IN BYTE* AID, IN int AIDLen, OUT BYTE* tlv, IN_OUT int *tlvLen);

/**
 * Set Application Data by AID
 *
 Sets the Application Data for CTLS as specified by TLV data

 @param tlv  Application data in TLV format
	The first tag of the TLV data must be the group number (FFE4).
	The second tag of the TLV data must be the AID (9F06)

	Example valid TLV, for Group #2, AID a0000000035010:
	"ffe401029f0607a0000000051010ffe10101ffe50110ffe30114ffe20106"
 * @param tlvLen the length of tlv data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()

 */
int ctls_setApplicationData(IN BYTE* tlv, IN int tlvLen);
/**
 * Remove Application Data by AID
 * Removes the Application Data for CTLS as specified by the AID name passed as a parameter

 * @param AID Name of ApplicationID Must be between 5 and 16 bytes

 * @param AIDLen the length of AID data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()
 */
int ctls_removeApplicationData(IN BYTE* AID, IN int AIDLen);
/**
 * Remove All Application Data
*
Removes all the Application Data


* @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()

*/
int ctls_removeAllApplicationData();
/**
 * Retrieve AID list
 *
 Returns all the AID names installed on the terminal for CTLS. .

 * @param AIDList  array of AID name byte arrays
 * @param AIDListLen  the length of AIDList array buffer

 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()

 */
int ctls_retrieveAIDList(OUT BYTE * AIDList, IN_OUT int *AIDListLen);

/**
 * Retrieve Terminal Data
 *

 * Retrieves the Terminal Data for CTLS. This is configuration group 0 (Tag FFEE - > FFEE0100).  The terminal data
	can also be retrieved by ctls_getConfigurationGroup(0).

 * @param tlv Response returned as a TLV
 * @param tlvLen the length of tlv data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()


 */
int ctls_retrieveTerminalData(OUT BYTE* tlv, IN_OUT int *tlvLen);
/**
* Set Terminal Data
*
Sets the Terminal Data for CTLS as specified by the TLV.  The first TLV must be Configuration Group Number (Tag FFE4).	The terminal global data
	is group 0, so the first TLV would be FFE40100.  Other groups can be defined using this method (1 or greater), and those can be
	retrieved with ctls_getConfigurationGroup(int group), and deleted with ctls_removeConfigurationGroup(int group).	You cannot
	delete group 0.

 * @param tlv TerminalData configuration file
 * @param tlvLen the length of tlv data buffer

 * @retval RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()

 */
int ctls_setTerminalData(IN BYTE* tlv, IN int tlvLen);

/**
 * Retrieve Certificate Authority Public Key
 *

 Retrieves the CAPK for CTLS as specified by the RID/Index	passed as a parameter.

 * @param capk 6 bytes CAPK = 5 bytes RID + 1 byte Index
 * @param capkLen the length of capk data buffer
 * @param key Response returned as a CAKey format:
    [5 bytes RID][1 byte Index][1 byte Hash Algorithm][1 byte Encryption Algorithm]
    [20 bytes HashValue][4 bytes Public Key Exponent][2 bytes Modulus Length][Variable bytes Modulus]
    Where:
     - Hash Algorithm: The only algorithm supported is SHA-1.The value is set to 0x01
     - Encryption Algorithm: The encryption algorithm in which this key is used. Currently support only one type: RSA. The value is set to 0x01.
     - HashValue: Which is calculated using SHA-1 over the following fields: RID & Index & Modulus & Exponent
     - Public Key Exponent: Actually, the real length of the exponent is either one byte or 3 bytes. It can have two values: 3 (Format is 0x00 00 00 03), or  65537 (Format is 0x00 01 00 01)
     - Modulus Length: LenL LenH Indicated the length of the next field.
     - Modulus: This is the modulus field of the public key. Its length is specified in the field above.
 * @param keyLen the length of key data buffer
 * * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()
 */
int ctls_retrieveCAPK(IN BYTE * capk, IN int capkLen, OUT BYTE * key, IN_OUT int *keyLen);
/**
 * Set Certificate Authority Public Key
 *
 Sets the CAPK as specified by the CAKey structure

 * @param capk CAKey format:
    [5 bytes RID][1 byte Index][1 byte Hash Algorithm][1 byte Encryption Algorithm][20 bytes HashValue][4 bytes Public Key Exponent][2 bytes Modulus Length][Variable bytes Modulus]
    Where:
     - Hash Algorithm: The only algorithm supported is SHA-1.The value is set to 0x01
     - Encryption Algorithm: The encryption algorithm in which this key is used. Currently support only one type: RSA. The value is set to 0x01.
     - HashValue: Which is calculated using SHA-1 over the following fields: RID & Index & Modulus & Exponent
     - Public Key Exponent: Actually, the real length of the exponent is either one byte or 3 bytes. It can have two values: 3 (Format is 0x00 00 00 03), or  65537 (Format is 0x00 01 00 01)
     - Modulus Length: LenL LenH Indicated the length of the next field.
     - Modulus: This is the modulus field of the public key. Its length is specified in the field above.
 * @param capkLen the length of capk data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()

 */
int ctls_setCAPK(IN BYTE * capk, IN int capkLen);
/**
 * Remove Certificate Authority Public Key
 *
 Removes the CAPK as specified by the RID/Index

 * @param capk 6 byte CAPK =  5 bytes RID + 1 byte INDEX
 * @param capkLen the length of capk data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()

 */
int ctls_removeCAPK(IN BYTE * capk, IN int capkLen);
/**
* Remove All Certificate Authority Public Key
 *
 Removes all the CAPK

* @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()

*/
int ctls_removeAllCAPK();
/**
 * Retrieve the Certificate Authority Public Key list
 *
 Returns all the CAPK RID and Index installed on the terminal.

 * @param keys [key1][key2]...[keyn], each key 6 bytes where
    key = 5 bytes RID + 1 byte index
 * @param keysLen the length of keys data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()
 */
int ctls_retrieveCAPKList(OUT BYTE * keys, IN_OUT int *keysLen);

/**
 * Set Configuration Group
 *
 Sets the Configuration Group for CTLS as specified by the TLV data

 @param tlv  Configuration Group Data in TLV format
	The first tag of the TLV data must be the group number (DFEE2D).
	A second tag must exist
* @param tlvLen the length of tlv data buffer


* @return RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()

 */

int ctls_setConfigurationGroup(IN BYTE* tlv, IN int tlvLen);
 /**
 * Get Configuration Group
 *
 Retrieves the Configuration for the specified Group.

* @param group Configuration Group
* @param tlv return data

* @param tlvLen the length of tlv data buffer
* @return RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()


 */

int ctls_getConfigurationGroup(IN int group, OUT BYTE* tlv, OUT int *tlvLen);

/**
* Retrieve All Configuration Groups
*
Returns all the Configuration Groups installed on the terminal for CTLS

* @param tlv  The TLV elements data
 * @param tlvLen the length of tlv data buffer.

* @return RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()

*/

int ctls_getAllConfigurationGroups(OUT BYTE * tlv, IN_OUT int *tlvLen);

/**
* Remove Configuration Group
*
Removes the Configuration as specified by the Group.  Must not by group 0

 @param group Configuration Group

* @retval RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()

*/
int ctls_removeConfigurationGroup( int group);

/**
 * Display Online Authorization Result
 *  Use this command to display the status of an online authorization request on the readerï¿½s display (OK or NOT OK).
 *  Use this command after the reader sends an online request to the issuer.
 *@param isOK TRUE = OK, FALSE = NOT OK
 *@param TLV Optional TLV for AOSA
 *@param TLVLen TLV Length
 *@return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int ctls_displayOnlineAuthResult(IN int isOK, IN BYTE *TLV, IN int TLVLen);

/**
Allow fallback for EMV transactions.  Default is TRUE
@param allow TRUE = allow fallback, FALSE = don't allow fallback
*/
void emv_allowFallback(IN int allow);

/**
  Enables authenticate for EMV transactions.  If a emv_startTranaction results in code 0x0010 (start transaction success),
	  then emv_authenticateTransaction can automatically execute if parameter is set to TRUE
  @param authenticate TRUE = auto authenticate, FALSE = manually authenticate
  */
void emv_setAutoAuthenticateTransaction(IN int authenticate);

/**
 Enables complete for EMV transactions.  If a emv_authenticateTranaction results in code 0x0004 (go online),
  then emv_completeTransaction can automatically execute if parameter is set to TRUE
 @param complete TRUE = auto complete, FALSE = manually complete
 */
void emv_setAutoCompleteTransaction(IN int complete);

/**
Gets auto authenticate value for EMV transactions.
@return RETURN_CODE:  TRUE = auto authenticate, FALSE = manually authenticate
*/
int emv_getAutoAuthenticateTransaction();

/**
Gets auto complete value for EMV transactions.
@return RETURN_CODE:  TRUE = auto complete, FALSE = manually complete
*/
int emv_getAutoCompleteTransaction();
/**
* Start EMV Transaction Request
*
Authorizes the EMV transaction for an ICC card

The tags will be returned in the callback routine.

 * @param amount Transaction amount value  (tag value 9F02) - SEE IMPORTANT NOTE BELOW
 * @param amtOther Other amount value, if any  (tag value 9F03) - SEE IMPORTANT NOTE BELOW
 * @param exponent Number of characters after decimal point
 * @param type Transaction type (tag value 9C).
 * @param timeout Timeout value in seconds.
 * @param tags Any other tags to be included in the request.  Passed as a TLV stream.  Example, tag 9F0C with amount 0x000000000100 would be 0x9F0C06000000000100
    If tags 9F02 (amount),9F03 (other amount), or 9C (transaction type) are included, they will take priority over these values supplied as individual parameters to this method.
@param tagsLen The length of tags
@param forceOnline TRUE = do not allow offline approval,  FALSE = allow ICC to approve offline if terminal capable
Note:  To request tags to be  included in default response, use tag DFEE1A, and specify tag list.  Example four tags 9F02, 9F36, 95, 9F37 to be included in response = DFEE1A079F029F369f9F37

@return RETURN_CODE:  Values can be parsed with device_getResponseCodeString

>>>>>IMPORTANT: parameters for amount and amtOther MUST BE PASSED AS A DOUBLE VALUE WITH DECIMAL POINT.  Example, do not pass 1, but instead pass 1.0 or 1.00. Otherwise, results will be unpredictable
 */
int emv_startTransaction(IN double amount, IN double amtOther, IN int exponent, IN int type, IN int timeout, IN BYTE* tags, IN int tagsLen, IN int forceOnline);


/**
 * Start EMV Transaction Request
 *
 Authorizes the EMV transaction for an ICC card

 The tags will be returned in the callback routine.


 * @param timeout Timeout value in seconds.
 * @param tags Tags to be included in the request.  Passed as a TLV stream.  Example, tag 9F0C with amount 0x000000000100 would be 0x9F0C06000000000100
   .
 * @param tagsLen Length of tags
 * @param forceOnline TRUE = do not allow offline approval,  FALSE = allow ICC to approve offline if terminal capable
 Note:  To request tags to be  included in default response, use tag DFEE1A, and specify tag list.  Example four tags 9F02, 9F36, 95, 9F37 to be included in response = DFEE1A079F029F369f9F37
 @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 >>>>>IMPORTANT: parameters for amount and amtOther MUST BE PASSED AS A DOUBLE VALUE WITH DECIMAL POINT.  Example, do not pass 1, but instead pass 1.0 or 1.00. Otherwise, results will be unpredictable
 */

int emv_activateTransaction(IN int timeout, IN BYTE* tags, IN int tagsLen, IN int forceOnline);

/**
* Authenticate EMV Transaction Request
*
Authenticates the EMV transaction for an ICC card.  Execute this after receiving response with result code 0x10 to emv_startTransaction

The tags will be returned in the callback routine.

 * @param updatedTLV  TLV stream that can be used to update the following values:
     - 9F02: Amount
     - 9F03: Other amount
     - 9C: Transaction type
     - 5F57: Account type
    In addition tag DFEE1A can be sent to specify tag list to include in results. Example four tags 9F02, 9F36, 95, 9F37 to be included in response = DFEE1A079F029F36959F37
 * @param updatedTLVLen
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
*/
int emv_authenticateTransaction(IN BYTE* updatedTLV, IN int updatedTLVLen);
/**
* Complete EMV Transaction Request
*
*
* Completes the EMV transaction for an ICC card when online authorization request is received from emv_authenticateTransaction
*
* The tags will be returned in the callback routine.

* @param commError Communication error with host.  Set to TRUE(1) if host was unreachable, or FALSE(0) if host response received.  If Communication error, authCode, iad, tlvScripts can be null.
* @param authCode Authorization code from host.  Two bytes.  Example 0x3030.  (Tag value 8A).  Required
* @param authCodeLen the length of authCode
* @param iad Issuer Authentication Data, if any.  Example 0x11223344556677883030 (tag value 91).
* @param iadLen the length of iadLen
* @param tlvScripts 71/72 scripts, if any
* @param tlvScriptsLen the length of tlvScriptsLen
* @param tlv  Additional TLV data to return with transaction results (if any)
* @param tlvLen the length of tlv
* @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
*/
int emv_completeTransaction(IN int commError, IN BYTE* authCode, IN int authCodeLen,
		IN BYTE* iad, IN int iadLen, IN BYTE* tlvScripts, IN int tlvScriptsLen, IN BYTE* tlv, IN int tlvLen);
/**
* Cancel EMV Transaction
*
* Cancels the currently executing EMV transaction.
* @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
*/
int emv_cancelTransaction();
/**
 * Retrieve Application Data by AID
 *
 Retrieves the Application Data as specified by the AID name passed as a parameter.

 * @param AID Name of ApplicationID. Must be between 5 and 16 bytes
 * @param AIDLen the length of AID data buffer.
 * @param tlv  The TLV elements of the requested AID
 * @param tlvLen the length of tlv data buffer.

 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()
 */
int emv_retrieveApplicationData(IN BYTE* AID, IN int AIDLen, OUT BYTE* tlv, IN_OUT int *tlvLen);
/**
 * Set Application Data by AID
 *
 Sets the Application Data as specified by the application name and TLV data

 * @param name Application name, 10-32 ASCII hex characters representing 5-16 bytes  Example "a0000000031010"
 * @param nameLen the length of name data buffer of Application name,
 * @param tlv  Application data in TLV format
 * @param tlvLen the length of tlv data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()

 */
int emv_setApplicationData(IN BYTE* name, IN int nameLen, IN BYTE* tlv, IN int tlvLen);
/**
 * Set Application Data by TLV
 *
 * Sets the Application Data as specified by the TLV data
 * @param tlv  Application data in TLV format
 * 		The first tag of the TLV data must be the group number (DFEE2D).
 * 		The second tag of the TLV data must be the AID (9F06)
 * 		Example valid TLV, for Group #2, AID a0000000035010:
 * 		"dfee2d01029f0607a0000000051010ffe10101ffe50110ffe30114ffe20106"
 * @param tlvLen the length of tlv data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_setApplicationDataTLV(IN BYTE* tlv, IN int tlvLen);
/**
 * Remove Application Data by AID
 * Removes the Application Data for CTLS as specified by the AID name passed as a parameter

 * @param AID Name of ApplicationID Must be between 5 and 16 bytes

 * @param AIDLen the length of AID data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()
 */
int emv_removeApplicationData(IN BYTE* AID, IN int AIDLen);
/**
 * Remove All Application Data
*
Removes all the Application Data


* @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()

*/
int emv_removeAllApplicationData();
/**
 * Retrieve AID list
 *
 Returns all the AID names installed on the terminal for CTLS. .

 * @param AIDList  array of AID name byte arrays
 * @param AIDListLen  the length of AIDList array buffer

 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()

 */
int emv_retrieveAIDList(OUT BYTE * AIDList, IN_OUT int *AIDListLen);

/**
 * Retrieve Terminal Data
 *

 * Retrieves the Terminal Data for CTLS. This is configuration group 0 (Tag FFEE - > FFEE0100).  The terminal data
	can also be retrieved by ctls_getConfigurationGroup(0).

 * @param tlv Response returned as a TLV
 * @param tlvLen the length of tlv data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()


 */
int emv_retrieveTerminalData(OUT BYTE* tlv, IN_OUT int *tlvLen);
/**
* Set Terminal Data
*
Sets the Terminal Data for CTLS as specified by the TLV.  The first TLV must be Configuration Group Number (Tag FFE4).	The terminal global data
	is group 0, so the first TLV would be FFE40100.  Other groups can be defined using this method (1 or greater), and those can be
	retrieved with emv_getConfigurationGroup(int group), and deleted with emv_removeConfigurationGroup(int group).	You cannot
	delete group 0.

 * @param tlv TerminalData configuration file
 * @param tlvLen the length of tlv data buffer

 * @retval RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()

 */
int emv_setTerminalData(IN BYTE* tlv, IN int tlvLen);

/**
 * Retrieve Certificate Authority Public Key
 *

 Retrieves the CAPK for CTLS as specified by the RID/Index	passed as a parameter.

 * @param capk 6 bytes CAPK = 5 bytes RID + 1 byte Index
 * @param capkLen the length of capk data buffer
 * @param key Response returned as a CAKey format:
    [5 bytes RID][1 byte Index][1 byte Hash Algorithm][1 byte Encryption Algorithm]
    [20 bytes HashValue][4 bytes Public Key Exponent][2 bytes Modulus Length][Variable bytes Modulus]
    Where:
     - Hash Algorithm: The only algorithm supported is SHA-1.The value is set to 0x01
     - Encryption Algorithm: The encryption algorithm in which this key is used. Currently support only one type: RSA. The value is set to 0x01.
     - HashValue: Which is calculated using SHA-1 over the following fields: RID & Index & Modulus & Exponent
     - Public Key Exponent: Actually, the real length of the exponent is either one byte or 3 bytes. It can have two values: 3 (Format is 0x00 00 00 03), or  65537 (Format is 0x00 01 00 01)
     - Modulus Length: LenL LenH Indicated the length of the next field.
     - Modulus: This is the modulus field of the public key. Its length is specified in the field above.
 * @param keyLen the length of key data buffer
 * * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()
 */
int emv_retrieveCAPK(IN BYTE * capk, IN int capkLen, OUT BYTE * key, IN_OUT int *keyLen);
/**
 * Set Certificate Authority Public Key
 *
 Sets the CAPK as specified by the CAKey structure

 * @param capk CAKey format:
    [5 bytes RID][1 byte Index][1 byte Hash Algorithm][1 byte Encryption Algorithm][20 bytes HashValue][4 bytes Public Key Exponent][2 bytes Modulus Length][Variable bytes Modulus]
    Where:
     - Hash Algorithm: The only algorithm supported is SHA-1.The value is set to 0x01
     - Encryption Algorithm: The encryption algorithm in which this key is used. Currently support only one type: RSA. The value is set to 0x01.
     - HashValue: Which is calculated using SHA-1 over the following fields: RID & Index & Modulus & Exponent
     - Public Key Exponent: Actually, the real length of the exponent is either one byte or 3 bytes. It can have two values: 3 (Format is 0x00 00 00 03), or  65537 (Format is 0x00 01 00 01)
     - Modulus Length: LenL LenH Indicated the length of the next field.
     - Modulus: This is the modulus field of the public key. Its length is specified in the field above.
 * @param capkLen the length of capk data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()

 */
int emv_setCAPK(IN BYTE * capk, IN int capkLen);
/**
 * Remove Certificate Authority Public Key
 *
 Removes the CAPK as specified by the RID/Index

 * @param capk 6 byte CAPK =  5 bytes RID + 1 byte INDEX
 * @param capkLen the length of capk data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()

 */
int emv_removeCAPK(IN BYTE * capk, IN int capkLen);
/**
* Remove All Certificate Authority Public Key
 *
 Removes all the CAPK

* @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()

*/
int emv_removeAllCAPK();
/**
 * Retrieve the Certificate Authority Public Key list
 *
 Returns all the CAPK RID and Index installed on the terminal.

 * @param keys [key1][key2]...[keyn], each key 6 bytes where
    key = 5 bytes RID + 1 byte index
 * @param keysLen the length of keys data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()
 */
int emv_retrieveCAPKList(OUT BYTE * keys, IN_OUT int *keysLen);

/**
 * Retrieve the EMV Exception List
 *
 Returns the EMV Exception entries on the terminal.
 @param exceptionList [Exception1][Exception2]...[Exceptionn], where  [Exception]  is 12 bytes:
    [1 byte Len][10 bytes PAN][1 byte Sequence Number]
 @param exceptionListLen The length of the exception list.

* @return RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()
 */
int emv_retrieveExceptionList(OUT BYTE *exceptionList, IN_OUT int *exceptionListLen);

/**
 * Set EMV Exception
 *
 Adds an entry to the EMV Exception List

 @param exception EMV Exception entry containing the PAN and Sequence Number
    where  [Exception]  is 12 bytes:
    [1 byte Len][10 bytes PAN][1 byte Sequence Number]
    PAN, in compressed numeric, padded with F if required (example 0x5413339000001596FFFF)
 @param exceptionLen The length of the exception.

* @return RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()
 */
int emv_setException(IN BYTE *exception, IN int exceptionLen);

/**
 * Remove EMV Exception
 *
 Removes an entry to the EMV Exception List

 @param exception EMV Exception entry containing the PAN and Sequence Number
    where  [Exception]  is 12 bytes:
    [1 byte Len][10 bytes PAN][1 byte Sequence Number]
    PAN, in compressed numeric, padded with F if required (example 0x5413339000001596FFFF)
 @param exceptionLen The length of the exception.

* @return RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()
 */
int emv_removeException(IN BYTE *exception, IN int exceptionLen);

/**
 * Remove All EMV Exceptions
 *
 Removes all entries from the EMV Exception List

* @return RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()
 */
int emv_removeAllExceptions();

/**
* Get EMV Exception Log Status
*
This command returns information about the EMV Exception log. The version number, record size,
    and number of records contained in the file are returned.

 @param exceptionLogStatus 12 bytes returned
    - bytes 0-3 = Version Number
    - bytes 4-7 = Number of records
    - bytes 8-11 = Size of record
 @param exceptionLogStatusLen The length of the exception log status.

* @return RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()
*/
int emv_retrieveExceptionLogStatus(OUT BYTE *exceptionLogStatus, IN_OUT int *exceptionLogStatusLen);

/**
* Clear Transaction Log
*
Clears the transaction  log.

* @return RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()
*/
int emv_removeTransactionLog();

/**
* Get Transaction Log Status
*
This command returns information about the EMV transaction log. The version number, record size,
    and number of records contained in the file are returned.

 @param transactionLogStatus 12 bytes returned
    - bytes 0-3 = Version Number
    - bytes 4-7 = Number of records
    - bytes 8-11 = Size of record
 @param transactionLogStatusLen The length of the transaction log status.

* @return RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()
*/
int emv_retrieveTransactionLogStatus(OUT BYTE *transactionLogStatus, IN_OUT int *transactionLogStatusLen);

/**
* Get Transaction Log Record
*
Retrieves  oldest transaction record on the Transaction Log. At successful completion, the oldest transaction
    record is deleted from the transaction log

 @param transactionLog Transaction Record
 @param transactionLogLen The length of the transaction log.
 @param remainingTransactionLogLen Number of records remaining on the transaction log

Length | Description | Type
----- | ----- | -----
4 | Transaction Log State (TLS) | Enum (4-byte number, LSB first), SENT ONLINE = 0, NOT SENT = 1
4 | Transaction Log Content (TLC) | Enum (4-byte number, LSB first), BATCH = 0, OFFLINE ADVICE = 1, ONLINEADVICE = 2, REVERSAL = 3
4 | AppExpDate | unsigned char [4]
3 | AuthRespCode | unsigned char [3]
3 | MerchantCategoryCode | unsigned char [3]
16 | MerchantID | unsigned char [16]
2 | PosEntryMode | unsigned char [2]
9 | TermID | unsigned char [9]
3 | AIP | unsigned char [3]
3 | ATC | unsigned char [3]
33 | IssuerAppData | unsigned char [33]
6 | TVR | unsigned char [6]
3 | TSI | unsigned char [3]
11 | Pan | unsigned char [11]
2 | PanSQNCNum | unsigned char [2]
3 | TermCountryCode | unsigned char [3]
7 | TranAmount | unsigned char [7]
3 | TranCurCode | unsigned char [3]
4 | TranDate | unsigned char [4]
2 | TranType | unsigned char [2]
9 | IFDSerialNum | unsigned char [9]
12 | AcquirerID | unsigned char [12]
2 | CID | unsigned char [2]
9 | AppCryptogram | unsigned char [9]
5 | UnpNum | unsigned char [5]
7 | AmountAuth | unsigned char [7]
4 | AppEffDate | unsigned char [4]
4 | CVMResults | unsigned char [4]
129 | IssScriptResults | unsigned char [129]
4 | TermCap | unsigned char [4]
2 | TermType | unsigned char [2]
20 | Track2 | unsigned char [20]
4 | TranTime | unsigned char [4]
7 | AmountOther | unsigned char [7]
1 | Unused | Unsigned char [1]

* @return RETURN_CODE:  Values can be parsed with errorCode.getErrorString()

*/
int emv_retrieveTransactionLog(OUT BYTE *transactionLog, IN_OUT int *transactionLogLen, IN_OUT int *remainingTransactionLogLen);

/**
 * Polls device for EMV Kernel Version
 *
 * @param version Response returned of Kernel Version
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 *
 */
int emv_getEMVKernelVersion(OUT char* version);

/**
* Get EMV Kernel check value info
*
* @param checkValue Response returned of Kernel check value info
* @param checkValueLen the length of checkValue
*
* @return RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()
*
*/
int emv_getEMVKernelCheckValue(OUT BYTE* checkValue, IN_OUT int *checkValueLen);

/**
 * Get EMV Kernel configuration check value info
 *
 * @param checkValue Response returned of Kernel configuration check value info
 * @param checkValueLen the length of checkValue
 * @return RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()
 *
 */
int emv_getEMVConfigurationCheckValue(OUT BYTE* checkValue, IN_OUT int *checkValueLen);

/**
 * Retrieve the Certificate Revocation List
 *
 Returns the CRL entries on the terminal.

 * @param list [CRL1][CRL2]...[CRLn], each CRL 9 bytes where
    CRL = 5 bytes RID + 1 byte index + 3 bytes serial number
 * @param lssLen the length of list data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_retrieveCRL(OUT BYTE* list, IN_OUT int* lssLen);

/**
 * Set Certificate Revocation List
 *
 Sets the CRL

 * @param list CRL Entries containing the RID, Index, and serial numbers to set
    [CRL1][CRL2]...[CRLn] where each [CRL]  is 9 bytes:
    [5 bytes RID][1 byte CAPK Index][3 bytes serial number]
 * @param lsLen the length of list data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()

 */
int emv_setCRL(IN BYTE* list, IN int lsLen);

/**
 * Retrieve the Certificate Revocation List
 *
 * Returns the CRL entries on the terminal.

 * @param list [CRL1][CRL2]...[CRLn], each CRL 9 bytes where
    CRL = 5 bytes RID + 1 byte index + 3 bytes serial number
 * @param lssLen the length of list data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_removeCRL(IN BYTE* list, IN int lsLen);
/**
 * Remove All Certificate Revocation List Entries
 *
 * Removes all CRLEntry entries


 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()

*/
int emv_removeAllCRL();

/**
  * Reset to Initial State
  *  This command places the reader UI into the idle state and displays the appropriate idle display.
  *
  * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
  */
int lcd_resetInitialState();

/**
  * Custom Display Mode
  *  Controls the LCD display mode to custom display. Keyboard entry is limited to the Cancel, Clear,
  *  Enter and the function keys, if present. PIN entry is not permitted while the
  *  reader is in Custom Display Mode
  *
  * @param enable TRUE = enabled, FALSE = disabled
  *
  * @return RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()
  */
int lcd_customDisplayMode(IN int enable);

/**
  * Set Foreground and Background Color
  *  This command sets the foreground and background colors of the LCD.
  *
  * @param foreRGB Foreground RGB. 000000 = black, FF0000 = red, 00FF00 = green, 0000FF = blue, FFFFFF = white
  * @param Length of foreRGB.  Must be 3.
  * @param backRGB Background RGB. 000000 = black, FF0000 = red, 00FF00 = green, 0000FF = blue, FFFFFF = white
  * @param Length of backRGB.  Must be 3.
  *
  * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
  */
int lcd_setForeBackColor(IN BYTE *foreRGB, IN int foreRGBLen, IN BYTE *backRGB, IN int backRGBLen);

/**
  * Clear Display
  *  Command to clear the display screen on the reader.It returns the display to the currently defined background color and terminates all events
  *
  * @param control for L100.
  * 	0:First Line
  *		1:Second Line
  *		2:Third Line
  *		3:Fourth Line
  *		0xFF: All Screen
  *
  * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
  */
int lcd_clearDisplay(IN BYTE control);

/**
  * Enables Signature Capture
  *  This command executes the signature capture screen.  Once a signature is captured, it is sent to the callback
  *  with DeviceState.Signature, and the data will contain a .png of the signature
  *
  * @param timeout  Timeout waiting for the signature capture
  *
  * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
  */
int lcd_captureSignature(IN int timeout);

/**
  * Start slide show
  *  You must send images to the readerï¿½s memory and send a Start Custom Mode command to the reader before it will respond to this commands. Image files must be in .bmp or .png format.
  *
  * @param files Complete paths and file names of the files you want to use, separated by commas. If a directory is specified, all files in the dirctory are displayed
  * @param filesLen Length of files
  * @param posX X coordinate in pixels,   Range 0-271
  * @param posY Y coordinate in pixels,   Range 0-479
  * @param posMode Position Mode
  *  - 0 = Center on Line Y
  *  - 1 = Display at (X,Y)
  *  - 2 - Center on screen
  * @param touchEnable TRUE = Image is touch sensitive
  * @param recursion TRUE = Recursively follow directorys in list
  * @param touchTerminate TRUE = Terminate slideshow on touch (if touch enabled)
  * @param delay Number of seconds between image displays
  * @param loops  Number of display loops.  A zero indicates continuous display
  * @param clearScreen  TRUE = Clear screen
  *
  * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
  */
int lcd_startSlideShow(IN char *files, IN int filesLen, IN int posX, IN int posY, IN int posMode, IN int touchEnable, IN int recursion, IN int touchTerminate, IN int delay, IN int loops, IN int clearScreen);

/**
  * Cancel slide show
  *  Cancel the slide show currently running
  *
  * @param statusCode If the return code is not Success (0), the kernel may return a four-byte Extended Status Code
  * @param statusCodeLen the length of the Extended Status Code (should be 4 bytes)
  *
  * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
  */
int lcd_cancelSlideShow(OUT BYTE *statusCode, IN_OUT int *statusCodeLen);

/**
  * Set Display Image
  *  You must send images to the readerï¿½s memory and send a Start Custom Mode command to the reader before it will respond to Image commands. Image files must be in .bmp or .png format.
  *
  * @param file Complete path and file name of the file you want to use. Example "file.png" will put in root directory, while "ss/file.png" will put in ss directory (which must exist)
  * @param fileLen Length of files
  * @param posX X coordinate in pixels,   Range 0-271
  * @param posY Y coordinate in pixels,   Range 0-479
  * @param posMode Position Mode
  *  - 0 = Center on Line Y
  *  - 1 = Display at (X,Y)
  *  - 2 - Center on screen
  * @param touchEnable TRUE = Image is touch sensitive
  * @param clearScreen  TRUE = Clear screen
  *
  * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
  */
int lcd_setDisplayImage(IN char *file, IN int fileLen, IN int posX, IN int posY, IN int posMode, IN int touchEnable, IN int clearScreen);
/**
  * Set Background Image
  *  You must send images to the readerï¿½s memory and send a Start Custom Mode command to the reader before it will respond to Image commands. Image files must be in .bmp or .png format.
  *
  * @param file Complete path and file name of the file you want to use. Example "file.png" will put in root directory, while "ss/file.png" will put in ss directory (which must exist)
  * @param fileLen Length of files
  * @param enable TRUE = Use Background Image, FALSE = Use Background Color
  *
  * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
  */
int lcd_setBackgroundImage(IN char *file, IN int fileLen, IN int enable);
/**
 * Displays text.
 *
 * Custom Display Mode must be enabled for custom text.
 * PIN pad entry is not allowed in Custom Display Mode but the Cancel, OK, and Clear keys remain active.
 *
 * @param posX X coordinate in pixels
 * @param posY Y coordinate in pixels
 * @param displayWidth Width of the display area in pixels (optional)
 * @param displayHeight Height of the display area in pixels (optional)
 * @param fontDesignation Font designation
 * 	1 - Default
 * @param fontID Font styling
 *    Font ID | Height in pixels | Font Properties
 * 	--------------------------------------------------------------------
 * 	| 1       | 13               | Regular                             |
 * 	| 2       | 17               | Regular                             |
 * 	| 3       | 17               | Bold                                |
 * 	| 4       | 22               | Regular                             |
 * 	| 5       | 20               | Regular                             |
 * 	| 6       | 20               | Bold                                |
 * 	| 7       | 29               | Regular                             |
 * 	| 8       | 38               | Regular                             |
 * 	| 9       | 38               | Bold            					   |
 * 	| 10      | 58               | Regular                             |
 * 	| 11      | 58               | Bold, mono-space					   |
 * 	| 12      | 14               | Regular, mono-space, 8 pixels wide  |
 * 	| 13      | 15               | Regular, mono-space, 9 pixels wide  |
 * 	| 14      | 17               | Regular, mono-space, 9 pixels wide  |
 * 	| 15      | 20               | Regular, mono-space, 11 pixels wide |
 * 	| 16      | 21               | Regular, mono-space, 12 pixels wide |
 * 	| 17      | 25               | Regular, mono-space, 14 pixels wide |
 * 	| 18      | 30               | Regular, mono-space, 17 pixels wide |
 * @param screenPosition Display position
 * 	0 - Center on line Y without clearing screen
 * 	1 - Center on line Y after clearing screen
 * 	2 - Display at (X, Y) without clearing screen
 * 	3 - Display at (X, Y) after clearing screen
 * 	4 - Display at center of screen without clearing screen
 * 	5 - Display at center of screen after clearing screen
 * 	6 - Display text right-justified without clearing screen
 * 	7 - Display text right-justified after clearing screen
 * @param displayText Display text (Maximum: 1900 characters)
 * @param graphicsID A four byte array containing the ID of the created element (optional)
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int lcd_displayText(IN int posX, IN int posY, IN int displayWidth,
		IN int displayHeight, IN int fontDesignation, IN int fontID,
		IN int screenPosition, IN char *displayText, OUT BYTE *graphicsID);
/**
 * Displays text with scroll feature.
 *
 * Custom Display Mode must be enabled.
 *
 * @param posX X coordinate in pixels
 * @param posY Y coordinate in pixels
 * @param displayWidth Width of the display area in pixels (Minimum: 40px)
 * 	0 or NULL - Use the full width to display text
 * @param displayHeight Height of the display area in pixels (Minimum: 100px)
 * 	0 or NULL - Use the full height to display text
 * @param fontDesignation Font designation
 * 	1 - Default
 * @param fontID Font styling
 *    Font ID | Height in pixels | Font Properties
 * 	--------------------------------------------------------------------
 * 	| 1       | 13               | Regular                             |
 * 	| 2       | 17               | Regular                             |
 * 	| 3       | 17               | Bold                                |
 * 	| 4       | 22               | Regular                             |
 * 	| 5       | 20               | Regular                             |
 * 	| 6       | 20               | Bold                                |
 * 	| 7       | 29               | Regular                             |
 * 	| 8       | 38               | Regular                             |
 * 	| 9       | 38               | Bold            					   |
 * 	| 10      | 58               | Regular                             |
 * 	| 11      | 58               | Bold, mono-space					   |
 * 	| 12      | 14               | Regular, mono-space, 8 pixels wide  |
 * 	| 13      | 15               | Regular, mono-space, 9 pixels wide  |
 * 	| 14      | 17               | Regular, mono-space, 9 pixels wide  |
 * 	| 15      | 20               | Regular, mono-space, 11 pixels wide |
 * 	| 16      | 21               | Regular, mono-space, 12 pixels wide |
 * 	| 17      | 25               | Regular, mono-space, 14 pixels wide |
 * 	| 18      | 30               | Regular, mono-space, 17 pixels wide |
 * @param displayProperties Display properties for the text
 *  0 - Center on line Y without clearing screen
 *  1 - Center on line Y after clearing screen
 *  2 - Display at (X, Y) without clearing screen
 *  3 - Display at (X, Y) after clearing screen
 *  4 - Center on screen without clearing screen
 *  5 - Center on screen after clearing screen
 * @param displayText Display text (Maximum: 3999 characters plus terminator)
 */
int lcd_displayParagraph(IN int posX, IN int posY, IN int displayWidth,
		IN int displayHeight, IN int fontDesignation, IN int fontID,
		IN int displayProperties, IN char *displayText);
/**
 * Displays an interactive button.
 *
 * @param posX X coordinate in pixels
 * @param posY Y coordinate in pixels
 * @param buttonWidth Width of the button
 * @param buttonHeight Height of the button
 * @param fontDesignation Font designation
 * 	1 - Default
 * @param Font ID Font styling
 *    Font ID | Height in pixels | Font Properties
 * 	--------------------------------------------------------------------
 * 	| 1       | 13               | Regular                             |
 * 	| 2       | 17               | Regular                             |
 * 	| 3       | 17               | Bold                                |
 * 	| 4       | 22               | Regular                             |
 * 	| 5       | 20               | Regular                             |
 * 	| 6       | 20               | Bold                                |
 * 	| 7       | 29               | Regular                             |
 * 	| 8       | 38               | Regular                             |
 * 	| 9       | 38               | Bold            					   |
 * 	| 10      | 58               | Regular                             |
 * 	| 11      | 58               | Bold, mono-space					   |
 * 	| 12      | 14               | Regular, mono-space, 8 pixels wide  |
 * 	| 13      | 15               | Regular, mono-space, 9 pixels wide  |
 * 	| 14      | 17               | Regular, mono-space, 9 pixels wide  |
 * 	| 15      | 20               | Regular, mono-space, 11 pixels wide |
 * 	| 16      | 21               | Regular, mono-space, 12 pixels wide |
 * 	| 17      | 25               | Regular, mono-space, 14 pixels wide |
 * 	| 18      | 30               | Regular, mono-space, 17 pixels wide |
 * @param displayPosition Button display position
 * 	0 - Center on line Y without clearing screen and without word wrap
 * 	1 - Center on line Y after clearing screen and without word wrap
 * 	2 - Display at (X, Y) without clearing screen and without word wrap
 * 	3 - Display at (X, Y) after clearing screen and without word wrap
 * 	4 - Center button on screen without clearing screen and without word wrap
 * 	5 - Center button on screen after clearing screen and without word wrap
 * 	64 - Center on line Y without clearing screen and with word wrap
 * 	65 - Center on line Y after clearing the screen and with word wrap
 * 	66 - Display at (X, Y) without clearing screen and with word wrap
 * 	67 - Display at (X, Y) after clearing screen and with word wrap
 * 	68 - Center button on screen without clearing screen and with word wrap
 * 	69 - Center button on screen after clearing screen and with word wrap
 * @param buttonLabel Button label text (Maximum: 31 characters)
 * @param buttonTextColorR - Red component for foreground color (0 - 255)
 * @param buttonTextColorG - Green component for foreground color (0 - 255)
 * @param buttonTextColorB - Blue component for foreground color (0 - 255)
 * @param buttonBackgroundColorR - Red component for background color (0 - 255)
 * @param buttonBackgroundColorG - Green component for background color (0 - 255)
 * @param buttonBackgroundColorB - Blue component for background color (0 - 255)
 * @param graphicsID A four byte array containing the ID of the created element (optional)
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int lcd_displayButton(IN int posX, IN int posY, IN int buttonWidth,
		IN int buttonHeight, IN int fontDesignation, IN int fontID,
		IN int displayPosition, IN char *buttonLabel, IN int buttonTextColorR,
		IN int buttonTextColorG, IN int buttonTextColorB, IN int buttonBackgroundColorR,
		IN int buttonBackgroundColorG, IN int buttonBackgroundColorB, OUT BYTE *graphicsID);
/**
 * Creates a display list.
 *
 * @param posX X coordinate in pixels
 * @param posY Y coordinate in pixels
 * @param numOfColumns Number of columns to display
 * @param numOfRows Number of rows to display
 * @param fontDesignation Font Designation
 * 	1 - Default font
 * @param fontID Font styling
 *    Font ID | Height in pixels | Font Properties
 * 	--------------------------------------------------------------------
 * 	| 1       | 13               | Regular                             |
 * 	| 2       | 17               | Regular                             |
 * 	| 3       | 17               | Bold                                |
 * 	| 4       | 22               | Regular                             |
 * 	| 5       | 20               | Regular                             |
 * 	| 6       | 20               | Bold                                |
 * 	| 7       | 29               | Regular                             |
 * 	| 8       | 38               | Regular                             |
 * 	| 9       | 38               | Bold            					   |
 * 	| 10      | 58               | Regular                             |
 * 	| 11      | 58               | Bold, mono-space					   |
 * 	| 12      | 14               | Regular, mono-space, 8 pixels wide  |
 * 	| 13      | 15               | Regular, mono-space, 9 pixels wide  |
 * 	| 14      | 17               | Regular, mono-space, 9 pixels wide  |
 * 	| 15      | 20               | Regular, mono-space, 11 pixels wide |
 * 	| 16      | 21               | Regular, mono-space, 12 pixels wide |
 * 	| 17      | 25               | Regular, mono-space, 14 pixels wide |
 * 	| 18      | 30               | Regular, mono-space, 17 pixels wide |
 * @param verticalScrollArrowsVisible Display vertical scroll arrows by default
 * @param borederedListItems Draw border around list items
 * @param borederedScrollArrows Draw border around scroll arrows (if visible)
 * @param touchSensitive List items are touch enabled
 * @param automaticScrolling Enable automatic scrolling of list when new items exceed display area
 * @param graphicsID A four byte array containing the ID of the created element (optional)
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int lcd_createList(IN int posX, IN int posY, IN int numOfColumns,
		IN int numOfRows, IN int fontDesignation, IN int fontID,
		IN int verticalScrollArrowsVisible, IN int borderedListItems, IN int borderdScrollArrows,
		IN int touchSensitive, IN int automaticScrolling, OUT BYTE *graphicsID);
/**
 * Adds an item to an existing list.
 *
 * Custom Display Mode must be enabled for custom text.
 *
 * @param listGraphicsID Existing list's graphics ID (4 byte array) that is provided during creation
 * @param itemName Item name (Maximum: 127 characters)
 * @param itemID Identifier for the item (Maximum: 31 characters)
 * @param selected If the item should be selected
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int lcd_addItemToList(IN BYTE *listGraphicsID, IN char *itemName, IN char *itemID,
		IN int selected);
/**
 * Retrieves the selected item's ID.
 *
 * @param listGraphicsID Existing list's graphics ID (4 byte array) that is provided during creation
 * @param itemID The selected item's ID (Maximum: 32 characters)
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int lcd_getSelectedListItem(IN BYTE *listGraphicsID, OUT char *itemID);

/**
 * Removes all entries from the event queue.
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int lcd_clearEventQueue();

/**
 * Requests input from the reader.
 *
 * @param timeout Timeout amount in seconds
 * 	0 - No timeout
 * @param dataReceived Indicates if an event occurred and data was received
 * 	0 - No data received
 * 	1 - Data received
 * @param eventType The event type (required to be at least 4 bytes), see table below
 * @param graphicsID The graphicID of the event (required to be at least 4 bytes)
 * @param eventData The event data, see table below (required to be at least 73 bytes)
 *
 * | Event Type         | Value (4 bytes) | Event Specific Data
 * | ------------------ | --------------- | ------------------------------------------------------------------- |
 * | Button Event       | 00030000h       | Length = Variable												    |
 * |				    |				  |	Byte 1: State (1 = Pressed, other values RFU)					    |
 * |					|				  |	Byte 2 - n: Null terminated caption 							    |
 * | ------------------ | --------------- | ------------------------------------------------------------------- |
 * | Checkbox Event     | 00030001h       | Length = 1 byte													    |
 * |					|			      |	Byte 1: State (1 = Checked, 0 = Unchecked)						    |
 * | ------------------ | --------------- | ------------------------------------------------------------------- |
 * | Line Item Event    | 00030002h       | Length = 5 bytes												    |
 * |					|				  |	Byte 1: State (1 = Item Selected, other values RFU)				    |
 * |					|				  |	Byte 2 - n: Caption of the selected item						    |
 * | ------------------ | --------------- | ------------------------------------------------------------------- |
 * | Keypad Event       | 030003h         | Length - 3 bytes												    |
 * |					|				  | Byte 1: State (1 = key pressed, 2 = key released, other values RFU) |
 * |					|				  | Byte 2 - 3: Key pressed and Key release                             |
 * |					|				  |		0030h - KEYPAD_KEY_0											|
 * |					|				  |		0031h - KEYPAD_KEY_1											|
 * |					|				  |		0032h - KEYPAD_KEY_2											|
 * |					|				  |		0033h - KEYPAD_KEY_3											|
 * |					|				  |		0034h - KEYPAD_KEY_4											|
 * |					|				  |		0035h - KEYPAD_KEY_5											|
 * |					|				  |		0036h - KEYPAD_KEY_6											|
 * |					|				  |		0037h - KEYPAD_KEY_7											|
 * |					|				  |		0038h - KEYPAD_KEY_8											|
 * |					|				  |		0039h - KEYPAD_KEY_9											|
 * |					|				  | Byte 2 - 3: Only Key pressed										|
 * |					|				  | 	000Dh - KEYPAD_KEY_ENTER										|
 * |					|				  | 	0008h - KEYPAD_KEY_CLEAR										|
 * |					|				  | 	001Bh - KEYPAD_KEY_CANCEL										|
 * |					|				  | 	0070h - FUNC_KEY_F1 (Vend III)									|
 * |					|				  | 	0071h - FUNC_KEY_F2 (Vend III)									|
 * |					|				  | 	0072h - FUNC_KEY_F3 (Vend III)									|
 * |					|				  | 	0073h - FUNC_KEY_F4 (Vend III)									|
 * | ------------------ | --------------- | ------------------------------------------------------------------- |
 * | Touchscreen Event  | 030004h         | Length = 1 - 33 bytes												|
 * |					|				  | Byte 1: State (not used)											|
 * |					|				  | Byte 2 - 33: Image name (zero terminated)							|
 * | ------------------ | --------------- | ------------------------------------------------------------------- |
 * | Slideshow Event    | 030005h         | Length = 1 byte														|
 * |					|				  | Byte 1: State (not used)											|
 * | ------------------ | --------------- | ------------------------------------------------------------------- |
 * | Transaction Event  | 030006h         | Length = 9 bytes													|
 * |					|				  | Byte 1: State (not used)											|
 * |					|				  | Byte 2 - 5: Card type (0 = unknown)									|
 * |					|				  | Byte 6 - 9: Status - A four byte, big-endian field					|
 * |					|				  |	Byte 9 is used to store the 1-byte status code						|
 * |					|				  |		00 - SUCCESS													|
 * |					|				  |		08 - TIMEOUT													|
 * |					|				  |		0A - FAILED														|
 * |					|				  | This is not related to the extended status codes					|
 * | ------------------ | --------------- | ------------------------------------------------------------------- |
 * | Radio Button Event | 030007h         | Length = 73 bytes													|
 * |					|				  | Byte 1: State (1 = Change ins selected button, other values RFU)	|
 * |					|				  | Byte 2 - 33: Null terminated group name								|
 * |					|				  | Byte 34 - 65: Radio button caption									|
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int lcd_getInputEvent(IN int timeout, OUT int *dataReceived, OUT BYTE *eventType,
		OUT BYTE *graphicsID, OUT BYTE *eventData);

int lcd_createInputField(IN BYTE *specs, IN int specsLen, OUT BYTE *graphicId);

int lcd_getInputFieldValue(IN BYTE *graphicId, OUT BYTE *retData, OUT int *retDataLen);

/**
 * Disable MSR Swipe
 * Cancels MSR swipe request.
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int msr_cancelMSRSwipe();
/**
 * Start MSR Swipe
 * Enables MSR, waiting for swipe to occur. Allows track selection. Returns IDTMSRData instance to deviceDelegate::swipeMSRData:()
 * @param timeout Swipe Timeout Value
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 * Note: if auto poll mode is on, it will return command not allowed error
 */
int msr_startMSRSwipe(IN int _timeout);

/**
  * Flush Track Data
  * Clears any track data being retained in memory by future PIN Block request.
  * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
  */
int msr_flushTrackData();

/**
 * Parser the MSR data from the buffer into IDTMSTData structure
 *@param resData MSR card data buffer
 *@param resLen the length of resData
 *@param cardData the parser result with IDTMSTData structure
 */
void parseMSRData(IN BYTE *resData, IN int resLen, IN_OUT IDTMSRData *cardData);

/**
   * Get Encrypted DUKPT PIN
   *
   * Requests PIN Entry for online authorization. PIN block and KSN returned in callback function DeviceState.TransactionData with cardData.pin_pinblock.
   * A swipe must be captured first before this function can execute
   * @param keyType PIN block key type. Valid values 0,3 for TDES, 4 for AES
   * @param timeout  PIN entry timout
   *
   * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
   */
int pin_getEncryptedOnlinePIN(IN int keyType, IN int timeout);

/**
  * Get PAN
  *
  * Requests PAN Entry on pinpad
  *
  * @param getCSC Include Customer Service Code (also known as CVV, CVC)
  * @param timeout  PAN entry timout
  *
  * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
  */
int pin_getPAN(IN int getCSC, IN int timeout);

/**
  * Prompt for Credit or Debit
  *
  * Requests prompt for Credit or Debit. Response returned in callback function as DeviceState.MenuItem with data MENU_SELECTION_CREDIT = 0, MENU_SELECTION_DEBIT = 1
  *
  * @param currencySymbol Allowed values are $ (0x24), ï¿½ (0xA5), ï¿½ (0xA3), ï¿½ (0xA4), or NULL
  * @param currencySymbolLen length of currencySymbol
  * @param displayAmount  Amount to display (can be NULL)
  * @param displayAmountLen  length of displayAmount
  * @param timeout  Menu entry timout. Valid values 2-20 seconds
  *
  * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
  */
int pin_promptCreditDebit(IN char *currencySymbol, IN int currencySymbolLen, IN char *displayAmount, IN int displayAmountLen, IN int timeout, OUT BYTE *retData, IN_OUT int *retDataLen);

/**
  * Request CSR
  *  Requests 3 sets of public keys: encrypting Keys, signing/validating keys, signing/validating 3rd party apps
  *
  * @param csr RequestCSR structure to return the data
  *
  * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
  */
int ws_requestCSR(OUT RequestCSR *csr);

/**
  * Load SSL Certificate
  *  Loads a SSL certificate
  *
  * @param name Certificate Name
  * @param nameLen Certificate Name Length
  * @param dataDER DER encoded certificate data
  * @param dataDERLen DER encoded certificate data length
  *
  * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
  */
int ws_loadSSLCert(IN char *name, IN int nameLen, IN char *dataDER, IN int dataDERLen);

/**
  * Revoke SSL Certificate
  *  Revokes a SSL Certificate by name
  *
  * @param name Name of certificate to revoke
  * @param nameLen Certificate Name Length
  *
  * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
  */
int ws_revokeSSLCert(IN char *name, IN int nameLen);

/**
  * Delete SSL Certificate
  *  Deletes a SSL Certificate by name
  *
  * @param name Name of certificate to delete
  * @param nameLen Certificate Name Length
  *
  * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
  */
int ws_deleteSSLCert(IN char *name, IN int nameLen);

/**
  * Get Certificate Chain Type
  *  Returns indicator for using test/production certificate chain
  *
  * @param type 0 = test certificate chain, 1 = production certificate chain
  *
  * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
  */
int ws_getCertChainType(OUT int *type);

/**
  * Update Root Certificate
  *  Updates the root certificate
  *
  * @param name Certificate Name
  * @param nameLen Certificate Name Length
  * @param dataDER DER encoded certificate data
  * @param dataDERLen DER encoded certificate data length
  * @param signature Future Root CA signed (RSASSA PSS SHA256) by current Root CA
  * @param signature length
  *
  * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
  */
int ws_updateRootCertificate(IN char *name, IN int nameLen, IN char *dataDER, IN int dataDERLen, IN char *signature, IN int signatureLen);

#ifdef __cplusplus
}
#endif


#endif

/*! \file libIDT_VP8800.h
 \brief VP8800 API.

 VP8800 Global API methods.
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
