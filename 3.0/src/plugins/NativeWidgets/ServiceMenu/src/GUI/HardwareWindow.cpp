/* @file Реализация компоненты для редактирования профилей устройств. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtConcurrentRun>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/IDevice.h>
#include <SDK/PaymentProcessor/Core/IDeviceService.h>
#include <SDK/PaymentProcessor/Core/ICore.h>

// Проект
#include "Backend/MessageBox.h"
#include "Backend/HardwareManager.h"
#include "Backend/NetworkManager.h"
#include "Backend/ServiceMenuBackend.h"
#include "DeviceSlot.h"
#include "HardwareWindow.h"

//------------------------------------------------------------------------
static const char * Types[] =
{
	QT_TRANSLATE_NOOP("Hardware::Types", "BillAcceptor"),
	QT_TRANSLATE_NOOP("Hardware::Types", "Dispenser"),
	QT_TRANSLATE_NOOP("Hardware::Types", "CoinAcceptor"),
	QT_TRANSLATE_NOOP("Hardware::Types", "Printer"),
	QT_TRANSLATE_NOOP("Hardware::Types", "FiscalRegistrator"),
	QT_TRANSLATE_NOOP("Hardware::Types", "DocumentPrinter"),
	QT_TRANSLATE_NOOP("Hardware::Types", "Modem"),
	QT_TRANSLATE_NOOP("Hardware::Types", "Watchdog"),
	QT_TRANSLATE_NOOP("Hardware::Types", "Scanner"),
	QT_TRANSLATE_NOOP("Hardware::Types", "Virtual"),
	QT_TRANSLATE_NOOP("Hardware::Types", "Printer"),
	QT_TRANSLATE_NOOP("Hardware::Types", "System Printer"),
	QT_TRANSLATE_NOOP("Hardware::Types", "CardReader"),
	QT_TRANSLATE_NOOP("Hardware::Types", "Camera"),
};

//------------------------------------------------------------------------
HardwareWindow::HardwareWindow(ServiceMenuBackend * aBackend, QWidget * aParent)
	: QWidget(aParent),
	  mBackend(aBackend),
	  mCreationMode(Default)
{
	ui.setupUi(this);

	connect(ui.btnDetect, SIGNAL(clicked()), SLOT(detectDevices()));
	connect(ui.btnAdd, SIGNAL(clicked()), SLOT(onShowAddSlotDialog()));
	connect(ui.btnCancel, SIGNAL(clicked()), SLOT(onShowSlots()));
	connect(ui.btnOk, SIGNAL(clicked()), SLOT(onCreateSlot()));
	connect(ui.lwTypes, SIGNAL(itemClicked(QListWidgetItem *)), SLOT(onTypeSelected()));

	connect(mBackend->getHardwareManager(), SIGNAL(deviceDetected(const QString &)),
		this, SLOT(onDeviceFound(const QString &)));

	connect(mBackend->getHardwareManager(), SIGNAL(detectionStopped()),
		this, SLOT(onDetectionFinished()));

	connect(mBackend->getHardwareManager(), SIGNAL(deviceStatusChanged(const QString &, const QString &, const QString &, SDK::Driver::EWarningLevel::Enum)), 
		this, SLOT(onDeviceStatusChanged(const QString &, const QString &, const QString &, SDK::Driver::EWarningLevel::Enum)));

	connect(&mApplyingWatcher, SIGNAL(finished()), this, SIGNAL(applyingFinished()));
	
	ui.lwTypes->clear();
	
	QString lastType;
	QStringList driverList = mBackend->getHardwareManager()->getDriverList();

	foreach (QString driver, driverList)
	{
		QString type = driver.section(".", 2, 2);
		QStringList modelList = mBackend->getHardwareManager()->getModelList()[driver];

		mTypes[type] = QCoreApplication::translate("Hardware::Types", type.toLatin1());
		
		if (lastType != type)
		{
			QListWidgetItem * item = new QListWidgetItem(mTypes[type]);
			item->setTextAlignment(Qt::AlignCenter);
			ui.lwTypes->addItem(item);
		}

		lastType = type;
		
		foreach (QString model, modelList)
		{
			SDeviceItem item;
			item.model = model;
			item.driver = driver;
			item.type = type;

			mDeviceList.insert(item);
		}
	}

	ui.lwTypes->setCurrentRow(0);

	ui.slotsWidget->setLayout(new QVBoxLayout());
	ui.slotsWidget->layout()->setContentsMargins(0, 0, 0, 0);
	ui.slotsWidget->layout()->setSpacing(0);
	ui.slotsWidget->layout()->setAlignment(Qt::AlignTop);

	ui.stackedWidget->setCurrentWidget(ui.wAllDevicesPage);
	connect(ui.stackedWidget, SIGNAL(currentChanged(int)), SIGNAL(currentFormChanged(int))); 
}

//------------------------------------------------------------------------
HardwareWindow::~HardwareWindow()
{
}

//------------------------------------------------------------------------
bool HardwareWindow::initialize()
{
	return true;
}

//------------------------------------------------------------------------
void HardwareWindow::shutdown()
{
	removeAllDeviceSlots(true);

	mApplyingWatcher.waitForFinished();
}

//------------------------------------------------------------------------
void HardwareWindow::setConfiguration(const QVariantMap & aConfiguration)
{
	removeAllDeviceSlots(true);

	// Выкусываем из имени конфигурации тип устройства
	foreach (const QString & item, aConfiguration.keys())
	{
		DeviceSlot * slot = addDeviceSlot(item);
		if (!slot)
		{
			continue;
		}

		slot->setParameterValues(aConfiguration[item].toMap());
	}

	mBackend->getHardwareManager()->updateStatuses();
}

//------------------------------------------------------------------------
QVariantMap HardwareWindow::getConfiguration() const
{
	QVariantMap configuration;

	foreach (const TDeviceSlotList::value_type & slot, mSlots)
	{
		if (!slot->getConfigurationName().isEmpty())
		{
			configuration[slot.data()->getConfigurationName()] = slot.data()->getParameterValues();
		}
	}

	return configuration;
}

//------------------------------------------------------------------------
DeviceSlot * HardwareWindow::addDeviceSlot(const QString & aConfigurationName, bool aUserSlot, const QString & aType)
{
	mSlots << QSharedPointer<DeviceSlot>(new DeviceSlot(mBackend, aConfigurationName, aUserSlot, aType));

	DeviceSlot * currentSlot = mSlots.last().data();

	ui.slotsWidget->layout()->addWidget(currentSlot->getWidget());

	connect(currentSlot, SIGNAL(edit()), SLOT(onEdit()));
	connect(currentSlot, SIGNAL(remove()), SLOT(onRemove()));

	return currentSlot;
}

//------------------------------------------------------------------------
EditorPane * HardwareWindow::getEditor(DeviceSlot * aSlot)
{
	mEditor.setSlot(this, aSlot);

	return &mEditor;
}

//------------------------------------------------------------------------
void HardwareWindow::onShowSlots()
{
	ui.stackedWidget->setCurrentIndex(0);
}

//------------------------------------------------------------------------
void HardwareWindow::onCreateSlot()
{
	QString type;

	for (QMap<QString, QString>::iterator it = mTypes.begin(); it != mTypes.end(); ++it)
	{
		if (it.value() == ui.lwTypes->currentItem()->text())
		{
			type = it.key();

			break;
		}
	}

	DeviceSlot * slot = addDeviceSlot(QString(), true, type);

	ui.stackedWidget->setCurrentIndex(0);

	if (mCreationMode == OpenEditorAfterCreation)
	{
		emit editSlot(slot, getEditor(slot));
	}
}

//------------------------------------------------------------------------
void HardwareWindow::detectDevices()
{
	emit detectionStarted();
	
	removeAllDeviceSlots();

	mBackend->getNetworkManager()->closeConnection();
	mBackend->getHardwareManager()->detect(ui.cbQuickDeviceSearch->checkState() == Qt::Checked);
}

//------------------------------------------------------------------------
void HardwareWindow::abortDetection()
{
	mBackend->getHardwareManager()->stopDetection();
}

//------------------------------------------------------------------------
void HardwareWindow::onDetectionFinished()
{
	emit detectionFinished();
	mBackend->getNetworkManager()->openConnection();
}

//------------------------------------------------------------------------
void HardwareWindow::onDeviceFound(const QString & aConfigName)
{
	// Сначала проверим, может быть, у нас уже есть такая железка, просто мы подтвердили статус её нахождения.
	// Хорошо бы проверять доп. параметры на случай если у нас подключено несколько железок одного типа и модели.
	DeviceSlot * slot = 0;

	foreach (const TDeviceSlotList::value_type & item, mSlots)
	{
		if (item.data()->getType() == aConfigName.section(".", 2, 2))
		{
			if (item.data()->getModel() == aConfigName.section(".", 3))
			{
				slot = item.data();

				break;
			}
		}
	}

	if (!slot)
	{
		slot =  addDeviceSlot(aConfigName);
	}

	slot->setParameterValues(mBackend->getHardwareManager()->getDeviceConfiguration(aConfigName));
	slot->updateConfigurationName(aConfigName);

	toLog(QString("FOUND new device: %1").arg(slot->getModel()));

	QVariantMap popupParams;
	popupParams[SDK::GUI::CMessageBox::TextAppendMode] = true;
	popupParams[SDK::GUI::CMessageBox::TextMessageExt] = slot->getModel();
	MessageBox::update(popupParams);
}

//------------------------------------------------------------------------
void HardwareWindow::onShowAddSlotDialog()
{
	ui.stackedWidget->setCurrentIndex(1);

	ui.btnOk->setEnabled(ui.lwTypes->currentItem() != 0);
}

//------------------------------------------------------------------------
void HardwareWindow::onTypeSelected()
{
	ui.btnOk->setEnabled(true);
}

//------------------------------------------------------------------------
void HardwareWindow::onEdit()
{
	DeviceSlot * slot = qobject_cast<DeviceSlot *>(sender());
	if (slot)
	{
		emit editSlot(slot, getEditor(slot));
	}
}

//------------------------------------------------------------------------
void HardwareWindow::onRemove()
{
	DeviceSlot * slot = qobject_cast<DeviceSlot *>(sender());
	if (slot)
	{
		emit removeSlot(slot);
	}
}

//------------------------------------------------------------------------
void HardwareWindow::removeDeviceSlot(DeviceSlot * aSlot, bool aUpdateConfig)
{
	ui.slotsWidget->layout()->removeWidget(aSlot->getWidget());

	if (aUpdateConfig)
	{
		QVariantMap newConfig(getConfiguration());
		newConfig.remove(aSlot->getConfigurationName());
		mBackend->getHardwareManager()->releaseDevice(aSlot->getConfigurationName());
		mBackend->getHardwareManager()->setConfigurations(newConfig.keys());
	}

	TDeviceSlotList::iterator it;

	for (it = mSlots.begin(); it != mSlots.end(); ++it)
	{
		if (it->data() == aSlot)
		{
			mSlots.erase(it);

			break;
		}
	}
}

//------------------------------------------------------------------------
void HardwareWindow::removeAllDeviceSlots(bool aIncludeUserSlots)
{
	if (aIncludeUserSlots)
	{
		while (!mSlots.isEmpty())
		{
			removeDeviceSlot(mSlots.first().data());
		}
	}
	else
	{
		bool onlyUserSlots = false;

		while (!onlyUserSlots)
		{
			onlyUserSlots = true;

			TDeviceSlotList::iterator it;

			for (it = mSlots.begin(); it != mSlots.end(); ++it)
			{
				if (!it->data()->isUserSlot())
				{
					onlyUserSlots = false;

					removeDeviceSlot(it->data(), true);

					break;
				}
			}
		}
	}
}

//------------------------------------------------------------------------
QStringList HardwareWindow::getModels(const QString & aType)
{
	QStringList result;

	const TDeviceByType & index = mDeviceList.get<TypeTag>();

	TDeviceByType::iterator i = index.lower_bound(aType);
	TDeviceByType::iterator j = index.upper_bound(aType);

	while (i != j)
	{
		result << (*i).model;
		++i;
	}

	return result;
}

//------------------------------------------------------------------------
SDK::Plugin::TParameterList HardwareWindow::getModelParameters(const QString & aType, const QString & aModel)
{
	const TDeviceByTypeModel & index = mDeviceList.get<TypeModelTag>();

	TDeviceByTypeModel::iterator it = index.find(boost::make_tuple(aType, aModel));

	if (it != mDeviceList.get<TypeModelTag>().end())
	{
		return mBackend->getHardwareManager()->getDriverParameters((*it).driver);
	}

	return SDK::Plugin::TParameterList();
}

//------------------------------------------------------------------------
HardwareManager * HardwareWindow::getHardwareManager() const
{
	return mBackend->getHardwareManager();
}

//------------------------------------------------------------------------
SDK::PaymentProcessor::ICore * HardwareWindow::getCore() const
{
	return mBackend->getCore();
}

//------------------------------------------------------------------------
void HardwareWindow::toLog(const QString & aMessage)
{
	mBackend->toLog(aMessage);
}

//------------------------------------------------------------------------
void HardwareWindow::setSlotCreationMode(SlotCreationMode aMode)
{
	mCreationMode = aMode;
}

//------------------------------------------------------------------------
void HardwareWindow::checkDeviceSlot(DeviceSlot * aSlot)
{
	emit applyingStarted();

	mApplyingWatcher.setFuture(QtConcurrent::run(this, &HardwareWindow::checkDeviceSlotConcurrent, aSlot));
}

//------------------------------------------------------------------------
void HardwareWindow::checkDeviceSlotConcurrent(DeviceSlot * aSlot)
{
	if (!aSlot->getConfigurationName().isEmpty())
	{
		mBackend->getHardwareManager()->releaseDevice(aSlot->getConfigurationName());
	}

	const TDeviceByModel & index = mDeviceList.get<ModelTag>();
	TDeviceByModel::iterator it = index.find(aSlot->getModel());

	if (it != mDeviceList.get<ModelTag>().end())
	{
		QString result;

		QMetaObject::invokeMethod(mBackend->getHardwareManager(), "createDevice", Qt::BlockingQueuedConnection,
			Q_RETURN_ARG(QString, result), Q_ARG(const QString &, (*it).driver), Q_ARG(const QVariantMap &, aSlot->getParameterValues()));

		aSlot->updateConfigurationName(result);
	}

	mBackend->getHardwareManager()->setConfigurations(getConfiguration().keys());
	mBackend->getHardwareManager()->updateStatuses();
}

//------------------------------------------------------------------------
void HardwareWindow::onDeviceStatusChanged(const QString & aConfigurationName, const QString & aNewStatus, const QString & aStatusColor, SDK::Driver::EWarningLevel::Enum aLevel)
{
	TDeviceSlotList::iterator it;

	for (it = mSlots.begin(); it != mSlots.end(); ++it)
	{
		if (it->data()->getConfigurationName() == aConfigurationName)
		{
			it->data()->updateDeviceStatus(aNewStatus, aStatusColor, aLevel);
			break;
		}
	}
}

//------------------------------------------------------------------------
