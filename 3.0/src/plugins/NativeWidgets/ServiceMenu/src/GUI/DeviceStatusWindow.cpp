/* @file Виджет, отображающий состояние устройства */

// Project
#include "Backend/HardwareManager.h"
#include "Backend/ServiceMenuBackend.h"

#include "DeviceStatusWindow.h"

//------------------------------------------------------------------------------
DeviceStatusWindow::DeviceStatusWindow(ServiceMenuBackend * aBackend, const QString & aConfigurationName, QWidget * aParent)
	: DeviceSlot(aBackend, aConfigurationName)
{
	setParent(aParent);
}

//------------------------------------------------------------------------------
DeviceStatusWindow::~DeviceStatusWindow()
{
}

//------------------------------------------------------------------------
QWidget * DeviceStatusWindow::createWidget()
{
	QFrame * widget = new QFrame(dynamic_cast<QWidget *>(parent()));

	ui.setupUi(widget);

	ui.lblDeviceType->setText(mType);

	QVariantMap deviceParams(mBackend->getHardwareManager()->getConfiguration()[mConfigurationName].toMap());
	ui.lblDeviceModel->setText(deviceParams["model_name"].toString());

	ui.btnRunTest->setEnabled(mDeviceTest ? mDeviceTest->isReady() : false);

	connect(ui.btnRunTest, SIGNAL(clicked()), this, SLOT(onDeviceRunTest()));

	return widget;
}

//------------------------------------------------------------------------------
void DeviceStatusWindow::onRepaint()
{
	// no need repaint
}

//------------------------------------------------------------------------------
void DeviceStatusWindow::updateDeviceStatus(const QString & aNewStatus, const QString & aStatusColor, SDK::Driver::EWarningLevel::Enum /*aLevel*/)
{
	ui.lblDeviceStatus->setText(aNewStatus);
	ui.lblDeviceModel->setStyleSheet("color:" + aStatusColor + ";");
	ui.lblDeviceStatus->setStyleSheet("color:" + aStatusColor + ";");
	ui.lblDeviceType->setStyleSheet("color:" + aStatusColor + ";");

	if (!mDeviceTest.isNull())
	{
		ui.btnRunTest->setEnabled(mDeviceTest->isReady());
	}
}

//------------------------------------------------------------------------------
