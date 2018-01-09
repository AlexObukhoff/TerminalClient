/* @file Интерфейс графического движка. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/GUI/IGraphicsHost.h>
#include <SDK/GUI/IGraphicsItem.h>
#include <SDK/GUI/GraphicsItemInfo.h>

class ILog;

namespace SDK { 
namespace GUI {

//---------------------------------------------------------------------------
/// Интерфейс графического движка.
class IGraphicsEngine
{
public:
	/// Возвращает указатель на владельца движка.
	virtual IGraphicsHost * getGraphicsHost() = 0;

	/// Возвращает лог.
	virtual ILog * getLog() const = 0;

protected:
	virtual ~IGraphicsEngine() {}
};

}} // namespace SDK::GUI

//---------------------------------------------------------------------------

