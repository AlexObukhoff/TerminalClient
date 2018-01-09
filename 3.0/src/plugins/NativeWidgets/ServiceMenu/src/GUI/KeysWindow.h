/* @file Окошко для генерации ключей в сервисном меню и первоначальной настройке. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QFutureWatcher>
#include "ui_keysWindow.h"
#include <Common/QtHeadersEnd.h>

class ServiceMenuBackend;

//------------------------------------------------------------------------
namespace CKeysWindow
{
	const QString WarningStyleSheet = "background-color: rgb(255, 192, 192);";
	const QString DefaultStyleSheet = "";
}

//------------------------------------------------------------------------
class KeysWindow : public QFrame, protected Ui_KeysWindow
{
	Q_OBJECT

public:
	KeysWindow(ServiceMenuBackend * aBackend, QWidget * aParent);

	virtual ~KeysWindow();

	/// Начальная инициализация формы.
	virtual void initialize(bool aHasRuToken, bool aRutokenOK);

	// Сгенерировать ключи
	void doGenerate();

	/// Сохраняет сгенерированные ключи.
	bool save();

signals:
	/// Начало и конец процедуры создания ключей.
	void beginGenerating();
	void endGenerating();

	/// Сигнал об ошибке во время создания или регистрации ключей.
	void error(QString aError);

protected slots:
	void onCreateButtonClicked();
	void onRepeatButtonClicked();

	void onGenerateTaskFinished();

private:
	void SetStyleSheet(QWidget * widget, const QString & styleSheet)
	{
		widget->setStyleSheet(QString(widget->metaObject()->className()) + "{" + styleSheet + "}");
	}

protected:
	QVariantMap mTaskParameters;

	QFutureWatcher<bool> mGenerateTaskWatcher;

	ServiceMenuBackend * mBackend;
};

//------------------------------------------------------------------------
