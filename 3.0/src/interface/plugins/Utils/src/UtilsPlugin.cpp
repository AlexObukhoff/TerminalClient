/* @file Плагин для отрисовки кнопок логотипов. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtDeclarative/QtDeclarative>
#include <Common/QtHeadersEnd.h>

// Проект
#include "Log.h"
#include "UtilsPlugin.h"
#include "SkinProvider.h"
#include "BarcodeProvider.h"
#include "Utils.h"

Q_EXPORT_PLUGIN2(utils, UtilsPlugin)

//------------------------------------------------------------------------------
void UtilsPlugin::registerTypes(const char * /*aUri*/)
{
}

//------------------------------------------------------------------------------
void UtilsPlugin::initializeEngine(QDeclarativeEngine * aEngine, const char * /*aUri*/)
{
	QObject * application = aEngine->rootContext()->contextProperty("Core").value<QObject *>();
	QString logoPath = ".";
	QString userLogoPath = ".";
	QString interfacePath = ".";

	if (application)
	{
		Log::initialize(application);

		logoPath = application->property("environment")
			.value<QObject *>()->property("terminal")
			.value<QObject *>()->property("contentPath").toString();

		userLogoPath = application->property("environment")
			.value<QObject *>()->property("terminal")
			.value<QObject *>()->property("dataPath").toString();

		interfacePath = application->property("environment")
			.value<QObject *>()->property("terminal")
			.value<QObject *>()->property("interfacePath").toString();
	}

	Utils * utils = new Utils(aEngine, interfacePath, userLogoPath);

	aEngine->rootContext()->setContextProperty("Utils", utils);

	aEngine->addImageProvider("ui", new SkinProvider(interfacePath, logoPath, userLogoPath, dynamic_cast<Skin *>(utils->getSkin())));
	
	aEngine->addImageProvider("barcode", new BarcodeProvider());
}

//------------------------------------------------------------------------------
