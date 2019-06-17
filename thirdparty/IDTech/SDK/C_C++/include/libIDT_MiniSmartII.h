#ifndef __LIBIDT_AUGUSTA_H___
#define __LIBIDT_AUGUSTA_H___


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
 * Define the MSR callback function to get pointer to the MSR card data <br/>
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
 - 3: "time out for task or CMD";
 - 4: "wrong parameter";
 - 5: "SDK is doing MSR or ICC task";
 - 6: "SDK is doing PINPad task";
 - 7: "SDK is doing CTLS task";
 - 8: "SDK is doing EMV task";
 - 9: "SDK is doing Other task";
 - 10: "err response or data";
 - 11: "no reader attached";
 - 12: "mono audio is enabled";
 - 13: "did connection";
 - 14: "audio volume is too low";
 - 15: "task or CMD be canceled";
 - 16: "UF wrong string format";
 - 17: "UF file not found";
 - 18: "UF wrong file format";
 - 19: "Attempt to contact online host failed";
 - 20: "Attempt to perform RKI failed";
 - 22: "Buffer size is not enough";
 - 0X300: "Key Type(TDES) of Session Key is not same as the related Master Key.";
 - 0X400: "Related Key was not loaded.";
 - 0X500: "Key Same.";
 - 0X501: "Key is all zero";
 - 0X502: "TR-31 format error";
 - 0X702: "PAN is Error Key.";
 - 0X705: "No Internal MSR PAN (or Internal MSR PAN is erased timeout)";
 - 0X0C01: "Incorrect Frame Tag";
 - 0X0C02: "Incorrect Frame Type";
 - 0X0C03: "Unknown Frame Type";
 - 0X0C04: "Unknown Command";
 - 0X0C05: "Unknown Sub-Command";
 - 0X0C06: "CRC Error";
 - 0X0C07: "Failed";
 - 0X0C08: "Timeout";
 - 0X0C0A: "Incorrect Parameter";
 - 0X0C0B: "Command Not Supported";
 - 0X0C0C: "Sub-Command Not Supported";
 - 0X0C0D: "Parameter Not Supported / Status Abort Command";
 - 0X0C0F: "Sub-Command Not Allowed";
 - 0X0D01: "Incorrect Header Tag";
 - 0X0D02: "Unknown Command";
 - 0X0D03: "Unknown Sub-Command";
 - 0X0D04: "CRC Error in Frame";
 - 0X0D05: "Incorrect Parameter";
 - 0X0D06: "Parameter Not Supported";
 - 0X0D07: "Mal-formatted Data";
 - 0X0D08: "Timeout";
 - 0X0D0A: "Failed / NACK";
 - 0X0D0B: "Command not Allowed";
 - 0X0D0C: "Sub-Command not Allowed";
 - 0X0D0D: "Buffer Overflow (Data Length too large for reader buffer)";
 - 0X0D0E: "User Interface Event";
 - 0X0D11: "Communication type not supported, VT-1, burst, etc.";
 - 0X0D12: "Secure interface is not functional or is in an intermediate state.";
 - 0X0D13: "Data field is not mod 8";
 - 0X0D14: "Pad  - 0X80 not found where expected";
 - 0X0D15: "Specified key type is invalid";
 - 0X0D1: "Could not retrieve key from the SAM(InitSecureComm)";
 - 0X0D17: "Hash code problem";
 - 0X0D18: "Could not store the key into the SAM(InstallKey)";
 - 0X0D19: "Frame is too large";
 - 0X0D1A: "Unit powered up in authentication state but POS must resend the InitSecureComm command";
 - 0X0D1B: "The EEPROM may not be initialized because SecCommInterface does not make sense";
 - 0X0D1C: "Problem encoding APDU";
 - 0X0D20: "Unsupported Index(ILM) SAM Transceiver error - problem communicating with the SAM(Key Mgr)";
 - 0X0D2: "Unexpected Sequence Counter in multiple frames for single bitmap(ILM) Length error in data returned from the SAM(Key Mgr)";
 - 0X0D22: "Improper bit map(ILM)";
 - 0X0D23: "Request Online Authorization";
 - 0X0D24: "ViVOCard3 raw data read successful";
 - 0X0D25: "Message index not available(ILM) ViVOcomm activate transaction card type(ViVOcomm)";
 - 0X0D26: "Version Information Mismatch(ILM)";
 - 0X0D27: "Not sending commands in correct index message index(ILM)";
 - 0X0D28: "Time out or next expected message not received(ILM)";
 - 0X0D29: "ILM languages not available for viewing(ILM)";
 - 0X0D2A: "Other language not supported(ILM)";
 - 0X0D41: "Unknown Error from SAM";
 - 0X0D42: "Invalid data detected by SAM";
 - 0X0D43: "Incomplete data detected by SAM";
 - 0X0D44: "Reserved";
 - 0X0D45: "Invalid key hash algorithm";
 - 0X0D46: "Invalid key encryption algorithm";
 - 0X0D47: "Invalid modulus length";
 - 0X0D48: "Invalid exponent";
 - 0X0D49: "Key already exists";
 - 0X0D4A: "No space for new RID";
 - 0X0D4B: "Key not found";
 - 0X0D4C: "Crypto not responding";
 - 0X0D4D: "Crypto communication error";
 - 0X0D4E: "Module-specific error for Key Manager";
 - 0X0D4F: "All key slots are full (maximum number of keys has been installed)";
 - 0X0D50: "Auto-Switch OK";
 - 0X0D51: "Auto-Switch failed";
 - 0X0D90: "Account DUKPT Key not exist";
 - 0X0D91: "Account DUKPT Key KSN exausted";
 - 0X0D00: "This Key had been loaded.";
 - 0X0E00: "Base Time was loaded.";
 - 0X0F00: "Encryption Or Decryption Failed.";
 - 0X1000: "Battery Low Warning (It is High Priority Response while Battery is Low.)";
 - 0X1800: "Send 'Cancel Command' after send 'Get Encrypted PIN' & 'Get Numeric' & 'Get Amount';
 - 0X1900: "Press 'Cancel' key after send 'Get Encrypted PIN' & 'Get Numeric' & 'Get Amount';
 - 0X30FF: "Security Chip is not connect";
 - 0X3000: "Security Chip is deactivation & Device is In Removal Legally State.";
 - 0X3101: "Security Chip is activation &  Device is In Removal Legally State.";
 - 0X5500: "No Admin DUKPT Key.";
 - 0X5501: "Admin  DUKPT Key STOP.";
 - 0X5502: "Admin DUKPT Key KSN is Error.";
 - 0X5503: "Get Authentication Code1 Failed.";
 - 0X5504: "Validate Authentication Code Error.";
 - 0X5505: "Encrypt or Decrypt data failed.";
 - 0X5506: "Not Support the New Key Type.";
 - 0X5507: "New Key Index is Error.";
 - 0X5508: "Step Error.";
 - 0X5509: "KSN Error";
 - 0X550A: "MAC Error.";
 - 0X550B: "Key Usage Error.";
 - 0X550C: "Mode Of Use Error.";
 - 0X550F: "Other Error.";
 - 0X6000: "Save or Config Failed / Or Read Config Error.";
 - 0X6200: "No Serial Number.";
 - 0X6900: "Invalid Command - Protocol is right, but task ID is invalid.";
 - 0X6A01: "Unsupported Command - Protocol and task ID are right, but command is invalid - In this State";
 - 0X6A00: "Unsupported Command - Protocol and task ID are right, but command is invalid.";
 - 0X6B00: "Unknown parameter in command - Protocol task ID and command are right, but parameter is invalid.";
 - 0X6C00: "Unknown parameter in command - Protocol task ID and command are right, but length is out of the requirement.";
 - 0X7200: "Device is suspend (MKSK suspend or press password suspend).";
 - 0X7300: "PIN DUKPT is STOP (21 bit 1).";
 - 0X7400: "Device is Busy.";
 - 0XE100: "Can not enter sleep mode";
 - 0XE200: "File has existed";
 - 0XE300: "File has not existed";
 - 0XE313: "IO line low -- Card error after session start";
 - 0XE400: "Open File Error";
 - 0XE500: "SmartCard Error";
 - 0XE600: "Get MSR Card data is error";
 - 0XE700: "Command time out";
 - 0XE800: "File read or write is error";
 - 0XE900: "Active 1850 error!";
 - 0XEA00: "Load bootloader error";
 - 0XEF00: "Protocol Error- STX or ETX or check error.";
 - 0XEB00: "Picture is not exist";
 - 0X2C02: "No Microprocessor ICC seated";
 - 0X2C06: "no card seated to request ATR";
 - 0X2D01: "Card Not Supported,";
 - 0X2D03: "Card Not Supported, wants CRC";
 - 0X690D: "Command not supported on reader without ICC support";
 - 0X8100: "ICC error time out on power-up";
 - 0X8200: "invalid TS character received - Wrong operation step";
 - 0X8300: "Decode MSR Error";
 - 0X8400: "TriMagII no Response";
 - 0X8500: "No Swipe MSR Card";
 - 0X8510: "No Financial Card";
 - 0X8600: "Unsupported F, D, or combination of F and D";
 - 0X8700: "protocol not supported EMV TD1 out of range";
 - 0X8800: "power not at proper level";
 - 0X8900: "ATR length too long";
 - 0X8B01: "EMV invalid TA1 byte value";
 - 0X8B02: "EMV TB1 required";
 - 0X8B03: "EMV Unsupported TB1 only 00 allowed";
 - 0X8B04: "EMV Card Error, invalid BWI or CWI";
 - 0X8B06: "EMV TB2 not allowed in ATR";
 - 0X8B07: "EMV TC2 out of range";
 - 0X8B08: "EMV TC2 out of range";
 - 0X8B09: "per EMV96 TA3 must be >  - 0XF";
 - 0X8B10: "ICC error on power-up";
 - 0X8B11: "EMV T=1 then TB3 required";
 - 0X8B12: "Card Error, invalid BWI or CWI";
 - 0X8B13: "Card Error, invalid BWI or CWI";
 - 0X8B17: "EMV TC1/TB3 conflict-";
 - 0X8B20: "EMV TD2 out of range must be T=1";
 - 0X8C00: "TCK error";
 - 0XA304: "connector has no voltage setting";
 - 0XA305: "ICC error on power-up invalid (SBLK(IFSD) exchange";
 - 0XE301: "ICC error after session start";
 - 0XFF00: "Request to go online";
 - 0XFF01: "EMV: Accept the offline transaction";
 - 0XFF02: "EMV: Decline the offline transaction";
 - 0XFF03: "EMV: Accept the online transaction";
 - 0XFF04: "EMV: Decline the online transaction";
 - 0XFF05: "EMV: Application may fallback to magstripe technology";
 - 0XFF06: "EMV: ICC detected tah the conditions of use are not satisfied";
 - 0XFF07: "EMV: ICC didn't accept transaction";
 - 0XFF08: "EMV: Transaction was cancelled";
 - 0XFF09: "EMV: Application was not selected by kernel or ICC format error or ICC missing data error";
 - 0XFF0A: "EMV: Transaction is terminated";
 - 0XFF0B: "EMV: Other EMV Error";
 - 0XFFFF: "NO RESPONSE";
 - 0XF002: "ICC communication timeout";
 - 0XF003: "ICC communication Error";
 - 0XF00F: "ICC Card Seated and Highest Priority, disable MSR work request";
 - 0XF200: "AID List / Application Data is not exist";
 - 0XF201: "Terminal Data is not exist";
 - 0XF202: "TLV format is error";
 - 0XF203: "AID List is full";
 - 0XF204: "Any CA Key is not exist";
 - 0XF205: "CA Key RID is not exist";
 - 0XF206: "CA Key Index it not exist";
 - 0XF207: "CA Key is full";
 - 0XF208: "CA Key Hash Value is Error";
 - 0XF209: "Transaction  format error";
 - 0XF20A: "The command will not be processing";
 - 0XF20B: "CRL is not exist";
 - 0XF20C: "CRL number  exceed max number";
 - 0XF20D: "Amount,Other Amount,Trasaction Type  are  missing";
 - 0XF20E: "The Identification of algorithm is mistake";
 - 0XF20F: "No Financial Card";
 - 0XF210: "In Encrypt Result state, TLV total Length is greater than Max Length";
 - 0X1001: "INVALID ARG";
 - 0X1002: "FILE_OPEN_FAILED";
 - 0X1003: "FILE OPERATION_FAILED";
 - 0X2001: "MEMORY_NOT_ENOUGH";
 - 0X3002: "SMARTCARD_FAIL";
 - 0X3003: "SMARTCARD_INIT_FAILED";
 - 0X3004: "FALLBACK_SITUATION";
 - 0X3005: "SMARTCARD_ABSENT";
 - 0X3006: "SMARTCARD_TIMEOUT";
 - 0X5001: "EMV_PARSING_TAGS_FAILED";
 - 0X5002: "EMV_DUPLICATE_CARD_DATA_ELEMENT";
 - 0X5003: "EMV_DATA_FORMAT_INCORRECT";
 - 0X5004: "EMV_NO_TERM_APP";
 - 0X5005: "EMV_NO_MATCHING_APP";
 - 0X5006: "EMV_MISSING_MANDATORY_OBJECT";
 - 0X5007: "EMV_APP_SELECTION_RETRY";
 - 0X5008: "EMV_GET_AMOUNT_ERROR";
 - 0X5009: "EMV_CARD_REJECTED";
 - 0X5010: "EMV_AIP_NOT_RECEIVED";
 - 0X5011: "EMV_AFL_NOT_RECEIVED";
 - 0X5012: "EMV_AFL_LEN_OUT_OF_RANGE";
 - 0X5013: "EMV_SFI_OUT_OF_RANGE";
 - 0X5014: "EMV_AFL_INCORRECT";
 - 0X5015: "EMV_EXP_DATE_INCORRECT";
 - 0X5016: "EMV_EFF_DATE_INCORRECT";
 - 0X5017: "EMV_ISS_COD_TBL_OUT_OF_RANGE";
 - 0X5018: "EMV_CRYPTOGRAM_TYPE_INCORRECT";
 - 0X5019: "EMV_PSE_NOT_SUPPORTED_BY_CARD";
 - 0X5020: "EMV_USER_SELECTED_LANGUAGE";
 - 0X5021: "EMV_SERVICE_NOT_ALLOWED";
 - 0X5022: "EMV_NO_TAG_FOUND";
 - 0X5023: "EMV_CARD_BLOCKED";
 - 0X5024: "EMV_LEN_INCORRECT";
 - 0X5025: "CARD_COM_ERROR";
 - 0X5026: "EMV_TSC_NOT_INCREASED";
 - 0X5027: "EMV_HASH_INCORRECT";
 - 0X5028: "EMV_NO_ARC";
 - 0X5029: "EMV_INVALID_ARC";
 - 0X5030: "EMV_NO_ONLINE_COMM";
 - 0X5031: "TRAN_TYPE_INCORRECT";
 - 0X5032: "EMV_APP_NO_SUPPORT";
 - 0X5033: "EMV_APP_NOT_SELECT";
 - 0X5034: "EMV_LANG_NOT_SELECT";
 - 0X5035: "EMV_NO_TERM_DATA";
 - 0X6001: "CVM_TYPE_UNKNOWN";
 - 0X6002: "CVM_AIP_NOT_SUPPORTED";
 - 0X6003: "CVM_TAG_8E_MISSING";
 - 0X6004: "CVM_TAG_8E_FORMAT_ERROR";
 - 0X6005: "CVM_CODE_IS_NOT_SUPPORTED";
 - 0X6006: "CVM_COND_CODE_IS_NOT_SUPPORTED";
 - 0X6007: "NO_MORE_CVM";
 - 0X6008: "PIN_BYPASSED_BEFORE";
 - 0X7001: "PK_BUFFER_SIZE_TOO_BIG";
 - 0X7002: "PK_FILE_WRITE_ERROR";
 - 0X7003: "PK_HASH_ERROR";
 - 0X8001: "NO_CARD_HOLDER_CONFIRMATION";
 - 0X8002: "GET_ONLINE_PIN";
 - 0XD000: "Data not exist";
 - 0XD001: "Data access error";
 - 0XD100: "RID not exist";
 - 0XD101: "RID existed";
 - 0XD102: "Index not exist";
 - 0XD200: "Maximum exceeded";
 - 0XD201: "Hash error";
 - 0XD205: "System Busy";
 - 0X0E01: "Unable to go online";
 - 0X0E02: "Technical Issue";
 - 0X0E03: "Declined";
 - 0X0E04: "Issuer Referral transaction";
 - 0X0F01: "Decline the online transaction";
 - 0X0F02: "Request to go online";
 - 0X0F03: "Transaction is terminated";
 - 0X0F05: "Application was not selected by kernel or ICC format error or ICC missing data error";
 - 0X0F07: "ICC didn't accept transaction";
 - 0X0F0A: "Application may fallback to magstripe technology";
 - 0X0F0C: "Transaction was cancelled";
 - 0X0F0D: "Timeout";
 - 0X0F0F: "Other EMV Error";
 - 0X0F10: "Accept the offline transaction";
 - 0X0F11: "Decline the offline transaction";
 - 0X0F21: "ICC detected tah the conditions of use are not satisfied";
 - 0X0F22: "No app were found on card matching terminal configuration";
 - 0X0F23: "Terminal file does not exist";
 - 0X0F24: "CAPK file does not exist";
 - 0X0F25: "CRL Entry does not exist";
 - 0X0FFE: "code when blocking is disabled";
 - 0X0FFF: "code when command is not applicable on the selected device";
 - 0XF005: "ICC Encrypted C-APDU Data Structure Length Error Or Format Error.";
 - 0XBBE0: "CM100 Success";
 - 0XBBE1: "CM100 Parameter Error";
 - 0XBBE2: "CM100 Low Output Buffer";
 - 0XBBE3: "CM100 Card Not Found";
 - 0XBBE4: "CM100 Collision Card Exists";
 - 0XBBE5: "CM100 Too Many Cards Exist";
 - 0XBBE6: "CM100 Saved Data Does Not Exist";
 - 0XBBE8: "CM100 No Data Available";
 - 0XBBE9: "CM100 Invalid CID Returned";
 - 0XBBEA: "CM100 Invalid Card Exists";
 - 0XBBEC: "CM100 Command Unsupported";
 - 0XBBED: "CM100 Error In Command Process";
 - 0XBBEE: "CM100 Invalid Command";

 - 0X9031: "Unknown command";
 - 0X9032: "Wrong parameter (such as the length of the command is incorrect)";

 - 0X9038: "Wait (the command couldnt be finished in BWT)";
 - 0X9039: "Busy (a previously command has not been finished)";
 - 0X903A: "Number of retries over limit";

 - 0X9040: "Invalid Manufacturing system data";
 - 0X9041: "Not authenticated";
 - 0X9042: "Invalid Master DUKPT Key";
 - 0X9043: "Invalid MAC Key";
 - 0X9044: "Reserved for future use";
 - 0X9045: "Reserved for future use";
 - 0X9046: "Invalid DATA DUKPT Key";
 - 0X9047: "Invalid PIN Pairing DUKPT Key";
 - 0X9048: "Invalid DATA Pairing DUKPT Key";
 - 0X9049: "No nonce generated";
 - 0X9949: "No GUID available.  Perform getVersion first.";
 - 0X9950: "MAC Calculation unsuccessful. Check BDK value.";

 - 0X904A: "Not ready";
 - 0X904B: "Not MAC data";

 - 0X9050: "Invalid Certificate";
 - 0X9051: "Duplicate key detected";
 - 0X9052: "AT checks failed";
 - 0X9053: "TR34 checks failed";
 - 0X9054: "TR31 checks failed";
 - 0X9055: "MAC checks failed";
 - 0X9056: "Firmware download failed";

 - 0X9060: "Log is full";
 - 0X9061: "Removal sensor unengaged";
 - 0X9062: "Any hardware problems";

 - 0X9070: "ICC communication timeout";
 - 0X9071: "ICC data error (such check sum error)";
 - 0X9072: "Smart Card not powered up";
 */
