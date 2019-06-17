# IDTech C++ SDK Library Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).

## [1.0.16] - 2018-11-02
### Changed
- Merged the SDK source code with the customer's (Mackaymeters).
- Changed the SDK version to 1.0.16.

### Added
- Added device_getTransactionResults() for  all the .java files in JNI_Bridge.
- Added rs232_device_init() to libIDT_KioskIII.h, libIDT_MiniSmartII.h, libIDT_NEO2.h, and libIDT_SpectrumPro.h.

## [1.0.15] - 2018-09-19
### Changed
- Changed device_getKeyStatus() for L100.
- Changed the SDK version to 1.0.15.

## [1.0.14] - 2018-09-17
### Changed
- Changed the comment of the function device_getKeyStatus() for NEO 2 devices to generate the correct document.
- Updated JNI_Bridge with missing functions.
- Changed the SDK version to 1.0.14.

## [1.0.13] - 2018-09-06
### Added
- Added emv_setTerminalMajorConfiguration().

### Changed
- Ignore LCD messages for commands of NEO 2 devices under L100 pass through mode.
- Changed ctls_activateTransaction() to check the interface tag DFEF37.
- Disabled the PIN functions for L100 pass through mode of NEO 2 devices.
- Updated LCD messages.
- Updated the tags to check auto poll and burst mode.
- Changed the SDK version to 1.0.13.

## [1.0.12] - 2018-08-29
### Added
- Added the function device_getL100PassThroughMode().

### Changed
- Changed L100 related functions to support L100 passthrough mode.
- Changed emv_activateTransaction() to use the command 0x60-10 instead of 0x02-40 for NEO2 devices.
- Changed LCD message tables for 4 languages.
- Changed DoCMD_data_justRead() to support L100 passthrough mode.
- Changed the SDK version to 1.0.12.
- Increased the array lcd_messages[] to 4096 bytes.

## [1.0.11] - 2018-08-08
### Added
- Added PIN functions to support L100.
- Added L100 demo.

### Changed
- Implementing the support for L100 passthrough mode (not finished yet).
- Changed threadProc_Read_CTLS() to not send the command 0x03-03 after EMV authenticate for VP8800.
- Changed ctls_startTransaction() and ctls_activateTransaction() to use the timeout value passed.
- Changed ctls_retrieveApplicationData() to not pass the tag DFEE4F for NEO 2 devices.
- Changed lcd_clearDisplay() to support L100.
- Changed parseEnhancedMSRData() for VP8800 with the new tags DFEF17 and DFEF18.
- Changed DoNGACmdUsb() to support L100 passthrough mode.
- Changed IDTPINData to support L100.

## [1.0.10] - 2018-07-27
### Added
- Added the tag DFEF37 for ctls_startTransaction() for NEO2 devices.

### Changed
- Changed processReceivedData() to parse the response data of EMV start/authenticate transaction for VP8800 correctly.
- Changed EMV_startTransaction() to set the correct tag value for the tag DFEF37 for VP3310.
- Changed threadProc_Read_MSR() to return error code for MSR transaction.
- Changed the device name checking string "FW Update" to "FW_UPDATE" in device_setCurrentDevice().
- Changed the version to 1.0.10.

## [1.0.9] - 2018-07-20
### Added
- Added the callback function for firmware update.
- Added the function device_registerFWCallBk().
- Added the function to write the log to a file instead of output to screen.
- Added NEO 2 FW Update in NEO2_Devices.xml.

### Changed
- Changed device_updateFirmware() for NEO 2 devices.
- Changed version to 1.0.9.

## [1.0.8] - 2018-07-16
### Changed
- Changed pin_getEncryptedOnlinePIN() to store correct KSN data.
- Changed SDK version to 1.0.8.

## [1.0.7] - 2018-07-05
### Added
- Added support for Kiosk IV by USB, and also added support for Kiosk IV and Kiosk III S by RS232.

### Changed
- Changed the DoIDGCMD_str() to set the data size to 0 when the return code is not IDG_P2_STATUS_CODE_DO_SUCCESS.
- Changed DoIDGCmdRS232() and ReadIDGRS232() to return the correct error code.
- Changed the SDK version to 1.0.7.
- Changed libIDT_KioskIII.h to replace msr_registerCallBk() and msr_registerCallBkp() with ctls_registerCallBk() and ctls_registerCallBkp().

### Removed
- Removed .$(VERSION) for LDFLAGS in the Makefiles.

## [1.0.6] - 2018-06-22
### Added
- Added device_enableExternalLCDMessages().
- Added device_setNEO2DevicesConfigs().

### Changed
- Increased ram size of NEO2 to 0x400000 bytes.
- Changed all the NEO2 functions to handle external LCD messages.
- Increased the MSR, CTLS, and EMV card data size to 4K.
- Changed EMV, CTLS, MSR, and device cancel transactions to wait for 150 ms.
- Changed device_setBurstMode() for NEO devices to use the new tag 0xDFEE7E for command data. 
- Changed lcd functions to support only VP8800.
- Changed processReceivedData() to parse CTLS data correctly.
- Changed config_getModelNumber() to support NEO2 devices.
- Changed the SDK version to 1.0.6.
- Changed device_init() to handle both xml file and xml data.

## [1.0.5] - 2018-06-13
### Changed
- Changed device_startTransaction() for EMV transaction for VP8800.
- Added device_configureButtons(), device_getButtonConfiguration(), device_disableBlueLED(), device_enableBlueLED(), device_lcdDisplayClear(), device_turnOffYellowLED(), device_turnOnYellowLED(), device_buzzerOnOff(), device_lcdDisplayLine1Message(), and device_lcdDisplayLine2Message().
- Added emv_getEMVKernelVersion(), emv_getEMVKernelCheckValue(), and emv_getEMVConfigurationCheckValue() in libIDT_NEO2.h.
- C++ SDK update makefiles to remove VP5300 demo folder
- Changed device_getKeyStatus() to support NEO devices.
- Changed threadProc_Read_EMV() for RS232 read timeout.
- Changed device_setCurrentDevice() to handle redundant LCD messages.
- Changed parserEMVCallback() to public.
- Added the function device_setConfigPath().
- Removed the MAJOR version from Makefile.
- Changed threadProc_Read_MSR() to return correct error code.

## [1.0.4] - 2018-06-06
### Changed
- Changed emv_activateTransaction to not require amount/type
- Changed ctls_activateTransaction to not require amount/type
- Changed device_activateTransaction to not require amount/type

## [1.0.3] - 2018-056-05
### Added
- Added emv_getEMVKernelVersion()
- Added emv_getEMVKernelCheckValue()
- Added emv_getEMVConfigurationCheckValue() in libIDT_NEO2.h.
- Added emv_retrieveTransactionResult() in libIDT_NEO2.h.
- Added emv_activateTransaction
- Added ctls_activateTransaction
- Added device_activateTransaction

### Changed
- C++ SDK update makefiles to remove VP5300 demo folder

