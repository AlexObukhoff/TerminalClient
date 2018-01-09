/* @file Класс-фильтр содержимого полей ввода пользователя. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegExp>
#include <Common/QtHeadersEnd.h>


// SDK
#include <SDK/PaymentProcessor/Settings/Provider.h>

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
struct SProvider;

//------------------------------------------------------------------------------
class SecurityFilter
{
public:
	SecurityFilter(const SProvider & aProvider, SProviderField::SecuritySubsystem aSubsustem);

	/// Проверка - подлежит ли поле маскированию
	bool haveFilter(const QString & aParameterName) const;

	/// Выполняет фильтрацию 
	QString apply(const QString & aParameterName, const QString & aValue) const;

private:
	/// Получить актуальный regexp для маскирования
	QRegExp getMask(const QString & aParameterName) const;

private:
	const SProvider & mProvider;
	SProviderField::SecuritySubsystem mSubsustem;

	Q_DISABLE_COPY(SecurityFilter)
};

//------------------------------------------------------------------------------
}} // PaymentProcessor::SDK
