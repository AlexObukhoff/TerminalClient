/* @file Класс для тестирования диспенсеров. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QFutureWatcher>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

// SDK
#include <Common/ObjectPointer.h>
#include <SDK/Drivers/IDispenser.h>
#include <SDK/PaymentProcessor/IDeviceTest.h>

namespace SDK {
namespace Driver {
	class IDevice;
}
namespace PaymentProcessor {
	class ICore;
}}

//------------------------------------------------------------------------------
class DispenserTest : public SDK::PaymentProcessor::IDeviceTest
{
	Q_OBJECT

public:
	DispenserTest(SDK::Driver::IDevice * aDevice, const QString & aConfigurationName, SDK::PaymentProcessor::ICore * aCore);

	/// Возвращает имена и описания тестов.
	virtual QList<QPair<QString, QString>> getTestNames() const;

	/// Запускает тестирование устройства.
	virtual bool run(const QString & aName = QString());

	/// Остановка процесса тестирования.
	virtual void stop();

	/// Можно тестировать?
	virtual bool isReady();

	/// Возвращает true, если тест устройства возвращает результат теста
	virtual bool hasResult();

private slots:
	void onDispensed(int aCashUnit, int aCount);
	void onRejected(int aCashUnit, int aCount);

private:
	ObjectPointer<SDK::Driver::IDispenser> mDispenser;
	QString mConfigurationName;
	SDK::PaymentProcessor::ICore * mCore;
	QStringList mResults;
};

//------------------------------------------------------------------------------
