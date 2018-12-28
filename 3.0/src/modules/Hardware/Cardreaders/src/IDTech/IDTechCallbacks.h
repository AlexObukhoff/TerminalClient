/* @file Колбеки для функционала dll кардридера IDTech. */

#include "IDTechReader.h"

#ifndef __FUNCTION_NAME__
	#if defined(WIN32) || defined(_WIN32)   // Windows
		#define __FUNCTION_NAME__   __FUNCTION__  
	#else          //*Nix
		#define __FUNCTION_NAME__   __func__ 
	#endif
#endif

//------------------------------------------------------------------------------
void logMessageHotplugOut(int, int);
void logSendingMessageOut(unsigned char *, int);
void logReadingMessageOut(unsigned char *, int);
void getEMVDataPOut(int, int, unsigned char *, int, IDTTransactionData *, EMV_Callback *, int);
void  getMSRCardDataOut (int, IDTMSRData);
void  getMSRCardDataPOut(int, IDTMSRData *);
void getCTLSCardDataOut (int, IDTMSRData);      //MSR: void ctls_registerCallBk( pMSR_callBack  pCTLSf) - libIDT_KioskIII.h
void getCTLSCardDataPOut(int, IDTMSRData *);    //MSR: void ctls_registerCallBkp(pMSR_callBackp pCTLSf) - libIDT_KioskIII.h
void   getPinpadDataPOut(int, IDTPINData *);
void getUpdatingStatusOut(int, int, int, int, int);

//------------------------------------------------------------------------------
