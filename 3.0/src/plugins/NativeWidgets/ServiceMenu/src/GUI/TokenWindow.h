/* @file Окошко для генерации ключей в сервисном меню и первоначальной настройке. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QFutureWatcher>
#include "ui_tokenWindow.h"
#include <Common/QtHeadersEnd.h>

// Modules
#include <Crypt/ICryptEngine.h>

class ServiceMenuBackend;

//------------------------------------------------------------------------
namespace CTokenWindow
{
	const QString WarningStyleSheet = "background-color: rgb(255, 192, 192);";
	const QString DefaultStyleSheet = "";
}

//------------------------------------------------------------------------
class TokenWindow : public QFrame, protected Ui_TokenWindow
{
	Q_OBJECT

public:
	TokenWindow(ServiceMenuBackend * aBackend, QWidget * aParent);

	virtual ~TokenWindow();

	/// Начальная инициализация формы.
	virtual void initialize(const CCrypt::TokenStatus & aStatus);

	// Отформатировать токен
	void doFormat();

signals:
	/// Начало и конец процедуры создания ключей.
	void beginFormat();
	void endFormat();

	/// Сигнал об ошибке во время создания или регистрации ключей.
	void error(QString aError);

protected slots:
	void onFormatButtonClicked();
	void onFormatTaskFinished();

private:
	void updateUI(const CCrypt::TokenStatus & aStatus);

protected:
	QVariantMap mTaskParameters;

	QFutureWatcher<bool> mFormatTaskWatcher;

	ServiceMenuBackend * mBackend;
};

//------------------------------------------------------------------------
