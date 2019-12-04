/* @file Класс для тестирования купюроприемников. */

// Project
#include "BillAcceptorTest.h"

using namespace SDK::Driver;

//------------------------------------------------------------------------------
namespace CBillAcceptorTest
{
	const QString TestEscrow = QT_TRANSLATE_NOOP("BillAcceptorTest", "#test_escrow");
}

//------------------------------------------------------------------------------
BillAcceptorTest::BillAcceptorTest(IDevice * aDevice)
{
	mBillAcceptor = dynamic_cast<ICashAcceptor *>(aDevice);

	connect(&mErasingTimer, SIGNAL(timeout()), this, SLOT(onEraseMessage()));
}

//------------------------------------------------------------------------------
QList<QPair<QString, QString>> BillAcceptorTest::getTestNames() const
{
	return QList<QPair<QString, QString>>() << qMakePair(CBillAcceptorTest::TestEscrow, tr("#insert_bill"));
}

//------------------------------------------------------------------------------
bool BillAcceptorTest::run(const QString & aName)
{
	if ((aName != CBillAcceptorTest::TestEscrow) || !mBillAcceptor->isDeviceReady())
	{
		return false;
	}

	mBillAcceptor->subscribe(SDK::Driver::ICashAcceptor::EscrowSignal, this, SLOT(onEscrow(SDK::Driver::SPar)));
	mBillAcceptor->subscribe(SDK::Driver::ICashAcceptor::StatusSignal, this, SLOT(onStatusChanged(SDK::Driver::EWarningLevel::Enum, const QString &, int)));

	mWorkingParList = mBillAcceptor->getParList();
	TParList testParList(mWorkingParList);

	for (int i = 0; i < testParList.size(); ++i)
	{
		testParList[i].enabled = true;
	}

	if (!isParListEqual(testParList, mWorkingParList))
	{
		mBillAcceptor->setParList(testParList);
	}

	return mBillAcceptor->setEnable(true);
}

//------------------------------------------------------------------------------
void BillAcceptorTest::stop()
{
	mErasingTimer.stop();

	mBillAcceptor->setEnable(false);
	TParList testParList = mBillAcceptor->getParList();

	if (!isParListEqual(testParList, mWorkingParList))
	{
		mBillAcceptor->setParList(mWorkingParList);
	}

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
void BillAcceptorTest::onEscrow(SPar aPar)
{
	QString message = QString("%1 %2").arg(tr("#bill_is_escrowed")).arg(aPar.nominal);

	for (int i = 0; i < mWorkingParList.size(); ++i)
	{
		if ((mWorkingParList[i] == aPar) && !mWorkingParList[i].enabled)
		{
			message += QString(" (%1)").arg(tr("#disabled"));
		}
	}

	mErasingTimer.stop();

	emit result("", message);

	mErasingTimer.start(CBillAcceptorTest::EscrowMessageTimeout);

	// Выбрасываем купюру
	mBillAcceptor->reject();
}

//------------------------------------------------------------------------------
void BillAcceptorTest::onEraseMessage()
{
	emit result("", " ");
}

//------------------------------------------------------------------------------
void BillAcceptorTest::onStatusChanged(EWarningLevel::Enum aWarningLevel, const QString & aTranslation, int aStatus)
{
	if ((aStatus == ECashAcceptorStatus::Cheated) || (aWarningLevel == EWarningLevel::Error))
	{
		mErasingTimer.stop();

		emit result("", aTranslation);
	}
}

//------------------------------------------------------------------------------
