/* @file Обёртка RAS API WIN32 */

#pragma once

// STL
#include <string>

// Windows
// Минимальная версия необходимая для работы с RAS API - WindowsXP
#define OLD_WINVER WINVER
#if defined(WINVER)
#undef WINVER
#endif
#define WINVER 0x501
#include <windows.h>
#include <ras.h>
#include <raserror.h>
#include <wininet.h>
#undef WINVER
#define WINVER OLD_WINVER
#undef OLD_WINVER

namespace RasApi 
{

//--------------------------------------------------------------------------------
/// Версии Windows для управления различиями в API.
namespace EOSVersion
{
	enum Enum 
	{
		Unknown = 0,
		Windows2000,
		WindowsXP,
		Windows2003,
		WindowsVista,
		Windows7,
		Windows8,
		Windows81,
		Windows10
	};
}

//--------------------------------------------------------------------------------
/// Свойства соединения для RASENTRY::dwfOptions.
namespace EConnectionOption 
{
	enum Enum 
	{
		UseCountryAndAreaCodes = RASEO_UseCountryAndAreaCodes,
		SpecificIpAddr         = RASEO_SpecificIpAddr,
		SpecificNameServers    = RASEO_SpecificNameServers,
		IpHeaderCompression    = RASEO_IpHeaderCompression,
		RemoteDefaultGateway   = RASEO_RemoteDefaultGateway,
		DisableLcpExtensions   = RASEO_DisableLcpExtensions,
		TerminalBeforeDial     = RASEO_TerminalBeforeDial,
		TerminalAfterDial      = RASEO_TerminalAfterDial, 
		ModemLights            = RASEO_ModemLights,
		SwCompression          = RASEO_SwCompression,
		RequireEncryptedPw     = RASEO_RequireEncryptedPw,
		RequireMsEncryptedPw   = RASEO_RequireMsEncryptedPw,
		RequireDataEncryption  = RASEO_RequireDataEncryption,
		NetworkLogon           = RASEO_NetworkLogon,
		UseLogonCredentials    = RASEO_UseLogonCredentials,
		PromoteAlternates      = RASEO_PromoteAlternates,
		SecureLocalFiles       = RASEO_SecureLocalFiles,
		RequireEAP             = RASEO_RequireEAP,
		RequirePAP             = RASEO_RequirePAP,
		RequireSPAP            = RASEO_RequireSPAP,
		Custom                 = RASEO_Custom,
		PreviewPhoneNumber     = RASEO_PreviewPhoneNumber,
		SharedPhoneNumbers     = RASEO_SharedPhoneNumbers,
		PreviewUserPw          = RASEO_PreviewUserPw,
		PreviewDomain          = RASEO_PreviewDomain,
		ShowDialingProgress    = RASEO_ShowDialingProgress,
		RequireCHAP            = RASEO_RequireCHAP,
		RequireMsCHAP          = RASEO_RequireMsCHAP,
		RequireMsCHAP2         = RASEO_RequireMsCHAP2,
		RequireW95MSCHAP       = RASEO_RequireW95MSCHAP,
		CustomScript           = RASEO_CustomScript
	};