## [1.0.2] - 2018-05-24
### Changed
- Changed the version to 1.0.2.
- Updated makefiles and NEO2 header
- Added NEO2_Devices.xml, update libIDT_NEO2.h
- Updated pipelines output file with branch name
- Moved version number to VERSION file
- comm library - add HEAD request for comm_httpRequest
- Added get CAPK for configMgr
- Modularized configMgr functions
- Added config meta and template functions for configmgr
- Added config aids for configmgr
- Added get terminal data and crl for configMgr
- Get NEO2 device name by the tag Product instead of Name.
- Added emv_setApplicationDataTLV()
- Added comm_setCAPath() and HEAD request for http request. Fixed the call back function HTTPRequestWriteCallback() for http async requests.
- Added emv_setApplicationDataTLV()
- Changed ctls_getAllConfigurationGroups() for NEO2 devices
- Changed device_startTransaction() to check timeout value for NEO2 devices

### Removed
- Removed AC100 makefiles
- Removed get version functions

## [1.0.1] - 2018-05-01
### Changed
- Changed the version to 1.0.1.
- Changed threadProc_Read_EMV() to handle MSR fallback data size larger than 512 bytes.

## [1.0.0] - 2018-04-27
### Changed
- Changed threadProc_Read_MSR() to handle more LCD messages.
- Changed threadProc_Read_EMV() to handle LCD messages and return correct error code for timeout.  Changed emv_cancelTransaction() to handle more LCD messages for VP5300.  Changed emv_retrieveTransactionResult() to get correct tlv data.
- Changed device_cancelTransaction() to call emv_cancelTransaction() for NEO2 devices.
- Changed threadProc_Read_CTLS() to handle LCD messages.
- Merge all original libraries into one shared library
- Proper Semantic Versioning
- Refactor makefiles for autobuilding

### Removed
- ADF Libraries from main SDK
- ADF Library header symbols

## [0.02.032.001] - 04/25/2018
- Changed emv_cancelTransaction() to handle LCD messages.
- Changed device_setPollMode() to handle LCD message.
- Changed the version to 0.02.032.001.
- Changed the comment for device_startTransaction().
- Changed processReceivedData() for VP8800 testing.
- Changed threadProc_Read_EMV() to handle the returned data for command 0x02-40 for NEO2 devices.  Changed emv_startTransaction() to handle the call from device_startTransaction() for NEO2 devices.
- Changed device_startTransaction() and device_getFirmwareVersion() for NEO2 devices.  Changed threadProc_AutoPoll_Read() to handle LCD messages for VP5300.

## [0.02.031.001] - 04/20/2018
- Changed ReadIDGUsb() to ignore LCD messages.
- Changed version to 0.02.031.001.  Added device_getDevicePid().
- Added more LCD message strings for each language.
- Changed threadProc_Read_MSR() to handle LCD messages.
- Changed threadProc_Read_EMV() to handle LCD messages.  Changed emv_startTransaction() to handle duplicate - tags.
- Changed device_controlUserInterface() to handle NULL parameter.
- Changed threadProc_Read_CTLS() to handle LCD messages.
- Added MSR_callBack_type_ERR_CODE in MSR_callBack_type.  Added errorCode in IDTMSRData.
- Checked NULL parameter for setAbsoluteLibraryPath().  Increased the buffer size of processTLV().
- Changed getAIDTags() to get the correct tag count.

## [0.02.030.001] - 04/11/2018
- Changed/Removed mssleep() functions to reduce read/write time.
- Removed device_selfCheck().  Changed emv_autoAuthenticate() to emv_setAutoAuthenticateTransaction().  Added emv_setAutoCompleteTransaction(), emv_getAutoAuthenticateTransaction(), and emv_getAutoCompleteTransaction().
- Reverted libIDT_LogMgr.h.
- Reverted libIDT_Log.h.
- Added #include <sqlite3.h> and #include <string.h>.
- Changed emv_autoAuthenticate() to emv_setAutoAuthenticateTransaction().  Added emv_setAutoCompleteTransaction(), emv_getAutoAuthenticateTransaction(), and emv_getAutoCompleteTransaction().
- Reverted log messages.  Changed parseNGAMSRFormat() for Augusta and MiniSmartII.
- Added emv_setAutoCompleteTransaction(), emv_getAutoAuthenticateTransaction(), and emv_getAutoCompleteTransaction().
- Added "CAPTURE_ENCODE_TYPE_Unknown" in CAPTURE_ENCODE_TYPES.  Added Java_IDTechSDK_IDTechSDKBridge_jemvsetAutoCompleteTransaction(), Java_IDTechSDK_IDTechSDKBridge_jemvgetAutoAuthenticateTransaction(), and Java_IDTechSDK_IDTechSDKBridge_jemvgetAutoCompleteTransaction().
- Added threadComplete() to handle auto EMV complete transaction.
- Changed the buffer to read MSR card data in threadProc_Read_MSR() to 2K.  Check the flag AutoPollOn for most of the functions.
- Reverted IDTech_logMgr.c.
- Fixed emv_setCRL() for NEO2 devices.  Check the flag AutoPollOn for most of the functions.  Added emv_setAutoCompleteTransaction(), emv_getAutoAuthenticateTransaction(), and emv_getAutoCompleteTransaction().
- Removed #include <sqlite3.h>.

## 04/10/2018
- Check the flag AutoPollOn in most of the functions..
- Added CAPTURE_ENCODE_TYPE_Unknown for CAPTURE_ENCODE_TYPE.  Reverted the enum LOG_LEVEL. Added mac and macKSN for IDTMSRData.
- Changed parseMSRData().
- Changed back to old log functions.
- Added .fp_emv_setAutoAuthenticate, .fp_emv_setAutoComplete, .fp_emv_getAutoAuthenticate, and .fp_emv_getAutoComplete in idtech_sdk_api_str.
- C++ SDK fix includes for devMgr and logMgr

## 03/23/2018
- C++ SDK config manager updates
- Implement remaining log manager functions.
- Update log_rawCommData() and log_raw() functions to accept raw binary data.
- Add ADF module function declarations for testing suite.
- Rename RAW_COMMUNICATION_DATA to DEBUG.
- Update IDT_LOG_MESSAGE() and rename RAW_COMMUNICATION_DATA to DEBUG.
- Update LOG_LEVEL enumeration and add logging modules response codes.
- Add logging manager pass-through functions.
- Removed VP5300 Pid.  Added isNEO2Pid().
- Changed get version functions to pass a string for storing the version.
- Changed device_init() to support NEO2 devices.
- Removed unsupported functions.
- Changed ws_getVersion() to return error for AC100.
- Changed msr_getVersion() to return error for AC100.  Changed VP5300 to NEO2.
- Changed lcd_getVersion() to return error for AC100.
- Changed icc_getVersion() to return error for AC100.
- Changed emv_getVersion() to return error for AC100.
- Changed device_getVersion() to return error for AC100.
- Changed ctls_getVersion() to return error for AC100.
- Changed config_getVersion() to return error for AC100.  Check NULL pointer for the functions.
- Added IDT_DEVICE_NEO2 to DEVICE_TYPE. Increased the size of value in IDTTag.  Added the variable name in IDTDeviceHandle.
- Changed get version functions to pass a string for storing the version.
- Reverted back to old log functions until new functions are done.
- Changed the version to 0l.02.030.001, and changed VP5300 to NEO2.

## 03/20/2018
- Added NEO2 devices.

## 03/19/2018
- Added AC100 Single File

