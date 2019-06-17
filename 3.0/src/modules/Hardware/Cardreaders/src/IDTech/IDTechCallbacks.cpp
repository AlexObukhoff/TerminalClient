/* @file Колбеки для функционала dll кардридера IDTech. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDebug>
#include <Common/QtHeadersEnd.h>

// Project
#include "IDTechCallbacks.h"

//------------------------------------------------------------------------------
void logMessageHotplugOut(int, int)
{
	qDebug() << __FUNCTION_NAME__;

	/**
	* Define the USB hot-plug callback function to monitor the info when plug in/out the reader. <br/>
	* It should be registered using the registerHotplugCallBk,
	* The first integer parameter is device type, and the second integer parameter is either 0: Device Plugged Out or 1: Device Plugged In
	*/
}

//------------------------------------------------------------------------------
void logSendingMessageOut(unsigned char *, int)
{
	qDebug() << __FUNCTION_NAME__;

	/**
	* Define the send command callback function to monitor the sending command into the reader. <br/>
	* It should be registered using the registerLogCallBk,
	*/
}

//------------------------------------------------------------------------------
void logReadingMessageOut(unsigned char *, int)
{
	qDebug() << __FUNCTION_NAME__;

	/**
	* Define the EMV callback function to get the transaction message/data/result. <br/>
	* It should be registered using the emv_registerCallBk,
	*/
}

//------------------------------------------------------------------------------
void getEMVDataPOut(int, int, unsigned char *, int, IDTTransactionData *, EMV_Callback *, int)
{
	qDebug() << __FUNCTION_NAME__;

	/**
	* Define the EMV callback function to get the transaction message/data/result. <br/>
	* It should be registered using the emv_registerCallBk,
	*/
}

//------------------------------------------------------------------------------
void getMSRCardDataOut(int, IDTMSRData)
{
	qDebug() << __FUNCTION_NAME__;

	/**
	* Define the MSR callback function to get the MSR card data <br/>
	* It should be registered using the msr_registerCallBk,
	*/
}

//------------------------------------------------------------------------------
void getMSRCardDataPOut(int, IDTMSRData *)
{
	qDebug() << __FUNCTION_NAME__;

	/**
	* Define the MSR callback function to get the pointer to the MSR card data <br/>
	* It should be registered using the msr_registerCallBk,
	*/
}

//------------------------------------------------------------------------------
void getCTLSCardDataOut(int aType, IDTMSRData aCardData1)    //MSR: void ctls_registerCallBk(pMSR_callBack pCTLSf) - libIDT_KioskIII.h
{
	qDebug() << __FUNCTION_NAME__;

	/**
	* Define the CTLS callback function to get the CTLS card data <br/>
	*/

	IDTechReader().getMSRCardData(aType, &aCardData1);
}

//------------------------------------------------------------------------------
void getCTLSCardDataPOut(int aType, IDTMSRData * aCardData1)    //MSR: void ctls_registerCallBkp(pMSR_callBackp pCTLSf) - libIDT_KioskIII.h
{
	qDebug() << __FUNCTION_NAME__;

	/**
	* Define the CTLS callback function to get the pointer to the CTLS card data <br/>
	*/

	IDTechReader().getMSRCardData(aType, aCardData1);
}

//------------------------------------------------------------------------------
void getPinpadDataPOut(int, IDTPINData *)
{
	qDebug() << __FUNCTION_NAME__;

	/**
	* Define the PINPad callback function to get the input PIN Pad data <br/>
	* It should be registered using the pin_registerCallBk,
	*/
}

//------------------------------------------------------------------------------
void getUpdatingStatusOut(int, int, int, int, int)
{
	qDebug() << __FUNCTION_NAME__;

	/**
	* Define the FW callback function to get the status of the firmware update <br/>
	* It should be registered using the device_registerFWCallBk,
	*/
}

//------------------------------------------------------------------------------
