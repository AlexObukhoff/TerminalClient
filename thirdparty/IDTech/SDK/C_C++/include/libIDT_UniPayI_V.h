#ifndef __LIBIDT_UNIPAYI_V_H___
#define __LIBIDT_UNIPAYI_V_H___


#define IN
#define OUT
#define IN_OUT
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
 *To register the pin callback function to get the PINPad data.
 */
void pin_registerCallBk(pPIN_callBack pPINf);

/**
 * Register Comm HTTP Async Callback
 *
 * @param cBack - HTTP Comm callback
 */
void comm_registerHTTPCallback(httpComm_callBack cBack);

/**
 * Register External V4 Protocol commands Callback
 *
 * @param cBack - V4 Protocol Comm callback
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
	IDT_DEVICE_UNKNOWN=0,
	IDT_DEVICE_AUGUSTA_HID,
	IDT_DEVICE_AUGUSTA_KB,
	IDT_DEVICE_AUGUSTA_S_HID,
	IDT_DEVICE_AUGUSTA_S_KB,
	IDT_DEVICE_AUGUSTA_S_TTK_HID,
	IDT_DEVICE_SPECTRUM_PRO,
	IDT_DEVICE_MINISMART_II,
	IDT_DEVICE_UNIPAY,
	IDT_DEVICE_UNIPAY_I_V,
	IDT_DEVICE_VP3300_AJ,
	IDT_DEVICE_L100,
	IDT_DEVICE_KIOSK_III,
	IDT_DEVICE_KIOSK_III_S,
	IDT_DEVICE_VENDI,
	IDT_DEVICE_VP3300_USB,
	IDT_DEVICE_UNIPAY_I_V_TTK,
	IDT_DEVICE_VP3300_BT,
	IDT_DEVICE_VP8800,
	IDT_DEVICE_NEO2,
	IDT_DEVICE_MINISMART_II_COM = IDT_DEVICE_NEO2+5,
	IDT_DEVICE_SPECTRUM_PRO_COM,
	IDT_DEVICE_KIOSK_III_COM,
	IDT_DEVICE_KIOSK_III_S_COM,
	IDT_DEVICE_NEO2_COM,
	IDT_DEVICE_MAX_DEVICES = IDT_DEVICE_NEO2_COM+5
	};
	@endcode
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
//int device_startRKI();


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
 * Polls device for Serial Number
 *
 * @param sNumber  Returns Serial Number
 *
 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString
 *
 */
int config_getSerialNumber(OUT char* sNumber);

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
 * @param tagsLen Length of tags
 * @param forceOnline TRUE = do not allow offline approval,  FALSE = allow ICC to approve offline if terminal capable
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
 Sets the terminal major configuration in ICS .
 @param configuration A configuration value, range 1-5
 - 1 = 1C
 - 2 = 2C
 - 3 = 3C
 - 4 = 4C
 - 5 = 5C

 *  @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_setTerminalMajorConfiguration(IN int configuration);

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
 * Parser the MSR data from the buffer into IDTMSTData structure
 *@param resData MSR card data buffer
 *@param resLen the length of resData
 *@param cardData the parser result with IDTMSTData structure
 */
void parseMSRData(IN BYTE *resData, IN int resLen, IN_OUT IDTMSRData *cardData);

#ifdef __cplusplus
}
#endif


#endif

/*! \file libIDT_UniPayI_V.h
 \brief UniPay 1.5 API.

 UniPay 1.5 Global API methods.
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
