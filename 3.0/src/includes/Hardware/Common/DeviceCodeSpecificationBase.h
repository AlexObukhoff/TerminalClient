/* @file Описатели состояний устройств. */

#pragma once

// Qt
#include "Hardware/Common/BaseStatus.h"
#include "Hardware/Common/Specifications.h"

//--------------------------------------------------------------------------------
/// Описатель состояний устройства, для внутреннего пользования в протоколах.
struct SDeviceCodeSpecification
{
	/// Общий статус-код.
	int statusCode;

	/// Описание специфичного состояния для конкретного протокола.
	QString description;

	bool operator==(const SDeviceCodeSpecification & aDeviceCodeSpecification) const
	{
		return (aDeviceCodeSpecification.statusCode  == statusCode) &&
		       (aDeviceCodeSpecification.description == description);
	}

	SDeviceCodeSpecification(): statusCode(DeviceStatusCode::OK::Unknown) {}
	SDeviceCodeSpecification(int aStatusCode, QString aDescription): statusCode(aStatusCode), description(aDescription) {}
};

const char UnknownDeviceCodeDescription[] = "unknown device code";

/// Спецификация состояний устройства.
template<class T>
class DeviceCodeSpecificationBase : public CSpecification<T, SDeviceCodeSpecification>
{
public:
	DeviceCodeSpecificationBase()
	{
		setDefault(SDeviceCodeSpecification(0, UnknownDeviceCodeDescription));
	}

	void appendStatus(T aCode, int aStatusCode, const QString & aDescription = "")
	{
		append(aCode, SDeviceCodeSpecification(aStatusCode, aDescription));
	}
};

//--------------------------------------------------------------------------------
