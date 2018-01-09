/* @file Интерфейс и механизмы инициализации сервисов. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QSet>
#include <QtCore/QVariantMap>
#include <Common/QtHeadersEnd.h>

namespace SDK { namespace PaymentProcessor {

//---------------------------------------------------------------------------
/// Базовый интерфейс для всех сервисов.
class IService
{
public:
	virtual ~IService() {}

	/// Инициализация сервиса.
	virtual bool initialize() = 0;

	/// Закончена инициализация всех сервисов
	virtual void finishInitialize() = 0;

	/// Возвращает false, если сервис не может быть остановлен в текущий момент.
	virtual bool canShutdown() = 0;

	/// Завершение работы сервиса. Возвращает false, если сервис не может быть остановлен в текущий момент.
	virtual bool shutdown() = 0;

	/// Получить имя сервиса.
	virtual QString getName() const = 0;

	/// Получение списка необходимых сервисов.
	virtual const QSet<QString> & getRequiredServices() const = 0;

	/// Получение cлужебной информации от сервиса.
	virtual QVariantMap getParameters() const = 0;

	/// Сброс служебной информации.
	virtual void resetParameters(const QSet<QString> & aParameters) = 0;

};

//---------------------------------------------------------------------------
}} // SDK::PP