void device_getResponseCodeString(IN int returnCode, OUT char* despcrition);
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
* @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
*
*/
int device_getFirmwareVersion(OUT char* firmwareVersion);
/**
* Get current active device type
* @return :  return the device type defined as IDT_DEVICE_TYPE in the IDTDef.h
*/

int device_getCurrentDeviceType();
/**
* Send a Command to device
*
* Sends a command  to the device .
*
* @param cmd buffer of command to execute.
* @param cmdLen, the length of the buffer cmd.
*
* @param data buffer of IDG command data.
* @param dataLen, the length of the buffer data.
*
* @param response Response data
* @param respLen, the length of Response data

* @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
*/

//int device_SendDataCommand(IN BYTE* cmd, IN int cmdLen, OUT BYTE* response, IN_OUT int *respLen);
int device_SendDataCommand(IN BYTE* cmd, IN int cmdLen, IN BYTE* data, IN int dataLen, OUT BYTE* response, IN_OUT int *respLen);

/**
 * Update Firmware
 * Updates the firmware of Augusta.
 * @param firmwareData Signed binary data of a firmware file provided by IDTech
 * @param firmwareDataLen Length of firmwareData
 * @param firmwareName Firmware name.
 *  - For example "Augusta_S_TTK_V1.00.002.fm"
 * @param encryptionType Encryption type
 - 0 : Plaintext
 - 1 : TDES ECB, PKCS#5 padding
 - 2 : TDES CBC, PKCS#5, IV is all 0
 * @param keyBlob Encrypted firmware session key blob, TR-31 Rev B, wrapped by FW Key (Optional, none if firmware is plaintext)
 * @param keyBlobLen Length of keyBlob
 * @return RETURN_CODE:  Values can be parsed with errorCode.getErrorString()
 *
 * Firmware update status is returned in the callback with the following values:
 * sender = AUGUSTA
 * state = DeviceState.FirmwareUpdate
 * data = File Progress.  Two bytes, with byte[0] = current block, and byte[1] = total blocks. 0x0310 = block 3 of 16
 * transactionResultCode:
 * - RETURN_CODE_DO_SUCCESS = Firmware Update Completed Successfully
 * - RETURN_CODE_BLOCK_TRANSFER_SUCCESS = Current block transferred successfully
 * - Any other return code represents an error condition
 *
 */
