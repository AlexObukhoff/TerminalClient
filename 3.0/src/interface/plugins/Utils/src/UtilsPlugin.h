/* @file Плагин для отрисовки кнопок логотипов. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtQml/QQmlExtensionPlugin>
#include <Common/QtHeadersEnd.h>

class QQmlEngine;

//------------------------------------------------------------------------------
class UtilsPlugin : public QQmlExtensionPlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "com.cyberplat.graphics.utils")

public:
	virtual void registerTypes(const char * aUri);
	virtual void initializeEngine(QQmlEngine * aEngine, const char * aUri);
};

//------------------------------------------------------------------------------
