/* @file Реализация компоненты для редактирования профилей устройств. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtCore/QBuffer>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/IDevice.h>
#include <SDK/Drivers/Components.h>
#include <SDK/PaymentProcessor/IDeviceTest.h>

// Project
#include "MessageBox/MessageBox.h"
#include "Backend/HardwareManager.h"
#include "Backend/ServiceMenuBackend.h"

#include "DeviceTests/GenericDeviceTest.h"
#include "DeviceTests/PrinterTest.h"
#include "DeviceTests/BillAcceptorTest.h"
#include "DeviceTests/CoinAcceptorTest.h"
#include "DeviceTests/HIDTest.h"
#include "DeviceTests/DispenserTest.h"

#include "DeviceSlot.h"

//------------------------------------------------------------------------
DeviceSlot::DeviceSlot(ServiceMenuBackend * aBackend, const QString & aConfigurationName, bool aIsUserSlot, const QString & aType)
	: mBackend(aBackend),
	  mConfigurationName(aConfigurationName),
	  mIsUserSlot(aIsUserSlot),
	  mType(aType)
{
	updateConfigurationName(mConfigurationName);
	QTimer::singleShot(0, this, SLOT(onRepaint()));
}

//------------------------------------------------------------------------
DeviceSlot::~DeviceSlot()
{
	if (mWidget)
	{
		mWidget->deleteLater();
	}
}

//------------------------------------------------------------------------
QWidget * DeviceSlot::createWidget()
{
	QWidget * widget = new QWidget();

	ui.setupUi(widget);

	connect(ui.btnChange, SIGNAL(clicked()), SLOT(onClick()));
	connect(ui.btnDelete, SIGNAL(clicked()), SLOT(onRemove()));
	connect(ui.btnRunTest, SIGNAL(clicked()), this, SLOT(onDeviceRunTest()));

	return widget;
}

//------------------------------------------------------------------------
QWidget * DeviceSlot::getWidget()
{
	if (!mWidget)
	{
		mWidget = createWidget();
	}

	return mWidget;
}

//------------------------------------------------------------------------
const QString & DeviceSlot::getType() const
{
	return mType;
}

//------------------------------------------------------------------------
QString DeviceSlot::getModel() const
{
	return mParameterValues.contains("model_name") ? mParameterValues["model_name"].toString() : QString();
}

//------------------------------------------------------------------------
bool DeviceSlot::isUserSlot() const
{
	return mIsUserSlot;
}

//------------------------------------------------------------------------
void DeviceSlot::setParameterValues(QVariantMap aValues)
{
	// Если сменили модель, устройство надо пересоздать
	if (!mParameterValues["model_name"].isNull() 
		&& mParameterValues["model_name"] != aValues["model_name"])
	{
		mBackend->getHardwareManager()->releaseDevice(mConfigurationName);
		mConfigurationName.clear();
	}

	mParameterValues = aValues;

	QTimer::singleShot(0, this, SLOT(onRepaint()));
}

//------------------------------------------------------------------------
const QVariantMap & DeviceSlot::getParameterValues() const
{
	return mParameterValues;
}

//------------------------------------------------------------------------
ServiceMenuBackend * DeviceSlot::getBackend() const
{
	return mBackend;
}

//------------------------------------------------------------------------
QString DeviceSlot::getConfigurationName() const
{
	return mConfigurationName;
}

//------------------------------------------------------------------------
void DeviceSlot::updateConfigurationName(const QString & aConfigurationName)
{
	if (aConfigurationName.isEmpty())
	{
		return;
	}

	mConfigurationName = aConfigurationName;
	mType = mConfigurationName.section(".", 2, 2);

	mDevice = mBackend->getHardwareManager()->getDevice(mConfigurationName);

	if (mDevice)
	{
		if (mType == SDK::Driver::CComponents::BillAcceptor)
		{
			mDeviceTest = QSharedPointer<SDK::PaymentProcessor::IDeviceTest>(new BillAcceptorTest(mDevice));
		}
		else if (mType == SDK::Driver::CComponents::CoinAcceptor)
		{
			mDeviceTest = QSharedPointer<SDK::PaymentProcessor::IDeviceTest>(new CoinAcceptorTest(mDevice));
		}
		else if (SDK::Driver::CComponents::isPrinter(mType))
		{
			mDeviceTest = QSharedPointer<SDK::PaymentProcessor::IDeviceTest>(new PrinterTest(mDevice, mBackend->getCore()));
		}
		else if (mType == SDK::Driver::CComponents::Scanner || mType == SDK::Driver::CComponents::Camera)
		{
			mDeviceTest = QSharedPointer<SDK::PaymentProcessor::IDeviceTest>(new HIDTest(mDevice, mConfigurationName));
		}
		else if (mType == SDK::Driver::CComponents::Dispenser)
		{
			mDeviceTest = QSharedPointer<SDK::PaymentProcessor::IDeviceTest>(new DispenserTest(mDevice, mConfigurationName, mBackend->getCore()));
		}
		else
		{
			mDeviceTest = QSharedPointer<SDK::PaymentProcessor::IDeviceTest>(new GenericDeviceTest(mDevice));
		}
	}
}

//------------------------------------------------------------------------
void DeviceSlot::updateDeviceStatus(const QString & aNewStatus, const QString & aStatusColor, SDK::Driver::EWarningLevel::Enum /*aLevel*/)
{
	ui.lbStatus->setText(aNewStatus);
	ui.lineStatus->setStyleSheet(ui.lineStatus->styleSheet() + ";color:" + aStatusColor + ";background-color: " + aStatusColor);

	if (!mDeviceTest.isNull())
	{
		ui.btnRunTest->setEnabled(mDeviceTest->isReady());
	}
}

