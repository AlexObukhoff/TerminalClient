/* @file Плагин для отображения flash */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtDeclarative/QDeclarativeExtensionPlugin>
#include <Common/QtHeadersEnd.h>

// Project
#include "FlashPlayerItem.h"

//--------------------------------------------------------------------------
class FlashPlayerItemPlugin : public QDeclarativeExtensionPlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "Cyberplat.GraphicsItems.FlashPlayer")

public:
	virtual void registerTypes(const char * aUri)
	{
		qmlRegisterType<FlashPlayerItem>(aUri, 1, 0, "FlashPlayer");
	}
};

//--------------------------------------------------------------------------
