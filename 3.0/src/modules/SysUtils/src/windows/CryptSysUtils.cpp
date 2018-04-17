/* @file Реализация CryptSysUtils. */

/* (c) https://support.microsoft.com/en-us/kb/323809 */

// windows
#include <windows.h>
#include <wincrypt.h>
#include <wintrust.h>
#include <softpub.h>

#include <delayimphlp.h>

// Модули
#include "SysUtils/ISysUtils.h"

#define ENCODING (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING)

//--------------------------------------------------------------------------------
typedef struct {
	LPWSTR lpszProgramName;
	LPWSTR lpszPublisherLink;
	LPWSTR lpszMoreInfoLink;
} SPROG_PUBLISHERINFO, *PSPROG_PUBLISHERINFO;

//--------------------------------------------------------------------------------
DL_USE_MODULE_BEGIN(wintrust, "wintrust.dll")
	DL_DECLARE_FUN_THROW(WinVerifyTrustEx, HRESULT, (IN HWND)(IN GUID *)(IN WINTRUST_DATA *))
DL_USE_MODULE_END()

//--------------------------------------------------------------------------------
DL_USE_MODULE_BEGIN(crypt, "crypt32.dll")
	DL_DECLARE_FUN_THROW(CryptDecodeObject, BOOL, (IN DWORD)(IN LPCSTR)(IN const BYTE *)(IN DWORD)(IN DWORD)(OUT void *)(DWORD *))
	DL_DECLARE_FUN_THROW(CryptMsgClose, BOOL, (IN HCRYPTMSG))
	DL_DECLARE_FUN_THROW(CryptMsgGetParam, BOOL, (IN HCRYPTMSG)(IN DWORD)(IN DWORD)(OUT void *)(DWORD *))
	DL_DECLARE_FUN_THROW(CertCloseStore, BOOL, (IN HCERTSTORE)(IN DWORD))
	DL_DECLARE_FUN_THROW(CertFindCertificateInStore, PCCERT_CONTEXT, (IN HCERTSTORE)(IN DWORD)(IN DWORD)(IN DWORD)(IN const void *)(IN PCCERT_CONTEXT))
	DL_DECLARE_FUN_THROW(CertFreeCertificateContext, BOOL, (IN PCCERT_CONTEXT))
	DL_DECLARE_FUN_THROW(CertGetNameString, BOOL, (IN PCCERT_CONTEXT)(IN DWORD)(IN DWORD)(IN void *)(OUT LPTSTR)(IN DWORD))
	DL_DECLARE_FUN_THROW(CryptQueryObject, BOOL, (IN DWORD)(IN const void *)(IN DWORD)(IN DWORD)(IN DWORD)(OUT DWORD *)(OUT DWORD *)(OUT DWORD *)(OUT HCERTSTORE *)(OUT HCRYPTMSG *)(OUT const void **))
DL_USE_MODULE_END()

//--------------------------------------------------------------------------------
BOOL GetProgAndPublisherInfo(PCMSG_SIGNER_INFO pSignerInfo, PSPROG_PUBLISHERINFO Info);
BOOL GetDateOfTimeStamp(PCMSG_SIGNER_INFO pSignerInfo, SYSTEMTIME *st);
BOOL GetTimeStampSignerInfo(PCMSG_SIGNER_INFO pSignerInfo, PCMSG_SIGNER_INFO *pCounterSignerInfo);

