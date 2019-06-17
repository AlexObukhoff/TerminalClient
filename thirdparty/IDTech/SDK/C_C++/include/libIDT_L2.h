#ifndef LIBIDT_L2_H_
#define LIBIDT_L2_H_

#include "IDTDef.h"

/**
 * Define the MSR callback function to get the MSR card data <br/>
 * It should be registered using the msr_registerCallBk,
 */
typedef void (*pMSR_callBack)(int, IDTMSRData);

/**
 * Define the EMV callback function to get the transaction message/data/result. <br/>
 * It should be registered using the emv_registerCallBk,
 */
typedef void (*pEMV_callBack)(int, int,unsigned char *, int,IDTTransactionData*,EMV_Callback*,int);

/**
* Start Transaction Request
*
Authorizes the transaction for an MSR/CTLS/ICC card

The tags will be returned in the callback routine.

@param amount Transaction amount value	(tag value 9F02) - SEE IMPORTANT NOTE BELOW
@param amtOther Other amount value, if any	(tag value 9F03) - SEE IMPORTANT NOTE BELOW
@param type Transaction type (tag value 9C).
@param timeout Timeout value in seconds.
@param tags Any other tags to be included in the request.  Passed as a byte array.	Example, tag 9F0C with amount 0x000000000100 would be 0x9F0C06000000000100
	If tags 9F02 (amount),9F03 (other amount), or 9C (transaction type) are included, they will take priority over these values supplied as individual parameters to this method.

@param tagsLen The length of tags data buffer.
@param interfaces Interfaces to use:
bit 0 = MSR
bit 1 = Contact
bit 2 = CTLS
bits 3-7 = RFU.


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
int L2_startTransaction(IN double amount, IN double amtOther, IN int type, IN const int _timeout, IN BYTE *tags, IN int tagsLen, IN BYTE interfaces);

/**
* Continue EMV Transaction after reviewing captured card tags
*
Continue the EMV transaction for an ICC card.  Execute this after receiving response with result code 0x10 to emv_startTransaction

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
int L2_continueTransaction(IN BYTE* updatedTLV, IN int updatedTLVLen);

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
* @param tlv  Additional TVL data to return with transaction results (if any)
* @param tlvLen the length of tlv
* @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
*/
int L2_completeTransaction(IN int commError, IN BYTE* authCode, IN int authCodeLen,
		IN BYTE* iad, IN int iadLen, IN BYTE* tlvScripts, IN int tlvScriptsLen, IN BYTE* tlv, IN int tlvLen);

/**
* Display Online Authorization Result
*
Use this command to display the status of an online authorization request on the reader’s display (OK or NOT OK). Use this command after the reader sends an online request to the issuer.


@param isOK 1 = OK, 0 = NOT OK
@param TLV Optional TLV for AOSA (9F5D)


* @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString
*/
int L2_displayOnlineAuthResult(IN int isOK, IN BYTE* tlv, IN int tlvLen);

/**
* Cancel Transaction
*
* Cancels the currently executing transaction.
* @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
*/
int L2_cancelTransaction();

/**
 *To register the emv callback function to get the EMV processing response.
 */
void L2_registerEMVCallBk(pEMV_callBack pEMVf);

/**
 *To register the callback function to get the MSR fallbaack processing response or the CTLS MSD processing response.
 */
void L2_registerMSRCallBk(pMSR_callBack pMSRf);

/**
* Callback Response to Menu Selection Request
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
int L2_callbackResponseMenu(IN int type, byte selection);

/**
 * Retrieve Transaction Results

 *
 Retrieves specified EMV tags from the currently executing transaction.

 * @param tags Tags to be retrieved.  Example 0x9F028A will retrieve tags 9F02 and 8A
 * @param tagsLen Length of tag list
 * @param cardData All requested tags returned as unencrypted, encrypted and masked TLV data in IDTTransactionData object

 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()

 */
int L2_retrieveTransactionResult(IN BYTE* tags, IN int tagsLen, IDTTransactionData * cardData);

