#ifndef __LIBIDT_L100_H___
#define __LIBIDT_L100_H___


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
 * Define the firmware update callback function to get the status of firmware update <br/>
 * It should be registered using the device_registerFWCallBk,
 */
typedef void (*pFW_callBack)(int, int, int, int, int);

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
 *To register the firmware update callback function to get the status of firmware update.
 */
void device_registerFWCallBk(pFW_callBack pFWf);

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
	IDT_DEVICE_MINISMART_II_COM,
	IDT_DEVICE_SPECTRUM_PRO_COM,
	IDT_DEVICE_KIOSK_III_COM,
	IDT_DEVICE_KIOSK_III_S_COM,
	IDT_DEVICE_MAX_DEVICES
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
	* Enter Stop Mode
	*
	Set device enter to stio mode. In stop mode, LCD display and backlight is off.
		Stop mode reduces power consumption to the lowest possible level.  A unit in stop mode can only be woken up by a physical key press.

	* @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString

	*/
int device_enterStopMode();

/**
	*  Set Sleep Mode Timer
	*
	Set device enter to sleep mode after the given time. In sleep mode, LCD display and backlight is off.
	   Sleep mode reduces power consumption to the lowest possible level.  A unit in Sleep mode can only be woken up by a physical key press.

	   @param time Enter sleep time value, in second.
	* @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString

	*/
int device_setSleepModeTime(int time);

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
	* Get Encrypted PIN

	* Requests PIN Entry
	* @param keyType
	- 0x00- MKSK-TDES:  External Plaintext PAN
	- 0x01- DUKPT-TDES:  External Plaintext PAN
	- 0x10 MKSK-TDES:  External Ciphertext PAN
	- 0x11 DUKPT-TDES:  External Ciphertext PAN
		@param PAN Account Number
		@param PANLen length of PAN
		@param message Message to display
		@param messageLen length of message
		@param timeout  PIN entry timeout

	* @return RETURN_CODE:  Values can be parsed with errorCode.getErrorString()
	*
	*/
int pin_getEncryptedPIN(int keyType, char *PAN, int PANLen, char *message, int messageLen, int timeout);