int device_updateFirmware(IN BYTE *firmwareData, IN int firmwareDataLen, IN char *firmwareName, IN int encryptionType, IN BYTE *keyBlob, IN int keyBlobLen);

/**
* Reboot Device
* Executes a command to restart the device.
- Card data is cleared, resetting card status bits.
- Response data of the previous command is cleared.
- Resetting firmware.
*
* @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
*/
int device_rebootDevice();
/**
* Control MSR LED
*
Controls the LED for the MSR


@param indexLED For Augusta, must be set to 1 (MSR LED)
@param control LED Status:
	- 00: OFF
	- 01: RED Solid
	- 02: RED Blink
	- 11: GREEN Solid
	- 12: GREEN Blink
	- 21: BLUE Solid
	- 22: BLUE Blink
@param intervalOn Blink interval ON, in ms	(Range 200 - 2000)
@param intervalOff Blink interval OFF, in ms  (Range 200 - 2000)


* @return RETURN_CODE:	Values can be parsed with device_getResponseCodeString

 */
 //intervalOn = 500, int intervalOff = 500
int device_controlLED(byte indexLED, byte control, int intervalOn , int intervalOff );


/**
* Control ICC LED
*
Controls the LED for the ICC card slot


@param controlMode 0 = off, 1 = solid, 2 = blink
@param interval Blink interval, in ms  (500 = 500 ms)


* @return RETURN_CODE:	Values can be parsed with device_getResponseCodeString

 */
