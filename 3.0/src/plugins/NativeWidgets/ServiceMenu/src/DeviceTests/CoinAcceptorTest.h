/* @file Класс для тестирования купюроприемников. */

#pragma once

// SDK
#include <Common/ObjectPointer.h>
#include <SDK/Drivers/ICashAcceptor.h>
#include <SDK/PaymentProcessor/IDeviceTest.h>

namespace SDK {
namespace Driver {
	class IDevice;
}}

//------------------------------------------------------------------------------
class CoinAcceptorTest : public SDK::PaymentProcessor::IDeviceTest
{
	Q_OBJECT

public:
	CoinAcceptorTest(SDK::Driver::IDevice * aDevice);

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
	void onStacked(SDK::Driver::TParList aNotes);

private:
	ObjectPointer<SDK::Driver::ICashAcceptor> mCoinAcceptor;
};

//------------------------------------------------------------------------------