## 03/08/2018
- Add device manager pass-through functions.
- Implement device manager API.
- Add device manager function declarations.
- Add RESOURCE_TYPE enum and add more response codes for the device manager module.
- Update device manager function documentation and add devMgr_removeCallerAndReleaseClaims().
- Copy devMgr headers to corresponding directories.
- Add log callback function declaration.

## 03/07/2018
- C++ SDK header updates for Security and ResMgr
- C++ SDK implement Resource Manager Library

## 03/05/2018
- C++ SDK security library bug fixes
- C++ SDK fix compile issues for Mac, Pisces, and Neo2

## 03/02/2018
- Updated MAC code to bypass mutex lock/unlock on libIDT_Device.c
- Fixed callback parameter for logging
- Excluded r_pthread.c from Windows makefile and Mac makefile

## 03/01/2018
- C++ SDK add Security Library
- NEO II Updates

## 02/26/2018
- Updated pthread functions for NEO II
- Added NEO II Methods
- Add preliminary device manager and resource manager function declarations.
- Add logging manager module function declarations and definitions (in progress).
- Add logging module function declarations.
- Reverse LOG_LEVEL enum and add module specific response enums.
- Add preliminary communication module function declarations.
- Add logging â€œpass-throughâ€� functions.
- Remove deprecated logging functions.
- Add logging function declarations.
- Fix FTP download progress error and add preprocessor directives for handling certificates on Windows.
- Check for both VP8800 and NEO2 preprocessor directives.
- Copy log and logMgr header files accordingly when building for VP8800.
- Reverted back to 6318
- Updated emv_completeTransaction to not check for parameters if commError = 1.
- Checkpoint upload.  Converting files to also work with NEO II.  Reverting multiple libraries into a single IDTechSDK for NEO II.

## 02/15/2018
- Adding ARM-M Support.  Adding NEO 2 headers

## 02/13/2018
- Add httpComm_callBack, v4Comm_callBack function pointer declarations and comm_registerHTTPCallback(), comm_registerV4Callback() function declarations to device headers.
- Add pass-through functions for comm_registerHTTPCallback() and comm_registerV4Callback().
- Implement async option for comm_httpRequest().
- Add communication library result code (COMM_ASYNC_FAIL) for when creating a new thread fails.
- Removed EXPORT_SYMBOLS from comm_uploadFTP().
- Changed the version to 0.02.029.001.
- Changed threadProc_Read_MSR() to check DPAS AID and the tag 9F71 when the return code of msr transaction is 0x0A to return MSR_callBack_type_FALLBACK_TO_CONTACT.
- Changed emv_retrieveAIDList() to check the parameter AIDListLen passed in for VP3300.
- Changed threadProc_Read_CTLS() to check DPAS AID and the tag 9F71 when the return code of ctls transaction is 0x0A to return MSR_callBack_type_FALLBACK_TO_CONTACT.
- Added MSR_callBack_type_FALLBACK_TO_CONTACT to enum MSR_callBack_type.

## 02/08/2018
- Add comm_httpRequest() declaration to device headers.

## 02/07/2018
- Add certificate option for comm_httpRequest() for Windows platform.
- Changed the SDK version to 0.02.028.001.
- Changed emv_completeTransaction() to handle NULL iad.
- Add new communication library return codes.
- Update comm_httpRequest() return codes.

## 02/02/2018
- C++ SDK bug fix emv_removeAllApplicationData() for VP8800

## 01/30/2018
- Add comm_downloadFTP() and comm_uploadFTP() declarations to header files.
- Update comm_downloadFTP() and comm_uploadFTP() functions with new result codes and add CAINFO option for Windows.
- Add result codes for communication library.
- C++ SDK bug fix: add static to private functions

## 01/29/2018
- C++ SDK bug fix header files

## 01/26/2018
- Added ftpComm_callBack.
- Added FTP upload, FTP download, and http request.
- Implemented FTP upload and download.
- Added P2 status code.
- Remove unused function.

## 01/25/2018
- C++ SDK update headers with L2 library
- Copy communication header to demo output.
- Add comm_sendEmail() declaration to header files.
- Implement comm_sendEmail().

## 01/24/2018
- C++ SDK tlv comment updates and bug fixes
- Add communication library result code enum.
- Add compiler _WIN32 definition to satisfy test suite.
- Remove obsolete filter out statement.

## 01/10/2018
- Added IDTech_comm.c and IDTech_comm.h.
- Added comm_registerHTTPCallback(), comm_registerV4Callback(), and comm_downloadFTP().
- Removed unused variables.
- C++ SDK add Initial Security Library
- Changed IDT_LOG_MESSAGE().
- Added httpComm_callBack and v4Comm_callBack.
- Added httpComm_callBack s_pComm_f and v4Comm_callBack s_pv4Comm_f.
- Changed IDT_LOG_MESSAGE().

## 01/09/2018
- C++ SDK add L2 functions

## 01/08/2018
- Update include statements.
- C++ SDK update library headers
- Update iso8583_derserializeFromXML to return RETURN_CODE_DO_SUCCESS when successful.

## 01/05/2018
- Changed _idtech_crypto_api_t to solve compile errors.
- Changed _idtech_sdk_api_t_ to solve compile errors.
- Changed void tlv_processIDTTLV() to int tlv_processIDTTLV().
- #define EXPORT_SYMBOLS extern for non-Windows platforms.
- C++ SDK Crypto Library bug fix
- C++ SDK Compression library update makefiles
- Fix incorrect assignment of field definition.
- Remove const modifier from ISO8583 structures.
- C++ SDK add Compression library and support for Pisces

## 01/04/2018
- Fixed the extern issue.

## 01/03/2018
- Fixed tlv_sortTLV().
- Changed device_transferFile() to limit the file size to 461000 bytes.
- Changed the parameters of Java_IDTechSDK_IDTechSDKBridge_jpinpromptCreditDebit().
- Added crypto_getVersion() and tlv_validateTLV().  Changed tlv functions to return an error code.
- Changed a_tlv_processIDTTLV, a_tlv_sortTLV, and a_tlv_getTagValue to return an error code.
- Added device_enhancedPassthrough() and lcd_cancelSlideShow().  Changed the tlv functions to return error code.
- Added DoIDGP3CMD_str().  Changed parseEnhancedMSRData() to assign isCTLS to cardData.
- Changed pin_promptCreditDebit() for VP8800.
- Changed processReceivedData() for VP8800.
- Added an internal function msr_readMSRData().
- Modified lcd_startSlideShow().  Added lcd_cancelSlideShow(), lcd_createInputField(), and lcd_getInputFieldValue().
- Changed icc_getICCReaderStatus() for VP8800.
- Changed emv_startTransaction() and emv_completeTransaction() to handle PIN entry and signature for VP8800.  Changed emv_completeTransaction() to have the iadLen between 8 and 16 bytes.
- Changed threadProc_Read_Device() to handle PIN entry and signature for VP8800.  Added device_enhancedPassthrough().
- Added crypto_getLibraryVersion().
- Added RETURN_CODE_EMV_PIN and RETURN_CODE_EMV_SIGNATURE to RETURN_CODE.
- Changed tlv_sortTLV(), tlv_getTagValue(), tlv_removeTLV(), tlv_countTLV(), tlv_validateTLV(), and pin_promptCreditDebit(). Added device_enhancedPassthrough(), lcd_cancelSlideShow(), lcd_createInputField(), lcd_getInputFieldValue(), and crypto_getVersion().

