/* @file Класс для тестирования купюроприемников. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTimer>
#include <Common/QtHeadersEnd.h>

// SDK
#include <Common/ObjectPointer.h>
#include <SDK/Drivers/ICashAcceptor.h>
#include <SDK/PaymentProcessor/IDeviceTest.h>

namespace SDK {
namespace Driver {
	class IDevice;
}}

namespace CBillAcceptorTest
{
	/// Таймаут сообщения о номинале купюры в эскроу, [мс].
	const int EscrowMessageTimeout = 5 * 1000;
}

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
	/// Показать номинал.
	void onEscrow(SDK::Driver::SPar aPar);

	/// Показать статус, если необходимо.
	void onStatusChanged(SDK::Driver::EWarningLevel::Enum, const QString &, int aParam);

	/// Удалить сообщение.
	void onEraseMessage();

private:
	/// Экземпляр класса купюроприемника.
	ObjectPointer<SDK::Driver::ICashAcceptor> mBillAcceptor;

	/// Набор разрешенных номиналов.
	SDK::Driver::TParList mWorkingParList;

	/// Таймер удаления сообщений.
	QTimer mErasingTimer;
};

//------------------------------------------------------------------------------