	typedef DWORD OptionSet;
}

//--------------------------------------------------------------------------------
/// Свойства соединения для RASENTRY::dwfOptions2.
namespace EConnectionOption2
{
	enum Enum 
	{
		SecureFileAndPrint          = RASEO2_SecureFileAndPrint,
		SecureClientForMSNet        = RASEO2_SecureClientForMSNet,
		DontNegotiateMultilink      = RASEO2_DontNegotiateMultilink,
		DontUseRasCredentials       = RASEO2_DontUseRasCredentials,
		UsePreSharedKey             = RASEO2_UsePreSharedKey,
		Internet                    = RASEO2_Internet,
		DisableNbtOverIP            = RASEO2_DisableNbtOverIP,
		UseGlobalDeviceSettings     = RASEO2_UseGlobalDeviceSettings,
		ReconnectIfDropped          = RASEO2_ReconnectIfDropped,
		SharePhoneNumbers           = RASEO2_SharePhoneNumbers,
		/* эти значения только для Висты и выше
		SecureRoutingCompartment    = RASEO2_SecureRoutingCompartment,
		IPv6SpecificNameServer      = RASEO2_IPv6SpecificNameServers,
		IPv6RemoteDefaultGateway    = RASEO2_IPv6RemoteDefaultGateway,
		RegisterIpWithDNS           = RASEO2_RegisterIpWithDNS,
		UseDNSSuffixForRegistration = RASEO2_UseDNSSuffixForRegistration,
		IPv4ExplicitMetric          = RASEO2_IPv4ExplicitMetric,
		IPv6ExplicitMetric          = RASEO2_IPv6ExplicitMetric,
		DisableIKENameEkuCheck      = RASEO2_DisableIKENameEkuCheck
		*/
		/* эти значения только для семёрки и выше
		DisableClassBasedStaticRoute = RASEO2_DisableClassBasedStaticRoute,
		SpecificIPv6Addr             = RASEO2_SpecificIPv6Addr,
		DisableMobility              = RASEO2_DisableMobility,
		RequireMachineCertificates   = RASEO2_RequireMachineCertificates
		*/
	};

	typedef DWORD OptionSet;
}

//--------------------------------------------------------------------------------
/// Свойства соединения для RASCONN::dwFlags.
namespace EConnectionFlag
{
	enum Enum 
	{
		AllUsers    = RASCF_AllUsers,
		GlobalCreds = RASCF_GlobalCreds
	};

	typedef size_t FlagSet;
}

//--------------------------------------------------------------------------------
/// Тип устройства для RASDEVINFO::szDeviceType.
namespace EDeviceType
{
	enum Enum 
	{
		Unknown,
		Modem,
		Isdn,
		X25,
		Vpn,
		Pad,
		Generic,
		Serial,
		FrameRelay,
		Atm,
		Sonet,
		SW56,
		Irda,
		Parallel,
		PPPoE
	};

	std::wstring ToString(Enum type);
	Enum ToEnum(const std::wstring &type);
}

//--------------------------------------------------------------------------------
/// Тип протокола для RASENTRY::dwProtocols.
namespace ENetworkProtocol
{
	enum Enum 
	{
		Ipx     = RASNP_Ipx,
		Ip      = RASNP_Ip,
		/* эти значения только для Висты и выше
		IpV6    = RASNP_Ipv6
		*/
	};

	typedef DWORD ProtocolSet;
}

//--------------------------------------------------------------------------------
/// Тип framing protocol для RASENTRY::dwFramingProtocol.
namespace EFramingProtocol
{
	enum Enum 
	{
		Ppp  = RASFP_Ppp,
		Slip = RASFP_Slip
	};
}

//--------------------------------------------------------------------------------
namespace EPhonebookEntry
{
	/// Тип телефонной книги для RASENTRYNAME::dwFlags.
	enum PhonebookTypeEnum 
	{
		AllUsers = REN_AllUsers,
		Private  = REN_User
	};

	/// Тип записи телефонной книги для RASENTRY::dwType.
	enum TypeEnum 
	{
		Phone     = RASET_Phone,
		Vpn       = RASET_Vpn,
		Internet  = RASET_Internet,
		Broadband = RASET_Broadband
	};
}

//--------------------------------------------------------------------------------
/// Способ набора multilink записей для RASENTRY::dwDialMode.
namespace EDialMode
{
	enum Enum
	{
		DialAll      = RASEDM_DialAll,
		DialAsNeeded = RASEDM_DialAsNeeded
	};
}

//--------------------------------------------------------------------------------
/// Тип шифрования для RASENTRY::dwEncryptionType.
namespace EEncryptionType
{
	enum Enum
	{
		None       = ET_None,
		Require    = ET_Require,
		RequireMax = ET_RequireMax,
		Optional   = ET_Optional
	};
}

//--------------------------------------------------------------------------------
/// Свойства VPN для RASENTRY::dwVpnStrategy.
namespace EVpnStrategy 
{
	enum Enum
	{
		Default    = VS_Default,
		PptpOnly   = VS_PptpOnly,
		PptpFirst  = VS_PptpFirst,
		L2tpOnly   = VS_L2tpOnly,
		L2tpFirst  = VS_L2tpFirst,
		/* эти значения только для Висты и выше
		SstpOnly   = VS_SstpOnly,
		SstpFirst  = VS_SstpFirst,
		*/
		/* эти значения только для семёрки и выше
		Ikev2Only  = VS_Ikev2Only,
		Ikev2First = VS_Ikev2First
		*/
	};
}

//--------------------------------------------------------------------------------
/// Коды ошибок, возвращаемые RAS API.
namespace EErrorCode
{
	enum Enum
	{
		NoError             = ERROR_SUCCESS,
		InvalidName         = ERROR_INVALID_NAME,
		InvalidBuffer       = ERROR_BUFFER_INVALID,
		CannotOpenPhonebook = ERROR_CANNOT_OPEN_PHONEBOOK,
		InvalidParameter    = ERROR_INVALID_PARAMETER,
		AlreadyExists       = ERROR_ALREADY_EXISTS
	};

