/* @file Окно настроек оборудования. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtGui/QShowEvent>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/Components.h>

// Проект
#include "MessageBox/MessageBox.h"
#include "Backend/NetworkManager.h"
#include "Backend/HardwareManager.h"
#include "Backend/ServiceMenuBackend.h"
#include "HardwareWindow.h"
#include "DeviceSlot.h"
#include "EditorPane.h"
#include "HardwareServiceWindow.h"

//------------------------------------------------------------------------
HardwareServiceWindow::HardwareServiceWindow(ServiceMenuBackend * aBackend, QWidget * aParent)
	: QFrame(aParent),
	  ServiceWindowBase(aBackend),
	  mWindow(new HardwareWindow(aBackend))
{
	setupUi(this);

	mWindow->setParent(this);
	mWindow->setSlotCreationMode(HardwareWindow::OpenEditorAfterCreation);

	wContainer->setCurrentWidget(wMainPage);

	wMainPage->setLayout(new QHBoxLayout);
	wMainPage->layout()->addWidget(mWindow);

	wEditorPage->setLayout(new QHBoxLayout);

	connect(mWindow, SIGNAL(detectionStarted()), SLOT(onDetectionStarted()));
	connect(mWindow, SIGNAL(detectionFinished()), SLOT(onDetectionFinished()));
	connect(mWindow, SIGNAL(applyingStarted()), SLOT(onApplyingStarted()));
	connect(mWindow, SIGNAL(applyingFinished()), SLOT(onApplyingFinished()));
	connect(mWindow, SIGNAL(editSlot(DeviceSlot *, EditorPane *)), SLOT(onEditSlot(DeviceSlot *, EditorPane *)));
	connect(mWindow, SIGNAL(removeSlot(DeviceSlot *)), SLOT(onRemoveSlot(DeviceSlot *)));
}

//------------------------------------------------------------------------
bool HardwareServiceWindow::activate()
{
	mWindow->setConfiguration(mBackend->getHardwareManager()->getConfiguration());

	return true;
}

//------------------------------------------------------------------------
bool HardwareServiceWindow::deactivate()
{
	wContainer->setCurrentWidget(wMainPage);
	mBackend->getHardwareManager()->setConfigurations(mWindow->getConfiguration().keys());
	mBackend->saveConfiguration();

	return true;
}

//------------------------------------------------------------------------
bool HardwareServiceWindow::initialize()
{
	if (!mWindow->initialize())
	{
		return false;
	}

	return true;
}

//------------------------------------------------------------------------
bool HardwareServiceWindow::shutdown()
{
	mWindow->shutdown();

	return true;
}

//------------------------------------------------------------------------
void HardwareServiceWindow::onDetectionStarted()
{
	GUI::MessageBox::wait(tr("#detecting_devices"), true);
	GUI::MessageBox::subscribe(this);

	QVariantMap params;
	params[SDK::GUI::CMessageBox::ButtonType] = SDK::GUI::MessageBoxParams::Text;
	params[SDK::GUI::CMessageBox::ButtonText] = tr("#stop_search");

	GUI::MessageBox::update(params);
}

//------------------------------------------------------------------------
void HardwareServiceWindow::onDetectionFinished()
{
	mBackend->getHardwareManager()->setConfigurations(mWindow->getConfiguration().keys());

	// Обновляем статусы найденных железок
	mBackend->getHardwareManager()->updateStatuses();

	GUI::MessageBox::hide();
}

//------------------------------------------------------------------------
void HardwareServiceWindow::onEditSlot(DeviceSlot * aSlot, EditorPane * aPane)
{
	connect(aPane, SIGNAL(finished()), SLOT(onEditFinished()), Qt::UniqueConnection);

	wContainer->setCurrentWidget(wEditorPage);

	wEditorPage->layout()->addWidget(aPane->getWidget());

	if (aSlot->getType() == SDK::Driver::CComponents::Modem)
	{
		GUI::MessageBox::wait(tr("#closing_connection"));
		mBackend->getNetworkManager()->closeConnection();
		GUI::MessageBox::hide();
	}
}

//------------------------------------------------------------------------
void HardwareServiceWindow::onRemoveSlot(DeviceSlot * aSlot)
{
	mWindow->removeDeviceSlot(aSlot, true);
}

//------------------------------------------------------------------------
void HardwareServiceWindow::onEditFinished()
{
	wContainer->setCurrentWidget(wMainPage);

	EditorPane * editor = qobject_cast<EditorPane *>(sender());
	if (editor)
	{
		wEditorPage->layout()->removeWidget(editor->getWidget());
		QString deviceType(editor->getSlot()->getType());
		
		if (editor->isChanged())
		{
			editor->getSlot()->setParameterValues(editor->getParameterValues());
			mWindow->checkDeviceSlot(editor->getSlot());

			mBackend->toLog(QString("UPDATE device: %1").arg(editor->getSlot()->getModel()));
		}
		else if (editor->getSlot()->getModel().isEmpty())
		{
			mWindow->removeDeviceSlot(editor->getSlot());
		}

		if (deviceType == SDK::Driver::CComponents::Modem)
		{
			mBackend->getNetworkManager()->openConnection();
		}
	}

	disconnect(sender(), SIGNAL(finished()), this, SLOT(onEditFinished()));
}

//------------------------------------------------------------------------
void HardwareServiceWindow::onApplyingStarted()
{
	GUI::MessageBox::wait(tr("#applying_configuration"));
}

//------------------------------------------------------------------------
void HardwareServiceWindow::onApplyingFinished()
{
	GUI::MessageBox::hide(true);

	// Для переинициализации свежедобавленного устройства. В противном случае не работает тест купюроприемника.
	mWindow->setConfiguration(mBackend->getHardwareManager()->getConfiguration());
}

//------------------------------------------------------------------------
void HardwareServiceWindow::onClicked(const QVariantMap & /*aParameters*/)
{
	GUI::MessageBox::hide();
	GUI::MessageBox::wait(tr("#waiting_stop_search"));
	
	mWindow->abortDetection();

	mBackend->toLog(QString("Button clicked: %1").arg(tr("#stop_search")));
}

//------------------------------------------------------------------------