int device_controlLED_ICC(int controlMode, int interval);

/**
 * Control the MSR LED
 *
 Controls the MSR / ICC LED
 This API not recommended to control ICC LED
 @param control
	 - 0x00 = off,
	 - 0x01 = RED Solid,
	 - 0x02 = RED Blink,
	 - 0x11 = GREEN Solid,
	 - 0x12 = GREEN Blink,
	 - 0x21 = BLUE Solid,
	 - 0x22 = BLUE Blink,
 @param intervalOn Blink interval on time last, in ms  (500 = 500 ms, valid from 200 to 2000)
 @param intervalOff Blink interval off time last, in ms  (500 = 500 ms, valid from 200 to 2000)


 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString

  */
 //  int intervalOn = 500, int intervalOff = 500)

 int device_controlLED_MSR(byte control, int intervalOn , int intervalOff );

/**
* Control Beep
*
Controls the Beeper


@param index For Augusta, must be set to 1 (only one beeper)
@param frequency Frequency, range 1000-20000 (suggest minimum 3000)
@param duration Duration, in milliseconds (range 1 - 65525)

* @return RETURN_CODE:	Values can be parsed with device_getResponseCodeString

 */
int device_controlBeep(int index, int frequency, int duration);

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
 * @return RETURN_CODE:    Values can be parsed with device_getResponseCodeString
 */
