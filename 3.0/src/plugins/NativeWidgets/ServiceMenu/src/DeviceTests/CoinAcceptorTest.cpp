/* @file Класс для тестирования купюроприемников. */

// Project
#include "CoinAcceptorTest.h"

//------------------------------------------------------------------------------
namespace CCoinAcceptorTest
{
	const QString TestStacked = QT_TRANSLATE_NOOP("CoinAcceptorTest", "#test_stacked");
}

//------------------------------------------------------------------------------
CoinAcceptorTest::CoinAcceptorTest(SDK::Driver::IDevice * aDevice)
{
	mCoinAcceptor = dynamic_cast<SDK::Driver::ICashAcceptor *>(aDevice);
}

//------------------------------------------------------------------------------
QList<QPair<QString, QString>> CoinAcceptorTest::getTestNames() const
{
	return QList<QPair<QString, QString>>() << qMakePair(CCoinAcceptorTest::TestStacked, tr("#insert_coin"));
}

//------------------------------------------------------------------------------
bool CoinAcceptorTest::run(const QString & aName)
{
	if (aName == CCoinAcceptorTest::TestStacked)
	{
		if (mCoinAcceptor->isDeviceReady() && mCoinAcceptor->setEnable(true))
		{
			mCoinAcceptor->subscribe(SDK::Driver::ICashAcceptor::StackedSignal, this, SLOT(onStacked(SDK::Driver::TParList)));
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------
void CoinAcceptorTest::stop()
{
	mCoinAcceptor->setEnable(false);
	mCoinAcceptor->unsubscribe(SDK::Driver::ICashAcceptor::StackedSignal, this);
}

//------------------------------------------------------------------------------
bool CoinAcceptorTest::isReady()
{
	return mCoinAcceptor && mCoinAcceptor->isDeviceReady();
}

//------------------------------------------------------------------------------
bool CoinAcceptorTest::hasResult()
{
	return true;
}

//------------------------------------------------------------------------------
void CoinAcceptorTest::onStacked(SDK::Driver::TParList aNotes)
{
	emit result("", QString("%1 %2 %3").arg(tr("#coin_is_stacked")).arg(aNotes.first().nominal).arg(aNotes.first().currency));
}

//------------------------------------------------------------------------------
