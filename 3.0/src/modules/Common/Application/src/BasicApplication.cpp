/* @file Классы приложений. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <Common/QtHeadersEnd.h>

#ifdef Q_OS_WIN
#include <Objbase.h>
#include <comutil.h>
#include <comip.h>
#include <comdef.h>
#endif

// Модули
#include <SysUtils/ISysUtils.h>
#include <DebugUtils/DebugUtils.h>

// Проект
#include <Common/Application.h>
#include <Common/SafeApplication.h>

//---------------------------------------------------------------------------
BasicApplication * BasicApplication::mInstance = 0;

//---------------------------------------------------------------------------
LONG WINAPI ExceptionFilter(EXCEPTION_POINTERS * aException)
{
	QStringList stack;
	DumpCallstack(stack, aException->ContextRecord);

	qCritical() << "Exited due to unknown exception. Callstack:\n" + stack.join("\n");
	
	return EXCEPTION_EXECUTE_HANDLER;
}

//---------------------------------------------------------------------------
bool SafeQApplication::notify(QObject * aReceiver, QEvent * aEvent)
{
	__try
	{
		return QApplication::notify(aReceiver, aEvent);
	}
	__except (ExceptionFilter(GetExceptionInformation()))
	{
		abort();
	}
}

//---------------------------------------------------------------------------
BasicApplication::BasicApplication(const QString& aName, const QString& aVersion, int aArgumentCount, char ** aArguments)
	: mName(aName), mVersion(aVersion), mArgumentCount(aArgumentCount), mArguments(aArguments)
{
	QFileInfo info(QString::fromLocal8Bit(mArguments[0]));

	QString settingsFilePath = QDir::toNativeSeparators(info.absolutePath() + "/" + info.completeBaseName() + ".ini");
	mSettings = QSharedPointer<QSettings>(new QSettings(settingsFilePath, QSettings::IniFormat));
	mSettings->setIniCodec("UTF-8");

	mWorkingDirectory = info.absolutePath();

	if (mSettings->contains("common/working_directory"))
	{
		QString directory = mSettings->value("common/working_directory").toString();
		mWorkingDirectory = QDir::toNativeSeparators(QDir::cleanPath((QDir::isAbsolutePath(directory) ? "" : (info.absolutePath() + "/")) + directory));
	}

	mInstance = this;

	// Выставим уровень логирования
	QSettings userSettings(mWorkingDirectory + QDir::separator() + mSettings->value("common/user_data_path").toString()
		+ QDir::separator() + "user.ini", QSettings::IniFormat);
	
	if (userSettings.contains("log/level"))
	{
		int level = userSettings.value("log/level").toInt();
		
		if (level < LogLevel::Off)
		{
			level = LogLevel::Off;
		}
		else if (level > LogLevel::Max)
		{
			level = LogLevel::Max;
		}
			  
		ILog::setGlobalLevel(static_cast<LogLevel::Enum>(level));
	}
	
	mLog = ILog::getInstance(mName);

	// Выводим стандартный заголовок в лог
	getLog()->write(LogLevel::Normal, "**********************************************************");
	getLog()->write(LogLevel::Normal, QString("Application: %1").arg(getName()));
	getLog()->write(LogLevel::Normal, QString("File: %1").arg(getFileName()));
	getLog()->write(LogLevel::Normal, QString("Version: %1").arg(getVersion()));
	getLog()->write(LogLevel::Normal, QString("Operating system: %1").arg(getOSVersion()));
	getLog()->write(LogLevel::Normal, "**********************************************************");

#ifdef Q_OS_WIN
	// Инициализация COM Security для работы с WMI
	CoInitializeEx(0, COINIT_APARTMENTTHREADED);
	HRESULT hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);

	if (FAILED(hr))
	{
		getLog()->write(LogLevel::Error, QString("CoInitializeSecurity failed: %1.").arg(QString::fromWCharArray(_com_error(hr).ErrorMessage())));
	}
#endif
}

//---------------------------------------------------------------------------
BasicApplication::~BasicApplication()
{
	mInstance = 0;
}

//---------------------------------------------------------------------------
QString BasicApplication::getOSVersion() const
{
	return ISysUtils::getOSVersionInfo();
}

//---------------------------------------------------------------------------
QString BasicApplication::getFileName() const
{
	return QString::fromLocal8Bit(mArguments[0]);
}

//---------------------------------------------------------------------------
QString BasicApplication::getName() const
{
	return mName;
}

//---------------------------------------------------------------------------
QString BasicApplication::getVersion() const
{
	return mVersion;
}

//---------------------------------------------------------------------------
ILog * BasicApplication::getLog() const
{
	return mLog;
}

//---------------------------------------------------------------------------
BasicApplication * BasicApplication::getInstance()
{
	return mInstance;
}

//---------------------------------------------------------------------------
QString BasicApplication::getWorkingDirectory() const
{
	return mWorkingDirectory;
}

//---------------------------------------------------------------------------
// Возвращает настройки приложения
QSettings & BasicApplication::getSettings() const
{
	return *mSettings;
}

//---------------------------------------------------------------------------
