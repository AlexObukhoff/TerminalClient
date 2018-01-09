/* @file Сервис для организации перехвата вызовов других сервисов. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QVariantMap>
#include <QtCore/QSignalMapper>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/Plugins/IPlugin.h>
#include <SDK/PaymentProcessor/Core/HookConstants.h>

// Модули
#include <Common/ILogable.h>

// Проект
#include "System/IApplication.h"

/** TODO: Возможно этот интерфейс потом вынесем в SDK 
//------------------------------------------------------------------------------
class IHookService
{
protected:
	virtual ~IHookService() {}

public:
	/// Вызывает метод у всех hook плагинов
	virtual bool invokeMethod(const QString & aMetodName, //Qt::ConnectionType aType, QGenericReturnArgument aRet,
		QGenericArgument aVal0 = QGenericArgument( 0 ),
		QGenericArgument aVal1 = QGenericArgument(), QGenericArgument aVal2 = QGenericArgument(),
		QGenericArgument aVal3 = QGenericArgument(), QGenericArgument aVal4 = QGenericArgument(),
		QGenericArgument aVal5 = QGenericArgument(), QGenericArgument aVal6 = QGenericArgument(),
		QGenericArgument aVal7 = QGenericArgument(), QGenericArgument aVal8 = QGenericArgument(),
		QGenericArgument aVal9 = QGenericArgument() ) = 0;

};
*/ 

//---------------------------------------------------------------------------
class HookService : public QObject, /*public SDK::PaymentProcessor::IHookService,*/ public SDK::PaymentProcessor::IService
{
	Q_OBJECT

public:
	/// Получение DatabaseServiceа.
	static HookService * instance(IApplication * aApplication);

	HookService(IApplication * aApplication);
	virtual ~HookService();

	/// Методы интерфейса IService

	/// IService: Инициализация сервиса.
	virtual bool initialize();

	/// IService: Закончена инициализация всех сервисов.
	virtual void finishInitialize();

	/// IService: Возвращает false, если сервис не может быть остановлен в текущий момент.
	virtual bool canShutdown();

	/// IService: Завершение работы сервиса.
	virtual bool shutdown();

	/// IService: Возвращает имя сервиса.
	virtual QString getName() const;

	/// IService: Список необходимых сервисов.
	virtual const QSet<QString> & getRequiredServices() const;

	/// IService: Получить параметры сервиса.
	virtual QVariantMap getParameters() const;

	/// IService: Сброс служебной информации.
	virtual void resetParameters(const QSet<QString> & aParameters);

	/// Методы интерфейса IHookService

	/// IHookService: Вызывает метод у всех hook плагинов
	virtual bool invokeHook(const QString & aMetodName,
		QGenericArgument aVal0 = QGenericArgument(0),
		QGenericArgument aVal1 = QGenericArgument(), QGenericArgument aVal2 = QGenericArgument(),
		QGenericArgument aVal3 = QGenericArgument(), QGenericArgument aVal4 = QGenericArgument(),
		QGenericArgument aVal5 = QGenericArgument(), QGenericArgument aVal6 = QGenericArgument(),
		QGenericArgument aVal7 = QGenericArgument(), QGenericArgument aVal8 = QGenericArgument(),
		QGenericArgument aVal9 = QGenericArgument());

private:
	IApplication * mApplication;

	QList<SDK::Plugin::IPlugin *> mHooks;

};

//---------------------------------------------------------------------------
