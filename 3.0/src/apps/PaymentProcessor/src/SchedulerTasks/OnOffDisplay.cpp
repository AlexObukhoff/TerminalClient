/* @file Реализация задачи включения энергосберегающего режима. */

// SDK
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Core/EventTypes.h>

// Modules
#include <System/IApplication.h>
#include <Common/Application.h>
#include <SysUtils/ISysUtils.h>

// Project
#include "OnOffDisplay.h"
#include "Services/ServiceNames.h"
#include "Services/GUIService.h"

namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
OnOffDisplay::OnOffDisplay(const QString & aName, const QString & aLogName, const QString & aParams) 
	: ITask(aName, aLogName, aParams),
	ILogable(aLogName),
	mEnable(false),
	mType(Standby)
{
	QStringList params = aParams.split(";");

	if (params.size() >= 2)
	{
		mFrom = QTime::fromString(params[0], "hh:mm");
		mTill = QTime::fromString(params[1], "hh:mm");

		mEnable = mFrom.isValid() && !mFrom.isNull() && mTill.isValid() && !mTill.isNull();
	}

	if (params.size() > 2)
	{
		if (params[2].compare("saver", Qt::CaseInsensitive) == 0)
		{
			mType = ScreenSaver;
		}

		if (params[2].compare("shutdown", Qt::CaseInsensitive) == 0)
		{
			mType = Shutdown;
		}
	}

}

//---------------------------------------------------------------------------
OnOffDisplay::~OnOffDisplay()
{
}

//---------------------------------------------------------------------------
void OnOffDisplay::execute()
{
	if (mEnable)
	{
		QTime now = QTime::currentTime();

		bool off = mTill < mFrom ? (now > mFrom || now < mTill) : (now > mFrom && now < mTill);

		if (off)
		{
			IApplication * app = dynamic_cast<IApplication *>(BasicApplication::getInstance());

			if (!app->getCore()->getService(CServices::GUIService)->canShutdown())
			{
				toLog(LogLevel::Normal, "Power saving disabled by GUI service");

				emit finished(mName, false);
				return;
			}

			switch (mType)
			{
			case ScreenSaver:
				ISysUtils::runScreenSaver();
				break;

			case Shutdown:
				app->getCore()->getEventService()->sendEvent(PPSDK::Event(PPSDK::EEventType::Shutdown));
				break;

			default:
				ISysUtils::displayOn(false);
			}
		}
		else
		{
			ISysUtils::displayOn(true);
		}

		toLog(LogLevel::Normal, QString("Display %1").arg(off ? "OFF" : "ON"));
	}

	emit finished(mName, true);
}

//---------------------------------------------------------------------------
bool OnOffDisplay::subscribeOnComplete(QObject * aReceiver, const char * aSlot)
{
	return connect(this, SIGNAL(finished(const QString &, bool)), aReceiver, aSlot);
}

//---------------------------------------------------------------------------
