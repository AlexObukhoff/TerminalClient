/* @file Настройки пользователя. */

#pragma once

// SDK
#include <SDK/PaymentProcessor/Settings/ISettingsAdapter.h>

#include <Common/ILogable.h>
#include <Common/PropertyTree.h>

namespace SDK {
namespace PaymentProcessor {

//----------------------------------------------------------------------------
class UserSettings: public ISettingsAdapter, public ILogable
{
public:
	UserSettings(TPtree & aProperties);
	virtual ~UserSettings();

	/// Валидация данных.
	virtual bool isValid() const;

	/// Получить имя адаптера.
	static QString getAdapterName();

	/// Настройка для мониторинга: выгружать все платежи, игнориря статус
	bool reportAllPayments() const;

	/// Требовать ввода номера стекера в момент инкассации
	bool useStackerID() const;

private:
	TPtree & mProperties;

private:
	Q_DISABLE_COPY(UserSettings);
};

}} // SDK::PaymentProcessor

//---------------------------------------------------------------------------
