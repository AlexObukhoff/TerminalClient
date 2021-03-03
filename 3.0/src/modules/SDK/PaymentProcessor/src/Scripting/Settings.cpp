/* @file Прокси класс для получения информации из конфигов в скриптах */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDir>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Core/IGUIService.h>
#include <SDK/PaymentProcessor/Scripting/Settings.h>
#include <SDK/PaymentProcessor/Scripting/ScriptArray.h>

namespace SDK {
namespace PaymentProcessor {
namespace Scripting {

namespace
{
	const QString DefaultSkin = "default";
	const QString CommonSkinDirectory = "skins";
}

//------------------------------------------------------------------------------
DealerSettings::DealerSettings(ICore * aCore)
{
	ISettingsService * settingsService = aCore->getSettingsService();

	mSettings = static_cast<SDK::PaymentProcessor::DealerSettings *>(settingsService->getAdapter(CAdapterNames::DealerAdapter));
	mPersonalSettings = mSettings->getPersonalSettings();
}	

//------------------------------------------------------------------------------
bool DealerSettings::isPaymentAllowed(const QVariantMap & aParameters) const
{
	return mSettings->isCustomerAllowed(aParameters);
}

//------------------------------------------------------------------------------
QObject * DealerSettings::getCommissions(qint64 aProvider, const QVariantMap & aParameters, double aAmount)
{
	ScriptArray * result = new ScriptArray(this);

	foreach (const Commission & commission, mSettings->getCommissions(aProvider, aParameters))
	{
		// Комиссия подходит для данной суммы если нет ограничений по сумме или выполняется условие "сумма <= maxLimit"
		if (!commission.hasLimits() || aAmount <= commission.getMaxLimit() || qFuzzyIsNull(aAmount))
		{
			result->append(new SCommission(commission, result));
		}
	}

	return result;
}

//------------------------------------------------------------------------------
TerminalSettings::TerminalSettings(ICore * aCore)
{
	ISettingsService * settingsService = aCore->getSettingsService();
	mTerminalSettings = static_cast<SDK::PaymentProcessor::TerminalSettings *>(settingsService->getAdapter(CAdapterNames::TerminalAdapter));

	mGuiService = aCore->getGUIService();
}

//------------------------------------------------------------------------------
bool TerminalSettings::isItServiceProvider(qint64 aProvider, const QVariantMap & aParameters)
{
	if (mTerminalSettings->getServiceMenuPasswords().operatorId == aProvider)
	{
		if (aParameters.size() && aParameters.value(aParameters.keys().first()).toString() == mTerminalSettings->getServiceMenuPasswords().phone)
		{
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------
QString TerminalSettings::getCurrentSkinPath() const
{
	QString name = mGuiService->getUiSettings("ui")["skin"].toString();
	
	return mTerminalSettings->getAppEnvironment().interfacePath +
		QDir::separator() + CommonSkinDirectory +
		QDir::separator() + (name.isEmpty() ? DefaultSkin : name);
}

//------------------------------------------------------------------------------
ExtensionsSettings::ExtensionsSettings(ICore * mCore)
{
	ISettingsService * settingsService = mCore->getSettingsService();
	mSettings = static_cast<SDK::PaymentProcessor::ExtensionsSettings *>(settingsService->getAdapter(CAdapterNames::Extensions));
}

//------------------------------------------------------------------------------
}}} // Scripting::PaymentProcessor::SDK