/**
 * Retrieve CTLS Application Data by AID
 *
 Retrieves the CTLS Application Data as specified by the AID name passed as a parameter.

 * @param AID Name of ApplicationID. Must be between 5 and 16 bytes
 * @param AIDLen the length of AID data buffer.
 * @param tlv  The TLV elements of the requested AID
 * @param tlvLen the length of tlv data buffer.

 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()
 */
int L2_ctls_retrieveApplicationData(IN BYTE* AID, IN int AIDLen, OUT BYTE* tlv, IN_OUT int *tlvLen);

/**
 * Set CTLS Application Data by AID
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
int L2_ctls_setApplicationData(IN BYTE* tlv, IN int tlvLen);

/**
 * Remove CTLS Application Data by AID
 * Removes the Application Data for CTLS as specified by the AID name passed as a parameter

 * @param AID Name of ApplicationID Must be between 5 and 16 bytes

 * @param AIDLen the length of AID data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()
 */
int L2_ctls_removeApplicationData(IN BYTE* AID, IN int AIDLen);

/**
 * Removes All CTLS Application Data
*
Removes all the Application Data


* @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()

*/
int L2_ctls_removeAllApplicationData();

/**
 * Retrieve All CTLS AIDs groups/names
 *
 Returns all the AIDs on the terminal for CTLS.  An AID is defined by the group it is in and the AID name

 * @param AIDList  AIDS a repeating sequence,  <1 byte group><1 byte AID len><AID name>. . .
 * @param AIDListLen  the length of AIDList buffer

 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()

 */
int L2_ctls_retrieveAIDList(OUT BYTE * AIDList, IN_OUT int *AIDListLen);

/**
 * Retrieve CTLS Terminal Data
 *

 * Retrieves the Terminal Data for CTLS. This is configuration group 0 (Tag DFEE2D - > DFEE2D0100).  The terminal data
	can also be retrieved by L2_ctls_getConfigurationGroup(0).

 * @param tlv Response returned as a TLV
 * @param tlvLen the length of tlv data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()


 */
int L2_ctls_retrieveTerminalData(OUT BYTE* tlv, IN_OUT int *tlvLen);

/**
* Set CTLS Terminal Data
*
Sets the Terminal Data for CTLS as specified by the TLV.  The first TLV must be Configuration Group Number (Tag DFEE2D).	The terminal global data
	is group 0, so the first TLV would be DFEE2D0100.  Other groups can be defined using this method (1 or greater), and those can be
	retrieved with ctls_getConfigurationGroup(int group), and deleted with ctls_removeConfigurationGroup(int group).	You cannot
	delete group 0.

 * @param tlv TerminalData configuration file
 * @param tlvLen the length of tlv data buffer

 * @retval RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()

 */
int L2_ctls_setTerminalData(IN BYTE* tlv, IN int tlvLen);

/**
 * Retrieve CTLS Certificate Authority Public Key
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
int L2_ctls_retrieveCAPK(IN BYTE * capk, IN int capkLen, OUT BYTE * key, IN_OUT int *keyLen);

/**
 * Set CTLSCertificate Authority Public Key
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
int L2_ctls_setCAPK(IN BYTE * capk, IN int capkLen);

/**
 * Remove CTLS Certificate Authority Public Key
 *
 Removes the CAPK as specified by the RID/Index

 * @param capk 6 byte CAPK =  5 bytes RID + 1 byte INDEX
 * @param capkLen the length of capk data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()

 */
int L2_ctls_removeCAPK(IN BYTE * capk, IN int capkLen);

/**
* Remove All CTLS Certificate Authority Public Key
 *
 Removes all the CTLS CAPK

* @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()

*/
int L2_ctls_removeAllCAPK();

/**
 * Retrieve the CTLS Certificate Authority Public Key list
 *
 Returns all the CTLS CAPK RID and Index installed on the terminal.

 * @param keys [key1][key2]...[keyn], each key 6 bytes where
    key = 5 bytes RID + 1 byte index
 * @param keysLen the length of keys data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()
 */
int L2_ctls_retrieveCAPKList(OUT BYTE * keys, IN_OUT int *keysLen);

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
int L2_setConfigurationGroup(IN BYTE* tlv, IN int tlvLen);

 /**
 * Get Configuration Group
 *
 Retrieves the Configuration for the specified Group.

* @param group Configuration Group
* @param tlv return data

* @param tlvLen the length of tlv data buffer
* @return RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()

 */

