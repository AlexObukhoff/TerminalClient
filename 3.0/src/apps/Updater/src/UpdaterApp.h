/* @file Класс, реализующий приложение для системы обновления. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtGui/QApplication>
#include <QtCore/QPointer>
#include <QtCore/QSharedPointer>
#include <Common/QtHeadersEnd.h>
#include <Common/Application.h>

// Modules
#include <Common/SafeApplication.h>
#include <WatchServiceClient/IWatchServiceClient.h>
#include <UpdateEngine/Updater.h>
#include <UpdateEngine/ReportBuilder.h>

// This app
#include "SplashScreen.h"

//---------------------------------------------------------------------------
namespace CUpdaterApp
{
	/// таймаут выхода с ошибкой, если не дождались закрытия client.exe
	const int ErrorExitTimeout = 15 * 60 * 1000;

	/// Глобальный таймаут скачивания обновлений
	const int MaxDownloadTime = 4 * 60 * 60 * 1000; // 4 часа

	/// Таймаут попыток повторнго запуска по финишу BITS
	const long BITSCompleteTimeout = 15 * 60;  // 15 минут

	typedef enum 
	{
		Download, // закачиваем обновления
		Deploy,   // устанавливаем обновления
		Finish    // окончание обновления
	} State;

	namespace ExitCode
	{
		enum Enum
		{
			NoError = 0,              /// Выход без ошибок.
			ErrorRunFromTempDir,      /// Ошибка запуска из временной папки
			NoWatchService,           /// Отсутствует соединение с WatchService
			UnknownCommand,           /// Неизвестная команда
			SecondInstance,           /// Повторный запуск приложения
			UnknownError,
			NetworkError,             /// Сетевая ошибка (не смог скачать)
			ParseError,               /// Ошибка разбора ответа сервера обновления
			DeployError,              /// Ошибка распаковки/копирования содержимого при обновлении/получении файлов
			Aborted,                  /// Команда прервана снаружи
			Blocked,                  /// Обновление заблокировано на стороне сервера
			FailIntegrity,            /// Проверка целостности закончилась неуспехом
			WorkInProgress,           /// Фоновая обработка задания
			ContunueExecution = 54321 /// Перезапуск из временной папки
		};
	}
}

//---------------------------------------------------------------------------
class UpdaterApp : public QObject, public BasicQtApplication<SafeQApplication>
{
	Q_OBJECT

public:
	UpdaterApp(int aArgc, char ** aArgv);
	~UpdaterApp();

	void run();

	CUpdaterApp::ExitCode::Enum bitsCheckStatus(bool aAlreadyRunning);

	int getResultCode() const;

	/// Возвращает рабочий каталог приложения (может быть задан в .ini файле).
	virtual QString getWorkingDirectory() const;

	static void qtMessageHandler(QtMsgType aType, const char * aMessage);

private slots:
	/// Система обновления в прочцессе работы, ожидаем следующей попытки
	void updateSystemIsWaiting();

	/// Переводим апдейтер в состояние разворачивания установки
	void onDeployment();

	/// Действие при завершении закачки файлов.
	void onDownloadComplete();

	/// Действие при заврешении обновления.
	void onUpdateComplete(CUpdaterErrors::Enum aError);

	/// Конфиги были скачаны и распакованы.
	void onConfigReady(CUpdaterErrors::Enum aError);

	/// Архивы с рекламой были скачаны и распакованы.
	void onPackReady(CUpdaterErrors::Enum aError);

	/// Обработчик сигнала о завершении работы.
	void onCloseCommandReceived();

	/// Обработчик уведомления о закрытии определенного модуля.
	void onModuleClosed(const QString & aModuleName);

	/// Оповещение о разрыве связи со сторожевым сервисом.
	void onDisconnected();

	/// обработчик ошибки остановки 
	void onFailStopClient();

	/// закрытие приложения с ошибкой
	void errorExit();

	/// Обработчик предельного времени закачки файлов
	void tooLondToDownload();

private:
	QSharedPointer<IWatchServiceClient> mWatchServiceClient;
	QPointer<Updater> mUpdater;

	SplashScreen mSplashScreen;
	ReportBuilder mReportBuilder;
	CUpdaterApp::State mState;
	QPointer<QTimer> mErrorStopTimer;

	int mResultCode_;
	QString mResultDescription;

private:
	/// Получить значение параметра командной строки
	QString getArgument(const char * aName, const QString & aDafaultValue = QString()) const;

	/// Загрузка настроек
	bool loadSettings();

	/// перезапускает апдейтер из временной папки
	bool reRunFromTempDirectory();

	/// Копируем updater в временную папку и оттуда запускаем
	bool CopyToTempPath();

	/// возвращает временную папку, откуда будем запускать апдейтер
	QString getUpdaterTempDir() const;

	/// Закрытие приложения через aTimeout секунд с признаком ошибки aError
	void delayedExit(int aTimeout, CUpdaterErrors::Enum aError);

	/// Выставить код возврата updater
	void setResultCode(CUpdaterErrors::Enum aResult, const QString aMessage = QString());
	void setResultCode(CUpdaterApp::ExitCode::Enum aExitCode, const QString aMessage = QString());
	void updateErrorDescription();

private:
	/// Скопировать файлы
	bool copyFiles(const QString & from, const QString & mask, const QString & to);

	/// Очистить папку от содержимого
	bool cleanDir(const QString & dirName);

	/// Список директорий - исключений
	QStringList exceptionDirs() const;

private:
	void startErrorTimer();
	void stopErrorTimer();
};

//---------------------------------------------------------------------------