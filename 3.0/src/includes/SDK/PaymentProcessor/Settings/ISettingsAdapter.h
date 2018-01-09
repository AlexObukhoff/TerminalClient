/* @file Интерфейс адаптера настроек. */

#pragma once

// Stl
#include <string>
#include <iostream>

class QString;

namespace SDK {
namespace PaymentProcessor {

//---------------------------------------------------------------------------
namespace CAdapterNames
{
	const char TerminalAdapter[] = "TerminalSettings";
	const char DealerAdapter[]   = "DealerSettings";
	const char UserAdapter[]     = "UserSettings";
	const char Directory[]       = "Directory";
	const char Extensions[]      = "Extensions";
};

//---------------------------------------------------------------------------
class ISettingsAdapter
{
public:
	/// Все ли нужные настройки загрузились?
	virtual bool isValid() const = 0;

	virtual ~ISettingsAdapter() {}
};

//---------------------------------------------------------------------------
}} // SDK::PaymentProcessor
