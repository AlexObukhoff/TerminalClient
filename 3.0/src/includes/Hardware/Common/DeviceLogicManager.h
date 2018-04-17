/* @file Помощник по логике работы устройства. */

#pragma once

// Common
#include <Common/ILogable.h>

// Project
#include "DeviceConfigManager.h"
#include "DeviceLogManager.h"

//--------------------------------------------------------------------------------
/// Обобщенные состояния выполнения запроса.
namespace ERequestStatus
{
	enum Enum
	{
		Success = 0,
		InProcess,
		Fail
	};
}

//--------------------------------------------------------------------------------
class DeviceLogicManager: public DeviceLogManager, public DeviceConfigManager
{
};

//--------------------------------------------------------------------------------