/**
        * Prompt for Key Input
        *
        Prompts for a numeric key using the secure message according to the following table

 Msg Id |English Prompt | Portuguese Prompt | Spanish Prompt | French Prompt
---------- | ---------- | ----------  | ---------- | ----------
1 | ENTER | ENTER | INGRESE | ENTREZ
2 | REENTER | RE-INTRODUZIR | REINGRESE | RE-ENTREZ
3 | ENTER YOUR | INTRODUZIR O SEU | INGRESE SU | ENTREZ VOTRE
4 | REENTER YOUR | RE-INTRODUZIR O SEU | REINGRESE SU | RE-ENTREZ VOTRE
5 | PLEASE ENTER | POR FAVOR DIGITE | POR FAVOR INGRESE | SVP ENTREZ
6 | PLEASE REENTER | POR FAVO REENTRAR | POR FAVO REINGRESE | SVP RE-ENTREZ
7 | PO NUMBER | NÚMERO PO | NUMERO PO | No COMMANDE
8 | DRIVER ID | LICENÇA | LICENCIA | ID CONDUCTEUR
9 | ODOMETER | ODOMETER | ODOMETRO | ODOMETRE
10 | ID NUMBER | NÚMERO ID | NUMERO ID | No IDENT
11 | EQUIP CODE | EQUIP CODE | CODIGO EQUIP | CODE EQUIPEMENT
12 | DRIVERS ID | DRIVER ID | ID CONDUCTOR | ID CONDUCTEUR
13 | JOB NUMBER | EMP NÚMERO | NUMERO EMP | No TRAVAIL
14 | WORK ORDER | TRABALHO ORDEM | ORDEN TRABAJO | FICHE TRAVAIL
15 | VEHICLE ID | ID VEÍCULO | ID VEHICULO | ID VEHICULE
16 | ENTER DRIVER | ENTER DRIVER | INGRESE CONDUCTOR | ENTR CONDUCTEUR
17 | ENTER DEPT | ENTER DEPT | INGRESE DEPT | ENTR DEPARTEMNT
18 | ENTER PHONE | ADICIONAR PHONE | INGRESE TELEFONO | ENTR No TELEPH
19 | ENTER ROUTE | ROUTE ADD | INGRESE RUTA | ENTREZ ROUTE
20 | ENTER FLEET | ENTER FROTA | INGRESE FLOTA | ENTREZ PARC AUTO
21 | ENTER JOB ID | ENTER JOB ID | INGRESE ID TRABAJO | ENTR ID TRAVAIL
22 | ROUTE NUMBER | NÚMERO PATH | RUTA NUMERO | No ROUTE
23 | ENTER USER ID | ENTER USER ID | INGRESE ID USUARIO | ID UTILISATEUR
24 | FLEET NUMBER | NÚMERO DE FROTA | FLOTA NUMERO | No PARC AUTO
25 | ENTER PRODUCT | ADICIONAR PRODUTO | INGRESE PRODUCTO | ENTREZ PRODUIT
26 | DRIVER NUMBER | NÚMERO DRIVER | CONDUCTOR NUMERO | No CONDUCTEUR
27 | ENTER LICENSE | ENTER LICENÇA | INGRESE LICENCIA | ENTREZ PERMIS
28 | ENTER FLEET NO | ENTER NRO FROTA | INGRESE NRO FLOTA | ENT No PARC AUTO
29 | ENTER CAR WASH | WASH ENTER | INGRESE LAVADO | ENTREZ LAVE-AUTO
30 | ENTER VEHICLE | ENTER VEÍCULO | INGRESE VEHICULO | ENTREZ VEHICULE
31 | ENTER TRAILER | TRAILER ENTER | INGRESE TRAILER | ENTREZ REMORQUE
32 | ENTER ODOMETER | ENTER ODOMETER | INGRESE ODOMETRO | ENTREZ ODOMETRE
33 | DRIVER LICENSE | CARTEIRA DE MOTORISTA | LICENCIA CONDUCTOR | PERMIS CONDUIRE
34 | ENTER CUSTOMER | ENTER CLIENTE | INGRESE CLIENTE | ENTREZ CLIENT
35 | VEHICLE NUMBER | NÚMERO DO VEÍCULO | VEHICULO NUMERO | No VEHICULE
36 | ENTER CUST DATA | ENTER CLIENTE INFO | INGRESE INFO CLIENTE | INFO CLIENT
37 | REENTER DRIVID | REENTRAR DRIVER ID | REINGRESE ID CHOFER | RE-ENTR ID COND
38 | ENTER USER DATA | ENTER INFO USUÁRIO | INGRESE INFO USUARIO | INFO UTILISATEUR
39 | ENTER CUST CODE | ENTER CODE. CLIENTE | INGRESE COD. CLIENTE | ENTR CODE CLIENT
40 | ENTER EMPLOYEE | ENTER FUNCIONÁRIO | INGRESE EMPLEADO | ENTREZ EMPLOYE
41 | ENTER ID NUMBER | ENTER NÚMERO ID | INGRESE NUMERO ID | ENTREZ No ID
42 | ENTER DRIVER ID | ENTER ID DRIVER | INGRESE ID CONDUCTOR | No CONDUCTEUR
43 | ENTER FLEET PIN | ENTER PIN FROTA | INGRESE PIN DE FLOTA | NIP PARC AUTO
44 | ODOMETER NUMBER | NÚMERO ODOMETER | ODOMETRO NUMERO | No ODOMETRE
45 | ENTER DRIVER LIC | ENTER DRIVER LIC | INGRESE LIC CONDUCTOR | PERMIS CONDUIRE
46 | ENTER TRAILER NO | NRO TRAILER ENTER | INGRESE NRO TRAILER | ENT No REMORQUE
47 | REENTER VEHICLE | REENTRAR VEÍCULO | REINGRESE VEHICULO | RE-ENTR VEHICULE
48 | ENTER VEHICLE ID | ENTER VEÍCULO ID | INGRESE ID VEHICULO | ENTR ID VEHICULE
49 | ENTER BIRTH DATE | INSERIR DATA NAC | INGRESE FECHA NAC | ENT DT NAISSANCE
50 | ENTER DOB MMDDYY | ENTER FDN MMDDYY | INGRESE FDN MMDDAA | NAISSANCE MMJJAA
51 | ENTER FLEET DATA | ENTER FROTA INFO | INGRESE INFO DE FLOTA | INFO PARC AUTO
52 | ENTER REFERENCE | ENTER REFERÊNCIA | INGRESE REFERENCIA | ENTREZ REFERENCE
53 | ENTER AUTH NUMBR | ENTER NÚMERO AUT | INGRESE NUMERO AUT | No AUTORISATION
54 | ENTER HUB NUMBER | ENTER HUB NRO | INGRESE NRO HUB | ENTREZ No NOYAU
55 | ENTER HUBOMETER | MEDIDA PARA ENTRAR HUB | INGRESE MEDIDO DE HUB | COMPTEUR NOYAU
56 | ENTER TRAILER ID | TRAILER ENTER ID | INGRESE ID TRAILER | ENT ID REMORQUE
57 | ODOMETER READING | QUILOMETRAGEM | LECTURA ODOMETRO | LECTURE ODOMETRE
58 | REENTER ODOMETER | REENTRAR ODOMETER | REINGRESE ODOMETRO | RE-ENT ODOMETRE
59 | REENTER DRIV. ID | REENTRAR DRIVER ID | REINGRESE ID CHOFER | RE-ENT ID CONDUC
60 | ENTER CUSTOMER ID | ENTER CLIENTE ID | INGRESE ID CLIENTE | ENTREZ ID CLIENT
61 | ENTER CUST. ID | ENTER CLIENTE ID | INGRESE ID CLIENTE | ENTREZ ID CLIENT
62 | ENTER ROUTE NUM | ENTER NUM ROUTE | INGRESE NUM RUTA | ENT No ROUTE
63 | ENTER FLEET NUM | FROTA ENTER NUM | INGRESE NUM FLOTA | ENT No PARC AUTO
64 | FLEET PIN | FROTA PIN | PIN DE FLOTA | NIP PARC AUTO
65 | DRIVER # | DRIVER # | CONDUCTOR # | CONDUCTEUR
66 | ENTER DRIVER # | ENTER DRIVER # | INGRESE CONDUCTOR # | ENT # CONDUCTEUR
67 | VEHICLE # | VEÍCULO # | VEHICULO # | # VEHICULE
68 | ENTER VEHICLE # | ENTER VEÍCULO # | INGRESE VEHICULO # | ENT # VEHICULE
69 | JOB # | TRABALHO # | TRABAJO # | # TRAVAIL
70 | ENTER JOB # | ENTER JOB # | INGRESE TRABAJO # | ENTREZ # TRAVAIL
71 | DEPT NUMBER | NÚMERO DEPT | NUMERO DEPTO | No DEPARTEMENT
72 | DEPARTMENT # | DEPARTAMENTO # | DEPARTAMENTO # | DEPARTEMENT
73 | ENTER DEPT # | ENTER DEPT # | INGRESE DEPTO # | ENT# DEPARTEMENT
74 | LICENSE NUMBER | NÚMERO DE LICENÇA | NUMERO LICENCIA | No PERMIS
75 | LICENSE # | LICENÇA # | LICENCIA # | # PERMIS
76 | ENTER LICENSE # | ENTER LICENÇA # | INGRESE LICENCIA # | ENTREZ # PERMIS
77 | DATA | INFO | INFO | INFO
78 | ENTER DATA | ENTER INFO | INGRESE INFO | ENTREZ INFO
79 | CUSTOMER DATA | CLIENTE INFO | INFO CLIENTE | INFO CLIENT
80 | ID # | ID # | ID # | # ID
81 | ENTER ID # | ENTER ID # | INGRESE ID # | ENTREZ # ID
82 | USER ID | USER ID | ID USUARIO | ID UTILISATEUR
83 | ROUTE # | ROUTE # | RUTA # | # ROUTE
84 | ENTER ROUTE # | ADD ROUTE # | INGRESE RUTA # | ENTREZ # ROUTE
85 | ENTER CARD NUM | ENTER NÚMERO DE CARTÃO | INGRESE NUM TARJETA | ENTREZ NO CARTE
86 | EXP DATE(YYMM) | VALIDADE VAL (AAMM) | FECHA EXP (AAMM) | DATE EXPIR(AAMM)
87 | PHONE NUMBER | TELEFONE | NUMERO TELEFONO | NO TEL
88 | CVV START DATE | CVV DATA DE INÍCIO | CVV FECHA INICIO | CVV DATE DE DEBUT
89 | ISSUE NUMBER | NÚMERO DE EMISSÃO | NUMERO DE EMISION | NO DEMISSION
90 | START DATE (MMYY) | DATA DE INÍCIO (AAMM) | FECHA INICIO (AAMM) | DATE DE DEBUT-AAMM

            @param messageID  Message (1-90)
            @param languageID 0=English Prompt, 1=Portuguese Prompt, 2=Spanish Prompt, 3=French Prompt
            @param maskInput  1 = entry is masked with '*', 0 = entry is displayed on keypad
            @param minLen  Minimum input length.  Cannot be less than 1
            @param maxLen Maximum input length.  Cannot be greater than 16
            @param timeout Timout value, in seconds

        *
       * @return RETURN_CODE:  Values can be parsed with errorCode.getErrorString()
        */