	std::wstring ToString(DWORD aCode);
}

//--------------------------------------------------------------------------------
/// Статус соединения для RASCONNSTATUS::rasconnstate.
namespace EConnectionStatus
{
	enum Enum
	{
		Connected    = RASCS_Connected,
		Disconnected = RASCS_Disconnected
	};
}

//--------------------------------------------------------------------------------
/// Время простоя соединения перед разрывом для RASENTRY::dwIdleDisconnectSeconds.
namespace EIdleDisconnect
{
	enum Enum
	{
		Disabled       = RASIDS_Disabled,
		UseGlobalValue = RASIDS_UseGlobalValue
	};
}

//------------------------------------------------------------------------------
// База для классов, возвращающих результат операции
class RasBase
{
public:
	DWORD getLastError() const { return mLastError; }
	bool isValid() const { return mLastError == ERROR_SUCCESS; }

protected:
	DWORD mLastError;
};

//------------------------------------------------------------------------------
/// IP-адрес.
class IpAddress
{
public:
	IpAddress();
	IpAddress(const RASIPADDR & aIpAddress);
	
	operator RASIPADDR* ();
	operator const RASIPADDR * () const;

	char byte(size_t size_t) const;
	void setByte(size_t size_t, char byte);

private:
	RASIPADDR  mAddress;
};

//------------------------------------------------------------------------------
/// Элемент телефонной книги.
class PhonebookEntryName
{
public:
	PhonebookEntryName();
	PhonebookEntryName(const RASENTRYNAME & aEntry);

	operator RASENTRYNAME * ();

	std::wstring name() const;
	void setName(const std::wstring & aName);

	std::wstring phonebookPath() const;
	void setPhonebookPath(const std::wstring & aPath);

	bool isSystem() const;
	void setIsSystem(bool aIsSystem);

private:
	RASENTRYNAME mEntry;
};

//------------------------------------------------------------------------------
/// Параметры элемента телефонной книги.
class PhonebookEntry : public RasBase
{
public:
	PhonebookEntry();
	~PhonebookEntry();
//	PhonebookEntry(const RASENTRY & aEntry);

	operator RASENTRY * ();

	EConnectionOption::OptionSet options() const;
	void setOptions(EConnectionOption::OptionSet aOptions);

	// Location/phone number.
	size_t countryId() const;
	void setCountryId(size_t aId);

	size_t countryCode() const;
	void setCountryCode(size_t aCode);

	std::wstring areaCode() const;
	void setAreaCode(const std::wstring & aCode);

	std::wstring localPhoneNumber() const;
	void setLocalPhoneNumber(const std::wstring & aNumber);

	// PPP/Ip
	IpAddress ip() const;
	void setIp(const IpAddress & aIp);

	IpAddress dnsIp() const;
	void setDnsIp(const IpAddress & aIp);
	
	IpAddress dnsAltIp() const;
	void setDnsAltIp(const IpAddress & aIp);

	IpAddress winsIp() const;
	void setWinsIp(const IpAddress & aIp);

	IpAddress winsAltIp() const;
	void setWinsAltIp(const IpAddress & aIp);
	
	// Framing
	size_t frameSize() const;
	void setFrameSize(size_t aSize);