int device_getKeyStatus(int *newFormat, BYTE* status,int* statusLen);

/**
 * Polls device for Model Number
 *
 * @param sNumber  Returns Model Number
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 *
 */
int config_getModelNumber(OUT char* sNumber);
/**
 * Polls device for Serial Number
 *
 * @param sNumber  Returns Serial Number
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 *
 */
int config_getSerialNumber(OUT char* sNumber);
/**
* Set the LED Controller
* Set the MSR / ICC LED controlled by software or firmware
* NOTE: The ICC LED always controlled by software.
@param firmwareControlMSRLED
 - 1:  firmware control the MSR LED
 - 0: software control the MSR LED
@param firmwareControlICCLED
 - 1:  firmware control the ICC LED
 - 0: software control the ICC LED
* @return RETURN_CODE:	Values can be parsed with device_getResponseCodeString

 */

int config_setLEDController(int firmwareControlMSRLED, int firmwareControlICCLED);
/**
* Get the LED Controller Status
* Get the MSR / ICC LED controlled status by software or firmware
* NOTE: The ICC LED always controlled by software.
@param firmwareControlMSRLED
 - 1:  firmware control the MSR LED
 - 0: software control the MSR LED
@param firmwareControlICCLED
 - 1:  firmware control the ICC LED
 - 0: software control the ICC LED
* @return RETURN_CODE:	Values can be parsed with device_getResponseCodeString

 */
