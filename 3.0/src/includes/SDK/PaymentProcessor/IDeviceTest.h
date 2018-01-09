/* @file Класс для тестирования устройств. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtCore/QList>
#include <QtCore/QPair>
#include <Common/QtHeadersEnd.h>

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
class IDeviceTest : public QObject
{
	Q_OBJECT

public:
	virtual ~IDeviceTest() {}

	/// Возвращает имена и описания тестов.
	virtual QList<QPair<QString, QString>> getTestNames() const = 0;

	/// Запускает тестирование устройства.
	virtual bool run(const QString & aName = QString()) = 0;

	/// Остановка процесса тестирования.
	virtual void stop() = 0;

	/// Готово ли устройство для тестирования.
	virtual bool isReady() = 0;

	/// Возвращает true, если тест устройства возвращает результат теста
	virtual bool hasResult() = 0;

signals:
	/// Сигнал о получении результатов теста.
	void result(const QString & aTestName, const QVariant & aTestResult);
};

//------------------------------------------------------------------------------
}} // SDK::PaymentProcessor

