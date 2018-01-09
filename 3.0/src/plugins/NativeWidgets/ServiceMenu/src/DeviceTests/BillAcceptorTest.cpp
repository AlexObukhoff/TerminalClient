/* @file Класс для тестирования купюроприемников. */

// Project
#include "BillAcceptorTest.h"

//------------------------------------------------------------------------------
namespace CBillAcceptorTest
{
	const QString TestEscrow = QT_TRANSLATE_NOOP("BillAcceptorTest", "#test_escrow");
}

//------------------------------------------------------------------------------
BillAcceptorTest::BillAcceptorTest(SDK::Driver::IDevice * aDevice) : mRejected(false)
{
	mBillAcceptor = dynamic_cast<SDK::Driver::ICashAcceptor *>(aDevice);
}

//------------------------------------------------------------------------------
QList<QPair<QString, QString>> BillAcceptorTest::getTestNames() const
{
	return QList<QPair<QString, QString>>() << qMakePair(CBillAcceptorTest::TestEscrow, tr("#insert_bill"));
}

//------------------------------------------------------------------------------
bool BillAcceptorTest::run(const QString & aName)
{
	if (aName == CBillAcceptorTest::TestEscrow)
	{
		if (mBillAcceptor->isDeviceReady() && mBillAcceptor->setEnable(true))
		{
			mBillAcceptor->subscribe(SDK::Driver::ICashAcceptor::EscrowSignal, this, SLOT(onEscrow(SDK::Driver::SPar)));
			mBillAcceptor->subscribe(SDK::Driver::ICashAcceptor::StatusSignal, this, SLOT(onStatusChanged(SDK::Driver::EWarningLevel::Enum, const QString &, int)));
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------
void BillAcceptorTest::stop()
{
	mBillAcceptor->setEnable(false);

	mBillAcceptor->unsubscribe(SDK::Driver::ICashAcceptor::EscrowSignal, this);
}

//------------------------------------------------------------------------------
bool BillAcceptorTest::isReady()
{
	return mBillAcceptor && mBillAcceptor->isDeviceReady();
}

//------------------------------------------------------------------------------
bool BillAcceptorTest::hasResult()
{
	return true;
}

//------------------------------------------------------------------------------
void BillAcceptorTest::onEscrow(SDK::Driver::SPar aPar)
{
	emit result("", QString("%1 %2").arg(tr("#bill_is_escrowed")).arg(aPar.nominal));

	// Выбрасываем купюру
	mRejected = true;
	mBillAcceptor->reject();
}

//------------------------------------------------------------------------------
void BillAcceptorTest::onStatusChanged(SDK::Driver::EWarningLevel::Enum, const QString &, int aParam)
{
	if (mRejected && SDK::Driver::ECashAcceptorStatus::Rejected == aParam)
	{
		emit result("", tr("#bill_is_rejected"));
	}
	
	mRejected = false;
}

//------------------------------------------------------------------------------
