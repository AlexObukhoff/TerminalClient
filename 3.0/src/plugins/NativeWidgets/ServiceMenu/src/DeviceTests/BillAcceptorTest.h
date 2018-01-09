/* @file Класс для тестирования купюроприемников. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QFutureWatcher>
#include <Common/QtHeadersEnd.h>

// SDK
#include <Common/ObjectPointer.h>
#include <SDK/Drivers/ICashAcceptor.h>
#include <SDK/PaymentProcessor/IDeviceTest.h>

namespace SDK {
namespace Driver {
	class IDevice;
}}

//------------------------------------------------------------------------------
class BillAcceptorTest : public SDK::PaymentProcessor::IDeviceTest
{
	Q_OBJECT

public:
	BillAcceptorTest(SDK::Driver::IDevice * aDevice);

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
	void onEscrow(SDK::Driver::SPar aPar);
	void onStatusChanged(SDK::Driver::EWarningLevel::Enum, const QString &, int aParam);

private:
	ObjectPointer<SDK::Driver::ICashAcceptor> mBillAcceptor;
	bool mRejected;
};

//------------------------------------------------------------------------------