## 12/26/2017
- C++ SDK update Crypto Library to include DES

## 12/21/2017
- C++ SDK bug fixes
- Add ISO8583 function declarations to device headers.
- Add iso8583_get1987Handler(), iso8583_get1993Handler(), iso8583_get2003Handler(), iso8583_getField(), iso8583_initializeMessage(), iso8583_getMessageField(), iso8583_setMessageField(), iso8583_removeMessageField(), iso8583_packMessage(), iso8583_unpackMessage(), iso8583_freeMessage(), iso8583_serializeToXML(), iso8583_deserializeFromXML(), and iso8583_displayMessage() functions.
- Add public ISO8583 structures.
- Add external ISO8583 library source files.
- Update Makefiles to ignore ISO8583 source files.

## 12/20/2017
- Update C++ Crypto Library to handle PEM, DER, PKCS7, and PKCS12 RSA Certificates

## 12/14/2017
- Update C++ Crypto Library with Cipher settings, RSA, and HMAC

## 12/11/2017
- Add NULL check to setMerchantRecord().

## 11/21/2017
- Added NEO2 flag for CONFIG, CRYPTO, and TLV.

## 11/17/2017
- Add support for NEOII to the crypto library (work in progress).
- Add support for NEOII to the configuration library (work in progress).
- Adding ARM-M Support
- Fixed a typo in the comment.
- Removed the â€˜,â€™ of the last element of enum.

## 11/16/2017
- Adding ARM-M Support

## 11/13/2017
- Changed IDTEnableLogging() to IDT_EnableLogging(), and added the function IDT_DisableLogging().
- Added the flag logEnabled.

## 11/10/2017
- Changed the version to 0.02.027.001.
- Use new function IDT_LOG_MESSAGE() instead of DoPrintF(), libpos_log_byte_array(), and libpos_log() for logging.
- Added enum LOG_LEVEL, COMM_TYPE, and FORMAT_TYPE.
- Implemented IDT_LOG_MESSAGE() and modified IDTLog() for logging.

## 11/06/2017
- Update header files to include IDTDef.h
- Update header files to include TLV and Crypto functions

## 11/01/2017
- Free symbol after usage.
- Add raw communication data and function entry / exit logging functions.
- Add constants relevant to logging.
- Add raw communication data logging.
- Add imagehlp library to linker options.

## 10/27/2017
- Changed device_getRTCDateTime() to support more tags in response data.

## 10/26/2017
- Changed the SDK version to 0.02.026.001.
- Added DoIDGP1CMD_str().  Changed parseEMVFormat1() to support enhanced MSR data.
- Changed processReceivedData() to support enhanced MSR data.
- Removed printData(), resetMSRData(), and parseMSRData().
- Changed threadProc_Read_EMV(), emv_startTransaction(), emv_authenticateTransaction(), emv_completeTransaction(), and emv_cancelTransaction() to reset the timeout value.
- Added device_getRTCDateTime() and device_setRTCDateTime().  Changed parseMSRData() for new enhanced MSR data.
- Added grsiP1Command() and grsiP3Command().

## 10/24/2017
- Move tlv and crypto functions into their own shared library.

## 10/20/2017
- Changed tlvCompare().
- Changed IDTCryptoData.

## 10/19/2017
- Updated Makefiles to show warnings. Fixed some warnings.

## 10/17/2017
- Added IDTCryptoData, getIPEK(), getDerivedKey(), getDataKey(), getPINKey(), getMACKey(), encryptTDES(), and encryptAES(), decryptTDES(), decryptAES(), encryptPinTDES(), encryptPinAES(), decryptPinTDES(), decryptPinAES(), processDUKPT(). Fixed a bug with convertBytesToHexString(). Updated Makefiles for OpenSSL.

## 10/11/2017
- Removed EXPORT_SYMBOLS for lcd_getInputEvent().
- Changed the comments to fix some Doxygen issue.

## 10/10/2017
- Changed version to 0.02.025.001.
- Changed rs232_timeout to be extern.
- Changed the comment for the documentation.
- Changed the timeout for ReadIDGRS232().

## 10/09/2017
- Fixed compiler error for ARM.
- Changed ReadIDGRS232() to return IDG_P2_STATUS_CODE_TIMEOUT for timeout.
- Changed emv_setApplicationData().
- Changed processReceivedData() to support VP5300.
- Added VP5300 and changed the version to 0.02.024.001.

## 10/06/2017
- Added processIDTTLV(), sortTLV(), getTagValue(), addTLV(), removeTLV(), countTLV(), and convertTagArray().

## 09/19/2017
- Add soname option when generating library for Java for x86_64.

## 09/18/2017
- Add static linker flag for Windows_Java Makefile to bundle functions at compile time.

## 09/15/2017
- Changed ReadDataUsb() to support auto poll and burst mode on for Mac.  Changed InitUsbLib() and CloseUsb() to fix open and close USB issues for Mac.

## 09/14/2017
- Exported the function convertNumberToASCII().
- Changed ReadDataUsb() to use hidlib instead of usblib for VP3300.
- Added emv_setTransactionParameters().
- Changed the version to 0.02.023.001 and added EMV parameters.
- Added jemvsetTransactionParameters().
- Changed device_startTransaction() to support EMV transaction even when auto poll is on.  Changed threadProc_AutoPoll_Read(), device_setBurstMode(), and device_setPollMode() to support 3-interface transactions when auto poll is on for VP3300.  Changed device_getTransactionResults() to handle error codes 0x23, 0x30, and 0x31.
- Added CVN17 card data parsing in processCTLSData().

## 08/31/2017
- Fix assignment of checKValueLen in emv_getEMVKernelCheckValue function.
- Add convertNumberToASCII function
- Update _CMD_DATA_MAX_LEN to 4096
- Add lcd_displayText, lcd_displayParagraph, lcd_displayButton, lcd_createList, lcd_addItemToList, lcd_clearEventQueue, and lcd_getInputEvent functions.

## 08/30/2017
- Added Vendi and changed the version to 0.02.022.001.
- Changed all IDG devices to use hid library instead of libusb.
- Added device_startTransaction() and device_cancelTransaction().
- Changed device_close() to prevent the application from crashing.

## 08/08/2017
- Update Makefiles (for Java bridge) to export JARs into corresponding folders automatically.

## 08/07/2017
- Adding Pisces Support Files

## 08/01/2017
- Add JNI C functions:
- - device_startTransaction(), device_cancelTransaction(), device_controlUserInterface(), device_controlIndicator(), device_calibrateParameters(), device_getDriveFreeSpace(), device_listDirectory(), device_createDirectory(), device_deleteDirectory(), device_transferFile(), and device_deleteFile()
- - ctls_displayOnlineAuthResult()
- - emv_retrieveExceptionList(), emv_setException(), emv_removeException(), emv_removeAllExceptions(), emv_retrieveExceptionLogStatus(), emv_removeTransactionLog(), emv_retrieveTransactionLogStatus(), and emv_retrieveTransactionLog()
- - lcd_resetInitialState(), lcd_customDisplayMode(), lcd_setForeBackColor(), lcd_clearDisplay(), lcd_captureSignature(), lcd_startSlideShow(), lcd_setDisplayImage(), and lcd_setBackgroundImage()
- - pin_getEncryptedOnlinePIN(), pin_getPAN(), and pin_promptCreditDebit()
- Update Makefile for ARM_Java to link with libusb-1.0.

