#ifndef ACR120_H
#define ACR120_H

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the ACR120U_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// ACR120_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#include <windows.h>
//#define ACR120_API __declspec(dllimport)
#define ACR120_API
#define AC_DECL __stdcall

#define DLL_VER "ACR120U DLL 1.5.1.2"

#define RECEIVED_RESPONSE_SIZE	128
#define CMD_BUFFER_SIZE			128

#define ERR_READER_NO_RESPONSE						-5000

#define SUCCESS_READER_OP							0
#define ERR_INTERNAL_UNEXPECTED						-1000
#define ERR_PORT_INVALID							-2000
#define ERR_PORT_OCCUPIED							-2010
#define ERR_HANDLE_INVALID							-2020
#define ERR_INCORRECT_PARAM							-2030
#define ERR_READER_NO_TAG							-3000
#define ERR_READER_OP_FAILURE						-3030
#define ERR_READER_UNKNOWN							-3040
#define ERR_READER_LOGIN_INVALID_STORED_KEY_FORMAT	-4010
#define ERR_READER_LOGIN_FAIL						-4011
#define ERR_READER_OP_AUTH_FAIL						-4012
#define ERR_READER_VALUE_DEC_EMPTY					-4030
#define ERR_READER_VALUE_INC_OVERFLOW				-4031
#define ERR_READER_VALUE_OP_FAILURE					-4032
#define ERR_READER_VALUE_INVALID_BLOCK				-4033
#define ERR_READER_VALUE_ACCESS_FAILURE				-4034


typedef unsigned char BYTE;
typedef signed char INT8;
typedef signed short INT16;
typedef unsigned char UINT8;
typedef unsigned short UINT16;

#define ACR120_USB1		0
#define ACR120_USB2		1
#define ACR120_USB3		2
#define ACR120_USB4		3
#define ACR120_USB5		4
#define ACR120_USB6		5
#define ACR120_USB7		6
#define ACR120_USB8		7

#define AC_MIFARE_LOGIN_KEYTYPE_A           0xAA
#define AC_MIFARE_LOGIN_KEYTYPE_B           0xBB
#define AC_MIFARE_LOGIN_KEYTYPE_DEFAULT_A   0xAD
#define AC_MIFARE_LOGIN_KEYTYPE_DEFAULT_B   0xBD
#define AC_MIFARE_LOGIN_KEYTYPE_DEFAULT_F   0xFD
#define AC_MIFARE_LOGIN_KEYTYPE_STORED_A    0xAF
#define AC_MIFARE_LOGIN_KEYTYPE_STORED_B    0xBF

#pragma pack(push, 1)

typedef struct struct_status
{
	// 0x00 = Type A; 0x01 = Type B; 0x03 = Type A + Type B
	UINT8 MifareInterfaceType;

	// Bit 0 = Mifare Light; Bit 1 = Mifare1K; Bit 2 = Mifare 4K; Bit 3 = Mifare DESFire
	// Bit 4 = Mifare UltraLight; Bit 5 = JCOP30; Bit 6 = Shanghai Transport
	// Bit 7 = MPCOS Combi; Bit 8 = ISO type B, Calypso
	// Bit 9 – Bit 31 = To be defined
	UINT32 CardsSupported;

	UINT8 CardOpMode;	// 0x00 = Type A; 0x01 = Type B TAG is being processed
						// 0xFF = No TAG is being processed.

	UINT8 FWI; // the current FWI value (time out value)

	UINT8 RFU; // To be defined

	UINT16 RFU2; // to be defined
} STRUCT_STATUS;

#pragma pack(pop)

