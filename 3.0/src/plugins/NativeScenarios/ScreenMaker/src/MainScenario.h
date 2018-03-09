/* @file Плагин сценария для создания скриншотов */

#pragma once

#include <windows.h>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtWidgets/QWidget>
#include <QtWidgets/QWidget>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QImage>
#include <QtGui/QClipboard>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtCore/QDebug>
#include <Common/QtHeadersEnd.h>

// Modules
#include <ScenarioEngine/Scenario.h>

// Plugin SDK
#include <SDK/Plugins/IFactory.h>
#include <SDK/Plugins/IPlugin.h>
#include <SDK/GUI/IGraphicsItem.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>

class DrawAreaWindow : public QWidget
{
	Q_OBJECT

public:
	DrawAreaWindow(SDK::PaymentProcessor::ICore * aCore, const QRect & aRect) :
		mCore(aCore),
		mPressed(false)
	{ 
		//setMinimumWidth(aRect.width()); setMinimumHeight(aRect.height());
		setGeometry(aRect);
		setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
	}

public:
	void updateImage(const QImage & aImage) { mRects.clear(); mImage = aImage; }

public:
	virtual void keyPressEvent(QKeyEvent * event)
	{
		if (event->key() == Qt::Key_Return)
		{
			QPainter painter;

			QPen pen;
			pen.setColor(QColor(0xff, 0x00, 0x00));
			pen.setWidth(5);

			painter.begin(&mImage);
			painter.setPen(pen);
			painter.setBrush(Qt::NoBrush);
			painter.drawRects(mRects);
			painter.end();

			QClipboard *clipboard = QApplication::clipboard();
			clipboard->setImage(mImage);
			
			mFilePath = QFileDialog::getSaveFileName(this, "Save Scene", mFilePath, "Image (*.png)");
			mImage.save(mFilePath);
		}

		finish();

		QWidget::keyPressEvent(event);
	}

	virtual void mouseMoveEvent(QMouseEvent * aEvent)
	{
		if (mPressed)
		{
			mCurrentPos = aEvent->pos();
			//update();
		}
	}

	virtual void mousePressEvent(QMouseEvent * aEvent)
	{
		if (aEvent->button() == Qt::LeftButton)
		{
			mPressed = true;
			mStartPos = mCurrentPos = aEvent->pos();
		}
		else if (aEvent->button() == Qt::RightButton && mRects.count() > 0)
		{
			mRects.remove(mRects.count() - 1);
			update();
		}
	}

	virtual void mouseReleaseEvent(QMouseEvent * aEvent)
	{
		if (aEvent->button() == Qt::LeftButton)
		{
			mPressed = false;
			mRects.append(QRect(mStartPos, aEvent->pos()));
		}
	}

	virtual void paintEvent(QPaintEvent * /*event*/)
	{
		QPainter painter(this);
		painter.setRenderHint(QPainter::Antialiasing, false);

		painter.drawImage(0, 0, mImage);

		QPen pen;
		pen.setWidth(5);

		painter.setBrush(Qt::NoBrush);

		// border
		pen.setColor(QColor(0x00, 0xff, 0x00));
		painter.setPen(pen);
		painter.drawRect(0, 0, mImage.width(), mImage.height());

		// current frame
		pen.setColor(QColor(0xff, 0x00, 0x00));
		painter.setPen(pen);
		painter.drawRects(mRects);

		if (mPressed)
		{
			painter.drawRect(QRect(mStartPos, mCurrentPos));
		}

		update();
	}

	void closeEvent(QCloseEvent *event)
	{
		finishScenario();
	}

private:
	void finish()
	{
		mRects.clear();
		finishScenario();
	}

	void finishScenario()
	{
		QVariantMap params;
		params["signal"] = "finish";

		SDK::PaymentProcessor::Event e(SDK::PaymentProcessor::EEventType::UpdateScenario, "", QVariant::fromValue(params));
		mCore->getEventService()->sendEvent(e);
	}

private:
	QPoint mStartPos;
	QPoint mCurrentPos;
	bool mPressed;
	QImage mImage;
	QVector<QRect> mRects;
	SDK::PaymentProcessor::ICore * mCore;
	QString mFilePath;
};

// Project

class IApplication;

namespace SDK {
	namespace PaymentProcessor {
		class ICore;

		namespace Scripting {
			class Core;
		}
	}

	namespace Plugin {
		class IEnvironment;
	}
}

namespace ScreenMaker
{

//---------------------------------------------------------------------------
class MainScenario : public GUI::Scenario
{
	Q_OBJECT

public:
	MainScenario(SDK::PaymentProcessor::ICore * aCore, ILog * aLog);
	virtual ~MainScenario();

public:
	/// Запуск сценария.
	virtual void start(const QVariantMap & aContext);

	/// Остановка сценария.
	virtual void stop();

	/// Приостановить сценарий с возможностью последующего возобновления.
	virtual void pause();

	/// Продолжение после паузы.
	virtual void resume(const QVariantMap & aContext);

	/// Инициализация сценария.
	virtual bool initialize(const QList<GUI::SScriptObject> & aScriptObjects);

	/// Обработка сигнала из активного состояния с дополнительными аргументами.
	virtual void signalTriggered(const QString & aSignal, const QVariantMap & aArguments = QVariantMap());

	/// Обработчик таймаута
	virtual void onTimeout();

	/// Возвращает false, если сценарий не может быть остановлен в текущий момент.
	virtual bool canStop();

public slots:
	/// Текущее состояние.
	virtual QString getState() const;

private:
	QVariantMap mContext;
	QString mLastSignal;
	SDK::PaymentProcessor::ICore * mCore;
	DrawAreaWindow * mDrawAreaWindow;
};

}

//--------------------------------------------------------------------------

