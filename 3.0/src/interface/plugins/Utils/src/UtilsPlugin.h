/* @file Плагин для отрисовки кнопок логотипов. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtDeclarative/QDeclarativeExtensionPlugin>
#include <QtDeclarative/QDeclarativeImageProvider>
#include <Common/QtHeadersEnd.h>

//------------------------------------------------------------------------------
class UtilsPlugin : public QDeclarativeExtensionPlugin
{
	Q_OBJECT

public:
	virtual void registerTypes(const char * aUri);
	virtual void initializeEngine(QDeclarativeEngine * aEngine, const char * aUri);
};

//------------------------------------------------------------------------------
