#ifndef __LIBIDT_DEVICE_H___
#define __LIBIDT_DEVICE_H___

#define IN
#define OUT
#define IN_OUT
#include "IDTDef.h"

/**
 * Define the USB hot-plug callback function to monitor the info when
 * plug in/out the reader. <br/>
 * It should be registered using the registerHotplugCallBk,
 * The first integer parameter is device type, and the second integer
 * parameter is either 0: Device Plugged Out or 1: Device Plugged In
 */
typedef void (*pMessageHotplug)(int,
                                int);

/**
 * Define the send command callback function to monitor the sending command
 * into the reader. <br/>
 * It should be registered using the registerLogCallBk,
 */
typedef void (*pSendDataLog)(unsigned char *,
                             int);

/**
 * Define the read response callback function to monitor the reading
 * response from the reader. <br/>
 * It should be registered using the registerLogCallBk,
 */
typedef void (*pReadDataLog)(unsigned char *,
                             int);

/**
 * Define the EMV callback function to get the transaction
 * message/data/result. <br/>
 * It should be registered using the emv_registerCallBk,
 */
typedef void (*pEMV_callBack)(int,
                              int,
                              unsigned char *,
                              int,
                              IDTTransactionData *,
                              EMV_Callback *,
                              int);

/**
 * Define the MSR callback function to get the MSR card data <br/>
 * It should be registered using the msr_registerCallBk,
 */
typedef void (*pMSR_callBack)(int,
                              IDTMSRData);

/**
 * Define the MSR callback function to get the pointer to the MSR card data <br/>
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
 * Define the FW callback function to get the status of the firmware update <br/>
 * It should be registered using the device_registerFWCallBk,
 */
typedef void (*pFW_callBack)(int,
                             int,
                             int,
                             int,
                             int);

/**
 * Define the comm callback function to get the async url data <br/>
 * It should be registered using the comm_registerHTTPCallback,
 */
typedef void (*httpComm_callBack)(BYTE *,
                                  int);

/**
 * Define the comm callback function to receive the V4 Protocol packets
 * received by the device from an external source (IP/USB/RS-232)
 * It should be registered using the comm_registerV4Callback,
 * Data callback will contain command, sub-command, and data from V4 packet
 */
typedef void (*v4Comm_callBack)( BYTE,
                                 BYTE,
                                 BYTE *,
                                 int);

/**
 * Define the comm callback function to get FTP file transfer status <br/>
 * It should be passed as a parameter in a FTP request,
 * Signature (int, int, int) = response code, current block, total blocks
 * RESPONSE CODES:
 *        100 = FILE DOWNLOAD STARTED
 *        101 = FILE BLOCK XX OF XX RECEIVED
 *        102 = FILE DOWNLOAD COMPLETED
 *        103 = FILE DOWNLOAD TERMINATED PREMATURELY
 */
typedef void (*ftpComm_callBack)(int,
                                 int,
                                 int);

/**
 * Define the log callback function to receive log messages.
 */
typedef void (*pLog_callback)(BYTE, char *);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * To register the USB HID hot-plug callback function which
 * implemented in the application to monitor the hotplug message from
 * the SDK.
 */
void registerHotplugCallBk(pMessageHotplug pMsgHotplug);

/**
 * To register the log callback function which implemented in the
 * application to monitor sending/reading data between application and
 * reader.
 */
void registerLogCallBk(pSendDataLog pFSend,
                       pReadDataLog pFRead);

/**
 * To register the emv callback function to get the EMV processing response.
 */
void emv_registerCallBk(pEMV_callBack pEMVf);

/**
 * To register the msr callback function to get the MSR card data.
 */
void msr_registerCallBk(pMSR_callBack pMSRf);

/**
 * To register the msr callback function to get the MSR card data pointer.
 */
void msr_registerCallBkp(pMSR_callBackp pMSRf);

/**
 * To register the pin callback function to get the PINPad data.
 */
void pin_registerCallBk(pPIN_callBack pPINf);

/**
 * To register the firmware update callback function to get the
 * firmware update status.
 */
void device_registerFWCallBk(pFW_callBack pFWf);

/**
 * To Get SDK version
 * @return return the SDK version string
 */
char * SDK_Version();

/**
 * Set the path to use when searching for ID TECH's libraries.  If
 * this is not set, the libraries will be searched for with the
 * system's default procedures.
 *
 * @param absoluteLibraryPath The absolute path to ID TECH's libraries.
 */
void setAbsoluteLibraryPath(const char * absoluteLibraryPath);

/**
 * Set the path to the config xml file(s) if any <br/>
 *
 * @param path The path to the config xml files (such as "NEO2_Devices.xml"
 * which contains the information of NEO2 devices).
 *    Only need to specify the path to the folder which contains the config files.
 *    File names are not needed.
 *    The maximum length of path is 200 characters including the '\0' at the end.
 */
void device_setConfigPath(const char * path);

/**
 * Pass the content of the config xml file ("NEO2_Devices.xml") as a string
 * to the SDK instead of reading the config xml file by the SDK
 *     It needs to be called before device_init(), otherwise the SDK will
 *     try to read the config xml file.
 *
 * @param configs The content read from the config xml file ("NEO2_Devices.xml"
 * which contains the information of NEO2 devices).
 * @param len The length of the string configs.  The maximum length is 5000
 * bytes.
 *
 */
