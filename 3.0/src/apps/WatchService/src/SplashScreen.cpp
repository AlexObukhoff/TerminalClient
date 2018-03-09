/* @file Вспомогательный экран, закрывающий рабочий стол. */

// STL
#include <algorithm>

// boost
#include <boost/bind.hpp>
#include <boost/bind/placeholders.hpp>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTimer>
#include <QtCore/QDir>
#include <QtWidgets/QHBoxLayout>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>
#include <SDK/PaymentProcessor/Settings/DealerSettings.h>

// Modules
#include <Common/Application.h>
#include <SettingsManager/SettingsManager.h>

// Project
#include "SplashScreen.h"

//----------------------------------------------------------------------------
namespace CSplashScreen
{
	const char DefaultBackgroundStyle[] = "QWidget#wgtBackground { background-color: #335599; }";
	const char CustomBackgroundStyle[] = "QWidget#wgtBackground { border-image: url(%1); }";
	const char StateImagesPath[] = ":/images/states/";
	const char StateImageExtension[] = ".png";
	const int MinStateShowSeconds = 2;
}

//----------------------------------------------------------------------------
SplashScreen::SplashScreen(const QString & aLog, QWidget * aParent):
	ILogable(aLog),
#ifdef _DEBUG
	QWidget(aParent)
#else
	QWidget(aParent, Qt::WindowStaysOnTopHint)
#endif
{
	ui.setupUi(this);

	// FIXME: временно для #18645
	//ui.lblSupportPhone->hide();
	//ui.lblTerminalName->hide();

	QTimer::singleShot(0, this, SLOT(onInit()));

	installEventFilter(this);
}

//----------------------------------------------------------------------------
SplashScreen::~SplashScreen()
{
}

//----------------------------------------------------------------------------
void SplashScreen::onInit()
{
}

//----------------------------------------------------------------------------
void SplashScreen::closeEvent(QCloseEvent * aEvent)
{
	toLog(LogLevel::Normal, "Close splash screen by event.");

	aEvent->ignore();
	showMinimized();

	emit hidden();
}

//----------------------------------------------------------------------------
bool SplashScreen::eventFilter(QObject * aObject, QEvent * aEvent)
{
	if (aEvent->type() == QEvent::MouseButtonPress)
	{
		// Проверим, что был сделан клик по некоторой области
		QMouseEvent * mouseEvent = dynamic_cast<QMouseEvent *>(aEvent);
		if (mouseEvent)
		{
			if (mAreas.isEmpty())
			{
				// Размеры виджета до первого показа кривые
				updateAreas();
			}

			TAreas::iterator area = std::find_if(mAreas.begin(), mAreas.end(), 
				boost::bind(&SplashScreen::testPoint, this, _1, mouseEvent->pos()));

			if (area != mAreas.end())
			{
				emit clicked(area->first);
			}
		}
	}

	return QWidget::eventFilter(aObject, aEvent);
}

//----------------------------------------------------------------------------
bool SplashScreen::testPoint(const TAreas::value_type & aArea, const QPoint & aPoint) const
{
	return aArea.second.contains(aPoint);
}

//----------------------------------------------------------------------------
void SplashScreen::updateAreas()
{
	int width = rect().width();
	int height = rect().height();

	mAreas 
		<< qMakePair(1, QRectF(QPointF(0, 0), QPointF(0.33 * width, 0.33 * height)))
		<< qMakePair(2, QRectF(QPointF(0.66 * width, 0), QPointF(width, 0.33 * height)))
		<< qMakePair(5, QRectF(QPointF(0.33 * width, 0.33 * height), QPointF(0.66 * width, 0.66 * height)))
		<< qMakePair(3, QRectF(QPointF(0, 0.66 * height), QPointF(0.33 * width, height)))
		<< qMakePair(4, QRectF(QPointF(0.66 * width, 0.66 * height), QPointF(width, height)));
}

//----------------------------------------------------------------------------
void SplashScreen::setState(const QString & aSender, const QString & aState)
{
	// Проверим, есть ли такое состояние у нас в списке.
	TStateList::iterator it;

	QString stateCode = aState.left(aState.indexOf('_'));
	QString stateStatus = aState.right(aState.size() - stateCode.size() - 1);

	for (it = mStates.begin(); it != mStates.end(); ++it)
	{
		QString oldStateCode = it->state.left(it->state.indexOf('_'));
		QString oldStateStatus = it->state.right(it->state.size() - oldStateCode.size() - 1);

		if (stateCode == oldStateCode)
		{
			if (stateStatus == oldStateStatus)
			{
				// Уже установлено точно такое же состояние.
				return;
			}

			if (it->date.secsTo(QDateTime::currentDateTime()) < CSplashScreen::MinStateShowSeconds)
			{
				QTimer::singleShot(CSplashScreen::MinStateShowSeconds * 1000, it->widget, SLOT(deleteLater()));
			}
			else
			{
				it->widget->deleteLater();
			}

			mStates.erase(it);

			break;
		}
	}

	QString stateImagePath(CSplashScreen::StateImagesPath + aState + CSplashScreen::StateImageExtension);

	if (!QFile::exists(stateImagePath))
	{
		return;
	}

	QString css = QString("QWidget { background-image: url(%1); min-width: 48; max-width: 48; min-height: 48; max-height: 48; }")
		.arg(stateImagePath);

	QScopedPointer<QWidget> widget(new QWidget(this));
	widget->setStyleSheet(css);

	// FIXME: временно для #21549
	//ui.stateLayout->addWidget(widget.get());

	mStates << SState(aSender, aState, widget.take());
}

//----------------------------------------------------------------------------
void SplashScreen::removeStates(const QString & aSender)
{
	TStateList::iterator it;

	for (it = mStates.begin(); it != mStates.end(); ++it)
	{
		if (aSender == it->sender)
		{
			if (it->date.secsTo(QDateTime::currentDateTime()) < CSplashScreen::MinStateShowSeconds)
			{
				QTimer::singleShot(CSplashScreen::MinStateShowSeconds * 1000, it->widget, SLOT(deleteLater()));
			}
			else
			{
				it->widget->deleteLater();
			}

			it = mStates.erase(it);
			
			--it;
		}
	}
}

//----------------------------------------------------------------------------
void SplashScreen::setCustomBackground(const QString & aPath)
{
	aPath.isEmpty() ?
		setStyleSheet(CSplashScreen::DefaultBackgroundStyle) :
		setStyleSheet(QString(CSplashScreen::CustomBackgroundStyle).arg(aPath));
}

//----------------------------------------------------------------------------
