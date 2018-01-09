#include <iostream>
#include <list>

#include "RasWrapper.h"

std::list<std::wstring> gConnections;
std::list<std::wstring> gDevices;

void testEnumPhonebookEntries()
{
	std::wcout << std::endl << "Enumerating phonebook entries:" << std::endl;

	RasApi::PhonebookEntryEnumerator e;

	if (e.isValid())
	{
		RasApi::PhonebookEntryName name;

		while (e.getEntry(name))
		{
			std::wcout 
				<< (name.isSystem() ? "SYSTEM " : "USER ")
				<< name.phonebookPath().c_str() << ": "
				<< name.name().c_str() << std::endl;
		}
	}
	else
	{
		std::wcout << "Failed to initialize PhonebookEntryEnumerator: " << e.getLastError() << std::endl;
	}
}

void testEnumDevices(RasApi::EDeviceType::Enum aType)
{
	std::wcout << std::endl << "Enumerating devices:" << std::endl;

	RasApi::DeviceEnumerator d;

	if (d.isValid())
	{
		RasApi::Device device;

		while (d.getDevice(device))
		{
			if (RasApi::EDeviceType::ToEnum(device.type()) == aType)
			{
				std::wcout 
					<< device.type().c_str() << ": "
					<< device.name().c_str() << std::endl;
			}

			if (RasApi::EDeviceType::ToEnum(device.type()) == RasApi::EDeviceType::Modem)
			{
				gDevices.push_back(device.name());
			}
		}
	}
	else
	{
		std::wcout << "Failed to initialize DeviceEnumerator: " << d.getLastError() << std::endl;
	}
}

void testEnumConnections()
{
	std::wcout << std::endl << "Enumerating connections:" << std::endl;

	RasApi::ConnectionEnumerator c;

	if (c.isValid())
	{
		RasApi::Connection cn;

		while (c.getConnection(cn))
		{
			gConnections.push_back(cn.entryName());

			std::wcout
				<< cn.entryName().c_str() 
				<< " on " << RasApi::EDeviceType::ToString(cn.deviceType()).c_str() << "(" << cn.deviceName().c_str() 
				<< "), GlobalCredsUsed=" << cn.isGlobalCredsUsed() << " System=" << cn.isSystem() << std::endl;
		}
	}
	else
	{
		std::wcout << "Failed to initialize ConnectionEnumerator: " << c.getLastError() << std::endl;
	}
}

void testConnectionStatus()
{
	std::wcout << std::endl << "Testing connection status:" << std::endl;

	gConnections.push_back(L"test_fake_connection");

	for (std::list<std::wstring>::iterator i = gConnections.begin(); i != gConnections.end(); ++i)
	{
		RasApi::EConnectionStatus::Enum status;
		RasApi::GetConnectionStatus(*i, status);

		std::wcout << *i << (status == RasApi::EConnectionStatus::Connected ? ": connected " : ": disconnected ") << std::endl;
	}
}

void testCreatePhonebookEntry()
{
	std::wcout << std::endl << "Testing connection creation..." << std::endl;

	if (gDevices.empty())
	{
		std::wcout << std::endl << "Cannot create, no modems available" << std::endl;
		return;
	}

	RasApi::PhonebookEntryName entryName;
	entryName.setIsSystem(false);
	entryName.setName(L"rasapitest_connection");

	// Проверяем нет ли уже такой записи
	DWORD raserror = RasApi::ValidatePhonebookEntryName(entryName);

	if (raserror != ERROR_SUCCESS)
	{
		std::wcout << L"RasApi: ValidatePhonebookEntryName failed: " 
			<< raserror << " " << RasApi::EErrorCode::ToString(raserror);
		return;
	}
	// Заполняем параметры
	RasApi::PhonebookEntry entry;
	raserror = entry.getLastError();

	if (raserror != ERROR_SUCCESS)
	{
		std::wcout << L"RasApi: PhonebookEntry failed to query structure size: " 
			<< raserror << " " << RasApi::EErrorCode::ToString(raserror);
		return;
	}

	entry.setLocalPhoneNumber(L"*99***1#");
	entry.setDeviceName(gDevices.front());
	entry.setDeviceType(RasApi::EDeviceType::Modem);
	entry.setNetProtocols(RasApi::ENetworkProtocol::Ip);
	entry.setFramingProtocol(RasApi::EFramingProtocol::Ppp);
	entry.setIdleDisconnectSeconds(RasApi::EIdleDisconnect::Disabled);
	entry.setRedialCount(0);
	entry.setPhonebookEntryType(RasApi::EPhonebookEntry::Phone);
	entry.setEncriptionType(RasApi::EEncryptionType::Optional);

	entry.setOptions(
		RasApi::EConnectionOption::RemoteDefaultGateway |
		RasApi::EConnectionOption::DisableLcpExtensions |
		RasApi::EConnectionOption::ModemLights |
		RasApi::EConnectionOption::SecureLocalFiles);

	entry.setOptions2(
		RasApi::EConnectionOption2::Internet | 
		RasApi::EConnectionOption2::SecureFileAndPrint | 
		RasApi::EConnectionOption2::SecureClientForMSNet |
		RasApi::EConnectionOption2::DisableNbtOverIP |
		RasApi::EConnectionOption2::DontNegotiateMultilink);

	// Создаём
	raserror = RasApi::CreateNewPhonebookEntry(entryName, entry);

	if (raserror != ERROR_SUCCESS)
	{
		std::wcout << L"RasApi: CreateNewPhonebookEntry failed: " 
			<< raserror << " " << RasApi::EErrorCode::ToString(raserror);
		return;
	}

	// Установка параметров соединения
	RasApi::DialParams dialParams;

	dialParams.setPhoneNumber(L"*99***1#");
	dialParams.setUserName(L"mts");
	dialParams.setPassword(L"mts");

	raserror = RasApi::SetEntryDialParams(entryName, dialParams);

	if (raserror != ERROR_SUCCESS)
	{
		std::wcout << L"RasApi: SetEntryDialParams failed: " 
			<< raserror << " " << RasApi::EErrorCode::ToString(raserror);
		return;
	}
}

void testDial()
{
	std::wcout << std::endl << "Testing dialing..." << std::endl;

	RasApi::PhonebookEntryName entry;
	entry.setName(L"rasapitest_connection");
	
	DWORD raserror = RasApi::Dial(entry);

	if (raserror != ERROR_SUCCESS)
	{
		std::wcout << L"RasApi: Dial failed: " 
			<< raserror << " " << RasApi::EErrorCode::ToString(raserror);
		return;
	}
}

void testHangUp()
{
	std::wcout << std::endl << "Testing hanging up..." << std::endl;
	
	DWORD raserror = RasApi::HangUp(L"rasapitest_connection");

	if (raserror != ERROR_SUCCESS)
	{
		std::wcout << L"RasApi: Dial failed: " 
			<< raserror << " " << RasApi::EErrorCode::ToString(raserror);
		return;
	}
}

int main(int argc, char* argv[])
{
	testEnumPhonebookEntries();
	testEnumDevices(RasApi::EDeviceType::Modem);
	testEnumConnections();
	testConnectionStatus();
	testCreatePhonebookEntry();
	testDial();
	Sleep(5000);
	testHangUp();

	return 0;
}

