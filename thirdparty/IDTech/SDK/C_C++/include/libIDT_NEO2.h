#ifndef __LIBIDT_NEO2_H___
#define __LIBIDT_NEO2_H___


#define IN
#define OUT
#define IN_OUT
#include "IDTDef.h"

/**
 * Define the USB hot-plug callback function to monitor the info when
 * plug in/out the reader. <br/> It should be registered using the
 * registerHotplugCallBk, The first integer parameter is device type,
 * and the second integer parameter is either 0: Device Plugged Out or
 * 1: Device Plugged In
 */
typedef void (*pMessageHotplug)(int,
                                int);

/**
 * Define the send command callback function to monitor the sending
 * command into the reader. <br/> It should be registered using the
 * registerLogCallBk
 */
typedef void (*pSendDataLog)(unsigned char *,
                             int);

/**
 * Define the read response callback function to monitor the reading
 * response from the reader. <br/> It should be registered using the
 * registerLogCallBk
 */
typedef void (*pReadDataLog)(unsigned char *,
                             int);

/**
 * Define the EMV callback function to get the transaction
 * message/data/result. <br/> It should be registered using the
 * emv_registerCallBk
 */
typedef void (*pEMV_callBack)(int,
                              int,
                              unsigned char *,
                              int,
                              IDTTransactionData*,
                              EMV_Callback*,
                              int);

/**
 * Define the firmware update callback function to get the firmware
 * update status <br/> It should be registered using the
 * device_registerFWCallBk
 */
typedef void (*pFW_callBack)(int,
                             int,
                             int,
                             int,
                             int);

/**
 * Define the MSR callback function to get the MSR card data <br/>
 * It should be registered using the msr_registerCallBk,
 */
typedef void (*pMSR_callBack)(int,
                              IDTMSRData);

/**
 * Define the MSR callback function to get the MSR card data <br/>
 * It should be registered using the msr_registerCallBk,
 */
typedef void (*pMSR_callBackp)(int,
                               IDTMSRData *);

/**
 * Define the PINPad callback function to get the input PIN Pad data <br/>
 * It should be registered using the pin_registerCallBk,
 */
typedef void (*pPIN_callBack)(int,
                              IDTPINData *);

/**
 * Define the comm callback function to get FTP file transfer status <br/>
 * It should be passed as a parameter in a FTP request,
 * Signature (int, int, int) = response code, current block, total blocks
 * RESPONSE CODES:
 *              100 = FILE DOWNLOAD STARTED
 *              101 = FILE BLOCK XX OF XX RECEIVED
 *              102 = FILE DOWNLOAD COMPLETED
 *              103 = FILE DOWNLOAD TERMINATED PREMATURELY

 */
typedef void (*ftpComm_callBack)(int,
                                 int,
                                 int);

/**
 * Define the comm callback function to get the async url data <br/>
 * It should be registered using the comm_registerHTTPCallback
 */
typedef void (*httpComm_callBack)(BYTE*,
                                  int);

/**
 * Define the comm callback function to receive the V4 Protocol packets
 * received by the device from an external source (IP/USB/RS-232)
 * It should be registered using the comm_registerV4Callback,
 * Data callback will contain command, sub-command, and data from V4 packet
 */
typedef void (*v4Comm_callBack)( BYTE,
                                 BYTE,
                                 BYTE*,
                                 int);