int pin_promptForKeyInput(int messageID, int languageID, int maskInput, int minLen, int maxLen, int timeout);

/**
        * Prompt for Amount Input
        *
        Prompts for amount input using the secure message according to the following table

 Msg Id |English Prompt | Portuguese Prompt | Spanish Prompt | French Prompt
---------- | ---------- | ----------  | ---------- | ----------
1 | ENTER | ENTER | INGRESE | ENTREZ
2 | REENTER | RE-INTRODUZIR | REINGRESE | RE-ENTREZ
3 | ENTER YOUR | INTRODUZIR O SEU | INGRESE SU | ENTREZ VOTRE
4 | REENTER YOUR | RE-INTRODUZIR O SEU | REINGRESE SU | RE-ENTREZ VOTRE
5 | PLEASE ENTER | POR FAVOR DIGITE | POR FAVOR INGRESE | SVP ENTREZ
6 | PLEASE REENTER | POR FAVO REENTRAR | POR FAVO REINGRESE | SVP RE-ENTREZ
7 | PO NUMBER | NÚMERO PO | NUMERO PO | No COMMANDE
8 | DRIVER ID | LICENÇA | LICENCIA | ID CONDUCTEUR
9 | ODOMETER | ODOMETER | ODOMETRO | ODOMETRE
10 | ID NUMBER | NÚMERO ID | NUMERO ID | No IDENT
11 | EQUIP CODE | EQUIP CODE | CODIGO EQUIP | CODE EQUIPEMENT
12 | DRIVERS ID | DRIVER ID | ID CONDUCTOR | ID CONDUCTEUR
13 | JOB NUMBER | EMP NÚMERO | NUMERO EMP | No TRAVAIL
14 | WORK ORDER | TRABALHO ORDEM | ORDEN TRABAJO | FICHE TRAVAIL
15 | VEHICLE ID | ID VEÍCULO | ID VEHICULO | ID VEHICULE
16 | ENTER DRIVER | ENTER DRIVER | INGRESE CONDUCTOR | ENTR CONDUCTEUR
17 | ENTER DEPT | ENTER DEPT | INGRESE DEPT | ENTR DEPARTEMNT
18 | ENTER PHONE | ADICIONAR PHONE | INGRESE TELEFONO | ENTR No TELEPH
19 | ENTER ROUTE | ROUTE ADD | INGRESE RUTA | ENTREZ ROUTE
20 | ENTER FLEET | ENTER FROTA | INGRESE FLOTA | ENTREZ PARC AUTO
21 | ENTER JOB ID | ENTER JOB ID | INGRESE ID TRABAJO | ENTR ID TRAVAIL
22 | ROUTE NUMBER | NÚMERO PATH | RUTA NUMERO | No ROUTE
23 | ENTER USER ID | ENTER USER ID | INGRESE ID USUARIO | ID UTILISATEUR
24 | FLEET NUMBER | NÚMERO DE FROTA | FLOTA NUMERO | No PARC AUTO
25 | ENTER PRODUCT | ADICIONAR PRODUTO | INGRESE PRODUCTO | ENTREZ PRODUIT
26 | DRIVER NUMBER | NÚMERO DRIVER | CONDUCTOR NUMERO | No CONDUCTEUR
27 | ENTER LICENSE | ENTER LICENÇA | INGRESE LICENCIA | ENTREZ PERMIS
28 | ENTER FLEET NO | ENTER NRO FROTA | INGRESE NRO FLOTA | ENT No PARC AUTO
29 | ENTER CAR WASH | WASH ENTER | INGRESE LAVADO | ENTREZ LAVE-AUTO
30 | ENTER VEHICLE | ENTER VEÍCULO | INGRESE VEHICULO | ENTREZ VEHICULE
31 | ENTER TRAILER | TRAILER ENTER | INGRESE TRAILER | ENTREZ REMORQUE
32 | ENTER ODOMETER | ENTER ODOMETER | INGRESE ODOMETRO | ENTREZ ODOMETRE
33 | DRIVER LICENSE | CARTEIRA DE MOTORISTA | LICENCIA CONDUCTOR | PERMIS CONDUIRE
34 | ENTER CUSTOMER | ENTER CLIENTE | INGRESE CLIENTE | ENTREZ CLIENT
35 | VEHICLE NUMBER | NÚMERO DO VEÍCULO | VEHICULO NUMERO | No VEHICULE
36 | ENTER CUST DATA | ENTER CLIENTE INFO | INGRESE INFO CLIENTE | INFO CLIENT
37 | REENTER DRIVID | REENTRAR DRIVER ID | REINGRESE ID CHOFER | RE-ENTR ID COND
38 | ENTER USER DATA | ENTER INFO USUÁRIO | INGRESE INFO USUARIO | INFO UTILISATEUR
39 | ENTER CUST CODE | ENTER CODE. CLIENTE | INGRESE COD. CLIENTE | ENTR CODE CLIENT
40 | ENTER EMPLOYEE | ENTER FUNCIONÁRIO | INGRESE EMPLEADO | ENTREZ EMPLOYE
41 | ENTER ID NUMBER | ENTER NÚMERO ID | INGRESE NUMERO ID | ENTREZ No ID
42 | ENTER DRIVER ID | ENTER ID DRIVER | INGRESE ID CONDUCTOR | No CONDUCTEUR
43 | ENTER FLEET PIN | ENTER PIN FROTA | INGRESE PIN DE FLOTA | NIP PARC AUTO
44 | ODOMETER NUMBER | NÚMERO ODOMETER | ODOMETRO NUMERO | No ODOMETRE
45 | ENTER DRIVER LIC | ENTER DRIVER LIC | INGRESE LIC CONDUCTOR | PERMIS CONDUIRE
46 | ENTER TRAILER NO | NRO TRAILER ENTER | INGRESE NRO TRAILER | ENT No REMORQUE
47 | REENTER VEHICLE | REENTRAR VEÍCULO | REINGRESE VEHICULO | RE-ENTR VEHICULE
48 | ENTER VEHICLE ID | ENTER VEÍCULO ID | INGRESE ID VEHICULO | ENTR ID VEHICULE
49 | ENTER BIRTH DATE | INSERIR DATA NAC | INGRESE FECHA NAC | ENT DT NAISSANCE
50 | ENTER DOB MMDDYY | ENTER FDN MMDDYY | INGRESE FDN MMDDAA | NAISSANCE MMJJAA
51 | ENTER FLEET DATA | ENTER FROTA INFO | INGRESE INFO DE FLOTA | INFO PARC AUTO
52 | ENTER REFERENCE | ENTER REFERÊNCIA | INGRESE REFERENCIA | ENTREZ REFERENCE
53 | ENTER AUTH NUMBR | ENTER NÚMERO AUT | INGRESE NUMERO AUT | No AUTORISATION
54 | ENTER HUB NUMBER | ENTER HUB NRO | INGRESE NRO HUB | ENTREZ No NOYAU
55 | ENTER HUBOMETER | MEDIDA PARA ENTRAR HUB | INGRESE MEDIDO DE HUB | COMPTEUR NOYAU
56 | ENTER TRAILER ID | TRAILER ENTER ID | INGRESE ID TRAILER | ENT ID REMORQUE
57 | ODOMETER READING | QUILOMETRAGEM | LECTURA ODOMETRO | LECTURE ODOMETRE
58 | REENTER ODOMETER | REENTRAR ODOMETER | REINGRESE ODOMETRO | RE-ENT ODOMETRE
59 | REENTER DRIV. ID | REENTRAR DRIVER ID | REINGRESE ID CHOFER | RE-ENT ID CONDUC
60 | ENTER CUSTOMER ID | ENTER CLIENTE ID | INGRESE ID CLIENTE | ENTREZ ID CLIENT
61 | ENTER CUST. ID | ENTER CLIENTE ID | INGRESE ID CLIENTE | ENTREZ ID CLIENT
62 | ENTER ROUTE NUM | ENTER NUM ROUTE | INGRESE NUM RUTA | ENT No ROUTE
63 | ENTER FLEET NUM | FROTA ENTER NUM | INGRESE NUM FLOTA | ENT No PARC AUTO
64 | FLEET PIN | FROTA PIN | PIN DE FLOTA | NIP PARC AUTO
65 | DRIVER # | DRIVER # | CONDUCTOR # | CONDUCTEUR
66 | ENTER DRIVER # | ENTER DRIVER # | INGRESE CONDUCTOR # | ENT # CONDUCTEUR
67 | VEHICLE # | VEÍCULO # | VEHICULO # | # VEHICULE
68 | ENTER VEHICLE # | ENTER VEÍCULO # | INGRESE VEHICULO # | ENT # VEHICULE
69 | JOB # | TRABALHO # | TRABAJO # | # TRAVAIL
70 | ENTER JOB # | ENTER JOB # | INGRESE TRABAJO # | ENTREZ # TRAVAIL
71 | DEPT NUMBER | NÚMERO DEPT | NUMERO DEPTO | No DEPARTEMENT
72 | DEPARTMENT # | DEPARTAMENTO # | DEPARTAMENTO # | DEPARTEMENT
73 | ENTER DEPT # | ENTER DEPT # | INGRESE DEPTO # | ENT# DEPARTEMENT
74 | LICENSE NUMBER | NÚMERO DE LICENÇA | NUMERO LICENCIA | No PERMIS
75 | LICENSE # | LICENÇA # | LICENCIA # | # PERMIS
76 | ENTER LICENSE # | ENTER LICENÇA # | INGRESE LICENCIA # | ENTREZ # PERMIS
77 | DATA | INFO | INFO | INFO
78 | ENTER DATA | ENTER INFO | INGRESE INFO | ENTREZ INFO
79 | CUSTOMER DATA | CLIENTE INFO | INFO CLIENTE | INFO CLIENT
80 | ID # | ID # | ID # | # ID
81 | ENTER ID # | ENTER ID # | INGRESE ID # | ENTREZ # ID
82 | USER ID | USER ID | ID USUARIO | ID UTILISATEUR
83 | ROUTE # | ROUTE # | RUTA # | # ROUTE
84 | ENTER ROUTE # | ADD ROUTE # | INGRESE RUTA # | ENTREZ # ROUTE
85 | ENTER CARD NUM | ENTER NÚMERO DE CARTÃO | INGRESE NUM TARJETA | ENTREZ NO CARTE
86 | EXP DATE(YYMM) | VALIDADE VAL (AAMM) | FECHA EXP (AAMM) | DATE EXPIR(AAMM)
87 | PHONE NUMBER | TELEFONE | NUMERO TELEFONO | NO TEL
88 | CVV START DATE | CVV DATA DE INÍCIO | CVV FECHA INICIO | CVV DATE DE DEBUT
89 | ISSUE NUMBER | NÚMERO DE EMISSÃO | NUMERO DE EMISION | NO DEMISSION
90 | START DATE (MMYY) | DATA DE INÍCIO (AAMM) | FECHA INICIO (AAMM) | DATE DE DEBUT-AAMM

            @param messageID  Message (1-90)
            @param languageID 0=English Prompt, 1=Portuguese Prompt, 2=Spanish Prompt, 3=French Prompt
            @param minLen  Minimum input length.  Cannot be less than 1
            @param maxLen Maximum input length.  Cannot be greater than 15
            @param timeout Timout value, in seconds

        *
       * @return RETURN_CODE:  Values can be parsed with errorCode.getErrorString()
        */