int device_setNEO2DevicesConfigs(IN const char * configs,
                                 IN int len);

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
 * enum IDT_DEVICE_TYPE {
 *     IDT_DEVICE_UNKNOWN=0,
 *     IDT_DEVICE_AUGUSTA_HID,
 *     IDT_DEVICE_AUGUSTA_KB,
 *     IDT_DEVICE_AUGUSTA_S_HID,
 *     IDT_DEVICE_AUGUSTA_S_KB,
 *     IDT_DEVICE_AUGUSTA_S_TTK_HID,
 *     IDT_DEVICE_SPECTRUM_PRO,
 *     IDT_DEVICE_MINISMART_II,
 *     IDT_DEVICE_UNIPAY,
 *     IDT_DEVICE_UNIPAY_I_V,
 *     IDT_DEVICE_VP3300_AJ,
 *     IDT_DEVICE_L100,
 *     IDT_DEVICE_KIOSK_III,
 *     IDT_DEVICE_KIOSK_III_S,
 *     IDT_DEVICE_VENDI,
 *     IDT_DEVICE_VP3300_USB,
 *     IDT_DEVICE_UNIPAY_I_V_TTK,
 *     IDT_DEVICE_VP3300_BT,
 *     IDT_DEVICE_VP8800,
 *     IDT_DEVICE_NEO2,
 *     IDT_DEVICE_MINISMART_II_COM = IDT_DEVICE_NEO2+5,
 *     IDT_DEVICE_SPECTRUM_PRO_COM,
 *     IDT_DEVICE_KIOSK_III_COM,
 *     IDT_DEVICE_KIOSK_III_S_COM,
 *     IDT_DEVICE_NEO2_COM,
 *     IDT_DEVICE_MAX_DEVICES = IDT_DEVICE_NEO2_COM+5
 * };
 * @endcode
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
 *
 * - 0: "no error, beginning task";
 * - 1: "no response from reader";
 * - 2: "invalid response data";
 * - 3: "time out for task or CMD";
 * - 4: "wrong parameter";
 * - 5: "SDK is doing MSR or ICC task";
 * - 6: "SDK is doing PINPad task";
 * - 7: "SDK is doing CTLS task";
 * - 8: "SDK is doing EMV task";
 * - 9: "SDK is doing Other task";
 * - 10: "err response or data";
 * - 11: "no reader attached";
 * - 12: "mono audio is enabled";
 * - 13: "did connection";
 * - 14: "audio volume is too low";
 * - 15: "task or CMD be canceled";
 * - 16: "UF wrong string format";
 * - 17: "UF file not found";
 * - 18: "UF wrong file format";
 * - 19: "Attempt to contact online host failed";
 * - 20: "Attempt to perform RKI failed";
 * - 22: "Buffer size is not enough";
 * - 0X300: "Key Type(TDES) of Session Key is not same as the related
 *           Master Key.";
 * - 0X400: "Related Key was not loaded.";
 * - 0X500: "Key Same.";
 * - 0X501: "Key is all zero";
 * - 0X502: "TR-31 format error";
 * - 0X702: "PAN is Error Key.";
 * - 0X705: "No Internal MSR PAN (or Internal MSR PAN is erased timeout)";
 * - 0X0C01: "Incorrect Frame Tag";
 * - 0X0C02: "Incorrect Frame Type";
 * - 0X0C03: "Unknown Frame Type";
 * - 0X0C04: "Unknown Command";
 * - 0X0C05: "Unknown Sub-Command";
 * - 0X0C06: "CRC Error";
 * - 0X0C07: "Failed";
 * - 0X0C08: "Timeout";
 * - 0X0C0A: "Incorrect Parameter";
 * - 0X0C0B: "Command Not Supported";
 * - 0X0C0C: "Sub-Command Not Supported";
 * - 0X0C0D: "Parameter Not Supported / Status Abort Command";
 * - 0X0C0F: "Sub-Command Not Allowed";
 * - 0X0D01: "Incorrect Header Tag";
 * - 0X0D02: "Unknown Command";
 * - 0X0D03: "Unknown Sub-Command";
 * - 0X0D04: "CRC Error in Frame";
 * - 0X0D05: "Incorrect Parameter";
 * - 0X0D06: "Parameter Not Supported";
 * - 0X0D07: "Mal-formatted Data";
 * - 0X0D08: "Timeout";
 * - 0X0D0A: "Failed / NACK";
 * - 0X0D0B: "Command not Allowed";
 * - 0X0D0C: "Sub-Command not Allowed";
 * - 0X0D0D: "Buffer Overflow (Data Length too large for reader buffer)";
 * - 0X0D0E: "User Interface Event";
 * - 0X0D11: "Communication type not supported, VT-1, burst, etc.";
 * - 0X0D12: "Secure interface is not functional or is in an intermediate state.";
 * - 0X0D13: "Data field is not mod 8";
 * - 0X0D14: "Pad  - 0X80 not found where expected";
 * - 0X0D15: "Specified key type is invalid";
 * - 0X0D1: "Could not retrieve key from the SAM(InitSecureComm)";
 * - 0X0D17: "Hash code problem";
 * - 0X0D18: "Could not store the key into the SAM(InstallKey)";
 * - 0X0D19: "Frame is too large";
 * - 0X0D1A: "Unit powered up in authentication state but POS must resend the InitSecureComm command";
 * - 0X0D1B: "The EEPROM may not be initialized because SecCommInterface does not make sense";
 * - 0X0D1C: "Problem encoding APDU";
 * - 0X0D20: "Unsupported Index(ILM) SAM Transceiver error - problem communicating with the SAM(Key Mgr)";
 * - 0X0D2: "Unexpected Sequence Counter in multiple frames for single bitmap(ILM) Length error in data returned from the SAM(Key Mgr)";
 * - 0X0D22: "Improper bit map(ILM)";
 * - 0X0D23: "Request Online Authorization";
 * - 0X0D24: "ViVOCard3 raw data read successful";
 * - 0X0D25: "Message index not available(ILM) ViVOcomm activate transaction card type(ViVOcomm)";
 * - 0X0D26: "Version Information Mismatch(ILM)";
 * - 0X0D27: "Not sending commands in correct index message index(ILM)";
 * - 0X0D28: "Time out or next expected message not received(ILM)";
 * - 0X0D29: "ILM languages not available for viewing(ILM)";
 * - 0X0D2A: "Other language not supported(ILM)";
 * - 0X0D41: "Unknown Error from SAM";
 * - 0X0D42: "Invalid data detected by SAM";
 * - 0X0D43: "Incomplete data detected by SAM";
 * - 0X0D44: "Reserved";
 * - 0X0D45: "Invalid key hash algorithm";
 * - 0X0D46: "Invalid key encryption algorithm";
 * - 0X0D47: "Invalid modulus length";
 * - 0X0D48: "Invalid exponent";
 * - 0X0D49: "Key already exists";
 * - 0X0D4A: "No space for new RID";
 * - 0X0D4B: "Key not found";
 * - 0X0D4C: "Crypto not responding";
 * - 0X0D4D: "Crypto communication error";
 * - 0X0D4E: "Module-specific error for Key Manager";
 * - 0X0D4F: "All key slots are full (maximum number of keys has been installed)";
 * - 0X0D50: "Auto-Switch OK";
 * - 0X0D51: "Auto-Switch failed";
 * - 0X0D90: "Account DUKPT Key not exist";
 * - 0X0D91: "Account DUKPT Key KSN exausted";
 * - 0X0D00: "This Key had been loaded.";
 * - 0X0E00: "Base Time was loaded.";
 * - 0X0F00: "Encryption Or Decryption Failed.";
 * - 0X1000: "Battery Low Warning (It is High Priority Response while Battery is Low.)";
 * - 0X1800: "Send 'Cancel Command' after send 'Get Encrypted PIN' & 'Get Numeric' & 'Get Amount';
 * - 0X1900: "Press 'Cancel' key after send 'Get Encrypted PIN' & 'Get Numeric' & 'Get Amount';
 * - 0X30FF: "Security Chip is not connect";
 * - 0X3000: "Security Chip is deactivation & Device is In Removal Legally State.";
 * - 0X3101: "Security Chip is activation &  Device is In Removal Legally State.";
 * - 0X5500: "No Admin DUKPT Key.";
 * - 0X5501: "Admin  DUKPT Key STOP.";
 * - 0X5502: "Admin DUKPT Key KSN is Error.";
 * - 0X5503: "Get Authentication Code1 Failed.";
 * - 0X5504: "Validate Authentication Code Error.";
 * - 0X5505: "Encrypt or Decrypt data failed.";
 * - 0X5506: "Not Support the New Key Type.";
 * - 0X5507: "New Key Index is Error.";
 * - 0X5508: "Step Error.";
 * - 0X5509: "KSN Error";
 * - 0X550A: "MAC Error.";
 * - 0X550B: "Key Usage Error.";
 * - 0X550C: "Mode Of Use Error.";
 * - 0X550F: "Other Error.";
 * - 0X6000: "Save or Config Failed / Or Read Config Error.";
 * - 0X6200: "No Serial Number.";
 * - 0X6900: "Invalid Command - Protocol is right, but task ID is invalid.";
 * - 0X6A01: "Unsupported Command - Protocol and task ID are right, but command is invalid - In this State";
 * - 0X6A00: "Unsupported Command - Protocol and task ID are right, but command is invalid.";
 * - 0X6B00: "Unknown parameter in command - Protocol task ID and command are right, but parameter is invalid.";
 * - 0X6C00: "Unknown parameter in command - Protocol task ID and command are right, but length is out of the requirement.";
 * - 0X7200: "Device is suspend (MKSK suspend or press password suspend).";
 * - 0X7300: "PIN DUKPT is STOP (21 bit 1).";
 * - 0X7400: "Device is Busy.";
 * - 0XE100: "Can not enter sleep mode";
 * - 0XE200: "File has existed";
 * - 0XE300: "File has not existed";
 * - 0XE313: "IO line low -- Card error after session start";
 * - 0XE400: "Open File Error";
 * - 0XE500: "SmartCard Error";
 * - 0XE600: "Get MSR Card data is error";
 * - 0XE700: "Command time out";
 * - 0XE800: "File read or write is error";
 * - 0XE900: "Active 1850 error!";
 * - 0XEA00: "Load bootloader error";
 * - 0XEF00: "Protocol Error- STX or ETX or check error.";
 * - 0XEB00: "Picture is not exist";
 * - 0X2C02: "No Microprocessor ICC seated";
 * - 0X2C06: "no card seated to request ATR";
 * - 0X2D01: "Card Not Supported, ";
 * - 0X2D03: "Card Not Supported, wants CRC";
 * - 0X690D: "Command not supported on reader without ICC support";
 * - 0X8100: "ICC error time out on power-up";
 * - 0X8200: "invalid TS character received - Wrong operation step";
 * - 0X8300: "Decode MSR Error";
 * - 0X8400: "TriMagII no Response";
 * - 0X8500: "No Swipe MSR Card";
 * - 0X8510: "No Financial Card";
 * - 0X8600: "Unsupported F, D, or combination of F and D";
 * - 0X8700: "protocol not supported EMV TD1 out of range";
 * - 0X8800: "power not at proper level";
 * - 0X8900: "ATR length too long";
 * - 0X8B01: "EMV invalid TA1 byte value";
 * - 0X8B02: "EMV TB1 required";
 * - 0X8B03: "EMV Unsupported TB1 only 00 allowed";
 * - 0X8B04: "EMV Card Error, invalid BWI or CWI";
 * - 0X8B06: "EMV TB2 not allowed in ATR";
 * - 0X8B07: "EMV TC2 out of range";
 * - 0X8B08: "EMV TC2 out of range";
 * - 0X8B09: "per EMV96 TA3 must be >  - 0XF";
 * - 0X8B10: "ICC error on power-up";
 * - 0X8B11: "EMV T=1 then TB3 required";
 * - 0X8B12: "Card Error, invalid BWI or CWI";
 * - 0X8B13: "Card Error, invalid BWI or CWI";
 * - 0X8B17: "EMV TC1/TB3 conflict-";
 * - 0X8B20: "EMV TD2 out of range must be T=1";
 * - 0X8C00: "TCK error";
 * - 0XA304: "connector has no voltage setting";
 * - 0XA305: "ICC error on power-up invalid (SBLK(IFSD) exchange";
 * - 0XE301: "ICC error after session start";
 * - 0XFF00: "Request to go online";
 * - 0XFF01: "EMV: Accept the offline transaction";
 * - 0XFF02: "EMV: Decline the offline transaction";
 * - 0XFF03: "EMV: Accept the online transaction";
 * - 0XFF04: "EMV: Decline the online transaction";
 * - 0XFF05: "EMV: Application may fallback to magstripe technology";
 * - 0XFF06: "EMV: ICC detected tah the conditions of use are not satisfied";
 * - 0XFF07: "EMV: ICC didn't accept transaction";
 * - 0XFF08: "EMV: Transaction was cancelled";
 * - 0XFF09: "EMV: Application was not selected by kernel or ICC format error or ICC missing data error";
 * - 0XFF0A: "EMV: Transaction is terminated";
 * - 0XFF0B: "EMV: Other EMV Error";
 * - 0XFFFF: "NO RESPONSE";
 * - 0XF002: "ICC communication timeout";
 * - 0XF003: "ICC communication Error";
 * - 0XF00F: "ICC Card Seated and Highest Priority, disable MSR work request";
 * - 0XF200: "AID List / Application Data is not exist";
 * - 0XF201: "Terminal Data is not exist";
 * - 0XF202: "TLV format is error";
 * - 0XF203: "AID List is full";
 * - 0XF204: "Any CA Key is not exist";
 * - 0XF205: "CA Key RID is not exist";
 * - 0XF206: "CA Key Index it not exist";
 * - 0XF207: "CA Key is full";
 * - 0XF208: "CA Key Hash Value is Error";
 * - 0XF209: "Transaction  format error";
 * - 0XF20A: "The command will not be processing";
 * - 0XF20B: "CRL is not exist";
 * - 0XF20C: "CRL number  exceed max number";
 * - 0XF20D: "Amount, Other Amount, Trasaction Type  are  missing";
 * - 0XF20E: "The Identification of algorithm is mistake";
 * - 0XF20F: "No Financial Card";
 * - 0XF210: "In Encrypt Result state, TLV total Length is greater than Max Length";
 * - 0X1001: "INVALID ARG";
 * - 0X1002: "FILE_OPEN_FAILED";
 * - 0X1003: "FILE OPERATION_FAILED";
 * - 0X2001: "MEMORY_NOT_ENOUGH";
 * - 0X3002: "SMARTCARD_FAIL";
 * - 0X3003: "SMARTCARD_INIT_FAILED";
 * - 0X3004: "FALLBACK_SITUATION";
 * - 0X3005: "SMARTCARD_ABSENT";
 * - 0X3006: "SMARTCARD_TIMEOUT";
 * - 0X5001: "EMV_PARSING_TAGS_FAILED";
 * - 0X5002: "EMV_DUPLICATE_CARD_DATA_ELEMENT";
 * - 0X5003: "EMV_DATA_FORMAT_INCORRECT";
 * - 0X5004: "EMV_NO_TERM_APP";
 * - 0X5005: "EMV_NO_MATCHING_APP";
 * - 0X5006: "EMV_MISSING_MANDATORY_OBJECT";
 * - 0X5007: "EMV_APP_SELECTION_RETRY";
 * - 0X5008: "EMV_GET_AMOUNT_ERROR";
 * - 0X5009: "EMV_CARD_REJECTED";
 * - 0X5010: "EMV_AIP_NOT_RECEIVED";
 * - 0X5011: "EMV_AFL_NOT_RECEIVED";
 * - 0X5012: "EMV_AFL_LEN_OUT_OF_RANGE";
 * - 0X5013: "EMV_SFI_OUT_OF_RANGE";
 * - 0X5014: "EMV_AFL_INCORRECT";
 * - 0X5015: "EMV_EXP_DATE_INCORRECT";
 * - 0X5016: "EMV_EFF_DATE_INCORRECT";
 * - 0X5017: "EMV_ISS_COD_TBL_OUT_OF_RANGE";
 * - 0X5018: "EMV_CRYPTOGRAM_TYPE_INCORRECT";
 * - 0X5019: "EMV_PSE_NOT_SUPPORTED_BY_CARD";
 * - 0X5020: "EMV_USER_SELECTED_LANGUAGE";
 * - 0X5021: "EMV_SERVICE_NOT_ALLOWED";
 * - 0X5022: "EMV_NO_TAG_FOUND";
 * - 0X5023: "EMV_CARD_BLOCKED";
 * - 0X5024: "EMV_LEN_INCORRECT";
 * - 0X5025: "CARD_COM_ERROR";
 * - 0X5026: "EMV_TSC_NOT_INCREASED";
 * - 0X5027: "EMV_HASH_INCORRECT";
 * - 0X5028: "EMV_NO_ARC";
 * - 0X5029: "EMV_INVALID_ARC";
 * - 0X5030: "EMV_NO_ONLINE_COMM";
 * - 0X5031: "TRAN_TYPE_INCORRECT";
 * - 0X5032: "EMV_APP_NO_SUPPORT";
 * - 0X5033: "EMV_APP_NOT_SELECT";
 * - 0X5034: "EMV_LANG_NOT_SELECT";
 * - 0X5035: "EMV_NO_TERM_DATA";
 * - 0X6001: "CVM_TYPE_UNKNOWN";
 * - 0X6002: "CVM_AIP_NOT_SUPPORTED";
 * - 0X6003: "CVM_TAG_8E_MISSING";
 * - 0X6004: "CVM_TAG_8E_FORMAT_ERROR";
 * - 0X6005: "CVM_CODE_IS_NOT_SUPPORTED";
 * - 0X6006: "CVM_COND_CODE_IS_NOT_SUPPORTED";
 * - 0X6007: "NO_MORE_CVM";
 * - 0X6008: "PIN_BYPASSED_BEFORE";
 * - 0X7001: "PK_BUFFER_SIZE_TOO_BIG";
 * - 0X7002: "PK_FILE_WRITE_ERROR";
 * - 0X7003: "PK_HASH_ERROR";
 * - 0X8001: "NO_CARD_HOLDER_CONFIRMATION";
 * - 0X8002: "GET_ONLINE_PIN";
 * - 0XD000: "Data not exist";
 * - 0XD001: "Data access error";
 * - 0XD100: "RID not exist";
 * - 0XD101: "RID existed";
 * - 0XD102: "Index not exist";
 * - 0XD200: "Maximum exceeded";
 * - 0XD201: "Hash error";
 * - 0XD205: "System Busy";
 * - 0X0E01: "Unable to go online";
 * - 0X0E02: "Technical Issue";
 * - 0X0E03: "Declined";
 * - 0X0E04: "Issuer Referral transaction";
 * - 0X0F01: "Decline the online transaction";
 * - 0X0F02: "Request to go online";
 * - 0X0F03: "Transaction is terminated";
 * - 0X0F05: "Application was not selected by kernel or ICC format error or ICC missing data error";
 * - 0X0F07: "ICC didn't accept transaction";
 * - 0X0F0A: "Application may fallback to magstripe technology";
 * - 0X0F0C: "Transaction was cancelled";
 * - 0X0F0D: "Timeout";
 * - 0X0F0F: "Other EMV Error";
 * - 0X0F10: "Accept the offline transaction";
 * - 0X0F11: "Decline the offline transaction";
 * - 0X0F21: "ICC detected tah the conditions of use are not satisfied";
 * - 0X0F22: "No app were found on card matching terminal configuration";
 * - 0X0F23: "Terminal file does not exist";
 * - 0X0F24: "CAPK file does not exist";
 * - 0X0F25: "CRL Entry does not exist";
 * - 0X0FFE: "code when blocking is disabled";
 * - 0X0FFF: "code when command is not applicable on the selected device";
 * - 0XF005: "ICC Encrypted C-APDU Data Structure Length Error Or Format Error.";
 * - 0XBBE0: "CM100 Success";
 * - 0XBBE1: "CM100 Parameter Error";
 * - 0XBBE2: "CM100 Low Output Buffer";
 * - 0XBBE3: "CM100 Card Not Found";
 * - 0XBBE4: "CM100 Collision Card Exists";
 * - 0XBBE5: "CM100 Too Many Cards Exist";
 * - 0XBBE6: "CM100 Saved Data Does Not Exist";
 * - 0XBBE8: "CM100 No Data Available";
 * - 0XBBE9: "CM100 Invalid CID Returned";
 * - 0XBBEA: "CM100 Invalid Card Exists";
 * - 0XBBEC: "CM100 Command Unsupported";
 * - 0XBBED: "CM100 Error In Command Process";
 * - 0XBBEE: "CM100 Invalid Command";
 *
 * - 0X9031: "Unknown command";
 * - 0X9032: "Wrong parameter (such as the length of the command is incorrect)";
 *
 * - 0X9038: "Wait (the command couldnt be finished in BWT)";
 * - 0X9039: "Busy (a previously command has not been finished)";
 * - 0X903A: "Number of retries over limit";
 *
 * - 0X9040: "Invalid Manufacturing system data";
 * - 0X9041: "Not authenticated";
 * - 0X9042: "Invalid Master DUKPT Key";
 * - 0X9043: "Invalid MAC Key";
 * - 0X9044: "Reserved for future use";
 * - 0X9045: "Reserved for future use";
 * - 0X9046: "Invalid DATA DUKPT Key";
 * - 0X9047: "Invalid PIN Pairing DUKPT Key";
 * - 0X9048: "Invalid DATA Pairing DUKPT Key";
 * - 0X9049: "No nonce generated";
 * - 0X9949: "No GUID available.  Perform getVersion first.";
 * - 0X9950: "MAC Calculation unsuccessful. Check BDK value.";
 *
 * - 0X904A: "Not ready";
 * - 0X904B: "Not MAC data";
 *
 * - 0X9050: "Invalid Certificate";
 * - 0X9051: "Duplicate key detected";
 * - 0X9052: "AT checks failed";
 * - 0X9053: "TR34 checks failed";
 * - 0X9054: "TR31 checks failed";
 * - 0X9055: "MAC checks failed";
 * - 0X9056: "Firmware download failed";
 *
 * - 0X9060: "Log is full";
 * - 0X9061: "Removal sensor unengaged";
 * - 0X9062: "Any hardware problems";
 *
 * - 0X9070: "ICC communication timeout";
 * - 0X9071: "ICC data error (such check sum error)";
 * - 0X9072: "Smart Card not powered up";
 */
void device_getResponseCodeString(IN int returnCode, OUT char * despcrition);

/**
 * Review the return code description.<br/>
 * @param returnCode  the response result.
 * @param description
 * @retval the string for description of response result
 *
 * - 0: "no error, beginning task";
 * - 1: "no response from reader";
 * - 2: "invalid response data";
 *
 * - 01: " Incorrect Header Tag";
 * - 02: " Unknown Command";
 * - 03: " Unknown Sub-Command";
 * - 04: " CRC Error in Frame";
 * - 05: " Incorrect Parameter";
 * - 06: " Parameter Not Supported";
 * - 07: " Mal-formatted Data";
 * - 08: " Timeout";
 * - 0A: " Failed / NACK";
 * - 0B: " Command not Allowed";
 * - 0C: " Sub-Command not Allowed";
 * - 0D: " Buffer Overflow (Data Length too large for reader buffer)";
 * - 0E: " User Interface Event";
 * - 10: " Need clear firmware(apply in boot loader only)";
 * - 11: " Communication type not supported, VT-1, burst, etc. Need encrypted firmware (apply in boot loader only)";
 * - 12: " Secure interface is not functional or is in an intermediate state.";
 * - 13: " Data field is not mod 8";
 * - 14: " Pad 0x80 not found where expected";
 * - 15: " Specified key type is invalid";
 * - 16: " Could not retrieve key from the SAM (InitSecureComm)";
 * - 17: " Hash code problem";
 * - 18: " Could not store the key into the SAM (InstallKey)";
 * - 19: " Frame is too large";
 * - 1A: " Unit powered up in authentication state but POS must resend the InitSecureComm command";
 * - 1B: " The EEPROM may not be initialized because SecCommInterface does not make sense";
 * - 1C: " Problem encoding APDU Module-Specific Status Codes ";
 * - 20: " Unsupported Index (ILM) SAM Transceiver error - problem communicating with the SAM (Key Mgr)";
 * - 21: " Unexpected Sequence Counter in multiple frames for single bitmap (ILM)Length error in data returned from the SAM (Key Mgr)
 * - 22: " Improper bit map (ILM)";
 * - 23: " Request Online Authorization";
 * - 24: " ViVOCard3 raw data read successful";
 * - 25: " Message index not available (ILM) ViVOcomm activate transaction card type (ViVOcomm)";
 * - 26: " Version Information Mismatch (ILM)";
 * - 27: " Not sending commands in correct index message index (ILM)";
 * - 28: " Time out or next expected message not received (ILM)";
 * - 29: " ILM languages not available for viewing (ILM)";
 * - 2A: " Other language not supported (ILM)";
 * - 41: " from 41 to 4F, Module-specific errors for Key Manager";
 *
 * - 50: " Auto-Switch OK";
 * - 51: " Auto-Switch failed";
 * - 70: " Antenna Error 80h Use another card";
 * - 81: " Insert or swipe card";
 * - 90: " Data encryption Key does not exist";
 * - 91: " Data encryption Key KSN exhausted";
 */
