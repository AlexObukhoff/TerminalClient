/* @file Вспомогательный экран, зкарывающий рабочий стол. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QDateTime>
#include <QtCore/QList>
#include <QtWidgets/QWidget>
#include <QtGui/QCloseEvent>
#include "ui_SplashScreen.h"
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/ILogable.h>

//----------------------------------------------------------------------------
/// Вспомогательный экран, закрывающий рабочий стол. Предоставляет доступ к сервисному меню.
class SplashScreen : public QWidget, protected ILogable
{
	Q_OBJECT

public:
	/// Конструктор.
	SplashScreen(const QString & aLog, QWidget * aParent = 0);

	/// Деструктор.
	virtual ~SplashScreen();

	/// Сворачивает окно вместо его закрытия.
	virtual void closeEvent(QCloseEvent * aEvent);

	/// Отслеживает клики по экрану для доступа в сервисное меню.
	virtual bool eventFilter(QObject * aObject, QEvent * aEvent);

	/// Установка произвольного изображения в качестве фона для защитного экрана.
	virtual void setCustomBackground(const QString & aPath);

public slots:
	/// Показывает значок для состояния aState, связанный с отправителем aSender.
	virtual void setState(const QString & aSender, const QString & aState);

	/// Убирает все показанные значки для aSender.
	virtual void removeStates(const QString & aSender);

signals:
	/// Сигнал о клике в определённую зону.
	void clicked(int aArea);

	/// Сигнал о закрытии окна по внешнему сигналу
	void hidden();

private slots:
	// Выполнение полезной инициализации после загрузки.
	void onInit();

private: // Методы
	typedef QList<QPair<int, QRectF> > TAreas;

	void updateAreas();
	bool testPoint(const TAreas::value_type & aArea, const QPoint & aPoint) const;

private: // Данные
	struct SState
	{
		SState(const QString & aSender, const QString & aState, QWidget * aWidget)
			: sender(aSender), state(aState), widget(aWidget), date(QDateTime::currentDateTime()) {}

		QString sender;
		QString state;
		QWidget * widget;
		QDateTime date;
	};

	typedef QList<SState> TStateList;

	Ui::SplashScreenClass ui;

	TAreas mAreas;
	TStateList mStates;
};

//----------------------------------------------------------------------------
