/* @file Реализация плагина. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtWebkit/QGraphicsWebView>
#include <QtWebkit/QWebPage>
#include <QtWebkit/QWebFrame>
#include <QtCore/QMetaEnum>
#include <QtNetwork/QSslConfiguration>
#include <QtNetwork/QSslKey>
#include <QtCore/QDir>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/PluginInitializer.h>
#include <SDK/Plugins/IPluginFactory.h>
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
// Проект
#include "WebKitBackend.h"

/// Константы.
namespace CWebKitBackend
{
	/// Название плагина.
	const char PluginName[] = "Webkit";

	/// Тип данного графического движка.
	const char Type[] = "web";

	/// Директория с ключами
	const QString KeysDir = "keys";

	/// Pem-файл
	const QString PemFile = "local.pem";

	/// Key-файл
	const QString KeyFile = "local.key";
}

//------------------------------------------------------------------------------
namespace
{

/// Конструктор экземпляра плагина.
SDK::Plugin::IPlugin * CreatePlugin(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath)
{
	return new WebKitBackend(aFactory, aInstancePath);
}

QVector<SDK::Plugin::SPluginParameter> EnumParameters()
{
	return QVector<SDK::Plugin::SPluginParameter>(1)
		<< SDK::Plugin::SPluginParameter(SDK::Plugin::Parameters::Debug, SDK::Plugin::SPluginParameter::Bool, false,
			QT_TRANSLATE_NOOP("WebkitBackendParameters", "#debug_mode"), QT_TRANSLATE_NOOP("WebkitBackendParameters", "#debug_mode_howto"), 
			false);
}

}

/// Регистрация плагина в фабрике.
REGISTER_PLUGIN_WITH_PARAMETERS(SDK::Plugin::makePath(SDK::PaymentProcessor::Application, SDK::PaymentProcessor::CComponents::GraphicsBackend, CWebKitBackend::PluginName), &CreatePlugin, &EnumParameters);

//------------------------------------------------------------------------------
WebKitBackend::WebKitBackend(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath)
	: mFactory(aFactory),
	  mInstancePath(aInstancePath),
	  mEngine(0),
	  mCoreProxy(0)
{
}

//------------------------------------------------------------------------------
WebKitBackend::~WebKitBackend()
{
	mItems.clear();
}

//------------------------------------------------------------------------------
QString WebKitBackend::getPluginName() const
{
	return CWebKitBackend::PluginName;
}

//------------------------------------------------------------------------------
QVariantMap WebKitBackend::getConfiguration() const
{
	return mParameters;
}

//------------------------------------------------------------------------------
void WebKitBackend::setConfiguration(const QVariantMap & aParameters)
{
	mParameters = aParameters;
}

//------------------------------------------------------------------------------
QString WebKitBackend::getConfigurationName() const
{
	return mInstancePath;
}

//------------------------------------------------------------------------------
bool WebKitBackend::saveConfiguration()
{
	// У плагина нет параметров
	return true;
}

//------------------------------------------------------------------------------
bool WebKitBackend::isReady() const
{
	return true;
}

//------------------------------------------------------------------------------
std::weak_ptr<SDK::GUI::IGraphicsItem> WebKitBackend::getItem(const SDK::GUI::GraphicsItemInfo & aInfo)
{
	QMap<QString, std::shared_ptr<WebGraphicsItem>>::iterator it = mItems.find(aInfo.name);

	if (it != mItems.end() && it.value()->getContext() == aInfo.context)
	{
		return it.value();
	}

	std::shared_ptr<WebGraphicsItem> item(new WebGraphicsItem(aInfo, mCoreProxy, mEngine->getLog()), SDK::GUI::GraphicsItemDeleter());

	if (item->isValid())
	{
		mItems.insert(aInfo.name, item);
	}
	else
	{
		mEngine->getLog()->write(LogLevel::Error, item->getError());
	}

	return item;
}

//------------------------------------------------------------------------------
bool WebKitBackend::removeItem(const SDK::GUI::GraphicsItemInfo & aInfo)
{
	foreach (auto item, mItems.values(aInfo.name))
	{
		if (item->getContext() == aInfo.context)
		{
			return mItems.remove(aInfo.name, item) != 0;
		}
	}

	return false;
}

//------------------------------------------------------------------------------
QString WebKitBackend::getType() const
{
	return CWebKitBackend::Type;
}

//------------------------------------------------------------------------------
QList<SDK::GUI::GraphicsItemInfo> WebKitBackend::getItemList()
{
	return QList<SDK::GUI::GraphicsItemInfo>();
}

//------------------------------------------------------------------------------
bool WebKitBackend::initialize(SDK::GUI::IGraphicsEngine * aEngine)
{
	mEngine = aEngine;
	mCoreProxy = static_cast<SDK::PaymentProcessor::Scripting::Core *>(
		mEngine->getGraphicsHost()->getInterface<QObject>(SDK::PaymentProcessor::Scripting::CProxyNames::Core));

	// Импорт ssl сертификата
	QFile pem(mFactory->getKernelDataDirectory() + QDir::separator() + 
		CWebKitBackend::KeysDir + QDir::separator() + CWebKitBackend::PemFile);
	if (pem.open(QIODevice::ReadOnly))
	{
		QSslConfiguration conf = QSslConfiguration::defaultConfiguration();

		QSslCertificate cert(pem.readAll());
		conf.setLocalCertificate(cert);
		mEngine->getLog()->write(LogLevel::Normal, "WebKitBackend: Pem certifiacate added.");

		QFile key(mFactory->getKernelDataDirectory() + QDir::separator() + 
			CWebKitBackend::KeysDir + QDir::separator() + CWebKitBackend::KeyFile);
		if (key.open(QIODevice::ReadOnly))
		{
			QSslKey k(key.readAll(), QSsl::Rsa);
			conf.setPrivateKey(k);
			mEngine->getLog()->write(LogLevel::Normal, "WebKitBackend: Key for certifiacate added.");
		}
		else
		{
			mEngine->getLog()->write(LogLevel::Error, "WebKitBackend: Can't open key file.");
		}

		QSslConfiguration::setDefaultConfiguration(conf);
	}
	else 
	{
		mEngine->getLog()->write(LogLevel::Error, "WebKitBackend: Can't open pem file.");
	}

	return true;
}

//------------------------------------------------------------------------------
void WebKitBackend::shutdown()
{
}

//------------------------------------------------------------------------------