void device_getIDGStatusCodeString(IN int returnCode, OUT char * despcrition);

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
 * Authorizes the transaction for an MSR/CTLS/ICC card
 *
 * The tags will be returned in the callback routine.
 *
 * @param amount Transaction amount value (tag value 9F02)
 *        - SEE IMPORTANT NOTE BELOW
 * @param amtOther Other amount value, if any (tag value 9F03)
 *        - SEE IMPORTANT NOTE BELOW
 * @param type Transaction type (tag value 9C).
 * @param timeout Timeout value in seconds.
 * @param tags Any other tags to be included in the request.  Passed
 * as TLV.  Example, tag 9F02 with amount 0x000000000100 would be
 * 0x9F0206000000000100.  If tags 9F02 (amount), 9F03 (other amount), or
 * 9C (transaction type) are included, they will take priority over
 * these values supplied as individual parameters to this method.
 *
 * @param tagsLen The length of tags data buffer.
 *
 * >>>>>IMPORTANT: parameters for amount and amtOther MUST BE PASSED
 *      AS A DOUBLE VALUE WITH DECIMAL POINT.  Example, do not pass 1,
 *      but instead pass 1.0 or 1.00. Otherwise, results will be
 *      unpredictable
 *
 *
 *
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 * Note: if auto poll is on, it will returm the error
 * IDG_P2_STATUS_CODE_COMMAND_NOT_ALLOWED
 *
 * NOTE ON APPLEPAY VAS:
 * To enable ApplePay VAS, first a merchant record must be defined in one of
 * the six available index positions (1-6) using device_setMerchantRecord,
 * then container tag FFEE06 must be sent as part of the additional tags
 * parameter of device_startTransaction.  Tag FFEE06 must contain tag 9F26
 * and 9F22, and can optionanally contain tags 9F2B and DF01.
 * Example FFEE06189F220201009F2604000000009F2B050100000000DF010101
 * 9F22 = two bytes = ApplePay Terminal Applicaiton Version Number.
 *        Hard defined as 0100 for now. (required)
 * 9F26 = four bytes = ApplePay Terminal Capabilities Information (required)
 *  - Byte 1 = RFU
 *  - Byte 2 = Terminal Type
 *  - - Bit 8 = VAS Support    (1=on, 0 = off)
 *  - - Bit 7 = Touch ID Required  (1=on, 0 = off)
 *  - - Bit 6 = RFU
 *  - - Bit 5 = RFU
 *  - - Bit 1, 2, 3, 4
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
                            IN BYTE * tags,
                            IN int tagsLen);

/**
 * Start Transaction Request
 *
 * Authorizes the transaction for an MSR/CTLS/ICC card
 *
 * The tags will be returned in the callback routine.
 *
 * @param timeout Timeout value in seconds.
 * @param tags The tags to be included in the request.  Passed as a TLV.
 *   Example, tag 9F02 with amount 0x000000000100 would be 0x9F0206000000000100
 * Be sure to include 9F02 (amount)and9C (transaction type).
 *
 * @param tagsLen The length of tags data buffer.
 *
 * >>>>>IMPORTANT: parameters for amount and amtOther MUST BE PASSED
 * AS A DOUBLE VALUE WITH DECIMAL POINT.  Example, do not pass 1, but
 * instead pass 1.0 or 1.00. Otherwise, results will be unpredictable
 *
 *
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 * Note: if auto poll is on, it will returm the error
 *       IDG_P2_STATUS_CODE_COMMAND_NOT_ALLOWED
 *
 * NOTE ON APPLEPAY VAS:
 * To enable ApplePay VAS, first a merchant record must be defined in
 * one of the six available index positions (1-6) using
 * device_setMerchantRecord, then container tag FFEE06 must be sent as
 * part of the additional tags parameter of device_startTransaction.
 * Tag FFEE06 must contain tag 9F26 and 9F22, and can optionanally
 * contain tags 9F2B and DF01.  Example
 * FFEE06189F220201009F2604000000009F2B050100000000DF010101
 *
 * 9F22 = two bytes = ApplePay Terminal Applicaiton Version Number.
 *        Hard defined as 0100 for now. (required)
 * 9F26 = four bytes = ApplePay Terminal Capabilities Information (required)
 *  - Byte 1 = RFU
 *  - Byte 2 = Terminal Type
 *  - - Bit 8 = VAS Support    (1=on, 0 = off)
 *  - - Bit 7 = Touch ID Required  (1=on, 0 = off)
 *  - - Bit 6 = RFU
 *  - - Bit 5 = RFU
 *  - - Bit 1, 2, 3, 4
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
 *  9F2B = 5 bytes = ApplePay VAS Filter.
 *         Each byte filters for that specific merchant index  (optional)
 *  DF01 = 1 byte = ApplePay VAS Protocol.  (optional)
 *  - - Bit 1 : 1 = URL VAS, 0 = Full VAS
 *  - - Bit 2 : 1 = VAS Beeps, 0 = No VAS Beeps
 *  - - Bit 3 : 1 = Silent Comm Error, 2 = EMEA Comm Error
 *  - - Bit 4-8 : RFU
 *
 */
int device_activateTransaction(IN const int _timeout,
                               IN BYTE * tags,
                               IN int tagsLen);

/**
 * Cancel Transaction
 *
 * Cancels the currently executing transaction.
 *
 *
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int device_cancelTransaction();

/**
 * Drive Free Space
 * This command returns the free and used disk space on the flash drive.
 * @param free Free bytes available on device
 * @param used Used bytes on on device
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int device_getDriveFreeSpace(OUT int * free,
                             OUT int * used);

/**
 * List Directory
 *  This command retrieves a directory listing of user
 *  accessible files from the reader.
 * @param directoryName Directory Name.  If null, root directory is listed
 * @param directoryNameLen Directory Name Length.  If null, root directory
 *        is listed
 * @param recursive Included sub-directories
 * @param onSD TRUE = use flash storage
 * @directory The returned directory information
 * @directoryLen The returned directory information length
 * @return RETURN_CODE: Values can be parsed with device_getErrorString()
 */
int device_listDirectory(IN char * directoryName,
                         IN int directoryNameLen,
                         IN int recursive,
                         IN int onSD,
                         OUT char * directory,
                         OUT int * directoryLen);

/**
 * Create Directory
 *  This command adds a subdirectory to the indicated path.
 * @param directoryName Directory Name.  The data for this command is
 *  ASCII string with the complete path and directory name you want to create.
 *  You do not need to specify the root directory. Indicate subdirectories
 *  with a forward slash (/).
 * @param directoryNameLen Directory Name Length.
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int device_createDirectory(IN char * directoryName,
                           IN int directoryNameLen);

/**
 * Delete Directory
 *  This command deletes an empty directory.
 * @param filename Complete path and file name of the directory you want to
 *    delete. You do not need to specify the root directory. Indicate
 *    subdirectories with a forward slash (/).
 * @param filenameLen File Name Length.
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int device_deleteDirectory(IN char * fileName,
                           IN int fileNameLen);

/**
 * Transfer File
 *  This command transfers a data file to the reader.
 * @param fileName Filename.  The data for this command is a ASCII string with
 *   the complete path and file name you want to create. You do not need to
 *   specify the root directory. Indicate subdirectories with a forward slash (/)
 * @param filenameLen File Name Length.
 * @param file The data file.
 * @param fileLen File Length.
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int device_transferFile(IN char * fileName,
                        IN int fileNameLen,
                        IN BYTE * file,
                        IN int fileLen);

/**
 * Delete File
 *  This command deletes a file or group of files.
 * @param filename Complete path and file name of the file you want to delete.
 *   You do not need to specify the root directory. Indicate subdirectories
 *   with a forward slash (/).
 * @param filenameLen File Name Length.
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int device_deleteFile(IN char * fileName, IN int fileNameLen);

/**
 * Polls device for Firmware Version
 *
 * @param firmwareVersion Response returned of Firmware Version
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 *
 */
int device_getFirmwareVersion(OUT char * firmwareVersion);

/**
 * Control MSR LED - AUGUSTA
 *
 * Controls the LED for the MSR
 *
 *
 * @param indexLED For Augusta, must be set to 1 (MSR LED)
 * @param control LED Status:
 *  - 00: OFF
 *  - 01: RED Solid
 *  - 02: RED Blink
 *  - 11: GREEN Solid
 *  - 12: GREEN Blink
 *  - 21: BLUE Solid
 *  - 22: BLUE Blink
 * @param intervalOn Blink interval ON, in ms    (Range 200 - 2000)
 * @param intervalOff Blink interval OFF, in ms  (Range 200 - 2000)
 *
 *
 * @return RETURN_CODE: Values can be parsed with device_getResponseCodeString
 *
 * intervalOn = 500, int intervalOff = 500
 */
int device_controlLED(byte indexLED,
                      byte control,
                      int intervalOn ,
                      int intervalOff );

/**
 * Control ICC LED - AUGUSTA
 *
 * Controls the LED for the ICC card slot
 *
 *
 * @param controlMode 0 = off, 1 = solid, 2 = blink
 * @param interval Blink interval, in ms  (500 = 500 ms)
 *
 * @return RETURN_CODE: Values can be parsed with device_getResponseCodeString
 */
int device_controlLED_ICC(int controlMode,
                          int interval);

/**
 * Control the MSR LED - AUGUSTA
 *
 * Controls the MSR / ICC LED
 * This API not recommended to control ICC LED
 * @param control
 *     - 0x00 = off,
 *     - 0x01 = RED Solid,
 *     - 0x02 = RED Blink,
 *     - 0x11 = GREEN Solid,
 *     - 0x12 = GREEN Blink,
 *     - 0x21 = BLUE Solid,
 *     - 0x22 = BLUE Blink,
 * @param intervalOn Blink interval on time last, in ms
 * (500 = 500 ms, valid from 200 to 2000)
 * @param intervalOff Blink interval off time last, in ms
 * (500 = 500 ms, valid from 200 to 2000)
 *
 *
 * @return RETURN_CODE: Values can be parsed with device_getResponseCodeString
 *
 * int intervalOn = 500, int intervalOff = 500)
 */
int device_controlLED_MSR(byte control,
                          int intervalOn,
                          int intervalOff );

/**
 * Control Beep - AUGUSTA
 *
 * Controls the Beeper
 *
 *
 * @param index For Augusta, must be set to 1 (only one beeper)
 * @param frequency Frequency, range 1000-20000 (suggest minimum 3000)
 * @param duration Duration, in milliseconds (range 1 - 65525)
 *
 * @return RETURN_CODE: Values can be parsed with device_getResponseCodeString
 */
int device_controlBeep(int index,
                       int frequency,
                       int duration);

/**
 * Get DRS Status - AUGUSTA TTK
 *
 * Gets the status of DRS(Destructive Reset).
 *
 * @param codeDRS
 *  the data format is [DRS SourceBlk Number] [SourceBlk1] ... [SourceBlkN]
 *  [DRS SourceBlk Number] is 2 bytes, format is NumL NumH. It is Number of [SourceBlkX]
 *  [SourceBlkX] is n bytes, Format is [SourceID] [SourceLen] [SourceData]
 *  [SourceID] is 1 byte
 *  [SourceLen] is 1 byte, it is length of [SourceData]
 *
 * [SourceID]    [SourceLen]  [SourceData]
 *    00                1     01 - Application Error
 *    01                1     01 - Application Error
 *    02                1     0x01 - EMV L2 Configuration Check Value Error
 *                            0x02 - Future Key Check Value Error
 *    10                1     01    - Battery Error
 *    11                1     Bit 0 - Tamper Switch 1 (0-No, 1-Error)
 *                            Bit 1 - Tamper Switch 2 (0-No, 1-Error)
 *                            Bit 2 - Tamper Switch 3 (0-No, 1-Error)
 *                            Bit 3 - Tamper Switch 4 (0-No, 1-Error)
 *                            Bit 4 - Tamper Switch 5 (0-No, 1-Error)
 *                            Bit 5 - Tamper Switch 6 (0-No, 1-Error)
 *
 *    12                1     01  - TemperatureHigh or Low
 *    13                1     01  - Voltage High or Low
 *    1F                4     Reg31~24bits, Reg23~16bits, Reg15~8bits, Reg7~0bits
 *
 *    @param codeDRSLen
 *        the length of codeDRS
 *
 * @return RETURN_CODE: Values can be parsed with device_getResponseCodeString()
 * Note: Only support TTK devices
 */
int device_getDRS(BYTE * codeDRS,
                  int * codeDRSLen);

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
int device_getKeyStatus(int * newFormat,
                        BYTE * status,
                        int * statusLen);

/**
  * Enter Stop Mode
  *
  * Set device enter to stop mode. In stop mode, LCD display and
  * backlight is off.  Stop mode reduces power consumption to the
  * lowest possible level.  A unit in stop mode can only be woken up
  * by a physical key press.

  * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
  */
int device_enterStopMode();

/**
  *  Set Sleep Mode Timer
  *
  * Set device enter to sleep mode after the given time. In sleep mode,
  * LCD display and backlight is off.
  * Sleep mode reduces power consumption to the lowest possible level.
  * A unit in Sleep mode can only be woken up by a physical key press.

  * @param time Enter sleep time value, in second.
  * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
  */
int device_setSleepModeTime(int time);


/**
 * Verify Backdoor Key to Unlock Security
 *
 * @return RETURN_CODE: Values can be parsed with device_getResponseCodeString
 *
 * Note: The function is only for TTK devices.
 */
int device_verifyBackdoorKey();

/**
 * Self check for TTK
 * If Self-Test function Failed, then work into De-activation State.
 * If device work into De-activation State, All Sensitive Data will be
 *  erased and it need be fixed in Manufacture.
 * @return RETURN_CODE: Values can be parsed with device_getResponseCodeString
 */
int device_selfCheck();

