/* @file Реализация компоненты для редактирования профилей устройств. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QVariant>
#include <QtCore/QSharedPointer>
#include <QtCore/QPointer>
#include <QtWidgets/QWidget>
#include "ui_DeviceSlot.h"
#include <Common/QtHeadersEnd.h>

// SDK
#include <Common/ObjectPointer.h>
#include <SDK/Drivers/WarningLevel.h>
#include <SDK/Plugins/PluginParameters.h>

namespace SDK {
namespace PaymentProcessor {
	class IDeviceTest;
}
namespace Driver {
	class IDevice;
}}

class IDeviceBackend;
class ServiceMenuBackend;

//------------------------------------------------------------------------
/// Слот подключённого устройства. Хранит информацию о настройках устройства,
/// текущем состоянии и умеет её визуализировать.

class DeviceSlot : public QObject
{
	Q_OBJECT

public:
	/// Состояние связанного со слотом устройства.
	enum State
	{
		Unknown = 0,  /// Статус устройства неизвестен
		Connected,    /// Устройство подключено
		Disconnected, /// Устройство не подключено
		Ambiguous     /// Недостоверный статус
	};

	DeviceSlot(ServiceMenuBackend * aBackend, const QString & aConfigurationName, bool aUserSlot = false, const QString & aType = QString());

	virtual ~DeviceSlot();

	/// Возвращает виджет, который используется для визуализации слота.
	virtual QWidget * getWidget();

	/// Возвращает тип устройства, с которым связан слот.
	const QString & getType() const;

	/// Возвращает модель устройства, если она не выбрана, возвращает пустую строку.
	QString getModel() const;

	/// Слот добавлен пользователем?
	bool isUserSlot() const;

	/// Возвращает параметры связанного устройства, сохранённые в слоте.
	const QVariantMap & getParameterValues() const;

	/// Возвращает бэкенд, хранящий информацию о моделях устройств.
	ServiceMenuBackend * getBackend() const;

public:
	virtual void updateDeviceStatus(const QString & aNewStatus, const QString & aStatusColor, SDK::Driver::EWarningLevel::Enum aLevel);

	// Название конфигурации
	QString getConfigurationName() const;

	void updateConfigurationName(const QString & aConfigurationName);

signals:
	/// Сигнал срабатывает, когда делается запрос на редактирование настроек.
	void edit();

	/// Сигнал срабатывается, когда делается запрос на удаление слота.
	void remove();

public slots:
	/// Обновление параметров связанного устройства.
	virtual void setParameterValues(QVariantMap aValues);

protected:
	/// Создание виджета, который будет использоваться для визуализации.
	/// Используется в getWidget().
	virtual QWidget * createWidget();

private slots:
	/// Перерисовка виджета, связанного со слотом.
	virtual void onRepaint();

	/// Клик по кнопке редактирования параметров.
	virtual void onClick();

	/// Обработчик кнопки удаления устройства
	virtual void onRemove();

	/// запуск теста устройства
	void onDeviceRunTest();

	/// получение и отображение результатов теста
	void onTestResult(const QString & aTestName, const QVariant & aTestResult);

	/// обработчик остановки теста устройства
	void onClicked(const QVariantMap & aParameters);

protected:
	QString mType;
	bool mIsUserSlot;
	QVariantMap mParameterValues;
	ServiceMenuBackend * mBackend;
	QString mConfigurationName;

	Ui::frmDeviceSlot ui;
	QPointer<QWidget> mWidget;

	ObjectPointer<SDK::Driver::IDevice> mDevice;
	QSharedPointer<SDK::PaymentProcessor::IDeviceTest> mDeviceTest;
};

//------------------------------------------------------------------------