int config_getLEDController( int *firmwareControlMSRLED,  int* firmwareControlICCLED);

/**
* Set the Beeper Controller
* Set the Beeper controlled by software or firmware
*
  @param firmwareControlBeeper	1 means firmware control the beeper, 0 means software control beeper.
* @return RETURN_CODE:	Values can be parsed with device_getResponseCodeString

 */

int config_setBeeperController(int firmwareControlBeeper);
/**
* Get the Beeper Controller Status
* Set the Beeper controlled Status by software or firmware
*
@param firmwareControlBeeper  1 means firmware control the beeper, 0 means software control beeper.
* @return RETURN_CODE:	Values can be parsed with device_getResponseCodeString

 */
int config_getBeeperController( int *firmwareControlBeeper);
/**
 * Set Encryption Control
 *
 Set Encryption Control to switch status between MSR and ICC/EMV function.
	 Following Encryption status supported:
	 - MSR ON, ICC/EMV ON,
	 - MSR ON, ICC/EMV OFF,
	 - MSR OFF, ICC/EMV OFF,

 @param msr
	 - 1:  enable MSR with Encryption,
	 - 0: disable MSR with Encryption,
 @param icc
	 - 1:  enable ICC with Encryption,
	 - 0: disable ICC with Encryption,

 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString

 */
int config_setEncryptionControl(int msr, int icc);
/**
* Get Encryption Control
*
Get Encryption Control to switch status between MSR and ICC/EMV function.
	Following Encryption status supported:
	- MSR ON, ICC/EMV ON,
	- MSR ON, ICC/EMV OFF,
	- MSR OFF, ICC/EMV OFF,

@param msr
	- 1:  enabled MSR with Encryption,
	- 0: disabled MSR with Encryption,
@param icc
	- 1:  enabled ICC with Encryption,
	- 0: disabled ICC with Encryption,

* @return RETURN_CODE:	Values can be parsed with device_getResponseCodeString

*/
int config_getEncryptionControl( int *msr, int* icc);

/**
 * ICC Function enable/disable
 * Enable ICC function with or without seated notification
 *
 * @param withNotification
  - 1:  with notification when ICC seated status changed,
  - 0: without notification.

 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 */
int icc_enable(IN int withNotification);
/**
 * ICC Function enable/disable
 * Disable ICC function
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 */
int icc_disable();
/**
 * Power On ICC
 *
 * Power up the currently selected microprocessor card in the ICC reader
 *@param ATR, the ATR data response when succeeded power on ICC,
 *@param inLen, the length of ATR data
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int icc_powerOnICC(OUT BYTE* ATR, IN_OUT int* inLen);
/**
 * Power Off ICC
 *
 * Powers down the ICC
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()

 If Success, empty
 If Failure, ASCII encoded data of error string
 */
int icc_powerOffICC();
/**
 * Exchange APDU with plain text
   For Non-SRED Augusta Only
 *
 * Sends an APDU packet to the ICC.  If successful, response is the APDU data in response parameter.

 @param c_APDU	APDU data packet
 @param cLen 	APDU data packet length
 @param reData    Unencrypted APDU response
 @param reLen      Unencrypted APDU response data length
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
  */