	ENetworkProtocol::ProtocolSet netProtocols() const;
	void setNetProtocols(ENetworkProtocol::ProtocolSet aProtocols);

	EFramingProtocol::Enum framingProtocol() const;
	void setFramingProtocol(EFramingProtocol::Enum aProtocol);

	// Scripting
	std::wstring script() const;
	void setScript(const std::wstring & aScript);

	// Device
	EDeviceType::Enum deviceType() const;
	void setDeviceType(EDeviceType::Enum aType);

	std::wstring deviceName() const;
	void setDeviceName(const std::wstring & aName);

	// Multilink and BAP
	size_t subEntries() const;
	void setSubEntries(size_t aEntries);

	EDialMode::Enum dialMode() const;
	void setDialMode(EDialMode::Enum aMode);

	size_t dialExtraPercent() const;
	void setDialExtraPercent(size_t aPercent);

	size_t dialExtraSampleSeconds() const;
	void setDialExtraSampleSeconds(size_t aSeconds);

	size_t hangUpExtraPercent() const;
	void setHangUpExtraPercent(size_t aPercent);

	size_t hangUpExtraSampleSeconds() const;
	void setHangUpExtraSampleSeconds(size_t aSeconds);

	// Idle time out
	size_t idleDisconnectSeconds() const;
	void setIdleDisconnectSeconds(size_t aSeconds);

	EPhonebookEntry::TypeEnum phonebookEntryType() const;
	void setPhonebookEntryType(EPhonebookEntry::TypeEnum aType);

	EEncryptionType::Enum encriptionType() const;
	void setEncriptionType(EEncryptionType::Enum aType);

	size_t customAuthKey() const;
	void setCustomAuthKey(size_t aKey);

	GUID bookEntryGuid() const;
	void setBookEntryGuid(const GUID & aGuid);

	std::wstring customDialDll() const;
	void setCustomDialDll(const std::wstring & aDll);

	EVpnStrategy::Enum vpnStrategy() const;
	void setVpnStrategy(EVpnStrategy::Enum aStrategy);

	EConnectionOption2::OptionSet options2() const;
	void setOptions2(EConnectionOption2::OptionSet aOptions);

	std::wstring dnsSuffix() const;
	void setDnsSuffix(const std::wstring & aSuffix);

	size_t	tcpWindowSize() const;
	void setTcpWindowSize(size_t aSize);

	std::wstring prerequisitePhonebook() const;
	void setPrerequisitePhonebook(const std::wstring & aPhonebook);

	std::wstring prerequisiteEntry() const;
	void setPrerequisiteEntry(const std::wstring & aEntry);

	size_t redialCount() const;
	void setRedialCount(size_t aCount);

	size_t redialPause() const;
	void setRedialPause(size_t aPause);

private:
	LPRASENTRY mEntry;
};

//------------------------------------------------------------------------------
/// Dialup-соединение
class Connection
{
public:
	Connection();
	Connection(const RASCONN & aConnection);

	void reset(const RASCONN & aConnection);
	operator RASCONN * ();

	HRASCONN handle() const;
	void setHandle(HRASCONN aHandle);

	std::wstring entryName() const;
	void setEntryName(const std::wstring & aName);

	EDeviceType::Enum deviceType() const;
	void setDeviceType(EDeviceType::Enum aType);

	std::wstring deviceName() const;
	void setDeviceName(const std::wstring & aName);

	std::wstring phonebookPath() const;
	void setPhonebookPath(const std::wstring & aPath);

	size_t subEntryIndex() const;
	void setSubEntryIndex(size_t aIndex);

	GUID entryGuid() const;
	void setEntryGuid(const GUID & aGuid);

	bool isSystem() const;
	void setIsSystem(bool aIsSystem);

	bool isGlobalCredsUsed() const;
	void setIsGlobalCredsUsed(bool aIsGlobalCredsUsed);

	LUID localSessionId() const;
	void setLocalSessionId(const LUID & aId);

private:
	RASCONN mConnection;
};

//------------------------------------------------------------------------------
/// Сетевое устройство.
class Device
{
public:
	Device();
	Device(const RASDEVINFO & aDevice);

	operator RASDEVINFO * ();

	std::wstring type() const;
	void setType(const std::wstring & aType);

