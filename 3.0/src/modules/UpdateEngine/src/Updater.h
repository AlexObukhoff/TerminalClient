/* @file Система обновления. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QSet>
#include <QtCore/QTimer>
#include <QtCore/QSharedPointer>
#include <QtCore/QSignalMapper>
#include <Common/QtHeadersEnd.h>

#include <NetworkTaskManager/NetworkTaskManager.h>

#include "Component.h"
#include "WindowsBITS.h"

class QDomElement;

//---------------------------------------------------------------------------
namespace CUpdater
{
	const char Name[] = "Updater";
}

//---------------------------------------------------------------------------
namespace CUpdaterErrors
{
	enum Enum
	{
		OK,
		UnknownError,
		NetworkError,
		ParseError,
		DeployError,
		UpdateBlocked,
		BitsInProgress,
	};
}

//---------------------------------------------------------------------------
class Updater : public QObject
{
	Q_OBJECT

public:
	bool bitsIsError();
	typedef QList<QSharedPointer<Component>> TComponentList;

	explicit Updater(QObject * aParent);
	explicit Updater(const QString & aConfigURL, const QString & aUpdateURL, const QString & aVersion, const QString & aAppId, const QString & aConfiguration, const QString & aPointId);

	bool bitsLoadState(QStringList * aParameters = nullptr);

	bool bitsIsComplete();

	void bitsCompleteAllJobs(int & aCount, int & aCountComplete, int & aCountError);

	/// Устанавливает прокси в формате хост:порт:имя пользователя:пароль:тип.
	void setProxy(const QString & aProxy);

	/// Указать принимаемый банковский ключ
	void setAcceptedKeys(const QString & aAcceptedKeys);

	/// Установить рабочую папку.
	void setWorkingDir(const QString & aDir);

	QString getWorkingDir() const { return mWorkingDir; }

	/// Добавить папку в скисок исключений. Файлы из этих папок не будут удалены после обновления,
	/// а также в случае обновления для них не будут созданы бекапы.
	void addExceptionDirs(const QStringList & aDirs);

	/// Добавить список компонент, подлежащих обновлению
	void addComponentForUpdate(const QStringList & aComponents);

	/// Указывает список опциональных компонент (можно пропустить при обновлении)
	void setOptionalComponents(const QStringList & aComponents);

	/// Указывает список файлов, которые обязательно должны быть в конфигурации
	void setConfigurationRequiredFiles(const QStringList & aRequredFiles);

	/// указываем контрольную сумму для скачиваемого архива
	void setMD5(const QString & aMD5);

	/// Запуск процедуры валидации установленного ПО.
	int checkInterity();

	/// Флаг - можно ли использовать BITS
	void useBITS(bool aUseBITS, int aJobPriority = CBITS::HIGH);

public slots:
	/// Запуск процедуры обновления.
	void runUpdate();

	/// Запрос на скачивание архива по адресу mURL и его распаковка в папку mWorkingDir.
	void downloadPackage();

private:
	/// Скачивает описание пакета обновлений и преобразует его в набор компонент.
	CUpdaterErrors::Enum getComponents(TComponentList & aComponents);

	/// Загрузка содержимого описания компонентов
	CUpdaterErrors::Enum loadComponents(const QByteArray & aContent, Updater::TComponentList & aComponents, QString & aRevision);

	/// Скачивает и устанавливает обновление.
	void downloadComponents(const TComponentList & aComponents);

	/// Сканирует рабочий каталог и свозвращает список файлов и контрольных сумм.
	TFileList getWorkingDirStructure() const throw (std::exception);

	/// Удаляет пустые папки.
	int removeEmptyFolders(const QString & aDir);

	/// возвращает признак что мы закачиваем не все компоненты
	bool haveSkippedComponents() const;

	/// возвращает пересечение первого списка файлов со вторым поиском только по имени
	TFileList intersectByName(const TFileList & aList1, const TFileList & aList2);

	/// Удаляет из первого списка файлов, второй поиском только по имени
	void substractByName(TFileList & aList1, const TFileList & aList2);

public:
	/// Возвращает список файлов в каталоге aPath.
	TFileList getWorkingDirStructure(const QString & aPath) const throw (std::exception);

	/// Удаляет список файлов.
	void deleteFiles(const TFileList & aFiles, bool aIgnoreError = false) throw (std::exception);

	/// Делает копирование файловой структуры (папки + каталоги).
	void copyFiles(const QString & aSrcDir, const QString & aDstDir, const TFileList & aFiles, bool aIgnoreError = false) throw (std::exception);

signals:
	/// Сигнал прогресса загрузки.
	void progress(int aAction);

	/// Сигнал о начала разворачивания архива
	void deployment();

	/// Сигнал завершения.
	void done(CUpdaterErrors::Enum aError);

	/// Загрузка завершена.
	void downloadAccomplished();

	/// Система обновлений формирует описание - ждем 5 минут
	void updateSystemIsWaiting();

private slots:
	void download();
	void deploy();
	void packageDownloaded(QObject * aPackage);
	void deployDownloadedPackage(QObject * aPackage);
	void downloadComplete();

	void showProgress();

private:
	/// Обработка результатов проверки контрольной суммы
	void checkTaskVerifierResult(NetworkTask * task);

	/// Сохранить текущую конфигурацию на диске
	void saveUpdateConfiguration();
	
	/// Получить список конфигураций обновления, сохраненных на диске
	QStringList getSavedConfigurations();

	QByteArray loadUpdateConfiguration(const QString & aRevision);
	bool validateConfiguration(const TComponentList & aComponents);

private:
	CBITS::CopyManager mBitsManager;

	/// Перейти к скачиванию файлов с помощью BITS
	bool bitsDownload();

	bool bitsInProgress();

	void bitsCleanupOldTasks();

	/// Получить имя задания для bits
	QString bitsJobName() const;

	void bitsSaveState();

private:
	QString mConfigURL;
	QString mUpdateURL;
	QString mVersion;
	QString mAP;
	QString mAcceptedKeys;
	QString mAppId;         // Id приложения, например TerminalClient
	QString mConfiguration; // Конфигурация, например 17ru
	QStringList mUpdateComponents; // список компонент, подлежащих обновлению
	QStringList mOptionalComponents; // список компонент, необязательных к обновлению
	QStringList mRequredFiles;
	QString mMD5;           // контрольная сумма для скачиваемой конфигурации или пользовательского архива

	NetworkTaskManager mNetworkTaskManager;

	QString mWorkingDir;          // Рабочий каталог
	QStringList mExceptionDirs;   // Список подкаталогов в которых обновление не ведется.

	//
	// Следующие поля описывают состояние во время установки конкретного обновления.

	// Содержимое конкретного обновления
	QByteArray mComponentsContent;
	QByteArray mComponentsSignature;
	QString mComponentsRevision;

	// Список обновляемых компонентов.
	TComponentList mComponents;

	// Спискок задач для закачки.
	QList<NetworkTask *> mActiveTasks;

	// Размер таска на момент начала загрузки
	qint64 mCurrentTaskSize;

	// При попытке получить обновление сервер просил подождать
	bool mWaitUpdateServer;

	// Число неудачных попыток скачать файл.
	int mFailCount;

	QSignalMapper mMapper;

private:
	QTimer mProgressTimer;
	int mAllTasksCount;
	int mProgressPercent;

	bool mUseBITS;
	int mJobPriority;
};

//---------------------------------------------------------------------------
