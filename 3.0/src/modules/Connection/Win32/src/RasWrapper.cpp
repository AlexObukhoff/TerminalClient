/* @file Обёртка RAS API WIN32 */

#define _SCL_SECURE_NO_WARNINGS

// STL
#include <iostream>
#include <vector>
#include <utility>
#include <cstring> 

// Проект
#include "RasWrapper.h"

namespace RasApi 
{

//------------------------------------------------------------------------------
EOSVersion::Enum GetOSVersion()
{
	static EOSVersion::Enum version = EOSVersion::Unknown;

	if (version == EOSVersion::Unknown)
	{
		OSVERSIONINFO info;
		info.dwOSVersionInfoSize = sizeof(info);
		if (GetVersionEx(&info))
		{
			DWORD winver = (info.dwMajorVersion << 8) | info.dwMinorVersion;

			if (winver == 0x0500)
			{
				version = EOSVersion::Windows2000;
			}
			else if (winver == 0x0501)
			{
				version = EOSVersion::WindowsXP;
			}
			else if (winver == 0x0502)
			{
				version = EOSVersion::Windows2003;
			}
			else if (winver == 0x0600)
			{
				version = EOSVersion::WindowsVista;
			}
			else if (winver == 0x0601)
			{
				version = EOSVersion::Windows7;
			}
			else if (winver == 0x0602)
			{
				version = EOSVersion::Windows8;
			}
			else if (winver == 0x0603)
			{
				version = EOSVersion::Windows81;
			}
			else if (winver >= 0xa000)
			{
				version = EOSVersion::Windows10;
			}

		}
		else
		{
			std::cerr << "RasApi: GetVersionEx() failed, will use WindowsXP as default version." << std::endl;
		}
	}

	return version;
}

//------------------------------------------------------------------------------
std::wstring EDeviceType::ToString(EDeviceType::Enum type)
{
	switch (type)
	{
	case Modem:      return RASDT_Modem;
	case Isdn:       return RASDT_Isdn;
	case X25:        return RASDT_X25;
	case Vpn:        return RASDT_Vpn;
	case Pad:        return RASDT_Pad;
	case Generic:    return RASDT_Generic;
	case Serial:     return RASDT_Serial;
	case FrameRelay: return RASDT_FrameRelay;
	case Atm:        return RASDT_Atm;
	case Sonet:      return RASDT_Sonet;
	case SW56:       return RASDT_SW56;
	case Irda:       return RASDT_Irda;
	case Parallel:   return RASDT_Parallel;
	case PPPoE:      return RASDT_PPPoE;
	}

	return L"Unknown device type";
}

//------------------------------------------------------------------------------
EDeviceType::Enum EDeviceType::ToEnum(const std::wstring &type)
{
	if (!_wcsicmp(type.c_str(), RASDT_Modem))      return Modem;
	if (!_wcsicmp(type.c_str(), RASDT_Isdn))       return Isdn;
	if (!_wcsicmp(type.c_str(), RASDT_X25))        return X25;
	if (!_wcsicmp(type.c_str(), RASDT_Vpn))        return Vpn;
	if (!_wcsicmp(type.c_str(), RASDT_Pad))        return Pad;
	if (!_wcsicmp(type.c_str(), RASDT_Generic))    return Generic;
	if (!_wcsicmp(type.c_str(), RASDT_Serial))     return Serial;
	if (!_wcsicmp(type.c_str(), RASDT_FrameRelay)) return FrameRelay;
	if (!_wcsicmp(type.c_str(), RASDT_Atm))        return Atm;
	if (!_wcsicmp(type.c_str(), RASDT_Sonet))      return Sonet;
	if (!_wcsicmp(type.c_str(), RASDT_SW56))       return SW56;
	if (!_wcsicmp(type.c_str(), RASDT_Irda))       return Irda;
	if (!_wcsicmp(type.c_str(), RASDT_Parallel))   return Parallel;
	if (!_wcsicmp(type.c_str(), RASDT_PPPoE))      return PPPoE;

	return Unknown;
}

//------------------------------------------------------------------------------
std::wstring EErrorCode::ToString(DWORD aCode)
{
	wchar_t buf[256];

	if (RasGetErrorString(aCode, buf, sizeof(buf)) == EErrorCode::NoError)
	{
		return buf;
	}
	else
	{
		return L"Unknown RAS error";
	}
}

//------------------------------------------------------------------------------
IpAddress::IpAddress()
{
	std::memset(&mAddress, 0, sizeof(mAddress));
}

//--------------------------------------------------------------------------------
IpAddress::IpAddress(const RASIPADDR & aIpAddress)
{
	mAddress = aIpAddress;
}

//--------------------------------------------------------------------------------
IpAddress::operator RASIPADDR * ()
{
	return &mAddress;
}

//--------------------------------------------------------------------------------
IpAddress::operator const RASIPADDR * () const
{
	return &mAddress;
}

//--------------------------------------------------------------------------------
char IpAddress::byte(size_t index) const
{
	switch (index)
	{
	case 0: return mAddress.a;
	case 1: return mAddress.b;
	case 2: return mAddress.c;
	case 3: return mAddress.d;
	}
	
	return 0;
}

//--------------------------------------------------------------------------------
void IpAddress::setByte(size_t index, char byte)
{
	switch (index)
	{
	case 0: mAddress.a = byte;
	case 1: mAddress.b = byte;
	case 2: mAddress.c = byte;
	case 3: mAddress.d = byte;
	}
}

//------------------------------------------------------------------------------
PhonebookEntryName::PhonebookEntryName() 
{
	std::memset(&mEntry, 0, sizeof(mEntry));
	mEntry.dwSize = sizeof(mEntry);
}

//--------------------------------------------------------------------------------
PhonebookEntryName::PhonebookEntryName(const RASENTRYNAME & aEntry) 
{
	mEntry = aEntry;
}

//--------------------------------------------------------------------------------
PhonebookEntryName::operator RASENTRYNAME * () 
{
	return &mEntry;
}

//--------------------------------------------------------------------------------
std::wstring PhonebookEntryName::name() const 
{
	return mEntry.szEntryName;
}

//--------------------------------------------------------------------------------
void PhonebookEntryName::setName(const std::wstring & aName) 
{
	std::memset(mEntry.szEntryName, 0, sizeof(mEntry.szEntryName));
	aName.copy(mEntry.szEntryName, aName.size());
}

//--------------------------------------------------------------------------------
std::wstring PhonebookEntryName::phonebookPath() const 
{
	return mEntry.szPhonebookPath;
}

//--------------------------------------------------------------------------------
void PhonebookEntryName::setPhonebookPath(const std::wstring & aPath) 
{
	std::memset(mEntry.szPhonebookPath, 0, sizeof(mEntry.szPhonebookPath));
	aPath.copy(mEntry.szPhonebookPath, aPath.size());
}

//--------------------------------------------------------------------------------
bool PhonebookEntryName::isSystem() const 
{
	return mEntry.dwFlags == EPhonebookEntry::AllUsers;
}

//--------------------------------------------------------------------------------
void PhonebookEntryName::setIsSystem(bool aIsSystem) 
{
	mEntry.dwFlags = aIsSystem ? EPhonebookEntry::AllUsers : EPhonebookEntry::Private;
}

//------------------------------------------------------------------------------
PhonebookEntry::PhonebookEntry()
{
	mEntry = 0; 
	DWORD size = 0;

	mLastError = RasGetEntryProperties(0, 0, 0, &size, 0, 0);

	if (mLastError == ERROR_BUFFER_TOO_SMALL)
	{
		mEntry = reinterpret_cast<LPRASENTRY>(new unsigned char[size]);
		std::memset(mEntry, 0, size);
		mEntry->dwSize = sizeof(RASENTRY);
		mLastError = ERROR_SUCCESS;
	}
}

//--------------------------------------------------------------------------------
/*
PhonebookEntry::PhonebookEntry(const RASENTRY & aEntry) 
{
	mEntry = aEntry;
}
*/
//--------------------------------------------------------------------------------
PhonebookEntry::~PhonebookEntry() 
{
	delete[] mEntry;
}

//--------------------------------------------------------------------------------
PhonebookEntry::operator RASENTRY * () 
{
	return mEntry;
}

//--------------------------------------------------------------------------------
EConnectionOption::OptionSet PhonebookEntry::options() const 
{
	return mEntry->dwfOptions;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setOptions(EConnectionOption::OptionSet aOptions) 
{
	mEntry->dwfOptions = aOptions;
}

//--------------------------------------------------------------------------------
size_t PhonebookEntry::countryId() const 
{
	return mEntry->dwCountryID;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setCountryId(size_t aId) 
{
	mEntry->dwCountryID = aId;
}

//--------------------------------------------------------------------------------
size_t PhonebookEntry::countryCode() const 
{
	return mEntry->dwCountryCode;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setCountryCode(size_t aCode) 
{
	mEntry->dwCountryCode = aCode;
}

//--------------------------------------------------------------------------------
std::wstring PhonebookEntry::areaCode() const 
{
	return mEntry->szAreaCode;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setAreaCode(const std::wstring & aCode) 
{
	std::memset(mEntry->szAreaCode, 0, sizeof(mEntry->szAreaCode));
	aCode.copy(mEntry->szAreaCode, aCode.size());
}

//--------------------------------------------------------------------------------
std::wstring PhonebookEntry::localPhoneNumber() const 
{
	return mEntry->szLocalPhoneNumber;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setLocalPhoneNumber(const std::wstring & aNumber) 
{
	std::memset(mEntry->szLocalPhoneNumber, 0, sizeof(mEntry->szLocalPhoneNumber));
	aNumber.copy(mEntry->szLocalPhoneNumber, aNumber.size());
}

//--------------------------------------------------------------------------------
IpAddress PhonebookEntry::ip() const 
{
	return mEntry->ipaddr;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setIp(const IpAddress & aIp) 
{
	mEntry->ipaddr = *static_cast<const RASIPADDR *>(aIp);
}

//--------------------------------------------------------------------------------
IpAddress PhonebookEntry::dnsIp() const 
{
	return mEntry->ipaddrDns;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setDnsIp(const IpAddress & aIp) 
{
	mEntry->ipaddrDns = *static_cast<const RASIPADDR *>(aIp);
}

//--------------------------------------------------------------------------------
IpAddress PhonebookEntry::dnsAltIp() const 
{
	return mEntry->ipaddrDnsAlt;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setDnsAltIp(const IpAddress & aIp) 
{
	mEntry->ipaddrDnsAlt = *static_cast<const RASIPADDR *>(aIp);
}

//--------------------------------------------------------------------------------
IpAddress PhonebookEntry::winsIp() const 
{
	return mEntry->ipaddrWins;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setWinsIp(const IpAddress & aIp) 
{
	mEntry->ipaddrWins = *static_cast<const RASIPADDR *>(aIp);
}

//--------------------------------------------------------------------------------
IpAddress PhonebookEntry::winsAltIp() const 
{
	return mEntry->ipaddrWinsAlt;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setWinsAltIp(const IpAddress & aIp) 
{
	mEntry->ipaddrWinsAlt = *static_cast<const RASIPADDR *>(aIp);
}

//--------------------------------------------------------------------------------
size_t PhonebookEntry::frameSize() const 
{
	return mEntry->dwFrameSize;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setFrameSize(size_t aSize) 
{
	mEntry->dwFrameSize = aSize;
}

//--------------------------------------------------------------------------------
ENetworkProtocol::ProtocolSet PhonebookEntry::netProtocols() const 
{
	return mEntry->dwfNetProtocols;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setNetProtocols(ENetworkProtocol::ProtocolSet aProtocols) 
{
	mEntry->dwfNetProtocols = aProtocols;
}

//--------------------------------------------------------------------------------
EFramingProtocol::Enum PhonebookEntry::framingProtocol() const 
{
	return EFramingProtocol::Enum(mEntry->dwFramingProtocol);
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setFramingProtocol(EFramingProtocol::Enum aProtocol) 
{
	mEntry->dwFramingProtocol = aProtocol;
}

//--------------------------------------------------------------------------------
std::wstring PhonebookEntry::script() const 
{
	return mEntry->szScript;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setScript(const std::wstring & aScript) 
{
	std::memset(mEntry->szScript, 0, sizeof(mEntry->szScript));
	aScript.copy(mEntry->szScript, aScript.size());
}

//--------------------------------------------------------------------------------
EDeviceType::Enum PhonebookEntry::deviceType() const 
{
	return EDeviceType::ToEnum(mEntry->szDeviceType);
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setDeviceType(EDeviceType::Enum aType) 
{
	std::wstring type = EDeviceType::ToString(aType);
	std::memset(mEntry->szDeviceType, 0, sizeof(mEntry->szDeviceType));
	type.copy(mEntry->szDeviceType, type.size());
}

//--------------------------------------------------------------------------------
std::wstring PhonebookEntry::deviceName() const 
{
	return mEntry->szDeviceName;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setDeviceName(const std::wstring & aName) 
{
	std::memset(mEntry->szDeviceName, 0, sizeof(mEntry->szDeviceName));
	aName.copy(mEntry->szDeviceName, aName.size());
}

//--------------------------------------------------------------------------------
size_t PhonebookEntry::subEntries() const 
{
	return mEntry->dwSubEntries;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setSubEntries(size_t aEntries) 
{
	mEntry->dwSubEntries = aEntries;
}

//--------------------------------------------------------------------------------
EDialMode::Enum PhonebookEntry::dialMode() const 
{
	return EDialMode::Enum(mEntry->dwDialMode);
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setDialMode(EDialMode::Enum aMode) 
{
	mEntry->dwDialMode = aMode;
}

//--------------------------------------------------------------------------------
size_t PhonebookEntry::dialExtraPercent() const 
{
	return mEntry->dwDialExtraPercent;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setDialExtraPercent(size_t aPercent) 
{
	mEntry->dwDialExtraPercent = aPercent;
}

//--------------------------------------------------------------------------------
size_t PhonebookEntry::dialExtraSampleSeconds() const
{
	return mEntry->dwDialExtraSampleSeconds;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setDialExtraSampleSeconds(size_t aSeconds)
{
	mEntry->dwDialExtraSampleSeconds = aSeconds;
}

//--------------------------------------------------------------------------------
size_t PhonebookEntry::hangUpExtraPercent() const
{
	return mEntry->dwHangUpExtraPercent;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setHangUpExtraPercent(size_t aPercent)
{
	mEntry->dwHangUpExtraPercent = aPercent;
}

//--------------------------------------------------------------------------------
size_t PhonebookEntry::hangUpExtraSampleSeconds() const
{
	return mEntry->dwHangUpExtraSampleSeconds;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setHangUpExtraSampleSeconds(size_t aSeconds)
{
	mEntry->dwHangUpExtraSampleSeconds = aSeconds;
}

//--------------------------------------------------------------------------------
size_t PhonebookEntry::idleDisconnectSeconds() const
{
	return mEntry->dwIdleDisconnectSeconds;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setIdleDisconnectSeconds(size_t aSeconds)
{
	mEntry->dwIdleDisconnectSeconds = aSeconds;
}

//--------------------------------------------------------------------------------
EPhonebookEntry::TypeEnum PhonebookEntry::phonebookEntryType() const
{
	return EPhonebookEntry::TypeEnum(mEntry->dwType);
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setPhonebookEntryType(EPhonebookEntry::TypeEnum aType)
{
	mEntry->dwType = aType;
}

//--------------------------------------------------------------------------------
EEncryptionType::Enum PhonebookEntry::encriptionType() const
{
	return EEncryptionType::Enum(mEntry->dwEncryptionType);
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setEncriptionType(EEncryptionType::Enum aType)
{
	mEntry->dwEncryptionType = aType;
}

//--------------------------------------------------------------------------------
size_t PhonebookEntry::customAuthKey() const
{
	return mEntry->dwCustomAuthKey;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setCustomAuthKey(size_t aKey)
{
	mEntry->dwCustomAuthKey = aKey;
}

//--------------------------------------------------------------------------------
GUID PhonebookEntry::bookEntryGuid() const
{
	return mEntry->guidId;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setBookEntryGuid(const GUID & aGuid)
{
	mEntry->guidId = aGuid;
}

//--------------------------------------------------------------------------------
std::wstring PhonebookEntry::customDialDll() const
{
	return mEntry->szCustomDialDll;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setCustomDialDll(const std::wstring & aDll)
{
	std::memset(mEntry->szCustomDialDll, 0, sizeof(mEntry->szCustomDialDll));
	aDll.copy(mEntry->szCustomDialDll, aDll.size());
}

//--------------------------------------------------------------------------------
EVpnStrategy::Enum PhonebookEntry::vpnStrategy() const
{
	return EVpnStrategy::Enum(mEntry->dwVpnStrategy);
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setVpnStrategy(EVpnStrategy::Enum aStrategy)
{
	mEntry->dwVpnStrategy = aStrategy;
}

//--------------------------------------------------------------------------------
EConnectionOption2::OptionSet PhonebookEntry::options2() const
{
	return EConnectionOption2::OptionSet(mEntry->dwfOptions2);
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setOptions2(EConnectionOption2::OptionSet aOptions)
{
	mEntry->dwfOptions2 = aOptions;
}

//--------------------------------------------------------------------------------
std::wstring PhonebookEntry::dnsSuffix() const
{
	return mEntry->szDnsSuffix;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setDnsSuffix(const std::wstring & aSuffix)
{
	std::memset(mEntry->szDnsSuffix, 0, sizeof(mEntry->szDnsSuffix));
	aSuffix.copy(mEntry->szDnsSuffix, aSuffix.size());
}

//--------------------------------------------------------------------------------
size_t PhonebookEntry::tcpWindowSize() const 
{
	return mEntry->dwTcpWindowSize;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setTcpWindowSize(size_t aSize)
{
	mEntry->dwTcpWindowSize = aSize;
}

//--------------------------------------------------------------------------------
std::wstring PhonebookEntry::prerequisitePhonebook() const
{
	return mEntry->szPrerequisitePbk;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setPrerequisitePhonebook(const std::wstring & aPhonebook)
{
	std::memset(mEntry->szPrerequisitePbk, 0, sizeof(mEntry->szPrerequisitePbk));
	aPhonebook.copy(mEntry->szPrerequisitePbk, aPhonebook.size());
}

//--------------------------------------------------------------------------------
std::wstring PhonebookEntry::prerequisiteEntry() const
{
	return mEntry->szPrerequisiteEntry;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setPrerequisiteEntry(const std::wstring & aEntry)
{
	std::memset(mEntry->szPrerequisiteEntry, 0, sizeof(mEntry->szPrerequisiteEntry));
	aEntry.copy(mEntry->szPrerequisiteEntry, aEntry.size());
}

//--------------------------------------------------------------------------------
size_t PhonebookEntry::redialCount() const
{
	return mEntry->dwRedialCount;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setRedialCount(size_t aCount)
{
	mEntry->dwRedialCount = aCount;
}

//--------------------------------------------------------------------------------
size_t PhonebookEntry::redialPause() const
{
	return mEntry->dwRedialPause;
}

//--------------------------------------------------------------------------------
void PhonebookEntry::setRedialPause(size_t aPause)
{
	mEntry->dwRedialPause = aPause;
}

//------------------------------------------------------------------------------
Connection::Connection() 
{
	std::memset(&mConnection, 0, sizeof(mConnection));
	mConnection.dwSize = sizeof(mConnection);
}

//--------------------------------------------------------------------------------
Connection::Connection(const RASCONN & aConnection)
{
	mConnection = aConnection;
}

//--------------------------------------------------------------------------------
void Connection::reset(const RASCONN & aConnection)
{
	mConnection = aConnection;
}

//--------------------------------------------------------------------------------
Connection::operator RASCONN * ()
{
	return &mConnection;
}

//--------------------------------------------------------------------------------
HRASCONN Connection::handle() const
{
	return mConnection.hrasconn;
}

//--------------------------------------------------------------------------------
void Connection::setHandle(HRASCONN aHandle)
{
	mConnection.hrasconn = aHandle;
}

//--------------------------------------------------------------------------------
std::wstring Connection::entryName() const
{
	return mConnection.szEntryName;
}

//--------------------------------------------------------------------------------
void Connection::setEntryName(const std::wstring & aName)
{
	std::memset(mConnection.szEntryName, 0, sizeof(mConnection.szEntryName));
	aName.copy(mConnection.szEntryName, aName.size());
}

//--------------------------------------------------------------------------------
EDeviceType::Enum Connection::deviceType() const
{
	return EDeviceType::ToEnum(mConnection.szDeviceType);
}

//--------------------------------------------------------------------------------
void Connection::setDeviceType(EDeviceType::Enum aType)
{
	std::wstring typeStr = EDeviceType::ToString(aType);
	std::memset(mConnection.szDeviceType, 0, sizeof(mConnection.szDeviceType));
	typeStr.copy(mConnection.szDeviceType, typeStr.size());
}

//--------------------------------------------------------------------------------
std::wstring Connection::deviceName() const
{
	return mConnection.szDeviceName;
}

//--------------------------------------------------------------------------------
void Connection::setDeviceName(const std::wstring & aName)
{
	std::memset(mConnection.szDeviceName, 0, sizeof(mConnection.szDeviceName));
	aName.copy(mConnection.szDeviceName, aName.size());
}

//--------------------------------------------------------------------------------
std::wstring Connection::phonebookPath() const
{
	return mConnection.szPhonebook;
}

//--------------------------------------------------------------------------------
void Connection::setPhonebookPath(const std::wstring & aPath)
{
	std::memset(mConnection.szPhonebook, 0, sizeof(mConnection.szPhonebook));
	aPath.copy(mConnection.szPhonebook, aPath.size());
}

//--------------------------------------------------------------------------------
size_t Connection::subEntryIndex() const
{
	return mConnection.dwSubEntry;
}

//--------------------------------------------------------------------------------
void Connection::setSubEntryIndex(size_t aIndex)
{
	mConnection.dwSubEntry = aIndex;
}

//--------------------------------------------------------------------------------
GUID Connection::entryGuid() const
{
	return mConnection.guidEntry;
}

//--------------------------------------------------------------------------------
void Connection::setEntryGuid(const GUID & aGuid)
{
	mConnection.guidEntry = aGuid;
}

//--------------------------------------------------------------------------------
bool Connection::isSystem() const
{
	return mConnection.dwFlags & EConnectionFlag::AllUsers;
}

//--------------------------------------------------------------------------------
void Connection::setIsSystem(bool aIsSystem)
{
	if (aIsSystem) 
	{
		mConnection.dwFlags |= EConnectionFlag::AllUsers;
	} 
	else 
	{
		mConnection.dwFlags &= ~EConnectionFlag::AllUsers;
	}
}

//--------------------------------------------------------------------------------
bool Connection::isGlobalCredsUsed() const
{
	return (mConnection.dwFlags & EConnectionFlag::GlobalCreds) != 0;
}

//--------------------------------------------------------------------------------
void Connection::setIsGlobalCredsUsed(bool aIsGlobalCredsUsed)
{
	if (aIsGlobalCredsUsed) 
	{
		mConnection.dwFlags |= EConnectionFlag::GlobalCreds;
	} 
	else 
	{
		mConnection.dwFlags &= ~EConnectionFlag::GlobalCreds;
	}
}

//--------------------------------------------------------------------------------
LUID Connection::localSessionId() const
{
	return mConnection.luid;
}

//--------------------------------------------------------------------------------
void Connection::setLocalSessionId(const LUID & aId)
{
	mConnection.luid = aId;
}

//------------------------------------------------------------------------------
Device::Device()
{
	std::memset(&mDevice, 0, sizeof(mDevice));
	mDevice.dwSize = sizeof(mDevice);
}

//--------------------------------------------------------------------------------
Device::Device(const RASDEVINFO & aDevice)
{
	mDevice = aDevice;
}

//--------------------------------------------------------------------------------
Device::operator RASDEVINFO * ()
{
	return &mDevice;
}

//--------------------------------------------------------------------------------
std::wstring Device::type() const
{
	return mDevice.szDeviceType;
}

//--------------------------------------------------------------------------------
void Device::setType(const std::wstring & aType)
{
	std::memset(mDevice.szDeviceType, 0, sizeof(mDevice.szDeviceType));
	aType.copy(mDevice.szDeviceType, aType.size());
}

//--------------------------------------------------------------------------------
std::wstring Device::name() const
{
	return mDevice.szDeviceName;
}

//--------------------------------------------------------------------------------
void Device::setName(const std::wstring & aName)
{
	std::memset(mDevice.szDeviceName, 0, sizeof(mDevice.szDeviceName));
	aName.copy(mDevice.szDeviceName, aName.size());
}

//------------------------------------------------------------------------------
DialParams::DialParams()
{
	std::memset(&mParams, 0, sizeof(mParams));
	mParams.dwSize = sizeof(mParams);
	mHasSavedPassword = false;
	mRemovePassword = false;
}

//--------------------------------------------------------------------------------
DialParams::DialParams(const RASDIALPARAMS & aParams)
{
	mParams = aParams;
	mHasSavedPassword = false;
	mRemovePassword = false;
}

//--------------------------------------------------------------------------------
DialParams::operator RASDIALPARAMS * ()
{
	return &mParams;
}

//--------------------------------------------------------------------------------
std::wstring DialParams::entryName() const
{
	return mParams.szEntryName;
}

//--------------------------------------------------------------------------------
void DialParams::setEntryName(const std::wstring & aName)
{
	std::memset(mParams.szEntryName, 0, sizeof(mParams.szEntryName));
	aName.copy(mParams.szEntryName, sizeof(mParams.szEntryName));
}

//--------------------------------------------------------------------------------
std::wstring DialParams::phoneNumber() const
{
	return mParams.szPhoneNumber;
}

//--------------------------------------------------------------------------------
void DialParams::setPhoneNumber(const std::wstring & aNumber)
{
	std::memset(mParams.szPhoneNumber, 0, sizeof(mParams.szPhoneNumber));
	aNumber.copy(mParams.szPhoneNumber, sizeof(mParams.szPhoneNumber));
}

//--------------------------------------------------------------------------------
std::wstring DialParams::callbackNumber() const
{
	return mParams.szCallbackNumber;
}

//--------------------------------------------------------------------------------
void DialParams::setCallbackNumber(const std::wstring & aNumber)
{
	std::memset(mParams.szCallbackNumber, 0, sizeof(mParams.szCallbackNumber));
	aNumber.copy(mParams.szCallbackNumber, sizeof(mParams.szCallbackNumber));
}

//--------------------------------------------------------------------------------
std::wstring DialParams::userName() const
{
	return mParams.szUserName;
}

//--------------------------------------------------------------------------------
void DialParams::setUserName(const std::wstring & aName)
{
	std::memset(mParams.szUserName, 0, sizeof(mParams.szUserName));
	aName.copy(mParams.szUserName, sizeof(mParams.szUserName));
}

//--------------------------------------------------------------------------------
std::wstring DialParams::password() const
{
	return mParams.szPassword;
}

//--------------------------------------------------------------------------------
void DialParams::setPassword(const std::wstring & aPassword)
{
	std::memset(mParams.szPassword, 0, sizeof(mParams.szPassword));
	aPassword.copy(mParams.szPassword, sizeof(mParams.szPassword));
}

//--------------------------------------------------------------------------------
std::wstring DialParams::domain() const
{
	return mParams.szDomain;
}

//--------------------------------------------------------------------------------
void DialParams::setDomain(const std::wstring & aDomain)
{
	std::memset(mParams.szDomain, 0, sizeof(mParams.szDomain));
	aDomain.copy(mParams.szDomain, sizeof(mParams.szDomain));
}

//--------------------------------------------------------------------------------
unsigned int DialParams::subEntry() const
{
	return mParams.dwSubEntry;
}

//--------------------------------------------------------------------------------
void DialParams::setSubEntry(unsigned int aIndex)
{
	mParams.dwSubEntry = aIndex;
}

//--------------------------------------------------------------------------------
unsigned long DialParams::callbackId() const
{
	return mParams.dwCallbackId;
}

//--------------------------------------------------------------------------------
void DialParams::setCallbackId(unsigned long aValue)
{
	mParams.dwCallbackId = aValue;
}

//--------------------------------------------------------------------------------
bool DialParams::hasSavedPassword() const
{
	return mHasSavedPassword;
}

//--------------------------------------------------------------------------------
void DialParams::setHasSavedPassword(bool aHasPassword)
{
	mHasSavedPassword = aHasPassword;
}

//--------------------------------------------------------------------------------
bool DialParams::removePassword() const
{
	return mRemovePassword;
}

//--------------------------------------------------------------------------------
void DialParams::setRemovePassword(bool aRemove)
{
	mRemovePassword = aRemove;
}

//------------------------------------------------------------------------------
PhonebookEntryEnumerator::PhonebookEntryEnumerator(const std::wstring & aPhonebookPath)
	: mRequestedBufSize(0), mEntries(0)
{
	reset(aPhonebookPath);
}

//------------------------------------------------------------------------------
PhonebookEntryEnumerator::~PhonebookEntryEnumerator()
{
	delete[] mEntries;
}

//------------------------------------------------------------------------------
bool PhonebookEntryEnumerator::getEntry(PhonebookEntryName & aEntry)
{
	if (mCurrentIndex < mEntryCount)
	{
		aEntry = mEntries[mCurrentIndex++];
		return true;
	}

	return false;
}

//------------------------------------------------------------------------------
void PhonebookEntryEnumerator::reset(const std::wstring & aPhonebookPath)
{
	mCurrentIndex = 0;
	mEntryCount = 0;
	mRequestedBufSize = 0;
	RASENTRYNAME dummy;
	dummy.dwSize = sizeof(dummy);

	mLastError = RasEnumEntries(0, 0, 
		GetOSVersion() < EOSVersion::WindowsVista ? &dummy : 0, // различие(баг) в апи
		&mRequestedBufSize, &mEntryCount);

	if (mLastError == ERROR_BUFFER_TOO_SMALL)
	{
		delete[] mEntries;
		mEntries = reinterpret_cast<LPRASENTRYNAME>(new char[mRequestedBufSize]);
		mEntries[0].dwSize = sizeof(mEntries[0]);
		mLastError = RasEnumEntries(0, aPhonebookPath.empty() ? 0 : aPhonebookPath.data(), 
			 mEntries, &mRequestedBufSize, &mEntryCount);
	}
}

//------------------------------------------------------------------------------
ConnectionEnumerator::ConnectionEnumerator()
	: mRequestedBufSize(0), mConnections(0)
{
	reset();
}

//--------------------------------------------------------------------------------
ConnectionEnumerator::~ConnectionEnumerator()
{
	delete[] mConnections;
}

//--------------------------------------------------------------------------------
bool ConnectionEnumerator::getConnection(Connection & aConnection)
{
	if (mCurrentIndex < mConnectionCount)
	{
		aConnection.reset(mConnections[mCurrentIndex++]);
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------
void ConnectionEnumerator::reset()
{
	mCurrentIndex = 0;
	mConnectionCount = 0;
	DWORD connectionsCount = 0;
	DWORD reqBytesCount = 0;

	RASCONN dummy;
	dummy.dwSize = sizeof(dummy);

	mLastError = RasEnumConnections(
		GetOSVersion() < EOSVersion::WindowsVista ? &dummy : 0, // различие(баг) в апи
		&reqBytesCount, &connectionsCount);

	mRequestedBufSize = reqBytesCount;
	mConnectionCount = connectionsCount;

	if (mLastError == ERROR_BUFFER_TOO_SMALL)
	{
		delete[] mConnections;
		mConnections = reinterpret_cast<LPRASCONN>(new char[mRequestedBufSize]);
		mConnections[0].dwSize = sizeof(mConnections[0]);
		mLastError = RasEnumConnections(mConnections, &mRequestedBufSize, &mConnectionCount);
	}
}

//------------------------------------------------------------------------------
DeviceEnumerator::DeviceEnumerator()
	: mRequestedBufSize(0), mDevices(0)
{
	reset();
}

//--------------------------------------------------------------------------------
DeviceEnumerator::~DeviceEnumerator()
{
	delete[] mDevices;
}

//--------------------------------------------------------------------------------
bool DeviceEnumerator::getDevice(Device & aDevice)
{
	if (mCurrentIndex < mDeviceCount)
	{
		aDevice = mDevices[mCurrentIndex++];

		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------
void DeviceEnumerator::reset()
{
	mCurrentIndex = 0;
	mDeviceCount = 0;

	mLastError = RasEnumDevices(0, &mRequestedBufSize, &mDeviceCount);

	if (mLastError == ERROR_BUFFER_TOO_SMALL && mDeviceCount)
	{
		delete[] mDevices;
		mDevices = new RASDEVINFO[mRequestedBufSize/mDeviceCount];
		mDevices[0].dwSize = sizeof(mDevices[0]);
		mLastError = RasEnumDevices(mDevices, &mRequestedBufSize, &mDeviceCount);
	}
}

//------------------------------------------------------------------------------
DWORD ValidatePhonebookEntryName(PhonebookEntryName & aEntry) 
{
	DWORD error = RasValidateEntryName(
		aEntry.phonebookPath().empty() ? 0 : aEntry.phonebookPath().data(), aEntry.name().data());

	return EErrorCode::Enum(error);
}

//------------------------------------------------------------------------------
DWORD CreateNewPhonebookEntry(const PhonebookEntryName & aEntryName, PhonebookEntry & aEntry)
{
	DWORD size = 0;

	DWORD error = RasGetEntryProperties(0, 0, 0, &size, 0, 0);

	if (error == ERROR_BUFFER_TOO_SMALL)
	{
		error = RasSetEntryProperties(aEntryName.phonebookPath().empty() ? 0 : aEntryName.phonebookPath().data(), 
			aEntryName.name().data(), (RASENTRY *)aEntry, size, 0, 0);
	}

	return error;
}

//------------------------------------------------------------------------------
DWORD RemovePhonebookEntry(const PhonebookEntryName & aEntryName)
{
	DWORD size = 0;

	DWORD error = RasGetEntryProperties(0, 0, 0, &size, 0, 0);

	if (error == ERROR_BUFFER_TOO_SMALL)
	{
		error = RasDeleteEntry(aEntryName.phonebookPath().empty() ? 0 : aEntryName.phonebookPath().data(), aEntryName.name().data());
	}

	return error;
}

//------------------------------------------------------------------------------
DWORD GetConnectionStatus(const std::wstring & aConnectionName, EConnectionStatus::Enum & aStatus)
{
	DWORD error = ERROR_SUCCESS;
	Connection conn;
	ConnectionEnumerator enumerator;

	if (enumerator.isValid())
	{
		bool found = false;

		while (enumerator.getConnection(conn))
		{
			if (conn.entryName() == aConnectionName)
			{
				found = true;
				break;
			}
		}

		if (found)
		{
			RASCONNSTATUS rasstatus;
			rasstatus.dwSize = sizeof(RASCONNSTATUS);
			error = RasGetConnectStatus(conn.handle(), &rasstatus);

			if (error == ERROR_SUCCESS && rasstatus.rasconnstate == RASCS_Connected)
			{
				aStatus = EConnectionStatus::Connected;
			}
		}
		else
		{
			aStatus = EConnectionStatus::Disconnected;
		}
	}
	else
	{
		error = enumerator.getLastError();
	}

	return error;
}

//------------------------------------------------------------------------------
DWORD GetEntryProperties(const PhonebookEntryName & aEntryName, PhonebookEntry & aEntry)
{
	DWORD size = sizeof(*(RASENTRY *)aEntry);

	DWORD error = RasGetEntryProperties(
		aEntryName.phonebookPath().empty() ? 0 : aEntryName.phonebookPath().data(), 
		aEntryName.name().data(), 
		(RASENTRY *)aEntry, &size, 0, 0);

	return error;
}

//------------------------------------------------------------------------------
DWORD GetEntryDialParams(const PhonebookEntryName & aEntryName, DialParams & aParameters)
{
	aParameters.setEntryName(aEntryName.name());

	BOOL hasSavedPassword = FALSE;

	DWORD error = RasGetEntryDialParams(
		aEntryName.phonebookPath().empty() ? 0 : aEntryName.phonebookPath().data(), 
		static_cast<RASDIALPARAMS *>(aParameters), &hasSavedPassword);

	if (error == ERROR_SUCCESS)
	{
		aParameters.setHasSavedPassword(hasSavedPassword == TRUE);
	}

	return error;
}

//------------------------------------------------------------------------------
DWORD SetEntryDialParams(const PhonebookEntryName & aEntryName, DialParams & aParams)
{
	aParams.setEntryName(aEntryName.name());

	DWORD error = RasSetEntryDialParams(
		aEntryName.phonebookPath().empty() ? 0 : aEntryName.phonebookPath().data(), 
		static_cast<RASDIALPARAMS *>(aParams), aParams.removePassword());

	return error;
}

//------------------------------------------------------------------------------
DWORD Dial(const PhonebookEntryName & aEntryName)
{
	DWORD error = ERROR_SUCCESS;

	// Проверяем наличие соединения
	PhonebookEntryName pbentry;
	PhonebookEntryEnumerator pbenum(aEntryName.phonebookPath());

	if (pbenum.isValid())
	{
		bool found = false;

		while (pbenum.getEntry(pbentry))
		{
			if (pbentry.name() == aEntryName.name())
			{
				found = true;
				break;
			}
		}

		if (found)
		{
			// Загружаем сохранённый пароль
			DialParams dialParams;
			error = GetEntryDialParams(pbentry, dialParams);
			
			if (error == ERROR_SUCCESS)
			{
				// Устанавливаем номер для дозвона
				PhonebookEntry entry;
				error = GetEntryProperties(pbentry, entry);

				if (error == ERROR_SUCCESS)
				{
					dialParams.setPhoneNumber(entry.localPhoneNumber());

					HRASCONN handle = 0;

					// Устанавливаем соединение
					error = RasDial(0, pbentry.phonebookPath().empty() ? 0 : pbentry.phonebookPath().data(), 
						static_cast<RASDIALPARAMS *>(dialParams), 0, 0, &handle);

					if (error != ERROR_SUCCESS && handle)
					{
						RasHangUp(handle);
					}
				}
			}
		}
		else
		{
			error = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
		}
	}
	else
	{
		error = pbenum.getLastError();
	}

	return error;
}

//------------------------------------------------------------------------------
DWORD HangUp(const std::wstring & aConnectionName)
{
	DWORD error;
	Connection conn;
	ConnectionEnumerator enumerator;
	
	if (enumerator.isValid())
	{
		bool found = false;

		while (enumerator.getConnection(conn))
		{
			if (conn.entryName() == aConnectionName)
			{
				found = true;
				break;
			}
		}

		if (found)
		{
			EConnectionStatus::Enum status;
			error = GetConnectionStatus(aConnectionName, status);

			if (error == ERROR_SUCCESS && status == EConnectionStatus::Connected)
			{
				error = RasHangUp(conn.handle());
			}
		}
		else
		{
			error = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
		}
	}
	else
	{
		error = enumerator.getLastError();
	}

	return error;
}

std::wstring getRegValue(HKEY aKey, const std::wstring & aValueName)
{
	wchar_t value[MAX_PATH] = {'\0'};
	DWORD len = sizeof(value);

	DWORD type;
	if (RegQueryValueEx(aKey, aValueName.c_str(), 0, &type, (LPBYTE)value, &len) == ERROR_SUCCESS)
	{
		switch (type)
		{
		case REG_SZ:
			return std::wstring(value);
		}
	}

	return std::wstring();
}

std::wstring getAttachedTo(const std::wstring & aDeviceName)
{
	std::wstring result;

	for (int index = 0; index < 20 && result.empty(); ++index)
	{
		wchar_t regPath[MAX_PATH] = {'\0'};
		swprintf(regPath, MAX_PATH, L"SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E96D-E325-11CE-BFC1-08002BE10318}\\%.4d", index);

		HKEY modem;
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, regPath, NULL, KEY_READ, &modem) == ERROR_SUCCESS)
		{
			if (getRegValue(modem, L"FriendlyName") == aDeviceName ||
				getRegValue(modem, L"DriverDesc") == aDeviceName || 
				getRegValue(modem, L"Model") == aDeviceName)
			{
				result = getRegValue(modem, L"AttachedTo");
			}

			RegCloseKey(modem);
		}
	}

	return result;
}

} // namespace RasApi
