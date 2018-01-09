/* @file Реализация базового класса для сетевого запроса. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QMap>
#include <QtCore/QUrl>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include <QtCore/QTimer>
#include <QtCore/QDateTime>
#include <QtCore/QVariant>
#include <QtCore/QSharedPointer>
#include <Common/QtHeadersEnd.h>

class IVerifier;
class DataStream;
class NetworkTaskManager;

//------------------------------------------------------------------------
class NetworkTask : public QObject
{
	Q_OBJECT

	friend class NetworkTaskManager;

public:
	typedef QMap<QByteArray, QByteArray> TByteMap;

	/// Ошибки закачки
	enum Error
	{
		NoError = 0,
		StreamWriteError = -1, /// Не удалось произвести запись в переданный поток данных.
		UnknownOperation = -2, /// Неизвестный тип запроса к серверу.
		Timeout = -3, /// Превышен таймаут ожидания запроса.
		BadTask = -4, /// Ошибочный запрос к серверу. Пример: пытаемся скачать кусок файла с неправильным смещением.
		VerifyFailed = -5, /// Запрос к серверу завершён без ошибок, но данные не прошли верификацию.
		TaskFailedButVerified = -6, /// Запрос к серверу завершился с ошибкой, но полученные данные успешно прошли верификацию.
		                            /// Пример: пытаемся докачать уже полностью скачанный файл.	
		NotReady = -7 /// Задача была создана, но не выполненна.
	};

	/// Флаги, влияющие на процесс выполнения запроса.
	enum Flags
	{
		None         = 0x0,
		BlockingMode = 0x1, /// Выполнить закачку в блокирующем режиме.
		Continue     = 0x2, /// Продолжить прерванную ранее закачку.
		IgnoreErrors = 0x4  /// При возникновении ошибок продолжать закачку до тех пор, пока не скачаем весь файл.
		                    /// Закачка прекращается при достижении таймаута.
	};

	/// Тип сетевого запроса.
	enum Type
	{
		Head = 0,
		Get,
		Post,
		Put
	};

	NetworkTask();
	virtual ~NetworkTask();

	NetworkTaskManager * getManager() const;

	void setType(Type aType);
	Type getType() const;

	void setUrl(const QUrl & aUrl);
	const QUrl & getUrl() const;

	void setTimeout(int aMsec);
	int getTimeout() const;

	void setFlags(Flags aFlags);
	Flags getFlags() const;

	/// Возвращает код ошибки.
	int getError() const;
	/// Возвращает текстовое описание ошибки.
	QString errorString();

	int getHttpError() const;

	void setTag(const QVariant & aTag);
	const QVariant & getTag() const;

	qint64 getSize() const;
	qint64 getCurrentSize() const;

	/// Указатель забирает внутрь, память освобождает самостоятельно.
	void setVerifier(IVerifier * aVerifier);
	IVerifier * getVerifier() const;

	/// Указатель забирает внутрь, память освобождает самостоятельно.
	void setDataStream(DataStream * aDataStream);
	DataStream * getDataStream() const;

	TByteMap & getRequestHeader();

	/// Ожидание завершения запроса.
	void waitForFinished();

	TByteMap & getResponseHeader();

	/// Получение времени из заголовка ответа сервера
	QDateTime getServerDate() const;

signals:
	/// Сигнал срабатывает при завершении задачи.
	void onComplete();

	/// Сигнал срабатывает при изменении прогресса выполнения задачи. aCurrent, aTotal - кол-во байт.
	void onProgress(qint64 aCurrent, qint64 aTotal);

private slots:
	void onTimeout();

protected:
	void setSize(qint64 aCurrent, qint64 aTotal);
	void setError(int aError, const QString & aMessage = "");
	void setHttpError(int aError);
	void setProcessing(NetworkTaskManager * aManager, bool aProcessing);

	/// Перезапустить таймер завершения таска по таймауту
	void resetTimer();

private:
	bool                        mProcessing;
	QMutex                      mProcessingMutex;
	QWaitCondition              mProcessingCondition;
	QTimer                      mTimer;
	NetworkTaskManager        * mManager;
	Type                        mType;
	QUrl                        mUrl;
	Flags                       mFlags;
	int                         mTimeout;
	int                         mError;
	int                         mHttpError;
	QString                     mNetworkReplyError;
	QSharedPointer<IVerifier>   mVerifier;
	QSharedPointer<DataStream>  mDataStream;
	TByteMap                    mRequestHeader;
	TByteMap                    mResponseHeader;
	QWeakPointer<QThread>       mParentThread;
	qint64                      mSize;
	qint64                      mCurrentSize;
	QVariant                    mTag;
};

//------------------------------------------------------------------------
Q_DECLARE_METATYPE(NetworkTask *);

//------------------------------------------------------------------------
