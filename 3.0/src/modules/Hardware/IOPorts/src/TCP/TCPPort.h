/* @file TCP-порт. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtNetwork/QTcpSocket>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/IOPorts/IOPortBase.h"

//--------------------------------------------------------------------------------
namespace CTCPPort
{
	/// Таймаут открытия порта, [мс].
	const int OpeningTimeout = 200;

	/// Таймаут закрытия порта, [мс].
	const int CloseningTimeout = 200;

	/// Пауза после закрытия порта, [мс].
	const int CloseningPause = 100;

	/// Маска адреса.
	const QString AddressMask = QString("^%1\\.%1\\.%1\\.%1$").arg("[0-9]{1,3}");

	/// Маска адреса для указания в логе.
	const char AddressMaskLog[] = "0xx.0xx.0xx.0xx";

	/// Пинг устройства во избежание применения алгоритма Нейгла.
	const char AntiNaglePing[] = "\xFF";

	/// Таймаут единичного чтения из порта, [мс].
	const int ReadingTimeout = 3;
}

#define PERFORM_IN_THREAD(aMethod, ...) invokeMethod(std::bind(&TCPPort::aMethod, this, __VA_ARGS__))

//--------------------------------------------------------------------------------
class TCPPort : public QObject, public IOPortBase
{
	Q_OBJECT

	SET_SERIES("TCP")

public:
	TCPPort();

	/// Опрашивает данные портов.
	virtual void initialize();

	/// Открыть порт.
	virtual bool open();

	/// Закрыть порт.
	virtual bool close();

	/// Прочитать данные.
	virtual bool read(QByteArray & aData, int aTimeout = DefaultReadTimeout, int aMinSize = 1);

	/// Передать данные.
	virtual bool write(const QByteArray & aData);

	/// Подключено новое устройство?
	virtual bool deviceConnected();

	/// Порт существует?
	virtual bool isExist();

signals:
	/// Сигнал для обработки указанного метода в нити объекта.
	void invoke(TBoolMethod aMethod, bool * aResult);

protected slots:
	/// Изменение состояния.
	void onStateChanged(QAbstractSocket::SocketState aSocketState);

	/// Изменение состояния.
	void onErrorChanged(QAbstractSocket::SocketError aSocketError);

	/// Данные готовы для чтения.
	void onReadyRead();

	/// Обработчик сигнала invoke.
	void onInvoke(TBoolMethod aMethod, bool * aResult);

protected:
	/// Вызывает метод в рабочем потоке и возвращает результат.
	bool invokeMethod(TBoolMethod aMethod);

	/// Открыть порт.
	bool performOpen();

	/// Закрыть порт.
	bool performClose();

	/// Прочитать данные.
	bool performRead(QByteArray & aData, int aTimeout = DefaultReadTimeout, int aMinSize = 1);

	/// Передать данные.
	bool performWrite(const QByteArray & aData);

	/// Проверить готовность порта.
	virtual bool performCheckReady();

	/// Проверить готовность порта.
	virtual bool checkReady();

	/// TCP-сокет.
	typedef QSharedPointer<QTcpSocket> PSocket;
	PSocket mSocket;

	/// Состояние TCP-сокета.
	QAbstractSocket::SocketState mState;

	/// Ошибка TCP-сокета.
	QAbstractSocket::SocketError mError;

	/// Для охраны операций с TCP-сокетом в рабочем потоке.
	QMutex mSocketGuard;

	/// Буфер прочитанных данных.
	QByteArray mDataFrom;

	/// Для охраны буфера прочитанных данных.
	QMutex mDataFromGuard;

	/// Последний лог с ошибкой открытия порта.
	QString mLastErrorLog;
};

//--------------------------------------------------------------------------------
