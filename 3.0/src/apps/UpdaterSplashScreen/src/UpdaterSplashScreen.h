/* @file Виджет заставки. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QFileSystemWatcher>
#include <QtCore/QFile>
#include <QtGui/QWidget>
#include "ui_updatersplashscreen.h"
#include <Common/QtHeadersEnd.h>

//---------------------------------------------------------------------------
class UpdaterSplashScreen : public QWidget
{
	Q_OBJECT

public:
	UpdaterSplashScreen(QWidget *parent = 0, Qt::WFlags flags = 0);
	~UpdaterSplashScreen();

private slots:
	void onFileUpdated(const QString & aPath);

private:
	void updateScreen();

private:
	Ui::mainWindow ui;
	QString m_progressFile;
	QFileSystemWatcher m_watcher;
	QFile m_file;

	QString m_status;
	int m_step;
	int m_stepCount;
};

//---------------------------------------------------------------------------
