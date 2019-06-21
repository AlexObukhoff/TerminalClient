/* @file Данные классов и интерфейсов USB-устройств.*/

#pragma once

// LibUSB SDK
#pragma warning(push, 1)
	#include "libusb/src/libusb/libusb.h"
#pragma warning(pop)

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QVariantMap>
#include <Common/QtHeadersEnd.h>

namespace USB
{
	/// Объект использования дескриптора.
	namespace DescriptorUsage
	{
		const char Device[] = "device";
		const char Interface[] = "interface";
		const char Both[] = "device and interface";
	}

	typedef QMap<uint8_t, QString> TProtocolData;
	typedef QMap<uint8_t, TProtocolData> TSubClassData;

	//--------------------------------------------------------------------------------
	struct SClassData
	{
		QString usage;
		QString description;
		TSubClassData subData;

		SClassData() {}
		SClassData(const QString & aUsage, const QString & aDescription): usage(aUsage), description(aDescription) {}
	};

	typedef QMap<uint8_t, SClassData> TClassData;
	typedef TClassData::iterator TClassDataIt;

	//--------------------------------------------------------------------------------
	class CClassDataIt: public TClassDataIt
	{
	public:
		CClassDataIt(const TClassDataIt & aOther)
		{
			operator = (aOther);
		}

		CClassDataIt & operator = (const TClassDataIt & aOther)
		{
			TClassDataIt * pSelf = dynamic_cast<TClassDataIt *>(this);
			*pSelf = aOther;

			return *this;
		}

		CClassDataIt addData(uint8_t aSubCode, uint8_t aProtocolCode, QString aDescription)
		{
			this->value().subData[aSubCode][aProtocolCode] = aDescription.replace('\t', "");

			return *this;
		}
	};

	//--------------------------------------------------------------------------------
	class CClassData
	{
	public:
		CClassData();

		QString getData(uint8_t aClass, uint8_t aSubClass, uint8_t aProtocol)
		{
			QString result = mData[aClass].description + ", descriptor usage - " + mData[aClass].usage;
			//TODO: доделать локализацию данных девайса

			return result;
		}

	private:
		CClassDataIt add(uint8_t aCode, const QString & aUsage, const QString & aDescription)
		{
			auto it = mData.insert(aCode, SClassData(aUsage, aDescription));

			return it;
		}

		TClassData mData;
	};

	static CClassData ClassData;
}

//--------------------------------------------------------------------------------