/**
 * Ping Device - NEO only
 *
 * Pings the reader.  If connected, returns success.  Otherwise, returns timeout.
 *
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int device_pingDevice();

/**
 * Control User Interface - NEO only
 *
 * Controls the User Interface:  Display, Beep, LED
 *
 * @param values Four bytes to control the user interface
 * Byte[0] = LCD Message
 * Messages 00-07 are normally controlled by the reader.
 * - 00h: Idle Message (Welcome)
 * - 01h: Present card (Please Present Card)
 * - 02h: Time Out or Transaction cancel (No Card)
 * - 03h: Transaction between reader and card is in the middle (Processing...)
 * - 04h: Transaction Pass (Thank You)
 * - 05h: Transaction Fail (Fail)
 * - 06h: Amount (Amount $ 0.00 Tap Card)
 * - 07h: Balance or Offline Available funds (Balance $ 0.00) Messages 08-0B
 *        are controlled by the terminal
 * - 08h: Insert or Swipe card (Use Chip & PIN)
 * - 09h: Try Again(Tap Again)
 * - 0Ah: Tells the customer to present only one card (Present 1 card only)
 * - 0Bh: Tells the customer to wait for authentication/authorization (Wait)
 * - FFh: indicates the command is setting the LED/Buzzer only.
 * Byte[1] = Beep Indicator
 * - 00h: No beep
 * - 01h: Single beep
 * - 02h: Double beep
 * - 03h: Three short beeps
 * - 04h: Four short beeps
 * - 05h: One long beep of 200 ms
 * - 06h: One long beep of 400 ms
 * - 07h: One long beep of 600 ms
 * - 08h: One long beep of 800 ms
 * Byte[2] = LED Number
 * - 00h: LED 0 (Power LED) 01h: LED 1
 * - 02h: LED 2
 * - 03h: LED 3
 * - FFh: All LEDs
 * Byte[3] = LED Status
 * - 00h: LED Off
 * - 01h: LED On
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int device_controlUserInterface(IN BYTE * values);

/**
 * Control Indicators
 *
 * Control the reader.  If connected, returns success.  Otherwise,
 * returns timeout.
 *
 * @param indicator description as follows:
 * - 00h: ICC LED
 * - 01h: Blue MSR
 * - 02h: Red MSR
 * - 03h: Green MSR
 * @param enable  TRUE = ON, FALSE = OFF
 * @return success or error code.  Values can be parsed with
 *        device_getResponseCodeString
 * @see ErrorCode
 */
int device_controlIndicator(IN int indicator, IN int enable);

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
 *
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int device_SendDataCommandNEO(IN int cmd,
                              IN int subCmd,
                              IN BYTE * data,
                              IN int dataLen,
                              OUT BYTE * response,
                              IN_OUT int * respLen);

/**
 * Send a Command to NGA device
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
 *
 * @return RETURN_CODE: Values can be parsed with device_getResponseCodeString()
 */
int device_SendDataCommand(IN BYTE * cmd,
                           IN int cmdLen,
                           IN BYTE * data,
                           IN int dataLen,
                           OUT BYTE * response,
                           IN_OUT int * respLen);

/**
 * Reboot Device - NGA
 * Executes a command to restart the device.
 * - Card data is cleared, resetting card status bits.
 * - Response data of the previous command is cleared.
 * - Resetting firmware.
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int device_rebootDevice();

/**
 * Update Firmware - NGA
 * Updates the firmware of the Spectrum Pro K21 HUB or Maxq1050.
 * @param firmwareData Signed binary data of a firmware file provided by IDTech
 * @param firmwareDataLen Length of firmwareData
 * @param firmwareName Firmware name. Must be one of the following two strings
 * (with appropriate version information)
 *  - "SP K21 APP Vx.xx.xxx"
 *  - "SP MAX APP Vx.xx.xxx"
 * @param encryptionType Encryption type
 * - 0 : Plaintext
 * - 1 : TDES ECB, PKCS#5 padding
 * - 2 : TDES CBC, PKCS#5, IV is all 0
 * @param keyBlob Encrypted firmware session key blob, TR-31 Rev B,
 *  wrapped by FW Key (Optional, none if firmware is plaintext)
 * @param keyBlobLen Length of keyBlob
 * @return RETURN_CODE:  Values can be parsed with errorCode.getErrorString()
 *
 * Firmware update status is returned in the callback with the following values:
 * sender = SPECTRUM_PRO
 * state = DeviceState.FirmwareUpdate
 * data = File Progress.  Two bytes, with byte[0] = current block,
 *                     and byte[1] = total blocks. 0x0310 = block 3 of 16
 * transactionResultCode:
 * - RETURN_CODE_DO_SUCCESS = Firmware Update Completed Successfully
 * - RETURN_CODE_BLOCK_TRANSFER_SUCCESS = Current block transferred successfully
 * - Any other return code represents an error condition
 */
int device_updateFirmware(IN BYTE * firmwareData,
                          IN int firmwareDataLen,
                          IN char * firmwareName,
                          IN int encryptionType,
                          IN BYTE * keyBlob,
                          IN int keyBlobLen);

/**
 * Polls device for Model Number
 *
 * @param sNumber  Returns Model Number
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 */
int config_getModelNumber(OUT char * sNumber);

/**
 * Polls device for Serial Number
 *
 * @param sNumber  Returns Serial Number
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 */
int config_getSerialNumber(OUT char * sNumber);

/**
 * Set the LED Controller - AUGUSTA
 * Set the MSR / ICC LED controlled by software or firmware
 * NOTE: The ICC LED always controlled by software.
 * @param firmwareControlMSRLED
 *  - 1:  firmware control the MSR LED
 *  - 0: software control the MSR LED
 * @param firmwareControlICCLED
 *  - 1:  firmware control the ICC LED
 *  - 0: software control the ICC LED
 * @return RETURN_CODE:    Values can be parsed with device_getResponseCodeString
 */
int config_setLEDController(int firmwareControlMSRLED,
                            int firmwareControlICCLED);

/**
 * Get the LED Controller Status - AUGUSTA
 * Get the MSR / ICC LED controlled status by software or firmware
 * NOTE: The ICC LED always controlled by software.
 * @param firmwareControlMSRLED
 *  - 1:  firmware control the MSR LED
 *  - 0: software control the MSR LED
 * @param firmwareControlICCLED
 *  - 1:  firmware control the ICC LED
 *  - 0: software control the ICC LED
 * @return RETURN_CODE:    Values can be parsed with device_getResponseCodeString
 */
int config_getLEDController( int * firmwareControlMSRLED,
                             int * firmwareControlICCLED);

/**
 * Set the Beeper Controller - AUGUSTA
 * Set the Beeper controlled by software or firmware
 *
 * @param firmwareControlBeeper  1 means firmware control the beeper,
 *                               0 means software control beeper.
 * @return RETURN_CODE:    Values can be parsed with device_getResponseCodeString
 */
int config_setBeeperController(int firmwareControlBeeper);

/**
 * Get the Beeper Controller Status - AUGUSTA
 * Set the Beeper controlled Status by software or firmware
 *
 * @param firmwareControlBeeper  1 means firmware control the beeper,
 *                               0 means software control beeper.
 * @return RETURN_CODE: Values can be parsed with device_getResponseCodeString
 */
int config_getBeeperController( int * firmwareControlBeeper);

/**
 * Set Encryption Control - AUGUSTA
 *
 * Set Encryption Control to switch status between MSR and ICC/EMV function.
 *     Following Encryption status supported:
 *     - MSR ON, ICC/EMV ON,
 *     - MSR ON, ICC/EMV OFF,
 *     - MSR OFF, ICC/EMV OFF,
 *
 * @param msr
 *     - 1:  enable MSR with Encryption,
 *     - 0: disable MSR with Encryption,
 * @param icc
 *     - 1:  enable ICC with Encryption,
 *     - 0: disable ICC with Encryption,
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 */
int config_setEncryptionControl(int msr, int icc);

/**
 * Get Encryption Control - AUGUSTA
 *
 * Get Encryption Control to switch status between MSR and ICC/EMV function.
 *     Following Encryption status supported:
 *     - MSR ON, ICC/EMV ON,
 *     - MSR ON, ICC/EMV OFF,
 *     - MSR OFF, ICC/EMV OFF,
 *
 * @param msr
 *     - 1:  enabled MSR with Encryption,
 *     - 0: disabled MSR with Encryption,
 * @param icc
 *     - 1:  enabled ICC with Encryption,
 *     - 0: disabled ICC with Encryption,
 *
 * @return RETURN_CODE:    Values can be parsed with device_getResponseCodeString
 */
int config_getEncryptionControl( int * msr,
                                 int * icc);

// /**
//  * Start Remote Key Injection - AUGUSTA
//  *
//  * Starts a remote key injection request with IDTech RKI servers.
//  * This function is reserved and not implemented.
//  *
//  * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString
//  */
// int device_startRKI();

/**
 * Enable Pass Through - NEO
 *
 * Enables Pass Through Mode for direct communication with L1
 * interface (power on icc, send apdu, etc).
 *
 * @param enablePassThrough 1 = pass through ON, 0 = pass through OFF
 *
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int device_enablePassThrough(int enablePassThrough);
/**
  * Enable L100 Pass Through
  *
  * Enables Pass Through Mode for direct communication to L100 hook up
  * to NEO II device
  *
  * @param enableL100PassThrough 1 = pass through ON, 0 = pass through OFF

  * @return RETURN_CODE: Values can be parsed with device_getResponseCodeString()
  */
int device_enableL100PassThrough(int enableL100PassThrough);

/**
  * Get L100 Pass Through Mode
  *
  * Get current Pass Through Mode for direct communication to L100
  * hook up to NEO II device
  *
  *
  * @return RETURN_CODE: return 1 if L100 Pass Through Mode is TRUE,
  *  0 if L100 Pass Through Mode is FALSE
  */
int device_getL100PassThroughMode();

/**
   * Enables pass through mode for ICC. Required when direct ICC
   * commands are required (power on/off ICC, exchange APDU)
   *
   * @param data The data includes Poll Timeout,
   *      Flags, Contact Interface to Use, Beep Indicator, LED Status,
   *      and Display Strings.
   * @param dataLen length of data
   * @return success or error code.  Values can be parsed
   *      with device_getIDGStatusCodeString
   * @see ErrorCode
   */
int device_enhancedPassthrough(IN BYTE * data,
                               IN int dataLen);

/**
 * Send Burst Mode - NEO
 *
 * Sets the burst mode for the device.
 *
 * @param mode 0 = OFF, 1 = Always On, 2 = Auto Exit
 *
 * @return success or error code.  Values can be parsed with
 *  device_getIDGStatusCodeString
 * @see ErrorCode
 */
int device_setBurstMode(IN BYTE mode);

/**
 * Set Poll Mode - NEO
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
 * Set Merchant Record - NEO
 * Sets the merchant record for ApplePay VAS
 *
 * @param index Merchant Record index, valid values 1-6
 * @param enabled Merchant Enabled/Valid flag
 * @param merchantID  Merchant unique identifer registered with Apple.
 *        Example com.idtechproducts.applePay
 * @param merchantURL Merchant URL, when applicable
 *
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 *
 */
int device_setMerchantRecord(int index,
                             int enabled,
                             char * merchantID,
                             char * merchantURL);

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
 *         with device_getIDGStatusCodeString()
 * @see ErrorCode
 */
int device_getMerchantRecord(IN int index,
                             OUT BYTE * record);

/**
 * Poll Card Reader
 *
 * Provides information about the state of the Card Reader
 *
 *    @param status Six bytes indicating card reader information
 *    Byte 0:
 *     - Bit 0: Device Manufacturing CA data valid
 *     - Bit 1: Device Manufacturing Secure data valid
 *     - Bit 2: HOST_CR_MASTER_DUKPT Key valid
 *     - Bit 3: HOST_CR_MAC Keys valid (Authenticated)
 *     - Bit 4: RFU
 *     - Bit 5: RFU
 *     - Bit 6: DATA_DUKPT Key Valid
 *     - Bit 7: Key is initialized (MFK and RSA Key pairs)
 *
 *    Byte 1:
 *     - Bit 0: Firmware Key Valid
 *     - Bit 1: RFU
 *     - Bit 2: CR_PINPAD_MASTER_DUKPT Key valid
 *     - Bit 3: CR_PINPAD_MAC Keys valid (Authenticated)
 *     - Bit 4: DATA Pairing DUKPT Key valid
 *     - Bit 5: PIN Pairing DUKPT Key Valid
 *     - Bit 6: RFU
 *     - Bit 7: RFU
 *
 *    Byte 2:
 *     - Bit 0: RFU
 *     - Bit 1: Tamper Switch #1 Error
 *     - Bit 2: Battery Backup Error
 *     - Bit 3: Temperature Error
 *     - Bit 4: Voltage Sensor Error
 *     - Bit 5: Firmware Authentication Error
 *     - Bit 6: Tamper Switch #2 Error
 *     - Bit 7: Removal Tamper Error
 *
 *    Byte 3:
 *     - Battery Voltage (example 0x32 = 3.2V, 0x24 = 2.4V)
 *
 *    Byte 4:
 *     - Bit 0: Log is Full
 *     - Bit 1: Mag Data Present
 *     - Bit 2: Card Insert
 *     - Bit 3: Removal Sensor connected
 *     - Bit 4: Card Seated
 *     - Bit 5: Latch Mechanism Active
 *     - Bit 6: Removal Sensor Active
 *     - Bit 7: Tamper Detector Active
 *
 *    Byte 5:
 *     - Bit 0: SAM Available
 *     - Bit 1: Chip Card Reader Available
 *     - Bit 2: Host Connected
 *     - Bit 3: Contactless Available
 *     - Bit 4: PINPAD connected
 *     - Bit 5: MSR Header connected
 *     - Bit 6: RFU
 *     - Bit 7: Production Unit
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int device_pollCardReader(OUT BYTE * status);

/**
 * Get DUKPT KSN
 *
 * Returns the KSN for the provided key index
 *
 * @param type Key type:
 *  - 0: Key Encryption Key (Master Key or KEK)
 *  - 2: Data Encryption Key (DEK)
 *  - 5: MAC Key (MAK)
 *  - 10: RKL Key Encryption Key (REK)
 *  - 20: HSM DUKPT Key
 *
 * @param KSN  Key Serial Number
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int device_getSpectrumProKSN(IN int type,
                             OUT BYTE * KSN);

/**
 * Calibrate reference parameters
 * @param delta Delta value (0x02 standard default value)
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int device_calibrateParameters(BYTE delta);

/**
 * get RTC date and time of the device
 *
 * @param dateTime
 *            <dateTime Data> is:
 *                 6 byte data for YYMMDDHHMMSS in hex.  For example 0x171003102547 stands for 2017 Oct 3rd 10:25:47
 *
 * @param dateTimeLen
 *                 return 6 bytes if successful
 *
 *
 * @return success or error code. Values can be parsed with device_getResponseCodeString
 * @see ErrorCode
 */
int device_getRTCDateTime(IN BYTE * dateTime,
                          IN_OUT int * dateTimeLen);

/**
 * set RTC date and time of the device
 *
 * @param dateTime
 *            <dateTime Data> is:
 *                 6 byte data for YYMMDDHHMMSS in hex.  For example 0x171003102547 stands for 2017 Oct 3rd 10:25:47
 *
 * @param dateTimeLen
 *                 should be always 6 bytes
 *
 * @return success or error code. Values can be parsed with device_getResponseCodeString
 * @see ErrorCode
 */
int device_setRTCDateTime(IN BYTE * dateTime,
                          IN int dateTimeLen);

