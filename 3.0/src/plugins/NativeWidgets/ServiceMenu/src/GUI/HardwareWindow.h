/* @file Реализация компоненты для редактирования профилей устройств. */

#pragma once

// boost
#ifndef Q_MOC_RUN
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/composite_key.hpp>
#endif

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QSharedPointer>
#include <QtCore/QFutureWatcher>
#include <QtWidgets/QWidget>
#include "ui_HardwareWindow.h"
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/WarningLevel.h>

// Project
#include "EditorPane.h"
#include "IDeviceBackend.h"

namespace SDK {
namespace PaymentProcessor {
	class IDeviceService;
}
namespace Driver {
	class IDevice;
}}

class DeviceSlot;
class ServiceMenuBackend;

//---------------------------------------------------------------------------
class HardwareWindow : public QWidget, public IDeviceBackend
{
	Q_OBJECT

public:
	HardwareWindow(ServiceMenuBackend * aBackend, QWidget * aParent = 0);	
	virtual ~HardwareWindow();

public:
	enum SlotCreationMode
	{
		/// В этом режиме ничего дополнительно не происходит.
		Default = 0,
		/// Для пользовательских слотов после создания сразу будет открыт редактор.
		OpenEditorAfterCreation
	};

	struct SDeviceItem
	{
		QString model;
		QString type;
		QString driver;
	};

	struct TypeTag {};
	struct ModelTag {};
	struct TypeModelTag {};
	
	struct TypeModelKey : boost::multi_index::composite_key<
		SDeviceItem,
		BOOST_MULTI_INDEX_MEMBER(SDeviceItem, QString, type),
		BOOST_MULTI_INDEX_MEMBER(SDeviceItem, QString, model)
	>{};

	typedef boost::multi_index::multi_index_container <
		SDeviceItem,
		boost::multi_index::indexed_by <
			boost::multi_index::ordered_non_unique <
				boost::multi_index::tag <TypeTag>,
				BOOST_MULTI_INDEX_MEMBER(SDeviceItem, QString, type) >,
			boost::multi_index::ordered_unique <
				boost::multi_index::tag <ModelTag>,
				BOOST_MULTI_INDEX_MEMBER(SDeviceItem, QString, model) >,
			boost::multi_index::ordered_unique < 
				boost::multi_index::tag <TypeModelTag>,
				TypeModelKey > > > TDeviceList;

	typedef TDeviceList::index<TypeTag>::type TDeviceByType;
	typedef TDeviceList::index<ModelTag>::type TDeviceByModel;
	typedef TDeviceList::index<TypeModelTag>::type TDeviceByTypeModel;

public:
	/// Инициализация окна, загрузка начальной конфигурации.
	virtual bool initialize();

	/// Освобождение ресурсов.
	virtual void shutdown();

	/// Создаёт новый слот устройства и возвращает на него указатель.
	virtual DeviceSlot * addDeviceSlot(const QString & aConfigurationName, bool aUserSlot = false, const QString & aType = QString());

	/// Удаляет слот устройства.
	virtual void removeDeviceSlot(DeviceSlot * aSlot, bool aUpdateConfig = false);

	/// Удаляет все слоты устройств.
	virtual void removeAllDeviceSlots(bool aIncludeUserSlots = false);

	/// Устанавливает начальную конфигурацию.
	virtual void setConfiguration(const QVariantMap & aConfiguration);

	/// Возвращает конфигурацию устройств.
	virtual QVariantMap getConfiguration() const;

	/// Устанавливает режим создания слота.
	virtual void setSlotCreationMode(SlotCreationMode aMode);

	/// Проверка на подключения устройства, описанного в слоте aSlot.
	virtual void checkDeviceSlot(DeviceSlot * aSlot);

private:
	void checkDeviceSlotConcurrent(DeviceSlot * aSlot);

signals:
	/// Сигнал срабатывает, когда производится попытка редактирования aSlot. aPane -
	/// найденный подходящий редактор для слота.
	void editSlot(DeviceSlot * aSlot, EditorPane * aPane);

	/// Срабатывает, когда был сделан запрос на удаление aSlot.
	void removeSlot(DeviceSlot * aSlot);

	/// Срабатывает, когда начинается поиск устройств.
	void detectionStarted();

	/// Срабатывает, когда поиск устройств завершён.
	void detectionFinished();

	void applyingStarted();
	void applyingFinished();

	/// Срабатывает при нахождении нового устройства.
	void devicesFound(const QString & aConfigName);

	/// Срабатывает при смене текущей формы (список устройств/выбор типа устройства).
	void currentFormChanged(int aIndex);

public slots:
	/// Поиск устройств.
	void detectDevices();

	/// Прерывает поиск устройтсв.
	void abortDetection();

	/// Поиск устройст закончен
	void onDetectionFinished();

private slots:
	void onDeviceStatusChanged(const QString & aConfigurationName, const QString & aNewStatus, const QString & aStatusColor, SDK::Driver::EWarningLevel::Enum aLevel);

protected:
	/// Возвращает редактор для указанного слота.
	virtual EditorPane * getEditor(DeviceSlot * aSlot);

	/// IDeviceBackend: Возвращает список поддерживаемых моделей, указанного типа.
	virtual QStringList getModels(const QString & aType);

	/// IDeviceBackend: Возвращает список параметров, указанной модели.
	virtual SDK::Plugin::TParameterList getModelParameters(const QString & aType, const QString & aModel);

	/// IDeviceBackend: Возвращает менеджер устройств
	virtual HardwareManager * getHardwareManager() const;

	/// IDeviceBackend: Возвращает "ядро"
	virtual SDK::PaymentProcessor::ICore * getCore() const;

	virtual void toLog(const QString & aMessage);

private slots:
	/// Срабатывает при необходимости показа диалога добавления слота.
	void onShowAddSlotDialog();

	/// Срабатывает при необходимости показа списка добавленных слотов.
	void onShowSlots();

	/// Процедура добавления нового слота.
	void onCreateSlot();

	/// Срабатывает, когда пользователь выбирает тип из списка создания нового слота.
	void onTypeSelected();

	/// Срабатывает, когда слот посылает запрос на редактирование.
	void onEdit();

	/// Срабатывает, когда слот посылает запрос на удаление.
	void onRemove();

	/// Обработка события "найдено новое устройство".
	void onDeviceFound(const QString & aConfigName);

private:
	typedef QList<QSharedPointer<DeviceSlot> > TDeviceSlotList;

	Ui::frmHardwareWindow ui;

	ServiceMenuBackend * mBackend;

	// Мапа тип устройства -> локализация
	QMap<QString, QString> mTypes;

	EditorPane mEditor;

	TDeviceSlotList mSlots;
	TDeviceList mDeviceList;

	QFutureWatcher<void> mApplyingWatcher;

	SlotCreationMode mCreationMode;
};

//---------------------------------------------------------------------------