int L2_getConfigurationGroup(IN int group, OUT BYTE* tlv, OUT int *tlvLen);

/**
Allow fallback for EMV transactions.  Default is TRUE
@param allow TRUE = allow fallback, FALSE = don't allow fallback
*/
void L2_allowFallback(IN int allow);

/**
* Retrieve All Configuration Groups
*
Returns all the Configuration Groups installed on the terminal for CTLS

* @param tlv  The TLV elements data
 * @param tlvLen the length of tlv data buffer.

* @return RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()

*/
int L2_getAllConfigurationGroups(OUT BYTE * tlv, IN_OUT int *tlvLen);

/**
* Remove Configuration Group
*
Removes the Configuration as specified by the Group.  Must not by group 0

 @param group Configuration Group

* @retval RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()

*/
int L2_removeConfigurationGroup( int group);

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
int L2_retrieveApplicationData(IN BYTE* AID, IN int AIDLen, OUT BYTE* tlv, IN_OUT int *tlvLen);

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
int L2_setApplicationData(IN BYTE* name, IN int nameLen, IN BYTE* tlv, IN int tlvLen);

/**
 * Set Application Data by TLV
 *
 * Sets the Application Data as specified by the TLV data
 * @param tlv  Application data in TLV format
 * @param tlvLen the length of tlv data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getResponseCodeString()
 */
int L2_setApplicationDataTLV(IN BYTE* tlv, IN int tlvLen);

/**
 * Remove Application Data by AID
 * Removes the Application Data for CTLS as specified by the AID name passed as a parameter

 * @param AID Name of ApplicationID Must be between 5 and 16 bytes

 * @param AIDLen the length of AID data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()
 */
int L2_removeApplicationData(IN BYTE* AID, IN int AIDLen);

/**
 * Remove All Application Data
*
Removes all the Application Data

* @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()
*/
int L2_removeAllApplicationData();

/**
 * Retrieve AID list
 *
 Returns all the AID names installed on the terminal for CTLS. .

 * @param AIDList  array of AID name byte arrays
 * @param AIDListLen  the length of AIDList array buffer

 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()

 */
int L2_retrieveAIDList(OUT BYTE * AIDList, IN_OUT int *AIDListLen);

/**
 * Retrieve Terminal Data
 *

 * Retrieves the Terminal Data for ICC EMV.

 * @param tlv Response returned as a TLV
 * @param tlvLen the length of tlv data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()
 */
int L2_retrieveTerminalData(OUT BYTE* tlv, IN_OUT int *tlvLen);

/**
* Set Terminal Data
*
Sets the Terminal Data for ICC EMV

 * @param tlv TerminalData TLV terminal data
 * @param tlvLen the length of tlv data buffer

 * @retval RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()

 */
int L2_setTerminalData(IN BYTE* tlv, IN int tlvLen);

/**
 * Retrieve Certificate Authority Public Key
 *

 Retrieves the CAPK for ICC EMV as specified by the RID/Index	passed as a parameter.

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
int L2_retrieveCAPK(IN BYTE * capk, IN int capkLen, OUT BYTE * key, IN_OUT int *keyLen);

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
int L2_setCAPK(IN BYTE * capk, IN int capkLen);

/**
 * Remove Certificate Authority Public Key
 *
 Removes the CAPK as specified by the RID/Index

 * @param capk 6 byte CAPK =  5 bytes RID + 1 byte INDEX
 * @param capkLen the length of capk data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()

 */
int L2_removeCAPK(IN BYTE * capk, IN int capkLen);

/**
* Remove All Certificate Authority Public Keys
 *
 Removes all the CAPKs

* @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()

*/
int L2_removeAllCAPK();

/**
 * Retrieve the Certificate Authority Public Key list
 *
 Returns all the CAPK RID and Index installed on the terminal.

 * @param keys [key1][key2]...[keyn], each key 6 bytes where
    key = 5 bytes RID + 1 byte index
 * @param keysLen the length of keys data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()
 */
