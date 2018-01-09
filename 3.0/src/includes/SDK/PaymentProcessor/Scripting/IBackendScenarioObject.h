/* @file Обертка для проброса в скрипты объекта c++.
*/

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QObjectList>
#include <Common/QtHeadersEnd.h>

namespace SDK {
namespace PaymentProcessor {
namespace Scripting {

//------------------------------------------------------------------------------
class IBackendScenarioObject : public QObject
{
	Q_OBJECT

public:
	IBackendScenarioObject(QObject * aParent = nullptr) : QObject(aParent) {}
	virtual ~IBackendScenarioObject() {}

public:
	virtual QString getName() const = 0;
};

//------------------------------------------------------------------------------
}}} // SDK::PaymentProcessor::SDK
