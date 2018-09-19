/* @file Сервис для организации перехвата вызовов других сервисов. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QGenericArgument>
#include <QtCore/QMetaObject>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Components.h>

// Модули
#include <Crypt/ICryptEngine.h>

// Проект
#include "DatabaseUtils/IPaymentDatabaseUtils.h"

#include "Services/ServiceNames.h"
#include "Services/HookService.h"
#include "Services/PluginService.h"
#include "Services/PaymentService.h"

namespace PPSDK = SDK::PaymentProcessor;


//---------------------------------------------------------------------------
HookService * HookService::instance(IApplication * aApplication)
{
	return static_cast<HookService *>(aApplication->getCore()->getService(CServices::HookService));
}

//---------------------------------------------------------------------------
HookService::HookService(IApplication * aApplication) 
	: mApplication(aApplication)
{

}

//---------------------------------------------------------------------------
HookService::~HookService()
{
}

//---------------------------------------------------------------------------
bool HookService::initialize()
{
	QStringList hookers =
		PluginService::instance(mApplication)->getPluginLoader()->getPluginList(QRegExp(QString("%1\\.%2\\..*").arg(PPSDK::Application, PPSDK::CComponents::Hook)));

	foreach (const QString & path, hookers)
	{
		SDK::Plugin::IPlugin * plugin = PluginService::instance(mApplication)->getPluginLoader()->createPlugin(path);
		if (plugin)
		{
			if (plugin->isReady())
			{
				mHooks << plugin;
			}
			else
			{
				PluginService::instance(mApplication)->getPluginLoader()->destroyPlugin(plugin);
			}
		}
	}

	return true;
}

//------------------------------------------------------------------------------
void HookService::finishInitialize()
{
}

//---------------------------------------------------------------------------
bool HookService::canShutdown()
{
	return true;
}

//---------------------------------------------------------------------------
bool HookService::shutdown()
{
	while (!mHooks.isEmpty())
	{
		PluginService::instance(mApplication)->getPluginLoader()->destroyPlugin(mHooks.first());
		mHooks.takeFirst();
	}

	return true;
}

//---------------------------------------------------------------------------
QString HookService::getName() const
{
	return CServices::HookService;
}

//---------------------------------------------------------------------------
const QSet<QString> & HookService::getRequiredServices() const
{
	static QSet<QString> requiredServices = QSet<QString>()
		<< CServices::PluginService;

	return requiredServices;
}

//---------------------------------------------------------------------------
QVariantMap HookService::getParameters() const
{
	return QVariantMap();
}

//---------------------------------------------------------------------------
void HookService::resetParameters(const QSet<QString> &)
{
}

//---------------------------------------------------------------------------
bool HookService::invokeHook(const QString & aMetodName,
	QGenericArgument aVal0,
	QGenericArgument aVal1, QGenericArgument aVal2,
	QGenericArgument aVal3, QGenericArgument aVal4,
	QGenericArgument aVal5, QGenericArgument aVal6,
	QGenericArgument aVal7, QGenericArgument aVal8,
	QGenericArgument aVal9)
{
	bool result = false;

	foreach (SDK::Plugin::IPlugin * plugin, mHooks)
	{
		QObject * pluginObject = dynamic_cast<QObject *> (plugin);

		if (pluginObject != nullptr)
		{
			// Всегда делаем Qt::DirectConnection независимо от того в каком процессе находится объект перехватчик
			result = QMetaObject::invokeMethod(pluginObject, aMetodName.toLatin1().data(), Qt::DirectConnection,
				aVal0, aVal1, aVal2, aVal3, aVal4, aVal5, aVal6, aVal7, aVal8, aVal9) || result;
		}
	}

	return result;
}

