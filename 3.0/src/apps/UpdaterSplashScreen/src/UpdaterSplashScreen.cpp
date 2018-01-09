#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

#include "UpdaterSplashScreen.h"

//---------------------------------------------------------------------------
UpdaterSplashScreen::UpdaterSplashScreen(QWidget *parent, Qt::WFlags flags)
	: QWidget(parent, flags),
	m_progressFile(QCoreApplication::applicationDirPath() + "/progress.txt"),
	m_step(0),
	m_stepCount(0),
	m_status("Preparing for update...")
{
	ui.setupUi(this);

	updateScreen();

	connect(&m_watcher, SIGNAL(fileChanged(const QString &)), this, SLOT(onFileUpdated(const QString &)));
	m_watcher.addPath(m_progressFile);
}

//---------------------------------------------------------------------------
UpdaterSplashScreen::~UpdaterSplashScreen()
{

}

//---------------------------------------------------------------------------
void UpdaterSplashScreen::onFileUpdated(const QString & aPath)
{
	QFile file(aPath);
	if (!file.open(QIODevice::ReadOnly))
	{
		return;
	}

	QString line;
	while (true)
	{
		QString buf = QString::fromLatin1(file.readLine().data()).replace("\n", "").replace("\r", "");
		if (buf.isEmpty())
			break;

		line = buf;
	}

	if (line.isEmpty())
		return;

	QStringList params = line.split(";");
	if (params.count() > 2)
	{
		m_stepCount = params.takeFirst().toInt();
		m_step = params.takeFirst().toInt();
		m_status = params.takeFirst();
	}
	else
	{
		m_stepCount = 0;
		m_step = 0;
		m_status = "Waiting...";
	}

	if (m_status.toLower() == "close")
	{
		QCoreApplication::quit();
		return;
	}

	updateScreen();
}

//---------------------------------------------------------------------------
void UpdaterSplashScreen::updateScreen()
{
	ui.progressBar->setMaximum(m_stepCount);
	ui.progressBar->setValue(m_step);
	ui.infoLabel->setText(m_status);
}

//---------------------------------------------------------------------------