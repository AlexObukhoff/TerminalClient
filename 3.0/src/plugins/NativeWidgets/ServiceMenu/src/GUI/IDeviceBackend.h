/* @file Реализация компоненты для редактирования профилей устройств. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

// Plugin SDK
#include <SDK/Plugins/PluginParameters.h>

namespace SDK {
namespace PaymentProcessor {
	class ICore;
}}

class HardwareManager;

//------------------------------------------------------------------------
class IDeviceBackend
{
public:
	/// Возвращает список поддерживаемых моделей, указанного типа.
	virtual QStringList getModels(const QString & aType) = 0;

	/// Возвращает список параметров, указанной модели.
	virtual SDK::Plugin::TParameterList getModelParameters(const QString & aType, const QString & aModel) = 0;

	/// Возвращает менеджер устройств
	virtual HardwareManager * getHardwareManager() const = 0;

	virtual SDK::PaymentProcessor::ICore * getCore() const = 0;

	virtual void toLog(const QString & aMessage) = 0;

protected:
	virtual ~IDeviceBackend() {}
};

//------------------------------------------------------------------------
