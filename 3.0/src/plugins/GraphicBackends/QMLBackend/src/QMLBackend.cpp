/* @file Реализация плагина. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeItem>
#include <Common/QtHeadersEnd.h>

// GUI SDK
#include <SDK/GUI/MessageBoxParams.h>

// Plugin SDK
#include <SDK/Plugins/PluginInitializer.h>
#include <SDK/Plugins/IPluginFactory.h>
#include <SDK/PaymentProcessor/Components.h>
// TODO убрать зависимость
#include <SDK/PaymentProcessor/Core/EventTypes.h>

// Модули
#include <Common/ILog.h>

// Проект
#include "QMLBackend.h"

//------------------------------------------------------------------------------
namespace CQMLBackend
{
	/// Тип данного графического движка.
	const char Type[] = "qml";
	const char PluginName[] = "QML";
	const char TypesExportNamespace[] = "Types";
}

//------------------------------------------------------------------------------
namespace
{

/// Конструктор экземпляра плагина.
SDK::Plugin::IPlugin * CreatePlugin(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath)
{
	return new QMLBackend(aFactory, aInstancePath);
}

QVector<SDK::Plugin::SPluginParameter> EnumParameters()
{
	return QVector<SDK::Plugin::SPluginParameter>(1)
		<< SDK::Plugin::SPluginParameter(SDK::Plugin::Parameters::Debug, SDK::Plugin::SPluginParameter::Bool, false, 
			QT_TRANSLATE_NOOP("QMLBackendParameters", "#debug_mode"), QT_TRANSLATE_NOOP("QMLBackendParameters", "#debug_mode_howto"), 
			false);
}

}

/// Регистрация плагина в фабрике.
REGISTER_PLUGIN_WITH_PARAMETERS(SDK::Plugin::makePath(SDK::PaymentProcessor::Application, SDK::PaymentProcessor::CComponents::GraphicsBackend, CQMLBackend::PluginName), &CreatePlugin, &EnumParameters);

//------------------------------------------------------------------------------
QMLBackend::QMLBackend(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath)
{
	mFactory = aFactory;
	mInstancePath = aInstancePath;
	mEngine = 0;

	// Регистрируем типы событий.
	qmlRegisterUncreatableType<SDK::PaymentProcessor::EEventType>(
		QString("%1.%2").arg(SDK::PaymentProcessor::Scripting::CProxyNames::Core).arg(CQMLBackend::TypesExportNamespace).toLatin1(), 1, 0, "EventType", "EventType enum is readonly.");
	qmlRegisterUncreatableType<SDK::GUI::MessageBoxParams>(
		QString("%1.%2").arg(SDK::PaymentProcessor::Scripting::CProxyNames::Core).arg(CQMLBackend::TypesExportNamespace).toLatin1(), 1, 0, "MessageBox", "MessageBoxParams enum is readonly.");

	connect(&mQMLEngine, SIGNAL(warnings(const QList<QDeclarativeError> &)), this, SLOT(onWarnings(const QList<QDeclarativeError> &)));
}

//------------------------------------------------------------------------------
QString QMLBackend::getPluginName() const
{
	return CQMLBackend::PluginName;
}

//------------------------------------------------------------------------------
QVariantMap QMLBackend::getConfiguration() const
{
	return mParameters;
}

//------------------------------------------------------------------------------
void QMLBackend::setConfiguration(const QVariantMap & aParameters)
{
	mParameters = aParameters;
}

//------------------------------------------------------------------------------
QString QMLBackend::getConfigurationName() const
{
	return mInstancePath;
}

//------------------------------------------------------------------------------
bool QMLBackend::saveConfiguration()
{
	// У плагина нет параметров
	return true;
}

//------------------------------------------------------------------------------
bool QMLBackend::isReady() const
{
	return true;
}

//------------------------------------------------------------------------------
SDK::GUI::IGraphicsItem * QMLBackend::getItem(const SDK::GUI::GraphicsItemInfo & aInfo)
{
	TGraphicItemsCache::iterator it = mCachedItems.find(aInfo.name);

	if (it != mCachedItems.end() && it.value().data()->getContext() == aInfo.context)
	{
		return it.value().data();
	}

	QSharedPointer<QMLGraphicsItem> item(new QMLGraphicsItem(aInfo, &mQMLEngine, mEngine->getLog()));

	if (item->isValid())
	{
		mCachedItems.insertMulti(aInfo.name, item);
	}
	else
	{
		mEngine->getLog()->write(LogLevel::Error, item->getError());
		return 0;
	}

	return item.data();
}

//------------------------------------------------------------------------------
QString QMLBackend::getType() const
{
	return CQMLBackend::Type;
}

//------------------------------------------------------------------------------
QList<SDK::GUI::GraphicsItemInfo> QMLBackend::getItemList()
{
	return QList<SDK::GUI::GraphicsItemInfo>();
}

//------------------------------------------------------------------------------
bool QMLBackend::initialize(SDK::GUI::IGraphicsEngine * aEngine)
{
	mEngine = aEngine;
	mCore = mEngine->getGraphicsHost()->getInterface<SDK::PaymentProcessor::ICore>(SDK::PaymentProcessor::CInterfaces::ICore);

	foreach (auto objectName, mEngine->getGraphicsHost()->getInterfacesName())
	{
		if (SDK::PaymentProcessor::CInterfaces::ICore != objectName)
		{
			auto object = mEngine->getGraphicsHost()->getInterface<QObject>(objectName);
			if (object)
			{
				mQMLEngine.rootContext()->setContextProperty(objectName, object);
			}
		}
	}

	return true;
}

//------------------------------------------------------------------------------
void QMLBackend::shutdown()
{
}

//------------------------------------------------------------------------------
void QMLBackend::onWarnings(const QList<QDeclarativeError> & aWarnings)
{
	QString warnings;
	foreach (QDeclarativeError e, aWarnings)
	{
		warnings += e.toString() + "\n";
	}

	mEngine->getLog()->write(LogLevel::Warning, warnings);
}

//------------------------------------------------------------------------------
