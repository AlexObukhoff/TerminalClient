/* @file Реализация соединения по локальной сети. */

#pragma once

// Project headers
#include "Common/ConnectionBase.h"

//--------------------------------------------------------------------------------
class LocalConnection : public ConnectionBase
{
public:
	/// Конструктор.
	LocalConnection(const QString & aName, NetworkTaskManager * aNetwork, ILog * aLog);

	/// Деструктор.
	virtual ~LocalConnection();

	/// Возвращает тип соединения.
	virtual EConnectionTypes::Enum getType() const;

private:
	/// Устанавливает соединение используя IP Helper API.
	virtual void doConnect() throw(...);

	/// Разрывает соединение используя IP Helper API.
	virtual void doDisconnect() throw(...);

	/// Проверяет наличие установленного соединения используя.
	virtual bool doIsConnected() throw(...);

	/// Использует метод HTTP HEAD базового класса.
	virtual bool doCheckConnection(const CheckUrl & aHost = CheckUrl());
};

//--------------------------------------------------------------------------------
