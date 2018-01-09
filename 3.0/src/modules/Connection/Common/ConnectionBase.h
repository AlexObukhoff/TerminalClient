/* @file Реализация базового функционала сетевого соединения. */

#pragma once

// Qt headers
#include "Common/QtHeadersBegin.h"
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QSet>
#include <QtCore/QTimer>
#include "Common/QtHeadersEnd.h"

// Common
#include <Common/ILogable.h>
#include <Common/ExceptionFilter.h>

// Project
#include "Connection/IConnection.h"

//--------------------------------------------------------------------------------
/// Константы
namespace CConnection
{
	/// Имя лога.
	const QString LogName = "Connection";
}

//--------------------------------------------------------------------------------
/// Базовый класс соединения. 
/// Управляет таймерами проверки соединения и реализует метод проверки соединения по HTTP.
class ConnectionBase : public IConnection
{
	Q_OBJECT

public:
	/// Конструктор.
	ConnectionBase(const QString & aName, NetworkTaskManager * aNetwork, ILog * aLog);

	/// Возвращает имя соединения.
	virtual QString getName() const;

	///	Устанавливает период проверки соединения.
	virtual void setCheckPeriod(int aMilliseconds);

	/// Осуществить попытку поднять соединение.
	virtual void open(bool aWatch = true) throw(...);

	/// Осуществить попытку закрыть соединение.
	virtual void close() throw(...);

	/// Проверяет установленно ли соединение.
	virtual bool isConnected(bool aUseCache) throw(...);

	/// Физически проверяет соединение выполняя HTTP запрос.
	virtual bool checkConnection(const CheckUrl & aHost = CheckUrl()) throw(...);

	/// Устанавливает список хостов для проверки соединения.
	virtual void setCheckHosts(const QList<IConnection::CheckUrl> & aHosts);

protected slots:
	void onCheckTimeout();

protected:
	virtual void doConnect() throw(...) = 0;
	virtual void doDisconnect() throw(...) = 0;
	virtual bool doIsConnected() throw(...) = 0;
	virtual bool doCheckConnection(const CheckUrl & aHost = CheckUrl()) = 0;

	bool httpCheckMethod(const IConnection::CheckUrl & aHost);

	void toLog(LogLevel::Enum aLevel, const QString & aMessage) const;

	NetworkTaskManager * mNetwork;

	QString mName;
	bool mConnected;
	bool mWatch;
	int mPingPeriod;
	int mCheckCount;
	QTimer mCheckTimer;
	QList<CheckUrl> mCheckHosts;
	ILog * mLog;
};

//--------------------------------------------------------------------------------