//--------------------------------------------------------------------------------
qlonglong ISysUtils::verifyTrust(const QString & aFile)
{
	LONG status = ERROR_SUCCESS;

	try
	{
		GUID WVTPolicyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;
		WINTRUST_DATA trustData;
		memset(&trustData, 0, sizeof(trustData));
		trustData.cbStruct = sizeof(trustData);
		trustData.pPolicyCallbackData = 0;
		trustData.pSIPClientData = 0;
		trustData.dwUIChoice = WTD_UI_NONE;
		trustData.fdwRevocationChecks = WTD_REVOKE_NONE;
		trustData.dwUnionChoice = WTD_CHOICE_FILE;
		trustData.dwStateAction = 0;
		trustData.hWVTStateData = 0;
		trustData.pwszURLReference = 0;
		trustData.dwUIContext = 0;

		WINTRUST_FILE_INFO fileData;
		memset(&fileData, 0, sizeof(fileData));
		fileData.cbStruct = sizeof(WINTRUST_FILE_INFO);
		fileData.hFile = 0;
		fileData.pgKnownSubject = 0;

		TCHAR path[MAX_PATH] = {'\0'};

		aFile.toWCharArray(path);
		fileData.pcwszFilePath = path;
		trustData.pFile = &fileData;

		status = wintrust::WinVerifyTrustEx(0, &WVTPolicyGUID, &trustData);
	}
	catch (delayload::CDynFunException & e)
	{
		status = ERROR_INVALID_FUNCTION;
		qCritical() << e.GetMessage();
	}

	return status;
}

//--------------------------------------------------------------------------------
LPWSTR AllocateAndCopyWideString(LPCWSTR inputString)
{
	LPWSTR outputString = NULL;

	outputString = (LPWSTR)LocalAlloc(LPTR, (wcslen(inputString) + 1) * sizeof(WCHAR));
	
	if (outputString != NULL)
	{
		lstrcpyW(outputString, inputString);
	}

	return outputString;
}

//--------------------------------------------------------------------------------
BOOL GetProgAndPublisherInfo(PCMSG_SIGNER_INFO pSignerInfo, PSPROG_PUBLISHERINFO Info)
{
	BOOL fReturn = FALSE;
	PSPC_SP_OPUS_INFO OpusInfo = NULL;
	DWORD dwData;
	BOOL fResult;

	try
	{
		// Loop through authenticated attributes and find
		// SPC_SP_OPUS_INFO_OBJID OID.
		for (DWORD n = 0; n < pSignerInfo->AuthAttrs.cAttr; n++)
		{
			if (lstrcmpA(SPC_SP_OPUS_INFO_OBJID,
				pSignerInfo->AuthAttrs.rgAttr[n].pszObjId) == 0)
			{
				// Get Size of SPC_SP_OPUS_INFO structure.
				fResult = crypt::CryptDecodeObject(ENCODING,
					SPC_SP_OPUS_INFO_OBJID,
					pSignerInfo->AuthAttrs.rgAttr[n].rgValue[0].pbData,
					pSignerInfo->AuthAttrs.rgAttr[n].rgValue[0].cbData,
					0,
					NULL,
					&dwData);
				if (!fResult)
				{
					qDebug() << "CryptDecodeObject failed with" << QString::number(GetLastError(), 16);
					throw false;
				}

				// Allocate memory for SPC_SP_OPUS_INFO structure.
				OpusInfo = (PSPC_SP_OPUS_INFO)LocalAlloc(LPTR, dwData);
				if (!OpusInfo)
				{
					qDebug() << "Unable to allocate memory for Publisher Info.\n";
					throw false;
				}

				// Decode and get SPC_SP_OPUS_INFO structure.
				fResult = crypt::CryptDecodeObject(ENCODING,
					SPC_SP_OPUS_INFO_OBJID,
					pSignerInfo->AuthAttrs.rgAttr[n].rgValue[0].pbData,
					pSignerInfo->AuthAttrs.rgAttr[n].rgValue[0].cbData,
					0,
					OpusInfo,
					&dwData);
				if (!fResult)
				{
					qDebug() << "CryptDecodeObject failed with" << QString::number(GetLastError(), 16);
					throw false;
				}

				// Fill in Program Name if present.
				if (OpusInfo->pwszProgramName)
				{
					Info->lpszProgramName =
						AllocateAndCopyWideString(OpusInfo->pwszProgramName);
				}
				else
					Info->lpszProgramName = NULL;

				// Fill in Publisher Information if present.
				if (OpusInfo->pPublisherInfo)
				{

					switch (OpusInfo->pPublisherInfo->dwLinkChoice)
					{
					case SPC_URL_LINK_CHOICE:
						Info->lpszPublisherLink =
							AllocateAndCopyWideString(OpusInfo->pPublisherInfo->pwszUrl);
						break;

					case SPC_FILE_LINK_CHOICE:
						Info->lpszPublisherLink =
							AllocateAndCopyWideString(OpusInfo->pPublisherInfo->pwszFile);
						break;

					default:
						Info->lpszPublisherLink = NULL;
						break;
					}
				}
				else
				{
					Info->lpszPublisherLink = NULL;
				}

				// Fill in More Info if present.
				if (OpusInfo->pMoreInfo)
				{
					switch (OpusInfo->pMoreInfo->dwLinkChoice)
					{
					case SPC_URL_LINK_CHOICE:
						Info->lpszMoreInfoLink =
							AllocateAndCopyWideString(OpusInfo->pMoreInfo->pwszUrl);
						break;

					case SPC_FILE_LINK_CHOICE:
						Info->lpszMoreInfoLink =
							AllocateAndCopyWideString(OpusInfo->pMoreInfo->pwszFile);
						break;

					default:
						Info->lpszMoreInfoLink = NULL;
						break;
					}
				}
				else
				{
					Info->lpszMoreInfoLink = NULL;
				}

				fReturn = TRUE;

				break; // Break from for loop.
			} // lstrcmp SPC_SP_OPUS_INFO_OBJID                 
		} // for 
	}
	catch (bool & e)
	{
		fReturn = e;
	}
	catch (delayload::CDynFunException & e)
	{
		qCritical() << e.GetMessage();
	}

	if (OpusInfo != NULL) LocalFree(OpusInfo);

	return fReturn;
}