int icc_exchangeAPDU(IN BYTE* c_APDU, IN int cLen, OUT BYTE* reData, IN_OUT int *reLen);
/**
 * Exchange APDU with encrypted data
 For SRED Augusta Only
 *
 * Sends an APDU packet to the ICC.  If successful, response is the APDU data in response parameter.

 @param c_APDU	KSN + encytpted APDU data packet, or no KSN (use last known KSN) + encrypted APDU data packet
			With KSN:  [0A][KSN][Encrypted C-APDU]
			Without KSN:  [00][Encrypted C-APDU]

	The format of Raw C-APDU Data Structure of [m-bytes Encrypted C-APDU] is below:
	 - m = 2 bytes Valid C-APDU Length + x bytes Valid C-APDU + y bytes Padding (0x00)
	Note:
	For TDES mode: 2+x should be multiple of 8. If it was not multiple of 8, unit should padded y bytes 0x00 automatically (2+x+y should be multiple of 8).
	For AES mode: 2+x should be multiple of 16. If it was not multiple of 16, unit should padded y bytes 0x00 automatically (2+x+y should be multiple of 16).
 @param cLen  data packet length
 @param reData	response encrypted APDU response.  Can be three options:
   - [00] + [Plaintext R-APDU]
   - [01] + [0A] + [KSN] + [n bytes Encrypted R-APDU without Status Bytes]	+ [2 bytes Status Bytes]
   - [01] + [00] + [n bytes Encrypted R-APDU without Status Bytes]	+ [2 bytes Status Bytes]

	The KSN, when provided, will be 10 bytes.  The KSN will only be provided when
	it has changed since the last provided KSN.  Each card Power-On generates a new KSN.
	During a sequence of commands where the KSN is identical, the first response
	will have a KSN length set to [0x0A] followed by the KSN, while subsequent
	commands with the same KSN value will have a KSN length of [0x00] followed by the
	Encrypted R-APDU without Status Bytes.
 @param reLen	   encrypted APDU response data length

 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 */
int icc_exchangeEncryptedAPDU(IN BYTE* c_APDU, IN int cLen, OUT BYTE* reData, IN_OUT int *reLen);
/**
 * Get APDU KSN

 *
 * Retrieves the KSN used in ICC Encypted APDU usage

 * @param KSN Returns the encrypted APDU packet KSN
 * @param inLen KSN data length
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 */
int icc_getAPDU_KSN(OUT BYTE* KSN, IN_OUT int* inLen);
/**
 * Get ICC Function status
 * Get ICC Function status about enable/disable and with or without seated notification
 *
 * @param enabled
  - 1:	ICC Function enabled,
  - 0: means disabled.
 * @param withNotification 1 means with notification when ICC seated status changed. 0 means without notification.

 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 */
int icc_getFunctionStatus(OUT int *enabled, OUT int *withNotification);
/**
 * Get Reader Status
 *
 * Returns the reader status
 *
 * @param status Pointer that will return with the ICCReaderStatus results.
 *	bit 0:  0 = ICC Power Not Ready, 1 = ICC Powered
 *	bit 1:  0 = Card not seated, 1 = card seated
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 */
int icc_getICCReaderStatus(OUT BYTE *status);

/**
 * Get Key Format For DUKPT
 *
 * Specifies how data will be encrypted with Data Key or PIN key (if DUKPT key loaded). This applies to both MSR and ICC
 *
 * @param format Response returned from method:
 - 'TDES': Encrypted card data with TDES if  DUKPT Key had been loaded.(default)
 - 'AES': Encrypted card data with AES if DUKPT Key had been loaded.
 - 'NONE': No Encryption.
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 */
int icc_getKeyFormatForICCDUKPT(OUT BYTE *format);

/**
 * Get Key Type for DUKPT
 *
 * Specifies the key type used for DUKPT encryption  This applies to both MSR and ICC
 *
 * @param type Response returned from method:
 - 'DATA': Encrypted card data with Data Key DUKPT Key had been loaded.(default)
 - 'PIN': Encrypted card data with PIN Key if DUKPT Key had been loaded.

 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString

 */
int icc_getKeyTypeForICCDUKPT(OUT BYTE *type);

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
*
* @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
*
*/
int emv_getEMVKernelCheckValue(OUT BYTE* checkValue, IN_OUT int *checkValueLen);
/**
 * Get EMV Kernel configuration check value info
 *
 * @param checkValue Response returned of Kernel configuration check value info
 * @param checkValueLen the length of checkValue
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 *
 */
int emv_getEMVConfigurationCheckValue(OUT BYTE* checkValue, IN_OUT int *checkValueLen);
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
Allow fallback for EMV transactions.  Default is TRUE
@param allow TRUE = allow fallback, FALSE = don't allow fallback
*/
void emv_allowFallback(IN int allow);

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
 * Retrieve Transaction Results

 *
 Retrieves specified EMV tags from the currently executing transaction.

 * @param tags Tags to be retrieved.  Example 0x9F028A will retrieve tags 9F02 and 8A
 * @param tagsLen Length of tag list
 * @param cardData All requested tags returned as unencrypted, encrypted and masked TLV data in IDTTransactionData object

 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()

 */
