/* @file Базовый класс для сценариев. */

#include "Scenario.h"

namespace GUI {

//---------------------------------------------------------------------------
Scenario::Scenario(const QString & aName, ILog * aLog)
	: mName(aName), mDefaultTimeout(0)
{
	setLog(aLog);
	connect(&mTimeoutTimer, SIGNAL(timeout()), SLOT(onTimeout()));

	mTimeoutTimer.setSingleShot(true);
}

//---------------------------------------------------------------------------
Scenario::~Scenario()
{
}

//---------------------------------------------------------------------------
QString Scenario::getName() const
{
	return mName;
}

//---------------------------------------------------------------------------
void Scenario::resetTimeout()
{
	if (mTimeoutTimer.interval() > 0)
	{
		mTimeoutTimer.start();
	}
}

//---------------------------------------------------------------------------
void Scenario::setStateTimeout(int aSec)
{
	if (aSec > 0)
	{
		mTimeoutTimer.setInterval(aSec * 1000);
		mTimeoutTimer.start();
	}
}

//---------------------------------------------------------------------------
void Scenario::setLog(ILog * aLog)
{
	if (!getLog())
	{
		ILogable::setLog(aLog);
	}
}

} // namespace GUI

//---------------------------------------------------------------------------