//--------------------------------------------------------------------------------
BOOL GetDateOfTimeStamp(PCMSG_SIGNER_INFO pSignerInfo, SYSTEMTIME *st)
{
	BOOL fResult;
	FILETIME lft, ft;
	DWORD dwData;
	BOOL fReturn = FALSE;

	try
	{
		// Loop through authenticated attributes and find
		// szOID_RSA_signingTime OID.
		for (DWORD n = 0; n < pSignerInfo->AuthAttrs.cAttr; n++)
		{
			if (lstrcmpA(szOID_RSA_signingTime,
				pSignerInfo->AuthAttrs.rgAttr[n].pszObjId) == 0)
			{
				// Decode and get FILETIME structure.
				dwData = sizeof(ft);
				fResult = crypt::CryptDecodeObject(ENCODING,
					szOID_RSA_signingTime,
					pSignerInfo->AuthAttrs.rgAttr[n].rgValue[0].pbData,
					pSignerInfo->AuthAttrs.rgAttr[n].rgValue[0].cbData,
					0,
					(PVOID)&ft,
					&dwData);
				if (!fResult)
				{
					qDebug() << "CryptDecodeObject failed with" << QString::number(GetLastError(), 16);
					break;
				}

				// Convert to local time.
				FileTimeToLocalFileTime(&ft, &lft);
				FileTimeToSystemTime(&lft, st);

				fReturn = TRUE;

				break; // Break from for loop.

			} //lstrcmp szOID_RSA_signingTime
		} // for 
	}
	catch (delayload::CDynFunException & e)
	{
		qCritical() << e.GetMessage();
	}

	return fReturn;
}

