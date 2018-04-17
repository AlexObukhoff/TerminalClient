/* @file Данные серверов ОФД. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <QtCore/QRegExp>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/ASCII.h"

// Project
#include "Hardware/Common/Specifications.h"

//--------------------------------------------------------------------------------
namespace CHardware
{
	const QRegExp regExps[] =
	{
		QRegExp("^[0-9a-z\\-\\_\\.]+\\.(ru|com|net)$"),
		QRegExp("^[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}$"),
		QRegExp("^[0-9]{4,5}$"),
	};

	template<int aIndex>
	struct SData
	{
		QString value;
		bool valid;

		SData() : valid(false) {}
		SData(const QString & aValue)
		{
			value = aValue.toLower();
			valid = regExps[aIndex].indexIn(value) != -1;

			if (valid && aIndex)
			{
				QStringList data = value.split(ASCII::Dot);
				valid = std::find_if(data.begin(), data.end(), [] (const QString & aData) -> bool { return aData.toInt(); }) != data.end();
			}
		}
	};

	typedef SData<0> SURL;
	typedef SData<1> SIP;
	typedef SData<2> SPort;

	struct SAddress
	{
		SURL URL;
		SIP IP;
		SPort port;

		SAddress() {}
		SAddress(const QString & aURl, const QString & aIP, const QString & aPort) : URL(aURl), IP(aIP), port(aPort) {}

		bool valid()
		{
			return port.valid && (URL.valid || IP.valid);
		}
	};

	struct SOFDData
	{
		SAddress address;
		SAddress testAddress;
	};

	class OFDData : public CSpecification<QString, SOFDData>
	{
	public:
		OFDData()
		{
			add("1-OFD",         "k-server.1-ofd.ru",    "91.107.114.11",    7777, "kkm-server-test.1-ofd.ru", "95.213.230.112",   7777);
			add("Taxcom",        "f1.taxcom.ru",         "193.0.214.11",     7777, "f1test.taxcom.ru",         "193.0.214.11",     7778);
			add("Peter-service", "gate.ofd.ru",          "185.15.172.18",    4000, "testgate.ofd.ru",          "52.29.225.198",    4001);
			add("Yarus",         "connect.ofd-ya.ru",    "91.107.67.212",    7779, "connect.ofd-ya.ru",        "91.107.67.212",    7790);
			add("Platforma",     "ofdp.platformaofd.ru", "185.170.204.91",  21101, "ofdt.platformaofd.ru",     "185.170.204.85",  19081);
			add("Yandex",        "kkt.ofd.yandex.net",   "185.32.186.252",  12345, "test.kkt.ofd.yandex.net",  "213.180.204.116", 12345);
			add("Korus",         "",                     "92.38.2.202",      7001, "",                         "92.38.2.78",       7001);
			add("Kontur",        "ofd.kontur.ru",        "46.17.204.250",    7777);
			add("Garant",        "ofd.garantexpress.ru", "141.101.203.186", 30801);
			add("Astral",        "ofd.astralnalog.ru",   "91.239.5.68",      7777);
			add("Sbis",          "kkt.sbis.ru",          "91.213.144.29",    7777);
			add("InitPro",       "kkt.ofd-initpro.ru",   "212.8.238.73",     9999);
			add("e-OFD",         "kkt.e-ofd.ru",         "176.122.30.30",    7777);
			add("MTS",           "ofd.nvg.ru",           "213.87.202.41",   21101);
			//"Beeline"
		}

	private:
		void add(const QString & aName, const QString & aURL, const QString & aIP, int aPort, const QString & aTestURL, const QString & aTestIP, int aTestPort)
		{
			SOFDData OFDData;
			OFDData.address = SAddress(aURL, aIP, QString::number(aPort));
			OFDData.testAddress = SAddress(aTestURL, aTestIP, QString::number(aTestPort));

			append(aName, OFDData);
		}

		void add(const QString & aName, const QString & aURL, const QString & aIP, int aPort)
		{
			SOFDData OFDData;
			OFDData.address = SAddress(aURL, aIP, QString::number(aPort));

			append(aName, OFDData);
		}
	};
}

//--------------------------------------------------------------------------------
