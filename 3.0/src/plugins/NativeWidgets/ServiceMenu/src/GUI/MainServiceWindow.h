/* @file Главное окно сервисного меню. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTimer>
#include "ui_MainServiceWindow.h"
#include <Common/QtHeadersEnd.h>

class WizardWindow;
class IServiceWindow;
class ServiceMenuBackend;

//------------------------------------------------------------------------
class MainServiceWindow : public QWidget, public Ui::MainServiceWindow
{
	Q_OBJECT

public:
	MainServiceWindow(ServiceMenuBackend * aBackend, QWidget * aParent = 0);

	bool initialize();
	void shutdown();

	QWidget * getMainWidget()
	{
		return wPasswordPage;
	}

	WizardWindow * getScreenByName(const QString & aScreenName);

	// Выход из сервисного меню
	bool closeServiceMenu(bool aExitByNotify, const QString & aMessage, bool aStartIdle = false);

private slots:
	// Активация/деактивация вкладок
	void onCurrentPageChanged(int aIndex);

	// Авторизация пользователя
	void onProceedLogin();

	void onCancelAuthorization();

	// Выход из сервисного меню
	void onCloseServiceMenu();

	// Перезагрузка приложения
	void onRebootApplication();

	// Останов приложения
	void onStopApplication();

	// Перезагрузка терминала
	void onRebootTerminal();

	// Блокирование/разблокирование терминала
	void onToggleLock();

	// Для обработки сигналов цифровой клавиатуры
	void onBackspaceClicked();
	void onClearClicked();
	void onDigitClicked();

	void onIdleTimeout();
	void onDateTimeRefresh();

	void onAbstractButtonClicked();

private:
	void applyConfiguration();
	bool applyAccessRights();
	void closeMenu(bool aStartIdle = false);

	void connectAllAbstractButtons(QWidget * aParentWidget);

private:
	int mCurrentPageIndex;
	QTimer mIdleTimer;
	QTimer mDateTimeTimer;
	QVariantMap mTerminalInfo;

	QList<IServiceWindow *> mServiceWindowList;
	ServiceMenuBackend * mBackend;
};

//------------------------------------------------------------------------