//------------------------------------------------------------------------------
void DeviceSlot::onDeviceRunTest()
{
	connect(mDeviceTest.data(), SIGNAL(result(const QString &, const QVariant &)), this, SLOT(onTestResult(const QString &, const QVariant &)));

	if (mDeviceTest->hasResult())
	{
		GUI::MessageBox::subscribe(this);
	}

	foreach (auto test, mDeviceTest->getTestNames())
	{
		if (mDeviceTest->hasResult())
		{
			GUI::MessageBox::info(test.second);
		}

		mDeviceTest->run(test.first);
	}

	mBackend->toLog(QString("Button clicked: %1; Device: %2:%3.")
		.arg(qobject_cast<QAbstractButton *>(sender())->text())
		.arg(mType).arg(getModel()));
}

//------------------------------------------------------------------------------
void DeviceSlot::onTestResult(const QString & aTestName, const QVariant & aTestResult)
{
	Q_UNUSED(aTestName);

	QVariantMap params;

	switch (aTestResult.type())
	{
	case QVariant::Image:
		{
			QBuffer buffer;
			if (aTestResult.value<QImage>().save(&buffer, "jpg", 70))
			{
				QByteArray data = buffer.data().toBase64();
				data.insert(0, "data:image/jpeg;base64,");
				data.squeeze();

				params[SDK::GUI::CMessageBox::Image] = QString::fromLatin1(data);
			}
		}
		break;

	default:
		params[SDK::GUI::CMessageBox::TextMessageExt] = aTestResult;
	}

	GUI::MessageBox::update(params);
}

//------------------------------------------------------------------------------
void DeviceSlot::onClicked(const QVariantMap & /*aParameters*/)
{
	mDeviceTest->stop();
	disconnect(mDeviceTest.data(), SIGNAL(result(const QString &, const QVariant &)), this, SLOT(onTestResult(const QString &, const QVariant &)));

	GUI::MessageBox::hide();
}

//------------------------------------------------------------------------
void DeviceSlot::onRepaint()
{
	QString model = getModel();
	QString title;

	if (!model.isEmpty())
	{
		title = model;
	}
	else
	{
		title = QCoreApplication::translate("Hardware::CommonParameters", QT_TRANSLATE_NOOP("Hardware::CommonParameters", "#unknown_model"));
	}

	// Для устройств вида Класс.Модель выкусываем название класса
	ui.btnChange->setText(QString("%1: %2").arg(QCoreApplication::translate("Hardware::Types", mType.section(".", 0, 0).toLatin1())).arg(title));
}

//------------------------------------------------------------------------
void DeviceSlot::onClick()
{
	emit edit();
}

//------------------------------------------------------------------------
void DeviceSlot::onRemove()
{
	mBackend->toLog(QString("Button clicked: %1; Device: %2:%3.")
		.arg(qobject_cast<QAbstractButton *>(sender())->text())
		.arg(mType).arg(getModel()));

	emit remove();
}

//------------------------------------------------------------------------