int emv_retrieveTransactionResult(IN BYTE* tags, IN int tagsLen, IDTTransactionData * cardData);
/**
* Callback Response LCD Display
*
Provides menu selection responses to the kernel after a callback was received with DeviceState.EMVCallback, and
callbackType = EMV_CALLBACK_TYPE.EMV_CALLBACK_TYPE_LCD, and lcd_displayMode = EMV_LCD_DISPLAY_MODE_MENU, EMV_LCD_DISPLAY_MODE_PROMPT,
or EMV_LCD_DISPLAY_MODE_LANGUAGE_SELECT

* @param type If Cancel key pressed during menu selection, then value is EMV_LCD_DISPLAY_MODE_CANCEL.  Otherwise, value can be EMV_LCD_DISPLAY_MODE_MENU, EMV_LCD_DISPLAY_MODE_PROMPT,
or EMV_LCD_DISPLAY_MODE_LANGUAGE_SELECT
* @param selection If type = EMV_LCD_DISPLAY_MODE_MENU or EMV_LCD_DISPLAY_MODE_LANGUAGE_SELECT, provide the selection ID line number. Otherwise, if type = EMV_LCD_DISPLAY_MODE_PROMPT
supply either 0x43 ('C') for Cancel, or 0x45 ('E') for Enter/accept

* @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()

*/
int emv_callbackResponseLCD(IN int type, byte selection);
/**
* Callback Response MSR Entry
*
Provides MSR information to kernel after a callback was received with DeviceState.EMVCallback, and
    callbackType = EMV_CALLBACK_MSR

* @param MSR Swiped track data
* @param MSRLen the length of Swiped track data

* @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
*/
int emv_callbackResponseMSR(IN BYTE* MSR, IN_OUT int MSRLen);
/**
 * Retrieve Application Data by AID
 *
 Retrieves the Application Data as specified by the AID name passed as a parameter.

 * @param AID Name of ApplicationID. Must be between 5 and 16 bytes
 * @param AIDLen the length of AID data buffer.
 * @param tlv  The TLV elements of the requested AID
 * @param tlvLen the length of tlv data buffer.

 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
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
 * Remove Application Data by AID
 *
 Removes the Application Data as specified by the AID name passed as a parameter

 * @param AID Name of ApplicationID Must be between 5 and 16 bytes
 * @param AIDLen the length of AID data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_removeApplicationData(IN BYTE* AID, IN int AIDLen);
/**
 * Remove All Application Data
*
Removes all the Application Data


* @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()

*/
int emv_removeAllApplicationData();
/**
 * Retrieve AID list
 *
 Returns all the AID names installed on the terminal.

 * @param AIDList  array of AID name byte arrays
 * @param AIDListLen  the length of AIDList array buffer

 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()

 */
int emv_retrieveAIDList(OUT BYTE * AIDList, IN_OUT int *AIDListLen);

/**
 * Retrieve Terminal Data
 *
 Retrieves the Terminal Data.

 * @param tlv Response returned as a TLV
 * @param tlvLen the length of tlv data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()


 */
int emv_retrieveTerminalData(OUT BYTE* tlv, IN_OUT int *tlvLen);
/**
* Set Terminal Data
*
Sets the Terminal Data as specified by the TerminalData structure passed as a parameter

 * @param tlv TerminalData configuration file
 * @param tlvLen the length of tlv data buffer

 * @retval RETURN_CODE:  Return codes listed as typedef enum in IDTCommon:RETURN_CODE.  Values can be parsed with device_getResponseCodeString:()

 */
int emv_setTerminalData(IN BYTE* tlv, IN int tlvLen);
/**
 * Remove Terminal Data
 *
 Removes the Terminal Data

* @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()

 */
int emv_removeTerminalData();
/**
 * Retrieve Certificate Authority Public Key
 *
 Retrieves the CAPK as specified by the RID/Index  passed as a parameter.

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
 * * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
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
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()

 */
int emv_setCAPK(IN BYTE * capk, IN int capkLen);
/**
 * Remove Certificate Authority Public Key
 *
 Removes the CAPK as specified by the RID/Index

 * @param capk 6 byte CAPK =  5 bytes RID + 1 byte INDEX
 * @param capkLen the length of capk data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()

 */
int emv_removeCAPK(IN BYTE * capk, IN int capkLen);
/**
* Remove All Certificate Authority Public Key
 *
 Removes all the CAPK

* @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()

*/
int emv_removeAllCAPK();
/**
 * Retrieve the Certificate Authority Public Key list
 *
 Returns all the CAPK RID and Index installed on the terminal.

 * @param keys [key1][key2]...[keyn], each key 6 bytes where
    key = 5 bytes RID + 1 byte index
 * @param keysLen the length of keys data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_retrieveCAPKList(OUT BYTE * keys, IN_OUT int *keysLen);
/**
Gets the terminal ID as printable characters .
* @param terminalID Terminal ID string

* @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
*/
int emv_retrieveTerminalID(OUT char* terminalID);
/**
Sets the terminal ID as printable characters .
* @param terminalID Terminal ID to set

* @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
*/
int emv_setTerminalID(IN char* terminalID);
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

#ifdef __cplusplus
}
#endif


#endif

/*! \file libIDT_MiniSmartII.h
 \brief MiniSmartII API.

 MiniSmartII Global API methods.
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