//--------------------------------------------------------------------------------
BOOL GetTimeStampSignerInfo(PCMSG_SIGNER_INFO pSignerInfo, PCMSG_SIGNER_INFO *pCounterSignerInfo)
{
	BOOL fReturn = FALSE;
	BOOL fResult;
	DWORD dwSize;

	try
	{
		*pCounterSignerInfo = NULL;

		// Loop through unathenticated attributes for
		// szOID_RSA_counterSign OID.
		for (DWORD n = 0; n < pSignerInfo->UnauthAttrs.cAttr; n++)
		{
			if (lstrcmpA(pSignerInfo->UnauthAttrs.rgAttr[n].pszObjId,
				szOID_RSA_counterSign) == 0)
			{
				// Get size of CMSG_SIGNER_INFO structure.
				fResult = crypt::CryptDecodeObject(ENCODING,
					PKCS7_SIGNER_INFO,
					pSignerInfo->UnauthAttrs.rgAttr[n].rgValue[0].pbData,
					pSignerInfo->UnauthAttrs.rgAttr[n].rgValue[0].cbData,
					0,
					NULL,
					&dwSize);
				if (!fResult)
				{
					qDebug() << "CryptDecodeObject failed with" << QString::number(GetLastError(), 16);
					throw false;
				}

				// Allocate memory for CMSG_SIGNER_INFO.
				*pCounterSignerInfo = (PCMSG_SIGNER_INFO)LocalAlloc(LPTR, dwSize);
				if (!*pCounterSignerInfo)
				{
					qDebug() << "Unable to allocate memory for timestamp info.";
					throw false;
				}

				// Decode and get CMSG_SIGNER_INFO structure
				// for timestamp certificate.
				fResult = crypt::CryptDecodeObject(ENCODING,
					PKCS7_SIGNER_INFO,
					pSignerInfo->UnauthAttrs.rgAttr[n].rgValue[0].pbData,
					pSignerInfo->UnauthAttrs.rgAttr[n].rgValue[0].cbData,
					0,
					(PVOID)*pCounterSignerInfo,
					&dwSize);
				if (!fResult)
				{
					qDebug() << "CryptDecodeObject failed with" << QString::number(GetLastError(), 16);
					throw false;
				}

				fReturn = TRUE;

				break; // Break from for loop.
			}
		}
	}
	catch (bool & e)
	{
		fReturn = e;
	}
	catch (delayload::CDynFunException & e)
	{
		qCritical() << e.GetMessage();
	}

	return fReturn;
}