extern "C" {

//openclose.c
ACR120_API INT16 AC_DECL ACR120_Open(INT16 ReaderPort);
ACR120_API INT16 AC_DECL ACR120_Close(INT16 hReader);

//reader.c
ACR120_API INT16 AC_DECL ACR120_Reset(INT16 hReader);
ACR120_API INT16 AC_DECL ACR120_Status(INT16 hReader, UINT8 pFirmwareVersion[20], STRUCT_STATUS *pReaderStatus);
ACR120_API INT16 AC_DECL ACR120_ReadRC500Reg(INT16 hReader, UINT8 RegNo,UINT8* pValue);
ACR120_API INT16 AC_DECL ACR120_WriteRC500Reg(INT16 hReader, UINT8 RegNo, UINT8 Value);
ACR120_API INT16 AC_DECL ACR120_DirectSend(INT16 hReader, UINT8 DataLength, UINT8* pData, UINT8* pResponseDataLength, UINT8* pResponseData, UINT16 TimedOut);
ACR120_API INT16 AC_DECL ACR120_DirectReceive(INT16 hReader, UINT8 RespectedDataLength, UINT8* pReceivedDataLength, UINT8* pReceivedData, UINT16 TimedOut);
ACR120_API INT16 AC_DECL ACR120_RequestDLLVersion(UINT8* pVersionInfoLength, UINT8* pVersionInfo);
ACR120_API INT16 AC_DECL ACR120_ReadEEPROM(INT16 hReader, UINT8 RegNo, UINT8* pEEPROMData);
ACR120_API INT16 AC_DECL ACR120_WriteEEPROM(INT16 hReader, UINT8 RegNo, UINT8 EEPROMData);
ACR120_API INT16 AC_DECL ACR120_ReadUserPort(INT16 hReader, UINT8* pUserPortState);
ACR120_API INT16 AC_DECL ACR120_WriteUserPort(INT16 hReader, UINT8 UserPortState);

//card.c
ACR120_API INT16 AC_DECL ACR120_Select(INT16 hReader, UINT8* pResultTagType, UINT8* pResultTagLength, UINT8 pResultSN[10]);
ACR120_API INT16 AC_DECL ACR120_Login(INT16 hReader, UINT8 Sector, UINT8 KeyType, INT8 StoredNo, UINT8 pKey[6]);
ACR120_API INT16 AC_DECL ACR120_Read(INT16 hReader, UINT8 Block, UINT8 pBlockData[16]);
ACR120_API INT16 AC_DECL ACR120_ReadValue(INT16 hReader, UINT8 Block, INT32* pValueData);
ACR120_API INT16 AC_DECL ACR120_Write(INT16 hReader, UINT8 Block, UINT8 pBlockData[16]);
ACR120_API INT16 AC_DECL ACR120_WriteValue(INT16 hReader, UINT8 Block, INT32 ValueData);
ACR120_API INT16 AC_DECL ACR120_WriteMasterKey(INT16 hReader, UINT8 KeyNo, UINT8 pKey[6]);
ACR120_API INT16 AC_DECL ACR120_Inc(INT16 hReader, UINT8 Block, INT32 Value, INT32* pNewValue);
ACR120_API INT16 AC_DECL ACR120_Dec(INT16 hReader, UINT8 Block, INT32 Value, INT32* pNewValue);
ACR120_API INT16 AC_DECL ACR120_Copy(INT16 hReader, UINT8 srcBlock, UINT8 desBlock, INT32* pNewValue);
ACR120_API INT16 AC_DECL ACR120_Power(INT16 hReader, INT8 State);
ACR120_API INT16 AC_DECL ACR120_ListTags(INT16 hReader, UINT8* pNumTagFound, UINT8 pTagType[4], UINT8 pTagLength[4], UINT8 pSN[4][10]);
ACR120_API INT16 AC_DECL ACR120_MultiTagSelect(INT16 hReader, UINT8 TagLength, UINT8 SN[10], UINT8* pResultTagType, UINT8* pResultTagLength, UINT8* pResultSN);
ACR120_API INT16 AC_DECL ACR120_TxDataTelegram(INT16 hReader, UINT8 SendDataLength, UINT8* pSendData, UINT8* pReceivedDataLength, UINT8* pReceivedData);
//ACR120_API INT16 AC_DECL ACR120_SetTxDataTelegramOption(INT16 hReader, UINT8 ParityOption, UINT8 CRC_Transmit, UINT8 CRC_Receive, UINT8 CRC_Formula, UINT8 Crypto_Deactivated);
//ACR120_API INT16 AC_DECL ACR120_GetTxDataTelegramOption(INT16 hReader, UINT8* pParityOption, UINT8* pCRC_Transmit, UINT8* pCRC_Receive, UINT8* pCRC_Formula, UINT8* pCrypto_Deactivated);

#define ACR120_ReadRC531Reg		ACR120_ReadRC500Reg
#define ACR120_WriteRC531Reg	ACR120_WriteRC500Reg

// Since v1.5.1.0
//xapdu.c
ACR120_API INT16 AC_DECL PICC_InitBlockNumber(INT16 FrameLength);
ACR120_API INT16 AC_DECL PICC_Xch_APDU(INT16 rHandle, BOOL typeA, INT16 *xLen, UINT8 *xData, INT16 *rLen, UINT8 *rData);
ACR120_API INT16 AC_DECL PICC_RATS(INT16 rHandle, UINT8 FSDI, UINT8 *atslen, UINT8 *ats);
ACR120_API INT16 AC_DECL PICC_Deselect(INT16 rHandle, BOOL typeA);

}

#endif
