/* @file Интерфейс сервиса ддя работы с настройками. */

#pragma once

#include <SDK/PaymentProcessor/Settings/ISettingsAdapter.h>

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
class ISettingsService
{
public:
	/// Получить адаптер настроек.
	virtual ISettingsAdapter * getAdapter(const QString & aAdapterName) = 0;

	/// Сохранить полную конфигурацию.
	virtual bool saveConfiguration() = 0;

protected:
	virtual ~ISettingsService() {}
};

//------------------------------------------------------------------------------
}} // SDK::PaymentProcessor