//--------------------------------------------------------------------------------
bool ISysUtils::getSignerInfo(const QString & aFile, SSignerInfo & aSigner)
{
	HCERTSTORE hStore = NULL;
	HCRYPTMSG hMsg = NULL;
	PCCERT_CONTEXT pCertContext = NULL;
	BOOL fResult = false;
	DWORD dwEncoding, dwContentType, dwFormatType;
	PCMSG_SIGNER_INFO pSignerInfo = NULL;
	DWORD dwSignerInfo;
	CERT_INFO CertInfo;
	LPTSTR szSignerName = NULL;

	try
	{
		// Get message handle and store handle from the signed file.
		WCHAR szFileName[MAX_PATH] = { '\0' };
		aFile.toWCharArray(szFileName);

		fResult = crypt::CryptQueryObject(CERT_QUERY_OBJECT_FILE,
			szFileName,
			CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED,
			CERT_QUERY_FORMAT_FLAG_BINARY,
			0,
			&dwEncoding,
			&dwContentType,
			&dwFormatType,
			&hStore,
			&hMsg,
			NULL);
		if (!fResult)
		{
			qDebug() << "CryptQueryObject failed with" << QString::number(GetLastError(), 16);
			throw false;
		}

		// Get signer information size.
		fResult = crypt::CryptMsgGetParam(hMsg,
			CMSG_SIGNER_INFO_PARAM,
			0,
			NULL,
			&dwSignerInfo);
		if (!fResult)
		{
			qDebug() << "CryptMsgGetParam failed with" << QString::number(GetLastError(), 16);
			throw false;
		}

		// Allocate memory for signer information.
		pSignerInfo = (PCMSG_SIGNER_INFO)LocalAlloc(LPTR, dwSignerInfo);
		if (!pSignerInfo)
		{
			qDebug() << "Unable to allocate memory for Signer Info.";
			throw false;
		}

		// Get Signer Information.
		fResult = crypt::CryptMsgGetParam(hMsg,
			CMSG_SIGNER_INFO_PARAM,
			0,
			(PVOID)pSignerInfo,
			&dwSignerInfo);
		if (!fResult)
		{
			qDebug() << "CryptMsgGetParam failed with" << QString::number(GetLastError(), 16);
			throw false;
		}

		// Search for the signer certificate in the temporary 
		// certificate store.
		CertInfo.Issuer = pSignerInfo->Issuer;
		CertInfo.SerialNumber = pSignerInfo->SerialNumber;

		pCertContext = crypt::CertFindCertificateInStore(hStore,
			ENCODING,
			0,
			CERT_FIND_SUBJECT_CERT,
			(PVOID)&CertInfo,
			NULL);
		if (!pCertContext)
		{
			qDebug() << "CertFindCertificateInStore failed with" << QString::number(GetLastError(), 16);
			throw false;
		}

		DWORD dwData;

		// Serial Number.
		dwData = pCertContext->pCertInfo->SerialNumber.cbData;
		QByteArray serial;
		for (DWORD n = 0; n < dwData; n++)
		{
			serial.append(pCertContext->pCertInfo->SerialNumber.pbData[dwData - (n + 1)]);
		}

		aSigner.serial = serial.toHex();

		// Get Issuer name size.
		if ((dwData = crypt::CertGetNameString(pCertContext,
			CERT_NAME_SIMPLE_DISPLAY_TYPE,
			CERT_NAME_ISSUER_FLAG,
			NULL,
			NULL,
			0)) == 0)
		{
			qDebug() << " Get Issuer name size failed.";
			throw false;
		}

		// Allocate memory for Issuer name.
		szSignerName = (LPTSTR)LocalAlloc(LPTR, dwData * sizeof(TCHAR));
		if (!szSignerName)
		{
			qDebug() << "Unable to allocate memory for issuer name.";
			throw false;
		}

		// Get Issuer name.
		if (!(crypt::CertGetNameString(pCertContext,
			CERT_NAME_SIMPLE_DISPLAY_TYPE,
			CERT_NAME_ISSUER_FLAG,
			NULL,
			szSignerName,
			dwData)))
		{
			qDebug() << "Get Issuer name failed.";
			throw false;
		}

		aSigner.issuer = QString::fromWCharArray(szSignerName);
		LocalFree(szSignerName);
		szSignerName = NULL;

		// Get Subject name size.
		if ((dwData = crypt::CertGetNameString(pCertContext,
			CERT_NAME_SIMPLE_DISPLAY_TYPE,
			0,
			NULL,
			NULL,
			0)) == 0)
		{
			qDebug() << "Get Subject name size failed.";
			throw false;
		}

		// Allocate memory for subject name.
		szSignerName = (LPTSTR)LocalAlloc(LPTR, dwData * sizeof(TCHAR));
		if (!szSignerName)
		{
			qDebug() << "Unable to allocate memory for subject name.";
			return false;
		}

		// Get subject name.
		if (!(crypt::CertGetNameString(pCertContext,
			CERT_NAME_SIMPLE_DISPLAY_TYPE,
			0,
			NULL,
			szSignerName,
			dwData)))
		{
			qDebug() << "Get subject name failed.";
			return false;
		}

		aSigner.name = QString::fromWCharArray(szSignerName);
	}
	catch (bool & e)
	{
		fResult = e;
	}
	catch (delayload::CDynFunException & e)
	{
		qCritical() << e.GetMessage();
	}
	
	// Clean up.
	if (pSignerInfo != NULL) LocalFree(pSignerInfo);
	if (pCertContext != NULL) crypt::CertFreeCertificateContext(pCertContext);
	if (hStore != NULL) crypt::CertCloseStore(hStore, 0);
	if (hMsg != NULL) crypt::CryptMsgClose(hMsg);

	if (szSignerName != NULL) LocalFree(szSignerName);

	return fResult;
}

//--------------------------------------------------------------------------------