int L2_retrieveCAPKList(OUT BYTE * keys, IN_OUT int *keysLen);



/**
 * Set Certificate Revocation List Entry
 *
 Sets the CRL into the list

 * @param crl CRL format:
    [5 bytes RID][1 byte Index][3 bytes Serial Number]
 * @param crlLen the length of crl data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()
 */
int L2_setCRLEntry(IN BYTE * crl, IN int crlLen);

/**
 * Remove a Certificate Revocation List Entry
 *
 Removes the CRL from the list

 * @param crl CRL format:
    [5 bytes RID][1 byte Index][3 bytes Serial Number]
 * @param crlLen the length of crl data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()
 */
int L2_removeCRLEntry(IN BYTE * crl, IN int crlLen);

/**
* Remove All CRL from the list
 *
 Removes all the CRLs

* @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()

*/
int L2_removeAllCRLEntries();

/**
 * Retrieve the Certificate revocation list
 *
 Returns all the CRL’s on the list.

 * @param keys [key1][key2]...[keyn], each key 9 bytes where
    key = 5 bytes RID + 1 byte index + 3 bytes serial number
 * @param keysLen the length of keys data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()
 */
int L2_retrieveCRL(OUT BYTE * keys, IN_OUT int *keysLen);

/**
 * Retrieve the Certificate revocation list info
 *
 Returns info about the on the list.

 * @param info  = 12 bytes:
	bytes 0-3 = Version Number
	bytes 4 – 7 = Number of Records
	bytes 8 – 11 = Size of Record
 * @param infoLen the length of info data buffer
 * @return RETURN_CODE:  Values can be parsed with device_getIDGStatusCodeString()
 */
int L2_retrieveCRLStatus(OUT BYTE * info, IN_OUT int *infoLen);

/**
 * Retrieve the EMV Exception List
 *
 Returns the EMV Exception entries on the terminal.
 @param exceptionList [Exception1][Exception2]...[Exceptionn], where  [Exception]  is 12 bytes:
    [1 byte Len][10 bytes PAN][1 byte Sequence Number]
 @param exceptionListLen The length of the exception list.

* @return RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()
 */
int L2_retrieveExceptionList(OUT BYTE *exceptionList, IN_OUT int *exceptionListLen);

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
int L2_setException(IN BYTE *exception, IN int exceptionLen);

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
int L2_removeException(IN BYTE *exception, IN int exceptionLen);

/**
 * Remove All EMV Exceptions
 *
 Removes all entries from the EMV Exception List

* @return RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()
 */
int L2_removeAllExceptions();

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
int L2_retrieveExceptionLogStatus(OUT BYTE *exceptionLogStatus, IN_OUT int *exceptionLogStatusLen);

/**
* Clear Transaction Log
*
Clears the transaction  log.

* @return RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()
*/
int L2_removeTransactionLog();

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
int L2_retrieveTransactionLogStatus(OUT BYTE *transactionLogStatus, IN_OUT int *transactionLogStatusLen);

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
int L2_retrieveTransactionLog(OUT BYTE *transactionLog, IN_OUT int *transactionLogLen, IN_OUT int *remainingTransactionLogLen);

/**
 * Polls device for EMV Kernel Version
 *
 * @param version Response returned of Kernel Version
 *
 * @return RETURN_CODE:  Values can be parsed with device_getErrorString()
 *
 */
int L2_getEMVKernelVersion(OUT char* version);

/**
* Get EMV Kernel check value info
*
* @param checkValue Response returned of Kernel check value info
* @param checkValueLen the length of checkValue
*
* @return RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()
*
*/
int L2_getEMVKernelCheckValue(OUT BYTE* checkValue, IN_OUT int *checkValueLen);

/**
 * Get EMV Kernel configuration check value info
 *
 * @param checkValue Response returned of Kernel configuration check value info
 * @param checkValueLen the length of checkValue
 * @return RETURN_CODE:	Values can be parsed with device_getIDGStatusCodeString()
 *
 */
int L2_getEMVConfigurationCheckValue(OUT BYTE* checkValue, IN_OUT int *checkValueLen);

#endif /* LIBIDT_L2_H_ */
