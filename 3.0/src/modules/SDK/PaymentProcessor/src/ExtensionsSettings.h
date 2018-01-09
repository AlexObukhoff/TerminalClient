/* @file Настройки расширений. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <QtCore/QList>
#include <QtCore/QVector>
#include <QtCore/QDateTime>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Settings/Range.h>
#include <SDK/PaymentProcessor/Settings/ISettingsAdapter.h>
#include <SDK/PaymentProcessor/Connection/Connection.h>

#include <Common/ILogable.h>
#include <Common/PropertyTree.h>

namespace SDK {
namespace PaymentProcessor {

//----------------------------------------------------------------------------
typedef QMap<QString, QString> TStringMap;

//----------------------------------------------------------------------------
class ExtensionsSettings: public ISettingsAdapter, public ILogable
{
public:
	ExtensionsSettings(TPtree & aProperties);
	virtual ~ExtensionsSettings();

	/// Валидация данных.
	virtual bool isValid() const;

	/// Получить имя адаптера.
	static QString getAdapterName();

	/// Получить настройки расширения, в случае отсутствия настроек вернет пустой QStringMap
	TStringMap getSettings(const QString & aExtensionName) const;

private:
	TPtree & mProperties;
	QMap<QString, TStringMap> mExtensionSettings;

private:
	Q_DISABLE_COPY(ExtensionsSettings);
};

}} // SDK::PaymentProcessor

//---------------------------------------------------------------------------