#ifdef __cplusplus
extern "C" {
#endif

/**
 *To register the USB HID hot-plug callback function which implemented
 *in the application to monitor the hotplug message from the SDK.
 */
void registerHotplugCallBk(pMessageHotplug pMsgHotplug);

/**
 *To register the log callback function which implemented in the
 *application to monitor sending/reading data between application and
 *reader.
 */
void registerLogCallBk(pSendDataLog pFSend,
                       pReadDataLog pFRead);

/**
 *To register the firmware update callback function to get the
 *firmware update processing response.
 */
void device_registerFWCallBk(pFW_callBack pFWf);

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
 * Set the path to use when searching for ID TECH's libraries.  If
 * this is not set, the libraries will be searched for with the
 * system's default procedures.
 *
 * @param absoluteLibraryPath The absolute path to ID TECH's libraries.
 */
void setAbsoluteLibraryPath(const char *absoluteLibraryPath);

/**
 * Set the path to the config xml file(s) if any <br/>
 *
 * @param path The path to the config xml files (such as
 *  "NEO2_Devices.xml" which contains the information of NEO2
 *  devices).  Only need to specify the path to the folder contains
 *  the config files.  File names are not needed.  The maximum length
 *  of path is 200 characters including the '\0' at the end.
 */
void device_setConfigPath(const char *path);

/**
 * Pass the content of the config xml file ("NEO2_Devices.xml") as a
 * string to the SDK instead of reading the config xml file by the SDK
 * It needs to be called before device_init(), otherwise the SDK will
 * try to read the config xml file.
 *
 * @param configs The content read from the config xml file ("NEO2_Devices.xml"
 *  which contains the information of NEO2 devices).
 * @param len The length of the string configs.  The maximum length is
 *  5000 bytes.
 */
int device_setNEO2DevicesConfigs(IN const char *configs,
                                 IN int len);

/**
 * Initial the device by USB<br/>
 * It will detect the device and trying connect. <br/>
 * The connect status can be checked by device_isConnected(). <br/>
 * @return RETURN_CODE: Values can be parsed with device_getErrorString()
 */
int device_init();

/**
 * Initial the device by RS232<br/>
 * It will try to connect to the device with provided deviceType, port_number, and brate. <br/>
 * @param deviceType Device to connect to
 * @param port_number Port number of the device
 *
 *   Port nr. |  Linux  | Windows
 * 	-----------------------------
 * 	| 0       | ttyS0   | COM1  |
 * 	| 1       | ttyS1   | COM2  |
 * 	| 2       | ttyS2   | COM3  |
 * 	| 3       | ttyS3   | COM4  |
 * 	| 4       | ttyS4   | COM5  |
 * 	| 5       | ttyS5   | COM6  |
 * 	| 6       | ttyS6   | COM7  |
 * 	| 7       | ttyS7   | COM8  |
 * 	| 8       | ttyS8   | COM9  |
 * 	| 9       | ttyS9   | COM10 |
 * 	| 10      | ttyS10  | COM11 |
 * 	| 11      | ttyS11  | COM12 |
 * 	| 12      | ttyS12  | COM13 |
 * 	| 13      | ttyS13  | COM14 |
 * 	| 14      | ttyS14  | COM15 |
 * 	| 15      | ttyS15  | COM16 |
 * 	| 16      | ttyUSB0 | n.a.  |
 * 	| 17      | ttyUSB1 | n.a.  |
 * 	| 18      | ttyUSB2 | n.a.  |
 * 	| 19      | ttyUSB3 | n.a.  |
 * 	| 20      | ttyUSB4 | n.a.  |
 * 	| 21      | ttyUSB5 | n.a.  |
 * 	| 22      | ttyAMA0 | n.a.  |
 * 	| 23      | ttyAMA1 | n.a.  |
 * 	| 24      | ttyACM0 | n.a.  |
 * 	| 25      | ttyACM1 | n.a.  |
 * 	| 26      | rfcomm0 | n.a.  |
 * 	| 27      | rfcomm1 | n.a.  |
 * 	| 28      | ircomm0 | n.a.  |
 * 	| 29      | ircomm1 | n.a.  |
 * 	| 30      | cuau0   | n.a.	|
 * 	| 31      | cuau1   | n.a.  |
 * 	| 32      | cuau2   | n.a.  |
 * 	| 33      | cuau3   | n.a.  |
 * 	| 34      | cuaU0   | n.a.  |
 * 	| 35      | cuaU1   | n.a.  |
 * 	| 36      | cuaU2   | n.a.  |
 * 	| 37      | cuaU3   | n.a.  |
 *
 * @param brate Bitrate of the device
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int rs232_device_init(int deviceType, int port_number, int brate);

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
 * Check if the device is attached to the USB port
 * The function device_init() must be called before this function.
 * @param deviceType, the device type of the USB device
 * @return 1 if the device is attached, or 0 if the device is not attached
 */
int device_isAttached(int deviceType);

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
  - 41: " from 41 to 4F, Module-specific errors for Key Manager";

  - 50: " Auto-Switch OK";
  - 51: " Auto-Switch failed";
  - 70: " Antenna Error 80h Use another card";
  - 81: " Insert or swipe card";
  - 90: " Data encryption Key does not exist";
  - 91: " Data encryption Key KSN exhausted";

*/
void device_getIDGStatusCodeString(IN int returnCode,
                                   OUT char* despcrition);

/**
 * Check the device conntected status
 * @return DEVICE_DISCONNECT=0, or DEVICE_CONNECTED = 1
 */
int device_isConnected();

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

Pings the reader.  If connected, returns success.  Otherwise, returns timeout.

* @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()
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
* @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
*/
int device_controlUserInterface(IN BYTE* values);

/**
* Get current active device type
* @return :  return the device type defined as IDT_DEVICE_TYPE in the IDTDef.h
*/
int device_getCurrentDeviceType();

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
int device_SendDataCommandNEO(IN int cmd,
                              IN int subCmd,
                              IN BYTE* data,
                              IN int dataLen,
                              OUT BYTE* response,
                              IN_OUT int *respLen);

//  /**
// * Start Remote Key Injection
// *
// Starts a remote key injection request with IDTech RKI servers.
// This function is reserved and not implemented.
//
// * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString
//
// */
//  int device_startRKI();


/**
* Enable Pass Through
*
* Enables Pass Through Mode for direct communication with L1 interface
* (power on icc, send apdu, etc).
*
* @param enablePassThrough 1 = pass through ON, 0 = pass through OFF

* @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()
*/
int device_enablePassThrough(int enablePassThrough);

/**
  * Enable L100 Pass Through
  *
  * Enables Pass Through Mode for direct communication to L100 hook up
  * to NEO II device
  *
  * @param enableL100PassThrough 1 = pass through ON, 0 = pass through OFF
  *
  * @return RETURN_CODE:  Values can be parsed with errorCode.getErrorString()
  */
int device_enableL100PassThrough(int enableL100PassThrough);

/**
  * Get L100 Pass Through Mode
  *
  * Get current Pass Through Mode for direct communication to L100
  * hook up to NEO II device
  *
  * @return RETURN_CODE: return 1 if L100 Pass Through Mode is TRUE,
  *  0 if L100 Pass Through Mode is FALSE
  */
int device_getL100PassThroughMode();

/**
 * Send Burst Mode
 *
 * Sets the burst mode for the device.
 *
 * @param mode 0 = OFF, 1 = Always On, 2 = Auto Exit
 *
 * @return success or error code.  Values can be parsed with
 * device_getIDGStatusCodeString
 * @see ErrorCode
 */
int device_setBurstMode(IN BYTE mode);

/**
 * Set Poll Mode
 *
 * Sets the poll mode forthe device. Auto Poll keeps reader active,
 * Poll On Demand only polls when requested by terminal
 *
 * @param mode 0 = Auto Poll, 1 = Poll On Demand
 *
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int device_setPollMode(IN BYTE mode);

/**
* Set Merchant Record
Sets the merchant record for ApplePay VAS
*
* @param index Merchant Record index, valid values 1-6
* @param enabled Merchant Enabled/Valid flag
* @param merchantID  Merchant unique identifer registered with Apple.
*   Example com.idtechproducts.applePay
* @param merchantURL Merchant URL, when applicable
*
* @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
*
*/

int device_setMerchantRecord(int index,
                             int enabled,
                             char* merchantID,
                             char* merchantURL);
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
 * @return success or error code.  Values can be parsed
 * with device_getIDGStatusCodeString()
 * @see ErrorCode
 */
int device_getMerchantRecord(IN int index,
                             OUT BYTE *record);

/**
 * Get Transaction Results
 * Gets the transaction results when the reader is functioning in
 * "Auto Poll" mode
 *
 * @param cardData The transaction results
 *
 * @return success or error code. Values can be parsed
 *         with device_getResponseCodeString
 * @see ErrorCode
 */
int device_getTransactionResults(IDTMSRData *cardData);

/**
 * Start Transaction Request
 *
 Authorizes the transaction for an MSR/CTLS/ICC card

 The tags will be returned in the callback routine.

 @param amount Transaction amount value (tag value 9F02)
        - SEE IMPORTANT NOTE BELOW
 @param amtOther Other amount value, if any (tag value 9F03)
        - SEE IMPORTANT NOTE BELOW
 @param type Transaction type (tag value 9C).
 @param timeout Timeout value in seconds.
 @param tags Any other tags to be included in the request.  Passed as TLV.
   Example, tag 9F02 with amount 0x000000000100 would be 0x9F0206000000000100
   If tags 9F02 (amount),9F03 (other amount), or 9C (transaction type) are
   included, they will take priority over these values supplied as individual
   parameters to this method.

 @param tagsLen The length of tags data buffer.

 >>>>>IMPORTANT: parameters for amount and amtOther MUST BE PASSED AS
      A DOUBLE VALUE WITH DECIMAL POINT.  Example, do not pass 1, but
      instead pass 1.0 or 1.00. Otherwise, results will be
      unpredictable

 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 * Note: if auto poll is on, it will returm the error
 * IDG_P2_STATUS_CODE_COMMAND_NOT_ALLOWED
 *
 * NOTE ON APPLEPAY VAS:
 * To enable ApplePay VAS, first a merchant record must be defined in
 * one of the six available index positions (1-6) using
 * device_setMerchantRecord, then container tag FFEE06 must be sent as
 * part of the additional tags parameter of device_startTransaction.
 * Tag FFEE06 must contain tag 9F26 and 9F22, and can optionanally
 * contain tags 9F2B and DFO1.  Example
 * FFEE06189F220201009F2604000000009F2B050100000000DF010101
 *
 * 9F22 = two bytes = ApplePay Terminal Applicaiton Version Number.
 *        Hard defined as 0100 for now. (required)
 * 9F26 = four bytes = ApplePay Terminal Capabilities Information (required)
 *  - Byte 1 = RFU
 *  - Byte 2 = Terminal Type
 *  - - Bit 8 = VAS Support     (1=on, 0 = off)
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
 *  9F2B = 5 bytes = ApplePay VAS Filter.  Each byte filters for that
 *         specific merchant index  (optional)
 *  DF01 = 1 byte = ApplePay VAS Protocol.  (optional)
 *  - - Bit 1 : 1 = URL VAS, 0 = Full VAS
 *  - - Bit 2 : 1 = VAS Beeps, 0 = No VAS Beeps
 *  - - Bit 3 : 1 = Silent Comm Error, 2 = EMEA Comm Error
 *  - - Bit 4-8 : RFU
 *
 */
int device_startTransaction(IN double amount,
                            IN double amtOther,
                            IN int type,
                            IN const int _timeout,
                            IN BYTE *tags,
                            IN int tagsLen);

/**
 * Activate Transaction Request
 *
 Authorizes the transaction for an MSR/CTLS/ICC card

 The tags will be returned in the callback routine.

 @param timeout Timeout value in seconds.
 @param tags The tags to be included in the request.  Passed as a TLV.
   Example, tag 9F02 with amount 0x000000000100 would be 0x9F0206000000000100
   Be sure to include 9F02 (amount)and 9C (transaction type).

 @param tagsLen The length of tags data buffer.

 >>>>>IMPORTANT: parameters for amount and amtOther MUST BE PASSED AS
      A DOUBLE VALUE WITH DECIMAL POINT.  Example, do not pass 1, but
      instead pass 1.0 or 1.00. Otherwise, results will be
      unpredictable

 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 * Note: if auto poll is on, it will returm the error
 * IDG_P2_STATUS_CODE_COMMAND_NOT_ALLOWED
 *
 * NOTE ON APPLEPAY VAS:
 * To enable ApplePay VAS, first a merchant record must be defined in
 * one of the six available index positions (1-6) using
 * device_setMerchantRecord, then container tag FFEE06 must be sent as
 * part of the additional tags parameter of device_startTransaction.
 * Tag FFEE06 must contain tag 9F26 and 9F22, and can optionanally
 * contain tags 9F2B and DFO1.  Example
 * FFEE06189F220201009F2604000000009F2B050100000000DF010101
 * 9F22 = two bytes = ApplePay Terminal Applicaiton Version Number.
 *        Hard defined as 0100 for now. (required)
 * 9F26 = four bytes = ApplePay Terminal Capabilities Information (required)
 *  - Byte 1 = RFU
 *  - Byte 2 = Terminal Type
 *  - - Bit 8 = VAS Support     (1=on, 0 = off)
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
 *  9F2B = 5 bytes = ApplePay VAS Filter.  Each byte filters for that
 *         specific merchant index  (optional)
 *  DF01 = 1 byte = ApplePay VAS Protocol.  (optional)
 *  - - Bit 1 : 1 = URL VAS, 0 = Full VAS
 *  - - Bit 2 : 1 = VAS Beeps, 0 = No VAS Beeps
 *  - - Bit 3 : 1 = Silent Comm Error, 2 = EMEA Comm Error
 *  - - Bit 4-8 : RFU
 */
int device_activateTransaction(IN const int _timeout,
                               IN BYTE *tags,
                               IN int tagsLen);

/**
 * Cancel Transaction request.
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int device_cancelTransaction();

/**
   * Configures the buttons on the ViVOpay Vendi reader
   * @param done
   *   0x01: the Done switch is enabled
   *   0x00: the Done switch is disabled
   *
   * @param swipe
   *   0x01: the Swipe Card switch is enabled
   *   0x00: the Swipe Card switch is disabled
   *
   * @param delay
   *   an unsigned delay value (<= 30) in seconds
   *
   * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
   */
int device_configureButtons(IN BYTE done,
                            IN BYTE swipe,
                            IN BYTE delay);

/**
 * Reads the button configuration from the ViVOpay Vendi reader
 * @param done
 *     0x01: the Done switch is enabled
 *     0x00: the Done switch is disabled
 *
 * @param swipe
 *     0x01: the Swipe Card switch is enabled
 *     0x00: the Swipe Card switch is disabled
 *
 * @param delay
 *     an unsigned delay value in seconds
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int device_getButtonConfiguration(OUT BYTE *done,
                                  OUT BYTE *swipe,
                                  OUT BYTE *delay);
/**
  * Stops the blue LEDs on the ViVOpay Vendi reader from flashing in
  *   left to right sequence and turns the LEDs off, and contactless
  *   function is disabled at the same time
  *
  * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
  */

int device_disableBlueLED();
/**
 * Use this function to control the blue LED behavior on the Vendi reader
 *
 * @param data
 *     Sequence data
 *       Byte 0 (Cycle): 0 = Cycle once, 1 = Repeat
 *       Byte 1 (LEDs): LED State Bitmap
 *       Byte 2-3 (Duration): Given in multiples of 10 millisecond
 *       Byte 4 (LED): LED State Bitmap
 *       Byte 5-6 (Duration): Given in multiples of 10 millisecond
 *       Byte 7-24 (Additional LED/Durations): Define up to 8 LED and
 *                 duration pars
 *
 *     LED State Bitmap:
 *             Bit 8: Left blue LED, 0 = off, 1 = on
 *             Bit 7: Center Blue LED, 0 = off, 1 = on
 *             Bit 6: Right Blue LED0 = off, 1 = on
 *             Bit 5: Yellow LED, 0 = off, 1 = on
 *             Bit 4: Reserved for future use
 *             Bit 3: Reserved for future use
 *             Bit 2: Reserved for future use
 *             Bit 1: Reserved for future use
 *
 * @param dataLen
 *     Length of the sequence data: 0 or 4 to 25 bytes
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int device_enableBlueLED(IN BYTE *data,
                         IN int dataLen);

/**
 * Use this function to clear the LCD display
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int device_lcdDisplayClear();

/**
 * Enable or disable the external LCD message It will turn off the
 *   external LCD messages including EMV transactions.  (For the
 *   users who only need MSR and/or CTLS transactions.)  The function
 *   only works for VP5300
 *
 *    @param enableExtLCDMsg  1 = ON, 0 = OFF
 *    @return success or error code.  Values can be parsed
 *    with device_getResponseCodeString
 *    @see ErrorCode
 */
int device_enableExternalLCDMessages(IN int enableExtLCDMsg);

/**
 * Use this function to turn off the ViVOpay Vendi reader yellow
 * LED. This LED is located below the three blue LEDs
 *
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int device_turnOffYellowLED();

/**
 * Use this function to turn on the ViVOpay Vendi reader yellow
 * LED. This LED is located below the three blue LEDs
 *
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int device_turnOnYellowLED();

/**
 * Use this function to make the buzzer beep once
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int device_buzzerOnOff();

/**
 * Use this function to display text on the LCD display.
 *      On the Vendi reader the LCD is a 2-line character display.
 *
 * @param message
 *      Valid messages for the first line of text are between 1 and 16
 *      printable characters long.
 *      If the text message is greater than 16 bytes but not more than 32 bytes,
 *           byte 17 and onward are displayed as a second row of text.
 *      All messages are left justified on the LCD display.
 *
 * @param messageLen
 *      Length of the message: 1 to 32 bytes
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int device_lcdDisplayLine1Message(IN BYTE *message,
                                  IN int messageLen);

/**
 * Use this function to display the message on line 2 of the LCD display.
 *      On the Vendi reader the LCD is a 2-line character display.
 *
 * @param message
 *      Valid messages are between 1 and 16 printable characters long.
 *      All messages are left justified on the LCD display.
 *
 * @param messageLen
 *      Length of the message: 1 to 16 bytes
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int device_lcdDisplayLine2Message(IN BYTE *message,
                                  IN int messageLen);

/**
 * Get Key Status
 *
 * Gets the status of loaded keys
 * @param status newFormat for Augusta and miniSmartII only
 *     1: new format of key status
 *     0: reserved format for support previous device
 * @param status
 *     For L100, Augusta and miniSmartII:
 *        When the newFormat is 0, data format as follows.
 *        For Augusta and miniSmartII:
 *          byte 0:  PIN DUKPT Key, Does not support, always 0
 *          byte 1:  PIN Master Key, Does not support, always 0
 *          byte 2:  PIN Session Key, Does not support, always 0
 *          byte 3:  Account/MSR DUKPT Key, 1 Exists, 0 None, 0xFF STOP
 *          byte 4:  Account/ICC DUKPT Key, 1 Exists, 0 None, 0xFF STOP
 *          byte 5:  Admin DUKPT Key, 1 Exists, 0 None, 0xFF STOP
 *        For L100:
 *          byte 0:  PIN DUKPT Key
 *          byte 1:  PIN Master Key
 *          byte 2:  Standard PIN Session Key
 *          byte 3:  Desjardins PIN Session Key
 *          byte 4:  Account/MSR DUKPT Key, 1 Exists, 0 None, 0xFF STOP, Does not support, always 0
 *          byte 5:  Account/ICC DUKPT Key, 1 Exists, 0 None, 0xFF STOP, Does not support, always 0
 *          byte 6:  Admin DUKPT Key, 1 Exists, 0 None, 0xFF STOP
 *          byte 7:  Data DUKPT Key, 1 Exists, 0 None, 0xFF STOP
 *          byte 8:  MAC DUKPT Key, 1 Exists, 0 None, 0xFF STOP
 *
 *        when the newFormat is 1, data format as follows.
 *         [Block Length] [KeyStatusBlock1] [KeyStatusBlock2]...[KeyStatusBlockN]
 *        Where:
 *        [Block Length] is 2 bytes, format is Len_L Len_H, is KeyStatusBlock Number
 *        [KeyStatusBlockX> is 4 bytes, format is [Key Index and Key Name] [key slot] [key status]:
 *        [Key Index and Key Name] is 1 byte. Please refer to following table
 *            0x14    LCL-KEK to Encrypt Other Keys
 *            0x02    Data encryption Key to Encrypt ICC/MSR
 *            0x05    MAC DUKPT Key for Host-Device - MAC Verification
 *            0x05    MTK DUKPT Key for TTK Self-Test
 *            0x0C    RKI-KEK for Remote Key Injection
 *        [key slot] is 2 bytes. Range is 0 - 9999
 *        the MTK DUKPT Key slot is 16, the others are all 0
 *        [key status] is 1 byte.
 *            0 - Not Exist
 *            1 - Exist
 *        0xFF - (Stop. Only Valid for DUKPT Key)
 *    For NEO2:
 *		Each unit of three bytes represents one key's parameters (index and slot).
 *			Key Name Index (1 byte):
 *				0x14 - LCL-KEK
 *				0x01 - Pin encryption Key
 *				0x02 - Data encryption Key
 *				0x05 - MAC DUKPT Key
 *				0x0A - PCI Pairing Key
 *			Key Slot (2 bytes):
 *				Indicate different slots of a certain Key Name
 *					Example: slot =5 (0x00 0x05), slot=300 (0x01 0x2C)
 *					For BTPay380, slot is always 0
 *		For example, 0x14 0x00 0x00 0x02 0x00 0x00 0x0A 0x00 0x00 will represent
 *			[KeyNameIndex=0x14,KeySlot=0x0000], [KeyNameIndex=0x02,KeySlot=0x0000] and [KeyNameIndex=0x0A,KeySlot=0x0000]
 *
 * @param statusLen the length of status
 *
 * @return RETURN_CODE:    Values can be parsed with device_getIDGStatusCodeString
 */
int device_getKeyStatus(int *newFormat, BYTE* status,int* statusLen);
/**
 * Update Firmware
 * Updates the firmware of NEO 2 devices.
 * @param firmwareData Signed binary data of a firmware file provided by IDTech
 * @param firmwareDataLen Length of firmwareData
 * @param firmwareName Firmware name.
 *  - For example "VP5300_v1.00.023.0167.S_Test.fm"
 * @param encryptionType Encryption type
 - 0 : Plaintext
 - 1 : TDES ECB, PKCS#5 padding
 - 2 : TDES CBC, PKCS#5, IV is all 0
 * @param keyBlob Encrypted firmware session key blob, TR-31 Rev B, wrapped by
 *  FW Key (Optional, none if firmware is plaintext)
 * @param keyBlobLen Length of keyBlob
 * @return RETURN_CODE:  Values can be parsed with errorCode.getErrorString()
 *
 * Firmware update status is returned in the callback with the following values:
 * sender = device type
 * state = DEVICE_FIRMWARE_UPDATE
 * current block
 * total blocks
 * ResultCode:
 * - RETURN_CODE_DO_SUCCESS = Firmware Update Completed Successfully
 * - RETURN_CODE_BLOCK_TRANSFER_SUCCESS = Current block transferred successfully
 * - Any other return code represents an error condition
 *
 */
int device_updateFirmware(IN BYTE *firmwareData,
                          IN int firmwareDataLen,
                          IN char *firmwareName,
                          IN int encryptionType,
                          IN BYTE *keyBlob,
                          IN int keyBlobLen);

/**
 * Polls device for Model Number
 *
 * @param sNumber  Returns Model Number
 *
 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString
 */
int config_getModelNumber(OUT char* sNumber);

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

 @param amount Transaction amount value (tag value 9F02)
 - SEE IMPORTANT NOTE BELOW
 @param amtOther Other amount value, if any (tag value 9F03)
 - SEE IMPORTANT NOTE BELOW
 @param type Transaction type (tag value 9C).
 @param timeout Timeout value in seconds.
 @param tags Any other tags to be included in the request.  Passed as
 TLV stream.  Example, tag 9F02 with amount 0x000000000100 would be
 0x9F0206000000000100.  If tags 9F02 (amount),9F03 (other amount), or 9C
 (transaction type) are included, they will take priority over these
 values supplied as individual parameters to this method.

 @param tagsLen The length of tags data buffer.

 >>>>>IMPORTANT: parameters for amount and amtOther MUST BE PASSED AS
      A DOUBLE VALUE WITH DECIMAL POINT.  Example, do not pass 1, but
      instead pass 1.0 or 1.00. Otherwise, results will be
      unpredictable

 * @return RETURN_CODE: Values can be parsed with
 * device_getIDGStatusCodeString() Note: if auto poll is on, it will
 * returm the error IDG_P2_STATUS_CODE_COMMAND_NOT_ALLOWED
 *
 * NOTE ON APPLEPAY VAS:
 * To enable ApplePay VAS, first a merchant record must be defined in
 * one of the six available index positions (1-6) using
 * device_setMerchantRecord, then container tag FFEE06 must be sent as
 * part of the additional tags parameter of ctls_startTransaction.
 * Tag FFEE06 must contain tag 9F26 and 9F22, and can optionanally
 * contain tags 9F2B and DF01.  Example
 * FFEE06189F220201009F2604000000009F2B050100000000DF010101
 * 9F22 = two bytes = ApplePay Terminal Applicaiton Version Number.
 *        Hard defined as 0100 for now. (required)
 * 9F26 = four bytes = ApplePay Terminal Capabilities Information (required)
 *  - Byte 1 = RFU
 *  - Byte 2 = Terminal Type
 *  - - Bit 8 = VAS Support     (1=on, 0 = off)
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
 *  9F2B = 5 bytes = ApplePay VAS Filter.  Each byte filters for that
 *         specific merchant index  (optional)
 *  DF01 = 1 byte = ApplePay VAS Protocol.  (optional)
 *  - - Bit 1 : 1 = URL VAS, 0 = Full VAS
 *  - - Bit 2 : 1 = VAS Beeps, 0 = No VAS Beeps
 *  - - Bit 3 : 1 = Silent Comm Error, 2 = EMEA Comm Error
 *  - - Bit 4-8 : RFU
 *
 */

int ctls_startTransaction(IN double amount,
                          IN double amtOther,
                          IN int type,
                          IN const int _timeout,
                          IN BYTE *tags,
                          IN int tagsLen);

/**
 * Start CTLS Transaction Request
 *
 Authorizes the CTLS transaction for an ICC card

 The tags will be returned in the callback routine.

 @param timeout Timeout value in seconds.
 @param tags The tags to be included in the request.  Passed as TLV stream.
     Example, tag 9F02 with amount 0x000000000100 would be 0x9F0206000000000100

 @param tagsLen The length of tags data buffer.

 >>>>>IMPORTANT: parameters for amount and amtOther MUST BE PASSED AS
      A DOUBLE VALUE WITH DECIMAL POINT.  Example, do not pass 1, but
      instead pass 1.0 or 1.00. Otherwise, results will be
      unpredictable

 * @return RETURN_CODE: Values can be parsed with
 * device_getIDGStatusCodeString() Note: if auto poll is on, it will
 * returm the error IDG_P2_STATUS_CODE_COMMAND_NOT_ALLOWED
 *
 * NOTE ON APPLEPAY VAS:
 * To enable ApplePay VAS, first a merchant record must be defined in
 * one of the six available index positions (1-6) using
 * device_setMerchantRecord, then container tag FFEE06 must be sent as
 * part of the additional tags parameter of ctls_startTransaction.
 * Tag FFEE06 must contain tag 9F26 and 9F22, and can optionanally
 * contain tags 9F2B and DF01.  Example
 * FFEE06189F220201009F2604000000009F2B050100000000DF010101
 * 9F22 = two bytes = ApplePay Terminal Applicaiton Version Number.
 *        Hard defined as 0100 for now. (required)
 * 9F26 = four bytes = ApplePay Terminal Capabilities Information (required)
 *  - Byte 1 = RFU
 *  - Byte 2 = Terminal Type
 *  - - Bit 8 = VAS Support (1=on, 0 = off)
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
 *  9F2B = 5 bytes = ApplePay VAS Filter.  Each byte filters for
 *         that specific merchant index  (optional)
 *  DF01 = 1 byte = ApplePay VAS Protocol.  (optional)
 *  - - Bit 1 : 1 = URL VAS, 0 = Full VAS
 *  - - Bit 2 : 1 = VAS Beeps, 0 = No VAS Beeps
 *  - - Bit 3 : 1 = Silent Comm Error, 2 = EMEA Comm Error
 *  - - Bit 4-8 : RFU
 *
 */
int ctls_activateTransaction( IN const int _timeout,
                              IN BYTE *tags,
                              IN int tagsLen);

/**
 * Cancel EMV Transaction

 Cancels the currently executing EMV transaction.

 @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()

*/
int ctls_cancelTransaction();


/**
 * Retrieve Application Data by AID
 *
 * Retrieves the Application Data as specified by the AID name passed as
 * a parameter.

 * @param AID Name of ApplicationID. Must be between 5 and 16 bytes
 * @param AIDLen the length of AID data buffer.
 * @param tlv  The TLV elements of the requested AID
 * @param tlvLen the length of tlv data buffer.

 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int ctls_retrieveApplicationData(IN BYTE* AID,
                                 IN int AIDLen,
                                 OUT BYTE* tlv,
                                 IN_OUT int *tlvLen);

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
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()

 */
int ctls_setApplicationData(IN BYTE* tlv,
                            IN int tlvLen);
/**
 * Remove Application Data by AID
 * Removes the Application Data for CTLS as specified by the AID name
 * passed as a parameter

 * @param AID Name of ApplicationID Must be between 5 and 16 bytes

 * @param AIDLen the length of AID data buffer
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int ctls_removeApplicationData(IN BYTE* AID,
                               IN int AIDLen);

/**
 * Remove All Application Data
 *
 * Removes all the Application Data
 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()
**/
int ctls_removeAllApplicationData();
/**
 * Retrieve AID list
 *
 Returns all the AID names installed on the terminal for CTLS. .

 * @param AIDList  array of AID name byte arrays
 * @param AIDListLen  the length of AIDList array buffer

 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()

 */
int ctls_retrieveAIDList(OUT BYTE * AIDList,
                         IN_OUT int *AIDListLen);

/**
 * Retrieve Terminal Data
 *

 * Retrieves the Terminal Data for CTLS. This is configuration group 0
 * (Tag FFEE - > FFEE0100).  The terminal data can also be retrieved
 * by ctls_getConfigurationGroup(0).

 * @param tlv Response returned as a TLV
 * @param tlvLen the length of tlv data buffer
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int ctls_retrieveTerminalData(OUT BYTE* tlv,
                              IN_OUT int *tlvLen);

/**
* Set Terminal Data

Sets the Terminal Data for CTLS as specified by the TLV.  The first
        TLV must be Configuration Group Number (Tag FFE4).  The
        terminal global data is group 0, so the first TLV would be
        FFE40100.  Other groups can be defined using this method (1 or
        greater), and those can be retrieved with
        ctls_getConfigurationGroup(int group), and deleted with
        ctls_removeConfigurationGroup(int group).  You cannot delete
        group 0.

 * @param tlv TerminalData configuration file
 * @param tlvLen the length of tlv data buffer
 * @retval RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int ctls_setTerminalData(IN BYTE* tlv,
                         IN int tlvLen);

/**
 * Retrieve Certificate Authority Public Key
 *

 Retrieves the CAPK for CTLS as specified by the RID/Index passed as a
 parameter.

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
int ctls_retrieveCAPK(IN BYTE * capk,
                      IN int capkLen,
                      OUT BYTE * key,
                      IN_OUT int *keyLen);

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
int ctls_setCAPK(IN BYTE * capk,
                 IN int capkLen);

/**
 * Remove Certificate Authority Public Key
 *
 Removes the CAPK as specified by the RID/Index

 * @param capk 6 byte CAPK =  5 bytes RID + 1 byte INDEX
 * @param capkLen the length of capk data buffer
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()

 */
int ctls_removeCAPK(IN BYTE * capk,
                    IN int capkLen);

/**
* Remove All Certificate Authority Public Key
 *
 Removes all the CAPK

* @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
*/
int ctls_removeAllCAPK();

/**
 * Retrieve the Certificate Authority Public Key list
 *
 Returns all the CAPK RID and Index installed on the terminal.

 * @param keys [key1][key2]...[keyn], each key 6 bytes where
    key = 5 bytes RID + 1 byte index
 * @param keysLen the length of keys data buffer
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int ctls_retrieveCAPKList(OUT BYTE * keys,
                          IN_OUT int *keysLen);

/**
 * Set Configuration Group

   Sets the Configuration Group for CTLS as specified by the TLV data

   @param tlv  Configuration Group Data in TLV format
          The first tag of the TLV data must be the group number (DFEE2D).
          A second tag must exist
 * @param tlvLen the length of tlv data buffer
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int ctls_setConfigurationGroup(IN BYTE* tlv,
                               IN int tlvLen);

/**
 * Get Configuration Group
 *
 Retrieves the Configuration for the specified Group.

* @param group Configuration Group
* @param tlv return data
* @param tlvLen the length of tlv data buffer
* @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()
*/
int ctls_getConfigurationGroup(IN int group,
                               OUT BYTE* tlv,
                               OUT int *tlvLen);

/**
* Retrieve All Configuration Groups
*
Returns all the Configuration Groups installed on the terminal for CTLS

* @param tlv  The TLV elements data
* @param tlvLen the length of tlv data buffer.
* @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()
*/
int ctls_getAllConfigurationGroups(OUT BYTE * tlv,
                                   IN_OUT int *tlvLen);

/**
 * Remove Configuration Group

Removes the Configuration as specified by the Group.  Must not by group 0

 @param group Configuration Group
 @retval RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
*/
int ctls_removeConfigurationGroup( int group);


/**
 * Polls device for EMV Kernel Version
 *
 * @param version Response returned of Kernel Version
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 *
 */
int emv_getEMVKernelVersion(OUT char* version);

/**
* Get EMV Kernel check value info
*
* @param checkValue Response returned of Kernel check value info
* @param checkValueLen the length of checkValue
* @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
*/
int emv_getEMVKernelCheckValue(OUT BYTE* checkValue,
                               IN_OUT int *checkValueLen);

/**
 * Get EMV Kernel configuration check value info
 *
 * @param checkValue Response returned of Kernel configuration check value info
 * @param checkValueLen the length of checkValue
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 *
 */
int emv_getEMVConfigurationCheckValue(OUT BYTE* checkValue,
                                      IN_OUT int *checkValueLen);
/**
Allow fallback for EMV transactions.  Default is TRUE
@param allow TRUE = allow fallback, FALSE = don't allow fallback
*/
void emv_allowFallback(IN int allow);

/**
  Enables authenticate for EMV transactions.  If a emv_startTranaction
          results in code 0x0010 (start transaction success), then
          emv_authenticateTransaction can automatically execute if
          parameter is set to TRUE
  @param authenticate TRUE = auto authenticate, FALSE = manually authenticate
  */
void emv_setAutoAuthenticateTransaction(IN int authenticate);

/**
 Enables complete for EMV transactions.  If a
  emv_authenticateTranaction results in code 0x0004 (go online), then
  emv_completeTransaction can automatically execute if parameter is
  set to TRUE
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
 * Set EMV Transaction Parameters
 *
 * Set the parameters to be used on EMV transactions for an ICC card
 * when Auto Poll is on
 *
 * The tags will be returned in the callback routine.
 *
 * @param amount Transaction amount value  (tag value 9F02)
 * @param amtOther Other amount value, if any  (tag value 9F03)
 * @param type Transaction type (tag value 9C).
 * @param timeout Timeout value in seconds.
 * @param tags Any other tags to be included in the request (Maximum
 *   Length = 500 bytes).  Passed as a string.  Example, tag 9F02 with
 *   amount 0x000000000100 would be "9F0206000000000100" If tags 9F02
 *   (amount),9F03 (other amount), or 9C (transaction type) are
 *   included, they will take priority over these values supplied as
 *   individual parameters to this method.  Note: To request tags to
 *   be included in default response, use tag DFEE1A, and specify tag
 *   list.  Example four tags 9F02, 9F36, 95, 9F37 to be included in
 *   response = DFEE1A079F029F369f9F37
 * @param tagsLen the length of tags
 */
void emv_setTransactionParameters(IN double amount,
                                  IN double amtOther,
                                  IN int type,
                                  IN int timeout,
                                  IN BYTE* tags,
                                  IN int tagsLen);

/**
 * Start EMV Transaction Request
 *
 Authorizes the EMV transaction for an ICC card

 The tags will be returned in the callback routine.

 * @param amount Transaction amount value (tag value 9F02) - SEE
 * IMPORTANT NOTE BELOW

 * @param amtOther Other amount value, if any (tag value 9F03) - SEE
 * IMPORTANT NOTE BELOW
 * @param exponent Number of characters after decimal point
 * @param type Transaction type (tag value 9C).
 * @param timeout Timeout value in seconds.
 * @param tags Any other tags to be included in the request.  Passed
 as a TLV stream.  Example, tag 9F02 with amount 0x000000000100 would
 be 0x9F0206000000000100 If tags 9F02 (amount),9F03 (other amount), or
 9C (transaction type) are included, they will take priority over
 these values supplied as individual parameters to this method.
 * @param tagsLen Length of tags
 * @param forceOnline TRUE = do not allow offline approval,  FALSE = allow
 * ICC to approve offline if terminal capable
 *
 * Note: To request tags to be included in default response, use tag
 * DFEE1A, and specify tag list.  Example four tags 9F02, 9F36, 95, 9F37
 * to be included in response = DFEE1A079F029F369f9F37
 @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 *>>>>>IMPORTANT: parameters for amount and amtOther MUST BE PASSED AS
       A DOUBLE VALUE WITH DECIMAL POINT.  Example, do not pass 1, but
       instead pass 1.0 or 1.00. Otherwise, results will be
       unpredictable
 */
int emv_startTransaction(IN double amount,
                         IN double amtOther,
                         IN int exponent,
                         IN int type,
                         IN int timeout,
                         IN BYTE* tags,
                         IN int tagsLen,
                         IN int forceOnline);

/**
 * Activate EMV Transaction Request
 *
 Authorizes the EMV transaction for an ICC card

 The tags will be returned in the callback routine.

 * @param timeout Timeout value in seconds.
 * @param tags Tags to be included in the request.  Passed as a TLV stream.
 * Example, tag 9F02 with amount 0x000000000100 would be 0x9F0206000000000100
 * @param tagsLen Length of tags
 * @param forceOnline TRUE = do not allow offline approval, FALSE =
 * allow ICC to approve offline if terminal capable
 * Note: To request tags to be included in default response, use tag
 * DFEE1A, and specify tag list.  Example four tags 9F02, 9F36, 95,
 * 9F37 to be included in response = DFEE1A079F029F369f9F37
 @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
*/
int emv_activateTransaction(IN int timeout,
                            IN BYTE* tags,
                            IN int tagsLen,
                            IN int forceOnline);
/**
* Authenticate EMV Transaction Request
*
Authenticates the EMV transaction for an ICC card.  Execute this after
receiving response with result code 0x10 to emv_startTransaction

The tags will be returned in the callback routine.

 * @param updatedTLV  TLV stream that can be used to update the following values:
     - 9F02: Amount
     - 9F03: Other amount
     - 9C: Transaction type
     - 5F57: Account type

    In addition tag DFEE1A can be sent to specify tag list to include
    in results. Example four tags 9F02, 9F36, 95, 9F37 to be included
    in response = DFEE1A079F029F36959F37
 * @param updatedTLVLen
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
*/
int emv_authenticateTransaction(IN BYTE* updatedTLV,
                                IN int updatedTLVLen);

/**
* Complete EMV Transaction Request
*
* Completes the EMV transaction for an ICC card when online
* authorization request is received from emv_authenticateTransaction
*
* The tags will be returned in the callback routine.
*
* @param commError Communication error with host.  Set to TRUE(1) if
* host was unreachable, or FALSE(0) if host response received.  If
* Communication error, authCode, iad, tlvScripts can be null.
* @param authCode Authorization code from host.  Two bytes.  Example
* 0x3030.  (Tag value 8A).  Required
* @param authCodeLen the length of authCode
* @param iad Issuer Authentication Data, if any.  Example
* 0x11223344556677883030 (tag value 91).
* @param iadLen the length of iadLen
* @param tlvScripts 71/72 scripts, if any
* @param tlvScriptsLen the length of tlvScriptsLen
* @param tlv  Additional TLV data to return with transaction results (if any)
* @param tlvLen the length of tlv
* @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
*/
int emv_completeTransaction(IN int commError,
                            IN BYTE* authCode,
                            IN int authCodeLen,
                            IN BYTE* iad,
                            IN int iadLen,
                            IN BYTE* tlvScripts,
                            IN int tlvScriptsLen,
                            IN BYTE* tlv,
                            IN int tlvLen);

/**
* Cancel EMV Transaction
*
* Cancels the currently executing EMV transaction.
* @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
*/
int emv_cancelTransaction();

/**
 * Retrieve Transaction Results
 *
 Retrieves specified EMV tags from the currently executing transaction.

 * @param tags Tags to be retrieved.  Example 0x9F028A will retrieve
 * tags 9F02 and 8A
 * @param tagsLen Length of tag list
 * @param cardData All requested tags returned as unencrypted, encrypted
 * and masked TLV data in IDTTransactionData object
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_retrieveTransactionResult(IN BYTE* tags,
                                  IN int tagsLen,
                                  OUT IDTTransactionData * cardData);

/**
 * Retrieve Application Data by AID
 *
 Retrieves the Application Data as specified by the AID name passed as
 a parameter.

 * @param AID Name of ApplicationID. Must be between 5 and 16 bytes
 * @param AIDLen the length of AID data buffer.
 * @param tlv  The TLV elements of the requested AID
 * @param tlvLen the length of tlv data buffer.

 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int emv_retrieveApplicationData(IN BYTE* AID,
                                IN int AIDLen,
                                OUT BYTE* tlv,
                                IN_OUT int *tlvLen);

/**
 * Set Application Data by AID
 *
 Sets the Application Data as specified by the application name and TLV data

 * @param name Application name, 10-32 ASCII hex characters representing
 * 5-16 bytes  Example "a0000000031010"
 * @param nameLen the length of name data buffer of Application name,
 * @param tlv  Application data in TLV format
 * @param tlvLen the length of tlv data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_setApplicationData(IN BYTE* name,
                           IN int nameLen,
                           IN BYTE* tlv,
                           IN int tlvLen);

/**
 * Set Application Data by TLV
 *
 * Sets the Application Data as specified by the TLV data
 * @param tlv  Application data in TLV format
 *              The first tag of the TLV data must be the group number (DFEE2D).
 *              The second tag of the TLV data must be the AID (9F06)
 *              Example valid TLV, for Group #2, AID a0000000035010:
 *              "dfee2d01029f0607a0000000051010ffe10101ffe50110ffe30114ffe20106"
 * @param tlvLen the length of tlv data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_setApplicationDataTLV(IN BYTE* tlv,
                              IN int tlvLen);

/**
 * Remove Application Data by AID
 * Removes the Application Data for CTLS as specified by the AID name passed
 * as a parameter
 * @param AID Name of ApplicationID Must be between 5 and 16 bytes
 * @param AIDLen the length of AID data buffer
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int emv_removeApplicationData(IN BYTE* AID,
                              IN int AIDLen);

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
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int emv_retrieveAIDList(OUT BYTE * AIDList,
                        IN_OUT int *AIDListLen);

/**
 * Retrieve Terminal Data
 *
 * Retrieves the Terminal Data for CTLS. This is configuration group 0
 * (Tag FFEE - > FFEE0100).  The terminal data can also be retrieved
 * by ctls_getConfigurationGroup(0).
 * @param tlv Response returned as a TLV
 * @param tlvLen the length of tlv data buffer
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int emv_retrieveTerminalData(OUT BYTE* tlv,
                             IN_OUT int *tlvLen);

/**
* Set Terminal Data
*
   Sets the Terminal Data for CTLS as specified by the TLV.  The first
   TLV must be Configuration Group Number (Tag FFE4).  The terminal
   global data is group 0, so the first TLV would be FFE40100.  Other
   groups can be defined using this method (1 or greater), and those
   can be retrieved with emv_getConfigurationGroup(int group), and
   deleted with emv_removeConfigurationGroup(int group).  You cannot
   delete group 0.

 * @param tlv TerminalData configuration file
 * @param tlvLen the length of tlv data buffer
 * @retval RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int emv_setTerminalData(IN BYTE* tlv,
                        IN int tlvLen);

/**
 Sets the terminal major configuration in ICS .
 @param configuration A configuration value, range 1-5
 - 1 = 1C
 - 2 = 2C
 - 3 = 3C
 - 4 = 4C
 - 5 = 5C

 *  @return RETURN_CODE: Values can be parsed with device_getResponseCodeString()
 */
int emv_setTerminalMajorConfiguration(IN int configuration);

/**
 * Retrieve Certificate Authority Public Key
 *
 Retrieves the CAPK for CTLS as specified by the RID/Index passed as a
 parameter.

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
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int emv_retrieveCAPK(IN BYTE * capk,
                     IN int capkLen,
                     OUT BYTE * key,
                     IN_OUT int *keyLen);

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
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int emv_setCAPK(IN BYTE * capk,
                IN int capkLen);

/**
 * Remove Certificate Authority Public Key
 *
 Removes the CAPK as specified by the RID/Index

 * @param capk 6 byte CAPK =  5 bytes RID + 1 byte INDEX
 * @param capkLen the length of capk data buffer
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int emv_removeCAPK(IN BYTE * capk,
                   IN int capkLen);

/**
* Remove All Certificate Authority Public Key
 *
 Removes all the CAPK

* @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()

*/
int emv_removeAllCAPK();

/**
 * Retrieve the Certificate Authority Public Key list
 *
 Returns all the CAPK RID and Index installed on the terminal.

 * @param keys [key1][key2]...[keyn], each key 6 bytes where
    key = 5 bytes RID + 1 byte index
 * @param keysLen the length of keys data buffer
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int emv_retrieveCAPKList(OUT BYTE * keys,
                         IN_OUT int *keysLen);

/**
 * Retrieve the Certificate Revocation List
 *
 Returns the CRL entries on the terminal.

 * @param list [CRL1][CRL2]...[CRLn], each CRL 9 bytes where
    CRL = 5 bytes RID + 1 byte index + 3 bytes serial number
 * @param lssLen the length of list data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_retrieveCRL(OUT BYTE* list,
                    IN_OUT int* lssLen);

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
int emv_setCRL(IN BYTE* list,
               IN int lsLen);

/**
 * Retrieve the Certificate Revocation List
 *
 * Returns the CRL entries on the terminal.

 * @param list [CRL1][CRL2]...[CRLn], each CRL 9 bytes where
    CRL = 5 bytes RID + 1 byte index + 3 bytes serial number
 * @param lssLen the length of list data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_removeCRL(IN BYTE* list,
                  IN int lsLen);
/**
 * Remove All Certificate Revocation List Entries
 *
 * Removes all CRLEntry entries
 * @return RETURN_CODE: Values can be parsed with device_getResponseCodeString()

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
 *
 * Enables MSR, waiting for swipe to occur. Allows track
 * selection. Returns IDTMSRData instance to swipeMSRData()
 * @param timeout Swipe Timeout Value
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 * Note: if auto poll mode is on, it will return command not allowed error
 */
int msr_startMSRSwipe(IN int _timeout);

/**
 * Parser the MSR data from the buffer into IDTMSTData structure
 * @param resData MSR card data buffer
 * @param resLen the length of resData
 * @param cardData the parser result with IDTMSTData structure
 */
void parseMSRData(IN BYTE *resData,
                  IN int resLen,
                  IN_OUT IDTMSRData *cardData);

#ifdef __cplusplus
}
#endif


#endif

/*! \file libIDT_NEO2.h
 \brief NEO2 API.

 NEO2 Global API methods.
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