/**
 * Configures the buttons on the ViVOpay Vendi reader
 * @param done
 *     0x01: the Done switch is enabled
 *     0x00: the Done switch is disabled
 *
 * @param swipe
 *     0x01: the Swipe Card switch is enabled
 *     0x00: the Swipe Card switch is disabled
 *
 * @param delay
 *     an unsigned delay value (<= 30) in seconds
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
int device_getButtonConfiguration(OUT BYTE * done,
                                  OUT BYTE * swipe,
                                  OUT BYTE * delay);

/**
 * Stops the blue LEDs on the ViVOpay Vendi reader from flashing in
 * left to right sequence and turns the LEDs off, and contactless
 * function is disable at the same time
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int device_disableBlueLED();

/**
 * Use this function to control the blue LED behavior on the Vendi reader
 *
 * @param data
 *     Sequence data
 *         Byte 0 (Cycle): 0 = Cycle once, 1 = Repeat
 *         Byte 1 (LEDs): LED State Bitmap
 *         Byte 2-3 (Duration): Given in multiples of 10 millisecond
 *         Byte 4 (LED): LED State Bitmap
 *         Byte 5-6 (Duration): Given in multiples of 10 millisecond
 *         Byte 7-24 (Additional LED/Durations): Define up to 8
 *                    LED and duration pairs
 *
 *     LED State Bitmap:
 *         Bit 8: Left blue LED, 0 = off, 1 = on
 *         Bit 7: Center Blue LED, 0 = off, 1 = on
 *         Bit 6: Right Blue LED0 = off, 1 = on
 *         Bit 5: Yellow LED, 0 = off, 1 = on
 *         Bit 4: Reserved for future use
 *         Bit 3: Reserved for future use
 *         Bit 2: Reserved for future use
 *         Bit 1: Reserved for future use
 *
 * @param dataLen
 *     Length of the sequence data: 0 or 4 to 25 bytes
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int device_enableBlueLED(IN BYTE * data, IN int dataLen);

/**
 * Use this function to clear the LCD display
 *
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int device_lcdDisplayClear();

/**
 * Enable or disable the external LCD message
 *   It will turn off the external LCD messages including EMV
 *   transactions.  (For the users who only need MSR and/or CTLS
 *   transactions.)
 *   The function only works for VP5300
 *
 *    @param enableExtLCDMsg  1 = ON, 0 = OFF
 *    @return success or error code.  Values can be parsed
 *           with device_getResponseCodeString
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
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int device_buzzerOnOff();

/**
 * Use this function to display text on the LCD display.
 *     On the Vendi reader the LCD is a 2-line character display.
 *
 * @param message
 *     Valid messages for the first line of text are between 1 and 16
 *     printable characters long.
 *     If the text message is greater than 16 bytes but not more than 32 bytes,
 *     byte 17 and onward are displayed as a second row of text.
 *     All messages are left justified on the LCD display.
 *
 * @param messageLen
 *     Length of the message: 1 to 32 bytes
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int device_lcdDisplayLine1Message(IN BYTE * message,
                                  IN int messageLen);

/**
 * Use this function to display the message on line 2 of the LCD display.
 *     On the Vendi reader the LCD is a 2-line character display.
 *
 * @param message
 *     Valid messages are between 1 and 16 printable characters long.
 *     All messages are left justified on the LCD display.
 *
 * @param messageLen
 *     Length of the message: 1 to 16 bytes
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int device_lcdDisplayLine2Message(IN BYTE * message,
                                  IN int messageLen);

/**
 * ICC Function enable/disable - AUGUSTA
 * Enable ICC function with or without seated notification
 *
 * @param withNotification
 *  - 1:  with notification when ICC seated status changed,
 *  - 0: without notification.
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 */
int icc_enable(IN int withNotification);

/**
 * ICC Function enable/disable - AUGUSTA
 * Disable ICC function
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 */
int icc_disable();

/**
 * Power On ICC
 *
 * Power up the currently selected microprocessor card in the ICC reader
 * @param ATR, the ATR data response when succeeded power on ICC,
 * @param inLen, the length of ATR data
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int icc_powerOnICC(OUT BYTE * ATR,
                   IN_OUT int * inLen);

/**
 * Power Off ICC
 *
 * Powers down the ICC
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 *
 * If Success, empty
 * If Failure, ASCII encoded data of error string
 */
int icc_powerOffICC();

/**
 * Exchange APDU with plain text - AUGUSTA
 * For Non-SRED Augusta Only
 *
 * Sends an APDU packet to the ICC.  If successful, response is the
 * APDU data in response parameter.
 *
 * @param c_APDU    APDU data packet
 * @param cLen     APDU data packet length
 * @param reData    Unencrypted APDU response
 * @param reLen      Unencrypted APDU response data length
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 */
int icc_exchangeAPDU(IN BYTE * c_APDU,
                     IN int cLen,
                     OUT BYTE * reData,
                     IN_OUT int * reLen);

/**
 * Exchange APDU with encrypted data - AUGUSTA
 * For SRED Augusta Only
 *
 * Sends an APDU packet to the ICC.  If successful, response is the
 * APDU data in response parameter.
 *
 * @param c_APDU    KSN + encytpted APDU data packet, or no KSN (use last known KSN) + encrypted APDU data packet
 *            With KSN:  [0A][KSN][Encrypted C-APDU]
 *            Without KSN:  [00][Encrypted C-APDU]
 *
 *    The format of Raw C-APDU Data Structure of [m-bytes Encrypted C-APDU] is below:
 *     - m = 2 bytes Valid C-APDU Length + x bytes Valid C-APDU + y bytes Padding (0x00)
 *    Note:
 *    For TDES mode: 2+x should be multiple of 8. If it was not multiple of 8, unit should padded y bytes 0x00 automatically (2+x+y should be multiple of 8).
 *    For AES mode: 2+x should be multiple of 16. If it was not multiple of 16, unit should padded y bytes 0x00 automatically (2+x+y should be multiple of 16).
 * @param cLen  data packet length
 * @param reData    response encrypted APDU response.  Can be three options:
 *   - [00] + [Plaintext R-APDU]
 *   - [01] + [0A] + [KSN] + [n bytes Encrypted R-APDU without Status Bytes]    + [2 bytes Status Bytes]
 *   - [01] + [00] + [n bytes Encrypted R-APDU without Status Bytes]    + [2 bytes Status Bytes]
 *
 *    The KSN, when provided, will be 10 bytes.  The KSN will only be provided when
 *    it has changed since the last provided KSN.  Each card Power-On generates a new KSN.
 *    During a sequence of commands where the KSN is identical, the first response
 *    will have a KSN length set to [0x0A] followed by the KSN, while subsequent
 *    commands with the same KSN value will have a KSN length of [0x00] followed by the
 *    Encrypted R-APDU without Status Bytes.
 * @param reLen       encrypted APDU response data length
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 */
int icc_exchangeEncryptedAPDU(IN BYTE * c_APDU,
                              IN int cLen,
                              OUT BYTE * reData,
                              IN_OUT int * reLen);

/**
 * Get APDU KSN - AUGUSTA
 *
 * Retrieves the KSN used in ICC Encypted APDU usage
 *
 * @param KSN Returns the encrypted APDU packet KSN
 * @param inLen KSN data length
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 */
int icc_getAPDU_KSN(OUT BYTE * KSN,
                    IN_OUT int * inLen);

/**
 * Get ICC Function status - AUGUSTA
 * Get ICC Function status about enable/disable and with or without
 * seated notification
 *
 * @param enabled
 * - 1:    ICC Function enabled,
 * - 0: means disabled.
 * @param withNotification 1 means with notification when ICC seated status
 *    changed. 0 means without notification.
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 */
int icc_getFunctionStatus(OUT int * enabled,
                          OUT int * withNotification);

/**
 * Get Reader Status - AUGUSTA
 *
 * Returns the reader status
 *
 * @param status Pointer that will return with the ICCReaderStatus results.
 *    bit 0:  0 = ICC Power Not Ready, 1 = ICC Powered
 *    bit 1:  0 = Card not seated, 1 = card seated
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 */
int icc_getICCReaderStatus(OUT BYTE * status);

/**
 * Get Key Format For DUKPT - AUGUSTA
 *
 * Specifies how data will be encrypted with Data Key or PIN key (if
 * DUKPT key loaded). This applies to both MSR and ICC
 *
 * @param format Response returned from method:
 * - 'TDES': Encrypted card data with TDES if DUKPT Key had been loaded.(default)
 * - 'AES': Encrypted card data with AES if DUKPT Key had been loaded.
 * - 'NONE': No Encryption.
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 */
int icc_getKeyFormatForICCDUKPT(OUT BYTE * format);

/**
 * Get Key Type for DUKPT - AUGUSTA
 *
 * Specifies the key type used for ICC DUKPT encryption This applies
 * to both MSR and ICC
 *
 * @param type Response returned from method:
 * - 'DATA': Encrypted card data with Data Key DUKPT Key had been loaded.
 *          (default)
 * - 'PIN': Encrypted card data with PIN Key if DUKPT Key had been loaded.
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 */
int icc_getKeyTypeForICCDUKPT(OUT BYTE * type);

/**
 * Set Key Format for DUKPT - AUGUSTA
 *
 * Sets how data will be encrypted, with either TDES or AES (if DUKPT
 * key loaded) This applies to both MSR and ICC
 *
 * @param format encryption Encryption Type
 * - 00: Encrypt with TDES
 * - 01: Encrypt with AES
 * - 02: Encrypt with TransArmor - AUGUSTA only
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 */
int icc_setKeyFormatForICCDUKPT(IN BYTE format);

/**
 * Set Key Type for DUKPT Key - AUGUSTA
 *
 * Sets which key the data will be encrypted with, with either Data
 * Key or PIN key (if DUKPT key loaded) This applies to both MSR and
 * ICC
 *
 * @param type Encryption Type
 * - 00: Encrypt with Data Key
 * - 01: Encrypt with PIN Key
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 */
int icc_setKeyTypeForICCDUKPT(IN BYTE type);

/**
 * Get the ISO8583 1987 version handler.
 *
 * @param ISOHandler A handler with knowledge of the ISO8583 1987 version fields
 *
 * @return RETURN_CODE: Values can be parsed with device_getErrorString()
 */
int iso8583_get1987Handler(OUT DL_ISO8583_HANDLER * ISOHandler);

/**
 * Get the ISO8583 1993 version handler.
 *
 * @param ISOHandler A handler with knowledge of the ISO8583 1993 version fields
 *
 * @return RETURN_CODE: Values can be parsed with device_getErrorString()
 */
int iso8583_get1993Handler(OUT DL_ISO8583_HANDLER * ISOHandler);

/**
 * Get the ISO8583 2003 version handler.
 *
 * @param ISOHandler A handler with knowledge of the ISO8583 2003 version fields
 *
 * @return RETURN_CODE: Values can be parsed with device_getErrorString()
 */
int iso8583_get2003Handler(OUT DL_ISO8583_HANDLER * ISOHandler);

/**
 * Get the specified field's information using the data field.
 *
 * @param dataField - The data field number
 * @param ISOHandler - The ISO8583 handler
 * @param field - The requested field
 *
 * @return RETURN_CODE: Values can be parsed with device_getErrorString()
 */
int iso8583_getField(IN DL_UINT16 dataField,
                     IN DL_ISO8583_HANDLER * ISOHandler,
                     OUT DL_ISO8583_FIELD_DEF * field);

/**
 * Initialize the ISO8583 message structure.
 *
 * @param ISOMessage - The initialized ISO8583 message structure
 *
 * @return RETURN_CODE: Values can be parsed with device_getErrorString()
 */
int iso8583_initializeMessage(OUT DL_ISO8583_MSG * ISOMessage);

/**
 * Get the specified message field using the data field.
 *
 * @param dataField - The data field number
 * @param ISOMessage - The ISO8583 message structure
 * @param messageField - The requested message field
 *
 * @return 0 if the if the setting was applied; otherwise, return -1 on failure
 */
int iso8583_getMessageField(IN DL_UINT16 dataField,
                            IN DL_ISO8583_MSG * ISOMessage,
                            OUT DL_ISO8583_MSG_FIELD * messageField);

/**
 * Set the specified message field.
 *
 * @param dataField - The data field number
 * @param data - The data to apply
 * @param ISOMessage - The ISO8583 message structure
 *
 * @return RETURN_CODE: Values can be parsed with device_getErrorString()
 */
int iso8583_setMessageField(IN DL_UINT16 dataField,
                            IN const DL_UINT8 * data,
                            OUT DL_ISO8583_MSG * ISOMessage);

/**
 * Remove the specified message field.
 *
 * @param dataField - The data field number
 * @param ISOMessage - The ISO8583 message structure
 *
 * @return RETURN_CODE: Values can be parsed with device_getErrorString()
 */
int iso8583_removeMessageField(IN DL_UINT16 dataField,
                               OUT DL_ISO8583_MSG * ISOMessage);

/**
 * Pack the message fields into an array.
 *
 * @param ISOHandler - The ISO8583 handler
 * @param ISOMessage - The ISO8583 message structure
 * @param packedData - The packaged data
 * @param packedDataLength - The packaged data's length
 *
 * @return RETURN_CODE: Values can be parsed with device_getErrorString()
 */
int iso8583_packMessage(IN const DL_ISO8583_HANDLER * ISOHandler,
                        IN const DL_ISO8583_MSG * ISOMessage,
                        OUT DL_UINT8 * packedData,
                        OUT DL_UINT16 * packedDataLength);

/**
 * Unpack the message field array into the ISO8583 message structure.
 *
 * @param ISOHandler - The ISO8583 handler
 * @param packedData - The packaged data
 * @param packedDataLength - The packaged data's length
 * @param ISOMessage - The ISO8583 message structure
 *
 * @return RETURN_CODE: Values can be parsed with device_getErrorString()
 */
int iso8583_unpackMessage(IN const DL_ISO8583_HANDLER * ISOHandler,
                          IN const DL_UINT8 * packedData,
                          IN DL_UINT16 packedDataLength,
                          OUT DL_ISO8583_MSG * ISOMessage);

/**
 * Deallocate the ISO8583 message structure's memory.
 *
 * @param ISOMessage - The ISO8583 message structure
 *
 * @return RETURN_CODE: Values can be parsed with device_getErrorString()
 */
int iso8583_freeMessage(IN DL_ISO8583_MSG * ISOMessage);

/**
 * Serialize the message fields into an XML format.
 *
 * @param ISOHandler - The ISO8583 handler
 * @param ISOMessage - The ISO8583 message structure
 * @param serializedMessage - The XML-formatted message
 * @param serializedMessageLength - The XML message's length
 *
 * @return RETURN_CODE: Values can be parsed with device_getErrorString()
 */
int iso8583_serializeToXML(IN DL_ISO8583_HANDLER * ISOHandler,
                           IN DL_ISO8583_MSG * ISOMessage,
                           OUT BYTE * serializedMessage,
                           OUT int * serializedMessageLength);

/**
 * Deserialize the XML-formatted ISO8583 message.
 *
 * @param serializedMessage - The XML-formatted message
 * @param serializedMessageLength - The length of the XML-formatted message
 * @param ISOHandler - A null ISO8583 handler
 * @param ISOMessage - The ISO8583 message structure
 *
 * @return RETURN_CODE: Values can be parsed with device_getErrorString()
 */
int iso8583_deserializeFromXML(IN BYTE * serializedMessage,
                               IN int serializedMessageLength,
                               OUT DL_ISO8583_HANDLER * ISOHandler,
                               OUT DL_ISO8583_MSG * ISOMessage);

/**
 * Display the messages in a formatted manner on the screen for
 * verifying results.
 *
 * @param ISOHandler - The ISO8583 handler
 * @param ISOMessage - The ISO8583 message structure
 *
 * @return RETURN_CODE: Values can be parsed with device_getErrorString()
 */
int iso8583_displayMessage(IN DL_ISO8583_HANDLER * ISOHandler,
                           IN DL_ISO8583_MSG * ISOMessage);

