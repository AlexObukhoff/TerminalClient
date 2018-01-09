/* @file Класс для тестирования сканеров. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QFutureWatcher>
#include <Common/QtHeadersEnd.h>

// SDK
#include <Common/ObjectPointer.h>
#include <SDK/Drivers/IHID.h>
#include <SDK/PaymentProcessor/IDeviceTest.h>

namespace SDK {
namespace Driver {
	class IDevice;
}}

//------------------------------------------------------------------------------
class HIDTest : public SDK::PaymentProcessor::IDeviceTest
{
	Q_OBJECT

public:
	HIDTest(SDK::Driver::IDevice * aDevice, const QString & aInstancePath);

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
	void onData(const QVariantMap & aData);

private:
	ObjectPointer<SDK::Driver::IHID> mHID;
	QList<QPair<QString, QString>> mTestNames;
};

//------------------------------------------------------------------------------
