/* @file Классы приложений. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtWidgets/QApplication>
#include <QtCore/QString>
#include <QtCore/QSettings>
#include <QtCore/QSharedPointer>
#include <QtCore/QStringList>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QTranslator>
#include <Common/QtHeadersEnd.h>

// Модули
#include <Common/ILog.h>

//---------------------------------------------------------------------------
/// Класс абстрактного приложения. Реализует полезные функции (загрузка конфигурации).
class BasicApplication
{
public:
	BasicApplication(const QString & aName, const QString & aVersion, int aArgumentCount, char ** aArguments);
	virtual ~BasicApplication();

	/// Возвращает имя приложения.
	virtual QString getName() const;

	/// Возвращает версию приложения.
	virtual QString getVersion() const;

	/// Возвращает имя исполняемого файла.
	QString getFileName() const;

	/// Возвращает тип/версию операционной системы.
	QString getOSVersion() const;

	/// Возвращает рабочий каталог приложения (может быть задан в .ini файле).
	virtual QString getWorkingDirectory() const;

	/// Возвращает настройки приложения
	virtual QSettings & getSettings() const;

	/// Возвращает лог приложения.
	virtual ILog * getLog() const;

	/// Возвращает экземпляр приложения.
	static BasicApplication * getInstance();

protected:
	static BasicApplication * mInstance;

	QString mName;
	QString mVersion;
	QString mWorkingDirectory;
	QSharedPointer<QSettings> mSettings;
	int mArgumentCount;
	char ** mArguments;
	ILog * mLog;
};

//---------------------------------------------------------------------------
/// Класс приложения, основанного на QCoreApplication/QApplication/QSingleApplication. Вдобавок к базе загружает локализацию.
template <class T>
class BasicQtApplication : public BasicApplication
{
public:
	/// Тип Qt приложения. Может быть QCoreApplication/QApplication/QSingleApplication.
	typedef T TApplication;

	BasicQtApplication(const QString & aName, const QString & aVersion, int & aArgumentCount, char ** aArguments);
	virtual ~BasicQtApplication() {}

	/// Запускает цикл обработки событий.
	int exec();

	/// Возвращает аргументы командной строки.
	QStringList getArguments() const;

	/// Возвращает экземпляр Qt приложения.
	TApplication & getQtApplication();

private:
	TApplication mQtApplication;
	QSharedPointer<QTranslator> mTranslator;
};

//---------------------------------------------------------------------------
// Реализация BasicQtApplication

template<typename T>
BasicQtApplication<T>::BasicQtApplication(const QString & aName, const QString & aVersion, int & aArgumentCount, char ** aArguments)
	: BasicApplication(aName, aVersion, aArgumentCount, aArguments), mQtApplication(aArgumentCount, aArguments)
{
	mQtApplication.setApplicationName(aName);
	mQtApplication.setApplicationVersion(aVersion);

	QFileInfo fileInfo(mQtApplication.applicationFilePath());
	QDir translations(getWorkingDirectory(), QString("%1_*.qm").arg(fileInfo.baseName()));
	
	if (translations.count()) 
	{
		QString translation = translations.entryInfoList().first().absoluteFilePath();
		mTranslator = QSharedPointer<QTranslator>(new QTranslator(&mQtApplication));

		if (mTranslator->load(translation))
		{
			mQtApplication.installTranslator(mTranslator.data());
			getLog()->write(LogLevel::Normal, QString("Translation %1 loaded.").arg(translation));
		}
		else
		{
			getLog()->write(LogLevel::Warning, QString("Failed to load translation %1.").arg(translation));
		}
	}
}

//---------------------------------------------------------------------------
template<typename T>
int BasicQtApplication<T>::exec()
{
	return mQtApplication.exec();
}

//---------------------------------------------------------------------------
template<typename T>
QStringList BasicQtApplication<T>::getArguments() const
{
	return mQtApplication.arguments();
}

//---------------------------------------------------------------------------
template<typename T>
T & BasicQtApplication<T>::getQtApplication()
{
	return mQtApplication;
}

//---------------------------------------------------------------------------