/**
 * Reset to Initial State.
 *	This command places the reader UI into the
 *  idle state and displays the appropriate idle display.
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int lcd_resetInitialState();

/**
 * Custom Display Mode
 *  Controls the LCD display mode to custom display. Keyboard entry is
 *  limited to the Cancel, Clear, Enter and the function keys, if
 *  present. PIN entry is not permitted while the reader is in Custom
 *  Display Mode
 *
 * @param enable TRUE = enabled, FALSE = disabled
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
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
int lcd_setForeBackColor(IN BYTE * foreRGB,
                         IN int foreRGBLen,
                         IN BYTE * backRGB,
                         IN int backRGBLen);

/**
  * Clear Display
  *  Command to clear the display screen on the reader. It returns the
  *  display to the currently defined background color and terminates
  *  all events
  *
  * @param control for L100.
  *     0:First Line
  *             1:Second Line
  *             2:Third Line
  *             3:Fourth Line
  *             0xFF: All Screen
  *
  * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
  */
int lcd_clearDisplay(IN BYTE control);

/**
 * Enables Signature Capture
 *  This command executes the signature capture screen.  Once a
 *  signature is captured, it is sent to the callback with
 *  DeviceState.Signature, and the data will contain a .png of the
 *  signature
 *
 * @param timeout  Timeout waiting for the signature capture
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int lcd_captureSignature(IN int timeout);

/**
 * Start slide show
 *  You must send images to the reader's memory and send a Start
 *  Custom Mode command to the reader before it will respond to this
 *  commands. Image files must be in .bmp or .png format.
 *
 * @param files Complete paths and file names of the files you want to use,
 *  separated by commas. If a directory is specified, all files in the dirctory
 *  are displayed
 * @param filesLen Length of files
 * @param posX X coordinate in pixels, Range 0-271
 * @param posY Y coordinate in pixels, Range 0-479
 * @param posMode Position Mode
 *  - 0 = Center on Line Y
 *  - 1 = Display at (X, Y)
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
int lcd_startSlideShow(IN char * files,
                       IN int filesLen,
                       IN int posX,
                       IN int posY,
                       IN int posMode,
                       IN int touchEnable,
                       IN int recursion,
                       IN int touchTerminate,
                       IN int delay,
                       IN int loops,
                       IN int clearScreen);

/**
 * Cancel slide show
 *  Cancel the slide show currently running
 *
 * @param statusCode If the return code is not Success (0), the kernel may
 * return a four-byte Extended Status Code
 * @param statusCodeLen the length of the Extended Status Code (should be
 * 4 bytes)
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int lcd_cancelSlideShow(OUT BYTE * statusCode,
                        IN_OUT int * statusCodeLen);

/**
 * Set Display Image
 *  You must send images to the reader's memory and send a Start
 *  Custom Mode command to the reader before it will respond to Image
 *  commands. Image files must be in .bmp or .png format.
 *
 * @param file Complete path and file name of the file you want to
 * use. Example "file.png" will put in root directory, while
 * "ss/file.png" will put in ss directory (which must exist)
 * @param fileLen Length of files
 * @param posX X coordinate in pixels, Range 0-271
 * @param posY Y coordinate in pixels, Range 0-479
 * @param posMode Position Mode
 *  - 0 = Center on Line Y
 *  - 1 = Display at (X, Y)
 *  - 2 - Center on screen
 * @param touchEnable TRUE = Image is touch sensitive
 * @param clearScreen  TRUE = Clear screen
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int lcd_setDisplayImage(IN char * file,
                        IN int fileLen,
                        IN int posX,
                        IN int posY,
                        IN int posMode,
                        IN int touchEnable,
                        IN int clearScreen);

/**
 * Set Background Image
 *  You must send images to the readerï¿½s memory and send a Start
 *  Custom Mode command to the reader before it will respond to Image
 *  commands. Image files must be in .bmp or .png format.
 *
 * @param file Complete path and file name of the file you want to
 * use. Example "file.png" will put in root directory, while
 * "ss/file.png" will put in ss directory (which must exist)
 * @param fileLen Length of files
 * @param enable TRUE = Use Background Image, FALSE = Use Background Color
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int lcd_setBackgroundImage(IN char * file, IN int fileLen, IN int enable);

/**
 * Polls device for EMV Kernel Version  - AUGUSTA
 *
 * @param version Response returned of Kernel Version
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 *
 */
int emv_getEMVKernelVersion(OUT char * version);

/**
 * Get EMV Kernel check value info
 *
 * @param checkValue Response returned of Kernel check value info
 * @param checkValueLen the length of checkValue
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 *
 */
int emv_getEMVKernelCheckValue(OUT BYTE * checkValue,
                               IN_OUT int * checkValueLen);

/**
 * Get EMV Kernel configuration check value info
 *
 * @param checkValue Response returned of Kernel configuration check value info
 * @param checkValueLen the length of checkValue
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 *
 */
int emv_getEMVConfigurationCheckValue(OUT BYTE * checkValue,
                                      IN_OUT int * checkValueLen);

/**
 * Enables authenticate for EMV transactions.  If a
 * emv_startTranaction results in code 0x0010 (start transaction
 * success), then emv_authenticateTransaction can automatically
 * execute if parameter is set to TRUE
 * @param authenticate TRUE = auto authenticate, FALSE = manually authenticate
 */
void emv_setAutoAuthenticateTransaction(IN int authenticate);

/**
 * Enables complete for EMV transactions.  If a
 *  emv_authenticateTranaction results in code 0x0004 (go online),
 *  then emv_completeTransaction can automatically execute if
 *  parameter is set to TRUE
 * @param complete TRUE = auto complete, FALSE = manually complete
 */
void emv_setAutoCompleteTransaction(IN int complete);

/**
 * Gets auto authenticate value for EMV transactions.
 * @return RETURN_CODE:  TRUE = auto authenticate, FALSE = manually authenticate
 */
int emv_getAutoAuthenticateTransaction();

/**
 * Gets auto complete value for EMV transactions.
 * @return RETURN_CODE:  TRUE = auto complete, FALSE = manually complete
 */
int emv_getAutoCompleteTransaction();

/**
 * Allow fallback for EMV transactions.  Default is TRUE
 * @param allow TRUE = allow fallback, FALSE = don't allow fallback
 */
void emv_allowFallback(IN int allow);

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
 * @param tags Any other tags to be included in the request (Maximum Length = 500 bytes).  Passed as a string.  Example, tag 9F0C with amount 0x000000000100 would be "9F0C06000000000100"
 *   If tags 9F02 (amount), 9F03 (other amount), or 9C (transaction type) are included, they will take priority over these values supplied as individual parameters to this method.
 * Note:  To request tags to be  included in default response, use tag DFEE1A, and specify tag list.
 *    Example four tags 9F02, 9F36, 95, 9F37 to be included in response = DFEE1A079F029F369f9F37
 * @param tagsLen the length of tags
 *
 */
void emv_setTransactionParameters(IN double amount,
                                  IN double amtOther,
                                  IN int type,
                                  IN int timeout,
                                  IN BYTE * tags,
                                  IN int tagsLen);

/**
 * Start EMV Transaction Request
 *
 * Authorizes the EMV transaction for an ICC card
 *
 * The tags will be returned in the callback routine.
 *
 * @param amount Transaction amount value  (tag value 9F02) - SEE IMPORTANT NOTE BELOW
 * @param amtOther Other amount value, if any  (tag value 9F03) - SEE IMPORTANT NOTE BELOW
 * @param exponent Number of characters after decimal point
 * @param type Transaction type (tag value 9C).
 * @param timeout Timeout value in seconds.
 * @param tags Any other tags to be included in the request.  Passed as a TLV stream.  Example, tag 9F0C with amount 0x000000000100 would be 0x9F0C06000000000100
 * If tags 9F02 (amount), 9F03 (other amount), or 9C (transaction type) are included, they will take priority over these values supplied as individual parameters to this method.
 * @param tagsLen Length of tags
 * @param forceOnline TRUE = do not allow offline approval, FALSE = allow ICC to approve offline if terminal capable
 * Note:  To request tags to be  included in default response, use tag DFEE1A, and specify tag list.  Example four tags 9F02, 9F36, 95, 9F37 to be included in response = DFEE1A079F029F369f9F37
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 * >>>>>IMPORTANT: parameters for amount and amtOther MUST BE PASSED AS A DOUBLE VALUE WITH DECIMAL POINT.  Example, do not pass 1, but instead pass 1.0 or 1.00. Otherwise, results will be unpredictable
 */
int emv_startTransaction(IN double amount,
                         IN double amtOther,
                         IN int exponent,
                         IN int type,
                         IN int timeout,
                         IN BYTE * tags,
                         IN int tagsLen,
                         IN int forceOnline);

/**
 * Start EMV Transaction Request
 *
 * Authorizes the EMV transaction for an ICC card
 *
 * The tags will be returned in the callback routine.
 *
 *
 * @param timeout Timeout value in seconds.
 * @param tags Tags to be included in the request.  Passed as a TLV
 * stream.  Example, tag 9F02 with amount 0x000000000100 would be
 * 0x9F0206000000000100
 *
 * @param tagsLen Length of tags
 * @param forceOnline TRUE = do not allow offline approval, FALSE = allow
 * ICC to approve offline if terminal capable
 * Note:  To request tags to be  included in default response, use tag DFEE1A,
 * and specify tag list.  Example four tags 9F02, 9F36, 95, 9F37 to be
 * included in response = DFEE1A079F029F369f9F37
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 * >>>>>IMPORTANT: parameters for amount and amtOther MUST BE PASSED AS A
 * DOUBLE VALUE WITH DECIMAL POINT.  Example, do not pass 1, but instead
 * pass 1.0 or 1.00. Otherwise, results will be unpredictable.
 */
int emv_activateTransaction(IN int timeout,
                            IN BYTE * tags,
                            IN int tagsLen,
                            IN int forceOnline);

