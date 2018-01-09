/* @file Базовые статусные типы. */

#pragma once

// SDK
#include <SDK/Drivers/WarningLevel.h>

// Project
#include "Hardware/Common/StatusCache.h"

// TStatusCodes в StatusCache.h;
typedef StatusCache<SDK::Driver::EWarningLevel::Enum> TStatusCollection;

typedef QSet<int> TDeviceCodes;

//--------------------------------------------------------------------------------