## 07/31/2017
- Changed the version to 0.02.021.001.
- Changed VP4880 to VP3300_USB, UniPayIII to VP3300_AJ, and VP3300 to VP3300_BT.
- Changed VP3300_AJ, VP3300_USB, and VP3300_BT to use libhid instead of libusb.
- Added device_setCurrentDevice().
- Changed the comment of device_setCurrentDevice().
- Moved the PID definition to poscom_usb.h.  Changed device types IDT_DEVICE_UNIPAY_III to IDT_DEVICE_VP3300_AJ, IDT_DEVICE_VP4880 to IDT_DEVICE_VP3300_USB, and IDT_DEVICE_VP3300 to IDT_DEVICE_VP3300_BT.
- Added the check for the flag isMSRBusy in functions.
- Added the function emv_clearBuffer().
- Changed threadProc_Read_Device() to support device_startTransaction().  Added more flags in device_cancelTransaction() to stop the thread threadProc_Read_Device().
- Changed VP4880 to VP3300_USB, UniPayIII to VP3300_AJ, and VP3300 to VP3300_BT.

## 07/27/2017
- Changed the value of IDG_P2_STATUS_CODE_CARD_INSERTED to 0xA3.

## 07/18/2017
- Release SDK version 0.02.020.001.
- Changed TLV_ParseTags() back to not ignore the last 5 bytes.
- Added IDG_P2_STATUS_CODE_ICC_CARD_INSERTED.
- Modified threadProc_Read_Device() to support device_startTransaction() for VP3300, VP4880, and UniPay III.

## 07/14/2017
- Add new EMV functions for the VP8800.
- Fix issue with incorrect storage of device handles when using HIDAPI.

## 07/13/2017
- Removed TLV_getTags() from libIDT_Device_shared.h.
- Added TLV_getTags() to libIDT_Device.h.
- Added device_sendVivoCommandP2_ext() and device_sendVivoCommandP3_ext().
- Added grsiP3Command().

## 07/12/2017
- Added IDTech_lcd.c and IDTech_ws.c.
- Added the int parameter deviceType to CloseUsb().
- Changed OpenUsb() and CloseUsb() to support VP8800.
- Added lcd_resetInitialState(), lcd_customDisplayMode(), lcd_setForeBackColor(), lcd_clearDisplay(), lcd_captureSignature(), lcd_startSlideShow(), lcd_setDisplayImage(), pin_getEncryptedOnlinePIN(), pin_getPAN(), pin_promptCreditDebit(), ws_requestCSR(), ws_loadSSLCert(), ws_revokeSSLCert(), ws_deleteSSLCert(), ws_getCertChainType(), and ws_updateRootCertificate().
- Changed the second parameter of pPIN_callBack() from IDTPINData to IDTPINData *.
- Added parsePINBlockData().
- Added lcd_resetInitialState(), lcd_customDisplayMode(), lcd_setForeBackColor(), lcd_clearDisplay(), lcd_captureSignature(), lcd_startSlideShow(), lcd_setDisplayImage(), lcd_setBackgroundImage(), pin_getEncryptedOnlinePIN(), pin_getPAN(), pin_promptCreditDebit(), ws_requestCSR(), ws_loadSSLCert(), ws_revokeSSLCert(), ws_deleteSSLCert(), ws_getCertChainType(), ws_updateRootCertificate(), and parsePINBlockData().
- Increased the buffer size of the return data for DoIDGCMD_str() from 1K to 4K.  Changed device_init() and device_close() to fix the crash issue when connecting VP8800 and other devices by USB.
- Added WS source file for VP8800.
- Added pin_getEncryptedOnlinePIN(), pin_getPAN(), pin_promptCreditDebit(), and parsePINBlockData().  Changed parsePINData().
- Changed processReceivedData() to fix the issue of no data for MSR fall back during EMV transaction for VP4880.
- Added the LCD source file.
- Added one more check in device_listDirectory().
- Changed ctls_setCAPK(), ctls_retrieveCAPKRIDList(), ctls_retrieveCAPKRIDIndexes(), ctls_removeCAPK(), ctls_removeAllCAPK(), and ctls_getConfigurationGroup() to support VP8800.
- Changed the structure IDTPINData and added the structure RequestCSR to support VP8800.
- Added LCD, PIN, and WS functions.

## 07/07/2017
- Add emv_removeTransactionLog, emv_retrieveTransactionLogStatus, and emv_retrieveTransactionLog functions for the VP8800.

## 07/06/2017
- Add emv_setException, emv_removeException, emv_removeAllExceptions, and emv_retrieveExceptionLogStatus functions for the VP8800.
- Update emv_retrieveCRL, emv_removeCRL, emv_setCRL, emv_removeAllCRL, and emv_retrieveCRLStatus functions for the VP8800.

## 07/05/2017
- Add emv_retrieveCRLStatus and emv_retrieveExceptionList functions for the VP8800.
- Update ctls_getConfigurationGroup and ctls_removeConfigurationGroup functions for the VP8800.
- Add ctls_resetConfigurationGroup function for the VP8800.
- Update emv_retrieveCAPK, emv_setCAPK, emv_removeCAPK, emv_removeAllCAPK, and emv_retrieveCAPKList functions for the VP8800.
- Update linker flag for ARM to point to libusb.
- Added ctls_displayOnlineAuthResult().
- Changed device_getDriveFreeSpace() to fix a compile error.
- Changed ctls_retrieveApplicationData() and ctls_retrieveCAPK(), and added ctls_displayOnlineAuthResult() to support VP8800.
- Added isPINBusy and isMSRBusy;