/**
 * Authenticate EMV Transaction Request
 *
 * Authenticates the EMV transaction for an ICC card.  Execute this
 * after receiving response with result code 0x10 to emv_startTransaction
 *
 * The tags will be returned in the callback routine.
 *
 * @param updatedTLV  TLV stream that can be used to update the following values:
 *   - 9F02: Amount
 *   - 9F03: Other amount
 *   - 9C: Transaction type
 *   - 5F57: Account type
 *  In addition tag DFEE1A can be sent to specify tag list to include in
 *  results. Example four tags 9F02, 9F36, 95, 9F37 to be included in
 *  response = DFEE1A079F029F36959F37
 * @param updatedTLVLen
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 */
int emv_authenticateTransaction(IN BYTE * updatedTLV, IN int updatedTLVLen);

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
 *        0x11223344556677883030 (tag value 91).
 * @param iadLen the length of iadLen
 * @param tlvScripts 71/72 scripts, if any
 * @param tlvScriptsLen the length of tlvScriptsLen
 * @param tlv  Additional TLV data to return with transaction results (if any)
 * @param tlvLen the length of tlv
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 */
int emv_completeTransaction(IN int commError,
                            IN BYTE * authCode,
                            IN int authCodeLen,
                            IN BYTE * iad,
                            IN int iadLen,
                            IN BYTE * tlvScripts,
                            IN int tlvScriptsLen,
                            IN BYTE * tlv,
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
 * Retrieves specified EMV tags from the currently executing transaction.
 *
 * @param tags Tags to be retrieved.  Example 0x9F028A will retrieve tags
 * 9F02 and 8A
 * @param tagsLen Length of tag list
 * @param cardData All requested tags returned as unencrypted,
 * encrypted and masked TLV data in IDTTransactionData object
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_retrieveTransactionResult(IN BYTE * tags,
                                  IN int tagsLen,
                                  IDTTransactionData * cardData);

/**
 * Callback Response LCD Display
 *
 * Provides menu selection responses to the kernel after a callback was received with DeviceState.EMVCallback, and
 * callbackType = EMV_CALLBACK_TYPE.EMV_CALLBACK_TYPE_LCD, and lcd_displayMode = EMV_LCD_DISPLAY_MODE_MENU, EMV_LCD_DISPLAY_MODE_PROMPT,
 * or EMV_LCD_DISPLAY_MODE_LANGUAGE_SELECT
 *
 * @param type If Cancel key pressed during menu selection, then value is EMV_LCD_DISPLAY_MODE_CANCEL.  Otherwise, value can be EMV_LCD_DISPLAY_MODE_MENU, EMV_LCD_DISPLAY_MODE_PROMPT,
 * or EMV_LCD_DISPLAY_MODE_LANGUAGE_SELECT
 * @param selection If type = EMV_LCD_DISPLAY_MODE_MENU or EMV_LCD_DISPLAY_MODE_LANGUAGE_SELECT, provide the selection ID line number. Otherwise, if type = EMV_LCD_DISPLAY_MODE_PROMPT
 * supply either 0x43 ('C') for Cancel, or 0x45 ('E') for Enter/accept
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_callbackResponseLCD(IN int type,
                            byte selection);

/**
 * Callback Response MSR Entry
 *
 * Provides MSR information to kernel after a callback was received
 *   with DeviceState.EMVCallback, and callbackType = EMV_CALLBACK_MSR
 *
 * @param MSR Swiped track data
 * @param MSRLen the length of Swiped track data
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_callbackResponseMSR(IN BYTE * MSR, IN_OUT int MSRLen);

/**
 * Retrieve Application Data by AID
 *
 * Retrieves the Application Data as specified by the AID name passed
 * as a parameter.
 *
 * @param AID Name of ApplicationID. Must be between 5 and 16 bytes
 * @param AIDLen the length of AID data buffer.
 * @param tlv  The TLV elements of the requested AID
 * @param tlvLen the length of tlv data buffer.
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_retrieveApplicationData(IN BYTE * AID,
                                IN int AIDLen,
                                OUT BYTE * tlv,
                                IN_OUT int * tlvLen);

/**
 * Set Application Data by AID
 *
 * Sets the Application Data as specified by the application name and TLV data
 *
 * @param name Application name, 10-32 ASCII hex characters
 *        representing 5-16 bytes  Example "a0000000031010"
 * @param nameLen the length of name data buffer of Application name,
 * @param tlv  Application data in TLV format
 * @param tlvLen the length of tlv data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_setApplicationData(IN BYTE * name,
                           IN int nameLen,
                           IN BYTE * tlv,
                           IN int tlvLen);

/**
 * Set Application Data by TLV
 *
 * Sets the Application Data as specified by the TLV data
 * @param tlv  Application data in TLV format
 *         The first tag of the TLV data must be the group number (DFEE2D).
 *         The second tag of the TLV data must be the AID (9F06)
 *         Example valid TLV, for Group #2, AID a0000000035010:
 *         "dfee2d01029f0607a0000000051010ffe10101ffe50110ffe30114ffe20106"
 * @param tlvLen the length of tlv data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_setApplicationDataTLV(IN BYTE * tlv,
                              IN int tlvLen);

/**
 * Remove Application Data by AID
 *
 * Removes the Application Data as specified by the AID name passed as
 * a parameter
 *
 * @param AID Name of ApplicationID Must be between 5 and 16 bytes
 * @param AIDLen the length of AID data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_removeApplicationData(IN BYTE * AID,
                              IN int AIDLen);

/**
 * Remove All Application Data
 *
 * Removes all the Application Data
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_removeAllApplicationData();

/**
 * Retrieve AID list
 *
 * Returns all the AID names installed on the terminal.
 *
 * @param AIDList  array of AID name byte arrays
 * @param AIDListLen  the length of AIDList array buffer
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_retrieveAIDList(OUT BYTE * AIDList,
                        IN_OUT int * AIDListLen);

/**
 * Retrieve Terminal Data
 *
 * Retrieves the Terminal Data.
 *
 * @param tlv Response returned as a TLV
 * @param tlvLen the length of tlv data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_retrieveTerminalData(OUT BYTE * tlv,
                             IN_OUT int * tlvLen);

/**
 * Set Terminal Data
 *
 * Sets the Terminal Data as specified by the TerminalData structure
 * passed as a parameter
 *
 * @param tlv TerminalData configuration file
 * @param tlvLen the length of tlv data buffer
 *
 * @retval RETURN_CODE:  Return codes listed as typedef enum in
 *         IDTCommon:RETURN_CODE.
 *         Values can be parsed with device_getResponseCodeString()
 */
int emv_setTerminalData(IN BYTE * tlv,
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
 * Remove Terminal Data
 *
 * Removes the Terminal Data
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_removeTerminalData();

/**
 * Retrieve Certificate Authority Public Key
 *
 * Retrieves the CAPK as specified by the RID/Index  passed as a parameter.
 *
 * @param capk 6 bytes CAPK = 5 bytes RID + 1 byte Index
 * @param capkLen the length of capk data buffer
 * @param key Response returned as a CAKey format:
 *  [5 bytes RID][1 byte Index][1 byte Hash Algorithm][1 byte Encryption Algorithm]
 *  [20 bytes HashValue][4 bytes Public Key Exponent][2 bytes Modulus Length][Variable bytes Modulus]
 *  Where:
 *   - Hash Algorithm: The only algorithm supported is SHA-1.The value is set to 0x01
 *   - Encryption Algorithm: The encryption algorithm in which this key is used. Currently support only one type: RSA. The value is set to 0x01.
 *   - HashValue: Which is calculated using SHA-1 over the following fields: RID & Index & Modulus & Exponent
 *   - Public Key Exponent: Actually, the real length of the exponent is either one byte or 3 bytes. It can have two values: 3 (Format is 0x00 00 00 03), or  65537 (Format is 0x00 01 00 01)
 *   - Modulus Length: LenL LenH Indicated the length of the next field.
 *   - Modulus: This is the modulus field of the public key. Its length is specified in the field above.
 * @param keyLen the length of key data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_retrieveCAPK(IN BYTE * capk,
                     IN int capkLen,
                     OUT BYTE * key,
                     IN_OUT int * keyLen);

/**
 * Set Certificate Authority Public Key
 *
 * Sets the CAPK as specified by the CAKey structure
 *
 * @param capk CAKey format:
 *  [5 bytes RID][1 byte Index][1 byte Hash Algorithm][1 byte Encryption Algorithm][20 bytes HashValue][4 bytes Public Key Exponent][2 bytes Modulus Length][Variable bytes Modulus]
 *  Where:
 *   - Hash Algorithm: The only algorithm supported is SHA-1.The value is set to 0x01
 *   - Encryption Algorithm: The encryption algorithm in which this key is used. Currently support only one type: RSA. The value is set to 0x01.
 *   - HashValue: Which is calculated using SHA-1 over the following fields: RID & Index & Modulus & Exponent
 *   - Public Key Exponent: Actually, the real length of the exponent is either one byte or 3 bytes. It can have two values: 3 (Format is 0x00 00 00 03), or  65537 (Format is 0x00 01 00 01)
 *   - Modulus Length: LenL LenH Indicated the length of the next field.
 *   - Modulus: This is the modulus field of the public key. Its length is specified in the field above.
 * @param capkLen the length of capk data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_setCAPK(IN BYTE * capk,
                IN int capkLen);

/**
 * Remove Certificate Authority Public Key
 *
 * Removes the CAPK as specified by the RID/Index
 *
 * @param capk 6 byte CAPK =  5 bytes RID + 1 byte INDEX
 * @param capkLen the length of capk data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_removeCAPK(IN BYTE * capk,
                   IN int capkLen);

/**
 * Remove All Certificate Authority Public Key
 *
 * Removes all the CAPK
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_removeAllCAPK();

/**
 * Retrieve the Certificate Authority Public Key list
 *
 * Returns all the CAPK RID and Index installed on the terminal.
 *
 * @param keys [key1][key2]...[keyn], each key 6 bytes where
 *  key = 5 bytes RID + 1 byte index
 * @param keysLen the length of keys data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_retrieveCAPKList(OUT BYTE * keys, IN_OUT int * keysLen);

/**
 * Gets the terminal ID as printable characters .
 * @param terminalID Terminal ID string
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_retrieveTerminalID(OUT char * terminalID);

/**
 * Sets the terminal ID as printable characters .
 * @param terminalID Terminal ID to set
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_setTerminalID(IN char * terminalID);

/**
 * Retrieve the Certificate Revocation List
 *
 * Returns the CRL entries on the terminal.
 *
 * @param list [CRL1][CRL2]...[CRLn], each CRL 9 bytes where
 *  CRL = 5 bytes RID + 1 byte index + 3 bytes serial number
 * @param lssLen the length of list data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_retrieveCRL(OUT BYTE * list,
                    IN_OUT int * lssLen);

/**
 * Set Certificate Revocation List
 *
 * Sets the CRL
 *
 * @param list CRL Entries containing the RID, Index, and serial numbers to set
 *  [CRL1][CRL2]...[CRLn] where each [CRL]  is 9 bytes:
 *  [5 bytes RID][1 byte CAPK Index][3 bytes serial number]
 * @param lsLen the length of list data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_setCRL(IN BYTE * list,
               IN int lsLen);

/**
 * Retrieve the Certificate Revocation List
 *
 * Returns the CRL entries on the terminal.
 *
 * @param list [CRL1][CRL2]...[CRLn], each CRL 9 bytes where
 *  CRL = 5 bytes RID + 1 byte index + 3 bytes serial number
 * @param lsLen the length of list data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_removeCRL(IN BYTE * list,
                  IN int lsLen);

/**
 * Remove All Certificate Revocation List Entries
 *
 * Removes all CRLEntry entries
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int emv_removeAllCRL();

/**
 * Clear MSR Data
 * Clears the MSR Data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int msr_clearMSRData();

/**
 * Get MSR Data
 * Reads the MSR Data buffer
 * @param reData Card data
 * @param reLen Card data length
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int msr_getMSRData(OUT BYTE * reData,
                   IN_OUT int * reLen);

/**
 * Disable MSR Swipe
 * Cancels MSR swipe request.
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int msr_cancelMSRSwipe();

/**
 * Start MSR Swipe
 * Enables MSR, waiting for swipe to occur. Allows track selection.
 * Returns IDTMSRData instance to MSR_callBack()
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
void parseMSRData(IN BYTE * resData, IN int resLen,
                  IN_OUT IDTMSRData * cardData);

/**
 * Get Key Format For DUKPT
 *
 * Specifies how data will be encrypted with Data Key or PIN key (if
 * DUKPT key loaded). This applies to both MSR and ICC
 *
 * @param format Response returned from method:
 * - 'TDES': Encrypted card data with TDES if  DUKPT Key had been loaded.
 *           (default)
 * - 'AES': Encrypted card data with AES if DUKPT Key had been loaded.
 * - 'NONE': No Encryption.
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 */
int msr_getKeyFormatForICCDUKPT(OUT BYTE * format);

/**
 * Get Key Type for DUKPT
 *
 * Specifies the key type used for ICC DUKPT encryption.  This applies
 * to both MSR and ICC.
 *
 * @param type Response returned from method:
 * - 'DATA': Encrypted card data with Data Key DUKPT Key had been loaded.
 *           (default)
 * - 'PIN': Encrypted card data with PIN Key if DUKPT Key had been loaded.
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 */
int msr_getKeyTypeForICCDUKPT(OUT BYTE * type);

/**
 * Set Key Format for DUKPT
 *
 * Sets how data will be encrypted, with either TDES or AES (if DUKPT
 * key loaded) This applies to both MSR and ICC
 *
 * @param format encryption Encryption Type
 * - 00: Encrypt with TDES
 * - 01: Encrypt with AES
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 */
int msr_setKeyFormatForICCDUKPT(IN BYTE format);

/**
 * Set Key Type for DUKPT Key
 *
 * Sets which key the data will be encrypted with, with either Data
 * Key or PIN key (if DUKPT key loaded) This applies to both MSR and
 * ICC
 *
 * @param type Encryption Type
 * - 00: Encrypt with Data Key
 * - 01: Encrypt with PIN Key
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 */
int msr_setKeyTypeForICCDUKPT(IN BYTE type);

/**
 * Set MSR Capture Mode.
 *
 * For Non-SRED Augusta Only
 *
 * Switch between Auto mode and Buffer mode. Auto mode only available
 * on KB interface
 *
 *
 * @param isBufferMode
 * - 1: Enter into Buffer mode.
 * - 0: Enter into Auto mode. KB mode only. Swipes automatically captured
 *      and sent to keyboard buffer
 * @param withNotification
 * - 1: with notification when swiped MSR data.
 * - 0: without notification when swiped MSR data.
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
 */
int msr_captureMode(int isBufferMode,
                    int withNotification);

/**
 * Flush Track Data
 * Clears any track data being retained in memory by future PIN Block request.
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int msr_flushTrackData();

/**
 * Get Encrypted PIN
 *
 * Requests PIN Entry
 * @param mode
 * - 0x00- Cancel: Cancels PIN entry = also can execute pin_cancelPINEntry(). All other parameters for this method will be ignored
 * - 0x01- Online PIN DUKPT
 * - 0x02- Online PIN MKSK
 * - 0x03- Offline PIN  (No need to define PAN Source or ICC PAN)
 * @param PANSource
 * - 0x00- ICC:  PAN Captured from ICC and must be provided in iccPAN parameter
 * - 0x01- MSR:  PAN Captured from MSR swipe and will be inserted by Spectrum Pro. No need to provide iccPAN parameter.
 * @param iccPAN  PAN captured from ICC.  When PAN captured from MSR, this parameter will be ignored
 * @param iccPANLen  the length of iccPAN
 * @param startTimeout  The amount of time allowed to start PIN entry before timeout
 * @param entryTimeout  The amount of time to enter the PIN after first digit selected before timeout
 * @param language  Valid values "EN" for English, "ES" for Spanish, "ZH" for Chinese, "FR" for French
 * @param languageLen the length of language
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int pin_getPIN(IN int mode,
               IN int PANSource,
               IN char * iccPAN,
               IN int iccPANLen,
               IN int startTimeout,
               IN int entryTimeout,
               IN char * language,
               IN int languageLen);

/**
 * Cancel PIN Entry
 *
 * Cancels PIN entry request
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int pin_cancelPINEntry();

/**
 * Get Encrypted DUKPT PIN
 *
 * Requests PIN Entry for online authorization. PIN block and KSN
 * returned in callback function DeviceState.TransactionData with
 * cardData.pin_pinblock.  A swipe must be captured first before this
 * function can execute
 * @param keyType PIN block key type. Valid values 0, 3 for TDES, 4 for AES
 * @param timeout  PIN entry timout
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int pin_getEncryptedOnlinePIN(IN int keyType,
                              IN int timeout);

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
int pin_getPAN(IN int getCSC,
               IN int timeout);

/**
 * Prompt for Credit or Debit
 *
 * Requests prompt for Credit or Debit. Response returned in callback function
 * as DeviceState.MenuItem with data MENU_SELECTION_CREDIT = 0,
 * MENU_SELECTION_DEBIT = 1
 *
 * @param currencySymbol Allowed values are $ (0x24), ï¿½ (0xA5), ï¿½ (0xA3), ï¿½ (0xA4), or NULL
 * @param currencySymbolLen length of currencySymbol
 * @param displayAmount  Amount to display (can be NULL)
 * @param displayAmountLen  length of displayAmount
 * @param timeout  Menu entry timout. Valid values 2-20 seconds
 *
 * @param retData If successful, the return code is zero and the data
 * is 1 byte (0: Credit 1: Debit).  If the return code is not zero, it
 * may be a four-byte Extended Status Code
 * @param currencySymbolLen length of currencySymbol
 *
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int pin_promptCreditDebit(IN char * currencySymbol,
                          IN int currencySymbolLen,
                          IN char * displayAmount,
                          IN int displayAmountLen,
                          IN int timeout,
                          OUT BYTE * retData,
                          IN_OUT int * retDataLen);

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
int pin_getEncryptedPIN(int keyType,
                        char *PAN,
                        int PANLen,
                        char *message,
                        int messageLen,
                        int timeout);

/**
  * Prompt for Key Input
  *
  * Prompts for a numeric key using the secure message according to the
  * following table

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

int pin_promptForKeyInput(int messageID,
                          int languageID,
                          int maskInput,
                          int minLen,
                          int maxLen,
                          int timeout);

/**
  * Prompt for Amount Input
  *
  Prompts for amount input using the secure message according to the
  following table

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
     * @return RETURN_CODE: Values can be parsed with errorCode.getErrorString()
     */
int pin_promptForAmountInput(int messageID,
                             int languageID,
                             int minLen,
                             int maxLen,
                             int timeout);

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
   * @return RETURN_CODE: Values can be parsed with device_getResponseCodeString
   */

int pin_sendBeep(int frequency,
                 int duration);

/**
  * Save Prompt
  *
  Saves a message prompt to L100 memory.
        @param promptNumber  Prompt number  (0-9)
        @param prompt Prompt string (up to 20 characters)
        @param promptLen length of prompt
  * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString

  */
int lcd_savePrompt(int promptNumber,
                   char *prompt,
                   int promptLen);

/**
  * Display Prompt on Line
  *
  Displays a message prompt from L100 memory.
        @param promptNumber  Prompt number  (0-9)
        @param lineNumber Line number to display message prompt (1-4)
  * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString

  */
int lcd_displayPrompt(int promptNumber,
                      int lineNumber);

/**
        * Display Message on Line
        *
        Displays a message on a display line.
          @param lineNumber  Line number to display message on  (1-4)
          @param message Message to display
          @param messageLen length of message
        * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString

        */
int lcd_displayMessage(int lineNumber,
                       char* message,
                       int messageLen);

/**
  * Enable/Disable LCD Backlight
  * Trns on/off the LCD back lighting.
  * @param enable  TRUE = turn ON backlight, FALSE = turn OFF backlight
  * @return RETURN_CODE: Values can be parsed with device_getResponseCodeString

  */
int lcd_enableBacklight(int enable);

/**
 * Get Backlight Status
 *
 * Returns the status of the LCD back lighting.
 * @param enabled  1 = Backlight is ON, 0 = Backlight is OFF
 * @return RETURN_CODE: Values can be parsed with device_getResponseCodeString
 */
int lcd_getBacklightStatus(int *enabled);

/**
 * Request CSR
 *  Requests 3 sets of public keys: encrypting Keys,
 *  signing/validating keys, signing/validating 3rd party apps
 *
 * @param csr RequestCSR structure to return the data
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int ws_requestCSR(OUT RequestCSR * csr);

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
int ws_loadSSLCert(IN char * name,
                   IN int nameLen,
                   IN char * dataDER,
                   IN int dataDERLen);

/**
 * Revoke SSL Certificate
 *  Revokes a SSL Certificate by name
 *
 * @param name Name of certificate to revoke
 * @param nameLen Certificate Name Length
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int ws_revokeSSLCert(IN char * name,
                     IN int nameLen);

/**
 * Delete SSL Certificate
 *  Deletes a SSL Certificate by name
 *
 * @param name Name of certificate to delete
 * @param nameLen Certificate Name Length
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int ws_deleteSSLCert(IN char * name,
                     IN int nameLen);

/**
 * Get Certificate Chain Type
 *  Returns indicator for using test/production certificate chain
 *
 * @param type 0 = test certificate chain, 1 = production certificate chain
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int ws_getCertChainType(OUT int * type);

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
int ws_updateRootCertificate(IN char * name,
                             IN int nameLen,
                             IN char * dataDER,
                             IN int dataDERLen,
                             IN char * signature,
                             IN int signatureLen);

/**
 * Start CTLS Transaction Request
 *
 * Authorizes the CTLS transaction for an ICC card
 *
 * The tags will be returned in the callback routine.
 *
 * @param amount Transaction amount value    (tag value 9F02) - SEE IMPORTANT NOTE BELOW
 * @param amtOther Other amount value, if any    (tag value 9F03) - SEE IMPORTANT NOTE BELOW
 * @param type Transaction type (tag value 9C).
 * @param timeout Timeout value in seconds.
 * @param tags Any other tags to be included in the request.  Passed as TLV stream.    Example, tag 9F0C with amount 0x000000000100 would be 0x9F0C06000000000100
 * If tags 9F02 (amount), 9F03 (other amount), or 9C (transaction type) are included, they will take priority over these values supplied as individual parameters to this method.
 *
 * @param tagsLen The length of tags data buffer.
 *
 * >>>>>IMPORTANT: parameters for amount and amtOther MUST BE PASSED AS A DOUBLE VALUE WITH DECIMAL POINT.  Example, do not pass 1, but instead pass 1.0 or 1.00. Otherwise, results will be unpredictable
 *
 *
 *
 * @return RETURN_CODE:    Values can be parsed with device_getIDGStatusCodeString()
 * Note: if auto poll is on, it will returm the error IDG_P2_STATUS_CODE_COMMAND_NOT_ALLOWED
 *
 * NOTE ON APPLEPAY VAS:
 * To enable ApplePay VAS, first a merchant record must be defined in one of the six available index positions (1-6) using device_setMerchantRecord, then container tag FFEE06
 * must be sent as part of the additional tags parameter of ctls_startTransaction.  Tag FFEE06 must contain tag 9F26 and 9F22, and can optionanally contain tags 9F2B and DF01.
 * Example FFEE06189F220201009F2604000000009F2B050100000000DF010101
 * 9F22 = two bytes = ApplePay Terminal Applicaiton Version Number.    Hard defined as 0100 for now. (required)
 * 9F26 = four bytes = ApplePay Terminal Capabilities Information (required)
 *  - Byte 1 = RFU
 *  - Byte 2 = Terminal Type
 *  - - Bit 8 = VAS Support    (1=on, 0 = off)
 *  - - Bit 7 = Touch ID Required  (1=on, 0 = off)
 *  - - Bit 6 = RFU
 *  - - Bit 5 = RFU
 *  - - Bit 1, 2, 3, 4
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
 */
int ctls_startTransaction(IN double amount,
                          IN double amtOther,
                          IN int type,
                          IN const int _timeout,
                          IN BYTE * tags,
                          IN int tagsLen);

/**
 * Start CTLS Transaction Request
 *
 * Authorizes the CTLS transaction for an ICC card
 *
 * The tags will be returned in the callback routine.
 *
 * @param timeout Timeout value in seconds.
 * @param tags The tags to be included in the request.  Passed as TLV stream.
 *    Example, tag 9F02 with amount 0x000000000100 would be 0x9F0206000000000100
 *
 *
 * @param tagsLen The length of tags data buffer.
 *
 * >>>>>IMPORTANT: parameters for amount and amtOther MUST BE PASSED AS A
 * DOUBLE VALUE WITH DECIMAL POINT.  Example, do not pass 1, but instead
 * pass 1.0 or 1.00. Otherwise, results will be unpredictable
 *
 *
 *
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
 * contain tags 9F2B and DF01.
 * Example FFEE06189F220201009F2604000000009F2B050100000000DF010101
 * 9F22 = two bytes = ApplePay Terminal Applicaiton Version Number.
 *        Hard defined as 0100 for now. (required)
 * 9F26 = four bytes = ApplePay Terminal Capabilities Information (required)
 *  - Byte 1 = RFU
 *  - Byte 2 = Terminal Type
 *  - - Bit 8 = VAS Support    (1=on, 0 = off)
 *  - - Bit 7 = Touch ID Required  (1=on, 0 = off)
 *  - - Bit 6 = RFU
 *  - - Bit 5 = RFU
 *  - - Bit 1, 2, 3, 4
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
 *  9F2B = 5 bytes = ApplePay VAS Filter.  Each byte filters for that specific
 *         merchant index  (optional)
 *  DF01 = 1 byte = ApplePay VAS Protocol.  (optional)
 *  - - Bit 1 : 1 = URL VAS, 0 = Full VAS
 *  - - Bit 2 : 1 = VAS Beeps, 0 = No VAS Beeps
 *  - - Bit 3 : 1 = Silent Comm Error, 2 = EMEA Comm Error
 *  - - Bit 4-8 : RFU
 */
int ctls_activateTransaction( IN const int _timeout,
                              IN BYTE * tags,
                              IN int tagsLen);

/**
 * Cancel EMV Transaction
 *
 * Cancels the currently executing EMV transaction.
 *
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int ctls_cancelTransaction();

/**
 * Retrieve Application Data by AID
 *
 * Retrieves the Application Data as specified by the AID name passed
 * as a parameter.
 *
 * @param AID Name of ApplicationID. Must be between 5 and 16 bytes
 * @param AIDLen the length of AID data buffer.
 * @param tlv  The TLV elements of the requested AID
 * @param tlvLen the length of tlv data buffer.
 *
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int ctls_retrieveApplicationData(IN BYTE * AID,
                                 IN int AIDLen,
                                 OUT BYTE * tlv,
                                 IN_OUT int * tlvLen);

/**
 * Set Application Data by AID
 *
 * Sets the Application Data for CTLS as specified by TLV data
 *
 * @param tlv  Application data in TLV format
 *    The first tag of the TLV data must be the group number (FFE4).
 *    The second tag of the TLV data must be the AID (9F06)
 *
 *    Example valid TLV, for Group #2, AID a0000000035010:
 *    "ffe401029f0607a0000000051010ffe10101ffe50110ffe30114ffe20106"
 * @param tlvLen the length of tlv data buffer
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int ctls_setApplicationData(IN BYTE * tlv,
                            IN int tlvLen);

/**
 * Remove Application Data by AID
 * Removes the Application Data for CTLS as specified by the AID name
 * passed as a parameter
 *
 * @param AID Name of ApplicationID Must be between 5 and 16 bytes
 *
 * @param AIDLen the length of AID data buffer
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int ctls_removeApplicationData(IN BYTE * AID,
                               IN int AIDLen);

/**
 * Remove All Application Data
 *
 * Removes all the Application Data
 *
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int ctls_removeAllApplicationData();

/**
 * Retrieve AID list
 *
 * Returns all the AID names installed on the terminal for CTLS. .
 *
 * @param AIDList  array of AID name byte arrays
 * @param AIDListLen  the length of AIDList array buffer
 *
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int ctls_retrieveAIDList(OUT BYTE * AIDList,
                         IN_OUT int * AIDListLen);

/**
 * Retrieve Terminal Data
 *
 * Retrieves the Terminal Data for CTLS. This is configuration group 0
 *  (Tag FFEE - > FFEE0100).  The terminal data can also be retrieved
 *  by ctls_getConfigurationGroup(0).
 *
 * @param tlv Response returned as a TLV
 * @param tlvLen the length of tlv data buffer
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int ctls_retrieveTerminalData(OUT BYTE * tlv,
                              IN_OUT int * tlvLen);

/**
 * Set Terminal Data
 *
 * Sets the Terminal Data for CTLS as specified by the TLV.  The first
 *  TLV must be Configuration Group Number (Tag FFE4).  The terminal
 *  global data is group 0, so the first TLV would be FFE40100.  Other
 *  groups can be defined using this method (1 or greater), and those
 *  can be retrieved with ctls_getConfigurationGroup(int group), and
 *  deleted with ctls_removeConfigurationGroup(int group).  You cannot
 *  delete group 0
 *
 * @param tlv TerminalData configuration file
 * @param tlvLen the length of tlv data buffer
 *
 * @retval RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int ctls_setTerminalData(IN BYTE * tlv,
                         IN int tlvLen);

/**
 * Retrieve Certificate Authority Public Key
 *
 * Retrieves the CAPK for CTLS as specified by the RID/Index passed as
 * a parameter.
 *
 * @param capk 6 bytes CAPK = 5 bytes RID + 1 byte Index
 * @param capkLen the length of capk data buffer
 * @param key Response returned as a CAKey format:
 *  [5 bytes RID][1 byte Index][1 byte Hash Algorithm][1 byte Encryption Algorithm]
 *  [20 bytes HashValue][4 bytes Public Key Exponent][2 bytes Modulus Length][Variable bytes Modulus]
 *  Where:
 *   - Hash Algorithm: The only algorithm supported is SHA-1.The value is set to 0x01
 *   - Encryption Algorithm: The encryption algorithm in which this key is used. Currently support only one type: RSA. The value is set to 0x01.
 *   - HashValue: Which is calculated using SHA-1 over the following fields: RID & Index & Modulus & Exponent
 *   - Public Key Exponent: Actually, the real length of the exponent is either one byte or 3 bytes. It can have two values: 3 (Format is 0x00 00 00 03), or  65537 (Format is 0x00 01 00 01)
 *   - Modulus Length: LenL LenH Indicated the length of the next field.
 *   - Modulus: This is the modulus field of the public key. Its length is specified in the field above.
 * @param keyLen the length of key data buffer
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int ctls_retrieveCAPK(IN BYTE * capk,
                      IN int capkLen,
                      OUT BYTE * key,
                      IN_OUT int * keyLen);

/**
 * Set Certificate Authority Public Key
 *
 * Sets the CAPK as specified by the CAKey structure
 *
 * @param capk CAKey format:
 *  [5 bytes RID][1 byte Index][1 byte Hash Algorithm][1 byte Encryption Algorithm][20 bytes HashValue][4 bytes Public Key Exponent][2 bytes Modulus Length][Variable bytes Modulus]
 *  Where:
 *   - Hash Algorithm: The only algorithm supported is SHA-1.The value is set to 0x01
 *   - Encryption Algorithm: The encryption algorithm in which this key is used. Currently support only one type: RSA. The value is set to 0x01.
 *   - HashValue: Which is calculated using SHA-1 over the following fields: RID & Index & Modulus & Exponent
 *   - Public Key Exponent: Actually, the real length of the exponent is either one byte or 3 bytes. It can have two values: 3 (Format is 0x00 00 00 03), or  65537 (Format is 0x00 01 00 01)
 *   - Modulus Length: LenL LenH Indicated the length of the next field.
 *   - Modulus: This is the modulus field of the public key. Its length is specified in the field above.
 * @param capkLen the length of capk data buffer
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int ctls_setCAPK(IN BYTE * capk,
                 IN int capkLen);

/**
 * Remove Certificate Authority Public Key
 *
 * Removes the CAPK as specified by the RID/Index
 *
 * @param capk 6 byte CAPK =  5 bytes RID + 1 byte INDEX
 * @param capkLen the length of capk data buffer
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int ctls_removeCAPK(IN BYTE * capk,
                    IN int capkLen);

/**
 * Remove All Certificate Authority Public Key
 *
 * Removes all the CAPK
 *
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int ctls_removeAllCAPK();

/**
 * Retrieve the Certificate Authority Public Key list
 *
 * Returns all the CAPK RID and Index installed on the terminal.
 *
 * @param keys [key1][key2]...[keyn], each key 6 bytes where
 *  key = 5 bytes RID + 1 byte index
 * @param keysLen the length of keys data buffer
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int ctls_retrieveCAPKList(OUT BYTE * keys,
                          IN_OUT int * keysLen);

/**
 * Set Configuration Group
 *
 * Sets the Configuration Group for CTLS as specified by the TLV data
 *
 * @param tlv  Configuration Group Data in TLV format
 *    The first tag of the TLV data must be the group number (FFE4 or DFEE2D).
 *    A second tag must exist
 * @param tlvLen the length of tlv data buffer
 *
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int ctls_setConfigurationGroup(IN BYTE * tlv,
                               IN int tlvLen);

/**
 * Get Configuration Group
 *
 * Retrieves the Configuration for the specified Group.
 *
 * @param group Configuration Group
 * @param tlv return data
 * @param tlvLen the length of tlv data buffer
 *
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int ctls_getConfigurationGroup(IN int group,
                               OUT BYTE * tlv,
                               OUT int * tlvLen);

/**
 * Retrieve All Configuration Groups
 *
 * Returns all the Configuration Groups installed on the terminal for CTLS
 *
 * @param tlv  The TLV elements data
 * @param tlvLen the length of tlv data buffer.
 *
 * @return RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int ctls_getAllConfigurationGroups(OUT BYTE * tlv,
                                   IN_OUT int * tlvLen);

/**
 * Remove Configuration Group
 *
 * Removes the Configuration as specified by the Group.  Must not by group 0
 *
 * @param group Configuration Group
 *
 * @retval RETURN_CODE: Values can be parsed with device_getIDGStatusCodeString()
 */
int ctls_removeConfigurationGroup(int group);

/**
 * Display Online Authorization Result
 *  Use this command to display the status of an online authorization request
 *  on the reader's display (OK or NOT OK).
 *  Use this command after the reader sends an online request to the issuer.
 * @param isOK TRUE = OK, FALSE = NOT OK
 * @param TLV Optional TLV for AOSA
 * @param TLVLen TLV Length
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 */
int ctls_displayOnlineAuthResult(IN int isOK,
                                 IN BYTE * TLV,
                                 IN int TLVLen);

/**
 * Parse the PIN block data from the buffer into IDTPINData structure
 * @param resData PIN card data buffer
 * @param resLen the length of resData
 * @param cardData the parser result with IDTPINData structure
 */
void parsePINBlockData(IN BYTE * resData,
                       IN int resLen,
                       IN_OUT IDTPINData * cardData);

/**
 * Parse the PIN data from the buffer into IDTPINData structure
 * @param resData PIN card data buffer
 * @param resLen the length of resData
 * @param cardData the parser result with IDTPINData structure
 */
void parsePINData(IN BYTE * resData,
                  IN int resLen,
                  IN_OUT IDTPINData * cardData);

#ifdef __cplusplus
}
#endif


#endif

/*! \file libIDT_Device.h
 \brief Windows C++ API.

 Windows C++ Global API methods.
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