	std::wstring name() const;
	void setName(const std::wstring & aName);

private:
	RASDEVINFO mDevice;
};

//------------------------------------------------------------------------------
/// Параметры соединения.
class DialParams
{
public:
	DialParams();
	DialParams(const RASDIALPARAMS & aParams);

	operator RASDIALPARAMS * ();

	std::wstring entryName() const;
	void setEntryName(const std::wstring & aName);

	std::wstring phoneNumber() const;
	void setPhoneNumber(const std::wstring & aNumber);

	std::wstring callbackNumber() const;
	void setCallbackNumber(const std::wstring & aNumber);

	std::wstring userName() const;
	void setUserName(const std::wstring & aName);

	std::wstring password() const;
	void setPassword(const std::wstring & aPassword);

	std::wstring domain() const;
	void setDomain(const std::wstring & aDomain);

	unsigned int subEntry() const;
	void setSubEntry(unsigned int aIndex);

	unsigned long callbackId() const;
	void setCallbackId(unsigned long aValue);

	bool hasSavedPassword() const;
	void setHasSavedPassword(bool aHasPassword);

	bool removePassword() const;
	void setRemovePassword(bool aRemove);

private:
	RASDIALPARAMS mParams;
	bool mHasSavedPassword;
	bool mRemovePassword;
};

//------------------------------------------------------------------------------
/// Перечислитель элементов телефонной книги.
class PhonebookEntryEnumerator : public RasBase
{
public:
	PhonebookEntryEnumerator(const std::wstring & aPhonebookPath = L"");
	~PhonebookEntryEnumerator();
	
	bool getEntry(PhonebookEntryName & aEntry);
	void reset(const std::wstring & aPhonebookPath = L"");

private:
	DWORD          mEntryCount;
	size_t         mCurrentIndex;
	DWORD          mRequestedBufSize;
	LPRASENTRYNAME mEntries;
};

//------------------------------------------------------------------------------
/// Перечислитель сетевых соединений в системе.
class ConnectionEnumerator : public RasBase
{
public:
	ConnectionEnumerator();
	~ConnectionEnumerator();
	
	bool getConnection(Connection & aConnection);
	void reset();

private:
	DWORD     mConnectionCount;
	size_t    mCurrentIndex;
	DWORD     mRequestedBufSize;
	LPRASCONN mConnections;
};

//------------------------------------------------------------------------------
/// Перечислитель сетевых устройств в системе.
class DeviceEnumerator : public RasBase
{
public:
	DeviceEnumerator();
	~DeviceEnumerator();

	bool getDevice(Device & aDevice);
	void reset();

private:
	DWORD        mDeviceCount;
	size_t       mCurrentIndex;
	DWORD        mRequestedBufSize;
	LPRASDEVINFO mDevices;
};

//------------------------------------------------------------------------------
DWORD ValidatePhonebookEntryName(PhonebookEntryName & aEntry);

//------------------------------------------------------------------------------
DWORD CreateNewPhonebookEntry(const PhonebookEntryName & aEntryName, PhonebookEntry & aEntry);

//------------------------------------------------------------------------------
DWORD GetConnectionStatus(const std::wstring & aConnectionName, EConnectionStatus::Enum & aStatus);

//------------------------------------------------------------------------------
DWORD GetEntryProperties(const PhonebookEntryName & aEntryName, PhonebookEntry & aEntry);
	
//------------------------------------------------------------------------------
DWORD GetEntryDialParams(const PhonebookEntryName & aEntryName, DialParams & aParameters);

DWORD SetEntryDialParams(const PhonebookEntryName & aEntryName, DialParams & aParams);

//------------------------------------------------------------------------------
DWORD Dial(const PhonebookEntryName & aEntryName);

//------------------------------------------------------------------------------
DWORD HangUp(const std::wstring & aConnectionName);

//------------------------------------------------------------------------------
DWORD RemovePhonebookEntry(const PhonebookEntryName & aEntryName);

//------------------------------------------------------------------------------
std::wstring getAttachedTo(const std::wstring & aDeviceName);

//------------------------------------------------------------------------------

} // namespace RasApi 

//------------------------------------------------------------------------------