## 07/03/2017
- Update emv_setApplicationData, ctls_getConfigurationGroup, ctls_removeConfigurationGroup, emv_retrieveCAPK, and emv_removeCAPK functions for the VP8800.
- Add ctls_resetConfigurationGroup function for the VP8800.
- Update USB communication to use HIDAPI for the VP8800.
- Update EMV AID functions for the VP8800.
- Changed the comment of ReadIDGRS232().
- Added device_getDriveFreeSpace(), device_listDirectory(), device_createDirectory(), device_deleteDirectory(), device_transferFile(), device_deleteFile(), device_controlIndicator(), device_calibrateParameters(), and msr_flushTrackData().  Removed device_setBurstMode() and device_setPollMode().
- Changed convertBytesToHexString(), DoNGACMD_str(), DoITPCMD_str(), DoIDGCMD_str(), DoDirectIDGCMD_str(), and DoCMD_data_justSend() to increase the command size to 4K.  Changed device_getIDGStatusCodeString() to support error code 0x98 and 0x99.
- Added msr_flushTrackData().
- Changed emv_retrieveAIDList() to support VP8800.
- Changed the return code of device_startTransaction() and device_cancelTransaction().  Changed device_SendDataCommand() to increase the command sizde to 4K.
- Added the error code IDG_P2_STATUS_CODE_NO_DRIVE_SPACE.
- Added the function getAidStringsCheckContact().
- Changed grsiP2Command(0 to increase the command size to 4K.  Added the function getAidStringsCheckContact().

## 06/30/2017
- Add HIDAPI source for Windows and Linux.
- Update USB communication to use HIDAPI for the VP8800.
- Update timeout value when starting an EMV transaction.
- Update emv_retrieveApplicationData and emv_removeApplicationData functions for the VP8800.
- Add error handling for error 0x36 (No Advice or Reversal Required (Declined)).

## 06/29/2017
- Changed WriteUsb() for VP8800.
- Added device_startTransaction() and device_cancelTransaction().
- Added VP8800 for parseEnhancedMSRData().
- Changed processReceivedData() for VP8800.
- Changed emv_startTransaction(), emv_authenticateTransaction(), and emv_completeTransaction() for VP8800.
- Added threadProc_Read_Device(), device_startTransaction(), and device_cancelTransaction().
- Removed the duplicated processTLV().  Changed threadProc_Read_CTLS() and ctls_startTransaction() for VP8800.

## 06/26/2017
- Changed BTPAY_MINI and PISCES to VP3300 and VP8800.
- Changed Pisces to VP8800.
- Changed BTPayMini to VP3300.
- Changed The comment of emv_setTerminalData().

## 06/21/2017
- Added Pisces and changed the version to 0.02.020.001.
- Implemented ReadIDGRS232().
- Added Pisces in emv_retrieveTerminalData().
- Added Pisces in DEVICE_TYPE.
- Added Pisces in getCurrentDeviceName().
- Changed the CRC order in grsiP2Command().

## 06/16/2017
- Added BTPayMini in getCurrentDeviceName().
- Add BTPay Mini support to Java bridge.

## 06/15/2017
- Changed _FIRMWARE_BUF_LEN from 1500000 to 1700000.

## 06/14/2017
- Changed the version to 0.02.019.001.

## 06/09/2017
- Update modifiers for shared variables and resolve issue with emv_canceled variable error for Mac OS.
- Added device_getTransactionResults().
- Changed device_setBurstMode() to only support mode 0 and mode 1.
- Changed SHARED_SYMBOLS to extern.
- Added test code for IDG devices.
- Added EXPORT_SYMBOLS to shared variables.
- Added a check to prevent processReceivedData() from crash.
- Added the flag isEMVCancelCmd to solve emv_cancelTransaction() block issue under Windows.
- Changed device_SendDataCommand() to solve emv_cancelTransaction() block issue under Windows.
- Changed EXPORT_SYMBOLS and SHARED_SYMBOLS.

## 06/06/2017
- Changed the version to 0.02.019.
- Removed EXPORT_SYMBOLS for variables.  Increased the buffer by 64 bytes for DoNGACMD_str() and DoCMD_str_back_OKdata().  Reset BurstModeOn and AutoPollOn in device_setCurrentDevice().  Modified rs232_device_init() to test Spectrum Pro by RS232.
- Increased c_APDU length form 500 bytes to 512 bytes.
- Added IDT_DEVICE_SPECTRUM_PRO_COM for emv_retrieveTerminalID().  Increased 64 buytes buffer for emv_retrieveCRL().  Fixed the timout value for emv_startTransaction() and emv_authenticateTransaction().  Turn LED light off in emv_cancelTransaction().
- Changed SHARED_SYMBOLS to be the same as EXPORT_SYMBOLS.
- Added EXPORT_SYMBOLS to setAbsoluteLibraryPath().  Changed getLibraryPath(), getFunction(), and startAutoPollThread() to fix pointer issues.

## 05/25/2017
- Check for prefix forward slash in absolute library path.
- Update SDK to produce separate libraries

## 05/23/2017
- Initialized the variables to 0.
- Added length check for byte array parameters.
- Added _CMD_DATA_MAX_LEN.
- Changed ReadNGAUsb() and ReadIDGUsb() to check the data size limit.
- Removed device_getDRS().
- Increased the data size of DoNGACMD_str(), DoCMD_str_noback(), and device_setCurrentDevice().
- Changed the comment of device_getDRS().  Removed msr_clearMSRData().
- Added more parameter check for msr_captureMode().
- Added more parameter check for icc_enable() and icc_exchangeAPDU().
- Increased the data size of emv_setTerminalID().  Changed the timeout requirement for emv_startTransaction().
- Added more parameter check for device_controlLED(), device_controlLED_ICC(), and device_controlBeep().
- Added more parameter check for config_setLEDController(), config_setBeeperController(), and config_setEncryptionControl().

## 04/21/2017
- Changed the version to 0.02.018.
- Added the internal functions GetReadTimeoutInterval() and ResetReadTimeoutInterval().
- Changed the comment for ctls_setTerminalData().
- Changed the comments for device_verifyBackdoorKey() and ctls_retrieveTerminalData().
- Added the flag PassThroughModeOn.  Changed DoCMD_str_noback() and DoCMD_str_back_OKdata() to return error code.  Changed device_setCurrentDevice() to check burst mode and auto poll.
- Changed pin_getPIN() to return error when the parameters are invalid.
- Changed threadProc_Read_MSR() to fix the card swipe issue for Spectrum Pro.
- Added the function startAutoPollThread().
- Reset the timout after ctls transaction.  Changed ctls_startTransaction() to return error when passthrough mode is on.  Removed redundant devices for ctls functions.
- Changed the return value of getAIDTags() to handle different errors.

## 04/12/2017
- Added device_verifyBackdoorKey() to libIDT_UniPayI_V.h, and removed device_verifyBackdoorKey() from libIDT_MiniSmartII.h.
- Changed device_verifyBackdoorKey() to support only Augusta TTK for now.

## 04/11/2017
- Changed SDK version to 0.02.017.
- Changed processReceivedData() and processEMVData() to handle the result code EMV_RESULT_CODE_SWIPE_NON_ICC for EMV transaction.

## 04/10/2017
- Changed the SDK version to 0.02.016.

## Removed typedef bool. Made all instances int.
- Removed "false". Made all instances 0.
- Removed "true". Made all instances 1.

## 04/06/2017
- Changed the SDK version to 0.02.015.
- Changed the comments of ctls_startTransaction() and msr_startMSRSwipe().
- Added the error code RETURN_CODE_ERR_CMD_NOT_ALLOWED.
- Changed msr_startMSRSwipe() to return error if auto poll is on.
- Changed emv_startTransaction() to attach the additional tags to the command and check the tags length.  Also changed emv_authenticateTransaction() and emv_completeTransaction() to check the tags length.
- Changed ctls_startTransaction() to return error when auto poll is on (fixed the issue KIOSKSDK-159).
- Changed bool to unsigned char, and removed True and False.

## 04/05/2017
- Changed SDK version to 0.02.014.
- Added the conditional compile flag __MACH__ to be compatible with Mac.
- Switched EMOCOMP and SDKCOMP for some Makefiles.
- Added SetReadTimeoutInterval() for RS232 devices.
- Added device_isAttached() and device_getMerchantRecord() in .h files.

## 04/04/2017
- Implemented functions for RS232 devices.  (Currently only support Spectrum Pro.)
- Added IDG_P2_STATUS_CODE for IDG devices, also added support for BTPay Mini.
- Fixed issues KIOSKSDK-137, KIOSKSDK-138, KIOSKSDK-143, KIOSKSDK-146, KIOSKSDK-147, KIOSKSDK-148, KIOSKSDK-144, KIOSKSDK-151, KIOSKSDK-152, KIOSKSDK-154, KIOSKSDK-150, KIOSKSDK-155, KIOSKSDK-156, KIOSKSDK-158, KIOSKSDK-157, KIOSKSDK-161, KIOSKSDK-162, KIOSKSDK-160, KIOSKSDK-153, and SSPLS-48.

## 03/21/2017
- Fixed emv_completeTransaction() for IDG devices.
- 0.02.013
- Updated Makefiles.
- Changed the comments of emv_startTransaction() and ctls_startTransaction().
- Changed the comment of the first parameter (commError) for emv_completeTransaction().
- Changed emv_completeTransaction() to fix Issue SSPLS-46.  Modified emv_callbackResponseLCD() to support LCD menus for UniPay III.
- Changed the value of the first parameter (commError) for emv_completeTransaction() from 1 to 0 in the demo.

## 03/20/2017
- Changed SDK version to 0.02.012.
- Commented out device_startRKI() to fix Issue SSPLS-47.
- Added mssleep(100) between write and read for DoNGACmdUsb() to fix Issue SSPLS-42.
- Changed emv_completeTransaction() to fix Issue SSPL-46.
- Changed pin_cancelPINEntry(); to r = pin_cancelPINEntry(); in Spectrum Pro demo.
- Added mssleep(100) in controlLED() to fix Issue AGSTSDK-47.

## 03/16/2017
- Changed emv_startTransaction() to check more parameters.  (Fixed Issue SSPLS-43.)
- Added msr_getKeyTypeForICCDUKPT(), msr_getKeyFormatForICCDUKPT(), msr_setKeyFormatForICCDUKPT(), and msr_setKeyTypeForICCDUKPT() to fix issue AGSTSDK-44.
- Removed MSR functions from MiniSmart II.
- Changed icc_setKeyFormatForICCDUKPT() to support TransArmor for Augusta (Issue AGSTSDK-45).
- Added msr_captureMode() to the libIDT_Augusta.h and libIDT_Device.h (Issue AGSTSDK-46).
- Change SDK version to 0.02.011
- Update OpenUSB() to correctly store handle from hidapi library
- Update hid_write() and hid_read_timeout() invocations to pass in the correct hidapi handle
- Update threadProc_Read_EMV() to handle read invocation return value accordingly for Mac platform
- Update emv_authenticateTransaction() and emv_completeTransaction() to add delay for Mac platform
- Update addTLVEntriesToHashMapObj() to cast tagNameArray accessors to unsigned char because of integer promotion
- Update getjByteArrayFromBYTEArray() and getStringFromCharArray() to add data length bound checking
- Add default case to all switch statements in IDTechSDK_IDTechSDKBridge.c

## 03/15/2017
- Changed SDK version to 0.02.010
- Changed DoIDGCmdUsb(), DoCMD_str_back_OKdata(), and device_setCurrentDevice() to fix Issue SSPLS-44.
- Changed emv_startTransaction() to limit tagLen to < 500 bytes (Fixed Issue SSPLS-43).
- Changed the error message for get Configuration Group in Kiosk III Demo.
- Removed old options 16. Set Swipe TDES., 17. Set Swipe AES., and 18. Get Swipe Encryption to fix Issue AGSTSDK-43.

## 03/14/2017
- Changed version to 0.02.009
- Changed DoCMD_data_back_OKdata() to fix the issues AGSTSDK-40 and AGSTSDK-41.
- Changed emv_startTransaction() to check the parameters (to fix the issue SSPLS-43).

## 03/13/2017
- 0.02.008
- Updated Makefiles to include the version number
- Changed IDT_device_hotplug_callback() to remove the log. KIOSKSDK-133

## 0.02.007
- Removed RS-232 for Mac
- Added RS232 library.
- Fixed log issue. KIOSKSDK-133
- Added RS232 devices.
- Changed charmsr_encTrack1Len to msr_encTrack1Len.
- Modified processReceivedData() to fix the issue of data parsing for MSR fallback of EMV transaction.
- Added ctls_removeConfigurationGroup(). KIOSKSDK-135.
- Add Spectrum Pro JNI bridge methods

## 3/8/2017
- 0.02.006
- Added MiniSmart II
- Moved the auto poll thread to IDTech_device.c.  Added some RS232 functions which are still under implementation.
- Modified device_setBurstMode() and device_setPollMode() to create a thread for auto poll (to lower CPU usage).  Also moved threadProc_AutoPoll_Read() to this file.

## 3/7/2017
- Update JNI bridge CAPTURE_ENCRYPT_TYPE switch

## 3/6/2017
- Add Kiosk III JNI bridge methods

## 3/2/2017
- Add Augusta JNI bridge methods

## 3/1/2017
- Added MiniSmart II.

## 2/28/2017
- Added MiniSmart II and defined PIDï¿½s.
- Fixed emv_startTransaction() for IDG devices.
- Added UniPay 1.5 TTK

## 2/23/2017
- DVT SDK 0.2.005

## 2/22/2017
- Changed isAIDTag().
- Changed bool to int for WIN32.
- Changed _UNIX_PLATFORM_ to __linux.
- 0.02.005.  Updated Shared Exports
- Fixed return data for ctls_retrieveTerminalData().
- 0.02.005.  Update IDG USB
- 0.02.005.  Updated Windows USB Routines.  Fixed startup loop failure
- 0.02.005.  Updated Mac USB Routines
- 0.02.005.  Updated Makefiles.  Updated External References.  Removed DUKPT/MAC routines
- Mac Functions
- New Makefiles

## 2/20/2017
- Fixed emv_retrieveTransactionResult() to parse tlv data for Augusta.

## 2/19/2017
- Mac Updates

## 2/17/2017
- Mac HID Updates

## 2/17/2017
- Changed SDK version to 0.02.004.
- Added parameter check for msr_startMSRSwipe().
- Added parameter check for emv functions, and also fixed the block issue for emv transaction.
- Added parameter check for device_controlLED().
- Added parameter check for ctls_startTransaction().

## 2/14/2017
- Changed parseEMVFormat1() for Augusta.
- Changed threadProc_Read_MSR() for Augusta.

## 2/13/2017
- Fixed compiling warnings for Eclipse under Windows and added fflush()ï¿½s.
- Added a MSR callback function with a IDTMSRData pointer.
- Added UniPay III.
- Added UniPay I_V.
- Added an NGA MSR parse function.
- Added void (*pMSR_callBackp)(int, IDTMSRData *).

## 2/2/2017
- Changed functions for contactless devices to have mssleep(150) before sending the command.  Also changed the error code for IDG Protocol 2 devices.

## 1/27/2017
- Merged to local copy which supports VP4880 and firmware update for Augusta and Kiosk III.
- Added MSR Transaction flags.

## 1/24/2017
- added LoadLibrary for Windowws

## 1/18/2017
- Mac Support

## 1/17/2017
- minGW Eclipse Update

## 1/5/2017
- Changed device_SendDataCommand().
- Added convertDoubleToBytes().
- Added IDG functions.
- Changed Start CTLS Swipe command to 0x0240.
- Added VP4880.

## 12/21/2016
- Eclipse Windows Build
- Modified libpos_open_device() to be working for Windows and Linux.
- Removed prototype of parseMSRData().

## 12/20/2016
- Added the flag to with between Windows and Linux.

## 12/16/2016
- Added support for Augusta S and Augusta S TTK.

## 12/15/2016
- Add Eclipse Project
- Restore Makefile
- Implemented config_getSerialNumber() for Augusta.
- Modified the comments.
- KioskIII, Augusta, SpectrumPro Support
- Modified the comment for ctls_cancelTransaction().

## 12/13/2016
- SDK: Remove the unnecessary log

## 12/12/2016
- Added read and write functions for Augusta KB for testing.
- Added ITP functions for Augusta KB for testing.
- Changed msr_switchUSBInterfaceMode().
- Changed the switch in get firmware version and controlLED_ICC for Augusta KB.

## 11/30/2016
- Fixed a bug for processReceivedData().
- Added timeout for threadProc_Read_EMV(), and LED control for Augusta EMV transaction.

## 11/28/2016
- Modified processReceivedData() for Augusta.
- Implemented EMV functions for Augusta.

## 11/23/2016
- SDK: Augusta SRED: 1. Add the MAC function for setWhiteList. 2. fixed the crash issue when do the MAC function 3. add the error when get MAC KSN failed.
- Modified threadProc_Read_EMV() to support Augusta.
- SDK: Augusta SRED: add the MAC supported for setWhiteList

## 11/22/2016
- Fixed emv_retrieveTerminalID().
- SDK: Augusta: EMV: added the CAPK and CRL functions, and fixed the return ACK code

## 11/21/2016
- 1. using DoCMD_str_noback instead of DoNGACMD_str
- 2. fixed the setterminaldata and emv_removeTerminalData and emv_setApplicationData
- 3. remove the unnecessary comments
- Implemented AID and Terminal functions for EMV.

## 11/18/2016
- Implemented part of EMV functions.

## 11/17/2016
- Changed threadProc_Read_MSR() to parse the MSR data for Augusta.
- SDK: ICC: Added the APIs same with windows SDK for Augusta.
- SDK: Device: rollback the changing of Chihlung
- SDK: MSR: fixed the read buffer mode for Augusta.

## 11/16/2016
- Implementing msr_getMSRData().
- Implemented ICC functions.
- Added the place holder for device_getKeyStatus().
- SDK: Augusta: MSR: Added the MSR APIs for Augusta as follows detailed:
- msr_captureMode
- msr_disable
- msr_getClearPANID
- msr_getExpirationMask
- msr_getFunctionStatus
- msr_getSetting
- msr_getSwipeEncryption
- msr_getSwipeForcedEncryptionOption
- msr_getSwipeMaskOption
- msr_RetrieveWhiteList
- msr_setClearPANID
- msr_setExpirationMask
- msr_setSetting
- msr_setSwipeEncryption
- msr_setSwipeForcedEncryptionOption
- msr_setSwipeMaskOption
- msr_setWhiteList
- msr_switchUSBInterfaceMode

## SDK: Remove unnecessary val

## 11/15/2016
- SDK: Augusta(S/TTK): add the API into the device module.
- SDK: Augusta: Add the config function

## 11/14/2016
- SDK: To support Augusta / Augusta S

## 11/9/2016
- SDK: removed the unnecessary log, especial for Card data output print log;
- Changed the unnecessary log

## 11/8/2016
- 1. removed the device_getDeviceTypeIndex and device_disconnected which looks seems the API, actually they all internal function.
- 2. added the hotplug_usb_connected and hotplug_usb_disconnected

## changed device_init() to fix hot plug issue.
- HOT-PLUG Debug

## 11/7/2016
- fixed the crash issue after reboot device
- remove unnecessary function in icc model.
- Removed printf("s_husb [%X]\n", s_husb);
- Added cardData->encryptedTagCount = 0;, cardData->maskedTagCount = 0; and cardData->unencryptedTagCount = 0; to resetMSRData.

## 11/3/2016
- Added ReadDataUsb() to read ctls data.
- Modified threadProc_AutoPoll_Read() to handle ctls data.
- Added mssleep(150) for each Kiosk III command.
- Added processCTLSData() to parse the ctls data.
- Added ctlsApplicationand isCTLS.
- Added getApplicationType().

## 11/2/2016
- removed printf().
- Implemented threadAutoPoll and the checking for Auto Poll mode for Kiosk III.
- Implemented the checking for Auto Poll mode for Kiosk III.
- Modify the SDK Version and Add the changing history
- fixed the SDK issue to build succeeded

## 11/1/2016
- fixed the API ctls_getAllConfigurationGroups to add the parameters
- add the device_getIDGStatusCodeString for the device with NEO/IDG protocol
- Double check the API description and add the API in the head file
- Added printAIDs().
- Changed retrieveAIDTags() and added printAIDs().
- Fix the device_XXX and ctls_XXX API command set

## 10/31/2016
- Changed retrieveAIDTags().
- Changed TLV_GetTag().
- Changed resData to resLen.
- Changed the command for ctls_retrieveApplicationData().
- Added retrieveAIDTags().
- Fixed the ctls_setApplication

## 10/28/2016
- Added getHashSha256().
- Implemented missing functions.
- API Adjustment:
- 1. ICC: changing the API such as icc_passthroughon / off to device_XXX
- 2. ctls: changing the emv_XXX to ctls_XXX for set/get AID and CAPK
- 3. Device: add some device_XXX API according windows SDK

## 10/27/2016
- Remove the lock and check startCTLSTransaction and cancelCTLSTransaction to enable cancel transaction.
- Remove the lock to enable cancel transaction.
- Added support for Kiosk III S.
- Added cancelCTLSTransaction, and startCTLSTransaction to check CTLS transaction status.
- Added MSR_callBack_type_TERMINATED.
- HOT-PLUG: add the hot-plug feature supported.
- Detailed:
- 1. first open current connected(attached device), and then register the usb hot-plug callback.
- (Limitation: if we don't open current device, the hot-plug cannot detect what has been connected device so that skipped these connected devices)
- 2. when received the hot-plug message, SDK will change the device status with connected/disconnected.
- 3. when received the device plugged in message, SDK will open the new coming device.

## 10/26/2016
- Modified read functions.
- Added DoIDGCmdUsb() and modified read and write functions.
- Implemented ctls parse functions.
- Comment out getEncryptedTrackDataLength() and getCardType().
- Changed ReadUsb() to ReadNGAUsb().
- Changed DoCmdUsb() to DoNGACmdUsb().

## 10/26/2016
- Implemented ctls_startTransaction() and the callback function.
- Added IDTTag.

## 10/25/2016
- add the API: bool device_isAttached(int deviceType)

## 10/24/2016
- rollback, skip add the hot-plug

## 10/21/2016
- add the hot-plug

## 10/20/2016
- Modified device_close() and device_isConnected().
- Added support for more devices.
- Added ctls functions.
- Fixed grsiP2Command().

## 1/19/2016
- Modified device_SendDataCommand().
- Modified device_init().
- Changed IDT_DEVICE_AUGUSTA to IDT_DEVICE_AUGUSTA_HID.
- Updated device_SendDataCommand().
- Modified device_getFirmwareVersion() and device_SendDataCommand().