int pin_promptForAmountInput(int messageID, int languageID, int minLen, int maxLen, int timeout);

/**
   * Get Function Key

   * Captures a function key

		- Backspace = B
		- Cancel = C
		- Enter = E
		- * = *
		- # = #
		- Help = ?
		- Function Key 1 = F1
		- Function Key 2 = F2
		- Function Key 3 = F3

		@param timeout Timeout, in seconds
   * @return RETURN_CODE:  Values can be parsed with errorCode.getErrorString()
   *
   */
int pin_getFunctionKey(int timeout);

/**
   * Send Beep
   *
   Executes a beep request.
	@param frequency  Frequency, range 200-20000Hz
   @param duration Duration, range 16-65535ms

   *
	* @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
   */

int pin_sendBeep(int frequency, int duration);

/**
  * Save Prompt
  *
  Saves a message prompt to L100 memory.
	@param promptNumber  Prompt number  (0-9)
	@param prompt Prompt string (up to 20 characters)
	@param promptLen length of prompt
  * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString

  */
int lcd_savePrompt(int promptNumber, char *prompt, int promptLen);

/**
  * Display Prompt on Line
  *
  Displays a message prompt from L100 memory.
	@param promptNumber  Prompt number  (0-9)
	@param lineNumber Line number to display message prompt (1-4)
  * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString

  */
int lcd_displayPrompt(int promptNumber, int lineNumber);

/**
	* Display Message on Line
	*
	Displays a message on a display line.
	  @param lineNumber  Line number to display message on  (1-4)
	  @param message Message to display
	  @param messageLen length of message
	* @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString

	*/
int lcd_displayMessage(int lineNumber, char* message, int messageLen);

/**
	* Enable/Disable LCD Backlight
	*
	Turns on/off the LCD back lighting.
	  @param enable  TRUE = turn ON backlight, FALSE = turn OFF backlight
	* @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString

	*/
int lcd_enableBacklight(int enable);

/**
	* Get Backlight Status
	*
	Returns the status of the LCD back lighting.
	  @param enabled  1 = Backlight is ON, 0 = Backlight is OFF
	* @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString

	*/
int lcd_getBacklightStatus(int *enabled);

#ifdef __cplusplus
}
#endif


#endif

/*! \file libIDT_L100.h
 \brief L100 API.

 L100 Global API methods.
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
