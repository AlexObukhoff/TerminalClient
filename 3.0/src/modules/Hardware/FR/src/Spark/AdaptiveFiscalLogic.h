/* @file Логика адаптивной печати фискального чека. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QPair>
#include <QtCore/QByteArray>
#include <QtCore/QStringList>
#include <QtCore/QVariantMap>
#include <Common/QtHeadersEnd.h>

// Modules
#include "AdaptiveFiscalLogicConstants.h"

/// Границы участка шаблона с денежными данными
typedef QPair<int, int> TMoneyPosition;

//--------------------------------------------------------------------------------
class AdaptiveFiscalLogic
{
public:
	AdaptiveFiscalLogic(const QVariantMap & aConfiguration);

	/// Уместить терминальную квитанцию в реквизиты ПФД-шаблона
	bool adjustReceipt(const QStringList & aReceipt);

	/// Получить текстовые реквизиты
	QStringList & getTextProperties();

private:
	/// Уместить терминальную квитанцию в реквизиты ПФД-шаблона
	bool adjustReceipt(const QStringList & aReceipt, const TMoneyPosition & aMoneyPosition, CSparkFR::TextProperties::EPosition::Enum aPosition);

	/// Получить границы участка шаблона с денежными данными
	TMoneyPosition getMoneyPosition();

	/// Получить буфер из строки, разделенный по максимальной длине строки
	QStringList getSeparatedBuffer(const QString & aLine);

	/// Удалить разделитель в чеке для вписывания его в ПФД
	void removeDelimeter(QStringList & aText, CSparkFR::TextProperties::EPosition::Enum aPosition);

	/// Уместить буфер текста в реквизит ПФД-шаблона
	bool adjustBuffer(QStringList & aBuffer, CSparkFR::TextProperties::EPosition::Enum aPosition = CSparkFR::TextProperties::EPosition::No);

	/// Конфигурация - настройки чека и шаблон
	QVariantMap mConfiguration;

	/// Текстовые реквизиты
	QStringList mTextProperties;
};

//--------------------------------------------------------------------------------
