/* @file Окно настройки железа. */

// Qt
#include <Common/QtHeadersBegin.h>
<<<<<<< HEAD
#include <QtCore/QTimer>
#include <QtWidgets/QStackedLayout>
=======
#include <QtGui/QStackedLayout>
>>>>>>> release
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/Components.h>

// Проект
#include "MessageBox/MessageBox.h"
#include "Backend/NetworkManager.h"
#include "Backend/HardwareManager.h"
#include "Backend/ServiceMenuBackend.h"
#include "GUI/DeviceSlot.h"
#include "GUI/HardwareWindow.h"
#include "HardwareWizardPage.h"

//------------------------------------------------------------------------
HardwareWizardPage::HardwareWizardPage(ServiceMenuBackend * aBackend, QWidget * aParent)
	: WizardPageBase(aBackend, aParent)
{
	QStackedLayout * layout = new QStackedLayout(this);

	setLayout(layout);

	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);

	// Создаём окно со списком устройств
	mHardwareWindow = new HardwareWindow(aBackend, this);
	mHardwareWindow->setSlotCreationMode(HardwareWindow::OpenEditorAfterCreation);

	// Создаём место для редактора устройств
	mEditorWindow = new QWidget(this);

	QHBoxLayout * editorLayout = new QHBoxLayout(mEditorWindow);

	mEditorWindow->setLayout(editorLayout);

	mEditorWindow->layout()->setSpacing(0);
	mEditorWindow->layout()->setContentsMargins(0, 0, 0, 0);

	layout->addWidget(mHardwareWindow);
	layout->addWidget(mEditorWindow);

	layout->setCurrentWidget(mHardwareWindow);

	connect(mHardwareWindow, SIGNAL(detectionStarted()), SLOT(onDetectionStarted()));
	connect(mHardwareWindow, SIGNAL(detectionFinished()), SLOT(onDetectionFinished()));
	connect(mHardwareWindow, SIGNAL(applyingStarted()), SLOT(onApplyingStarted()));
	connect(mHardwareWindow, SIGNAL(applyingFinished()), SLOT(onApplyingFinished()));
	connect(mHardwareWindow, SIGNAL(editSlot(DeviceSlot *, EditorPane *)), SLOT(onEditSlot(DeviceSlot *, EditorPane *)));
	connect(mHardwareWindow, SIGNAL(removeSlot(DeviceSlot *)), SLOT(onRemoveSlot(DeviceSlot *)));
	connect(mHardwareWindow, SIGNAL(currentFormChanged(int)), SLOT(onCurrentFormChanged(int)));
}

//------------------------------------------------------------------------
bool HardwareWizardPage::initialize()
{
	if (!mHardwareWindow->initialize())
	{
		return false;
	}

	return true;
}

//------------------------------------------------------------------------
bool HardwareWizardPage::shutdown()
{
	mHardwareWindow->shutdown();

	return true;
}

//------------------------------------------------------------------------
bool HardwareWizardPage::activate()
{
	mHardwareWindow->setConfiguration(mBackend->getHardwareManager()->getConfiguration());

	return true;
}

//------------------------------------------------------------------------
bool HardwareWizardPage::deactivate()
{
	QStackedLayout * layout = qobject_cast<QStackedLayout *>(this->layout());
	if (layout)
	{
		layout->setCurrentWidget(mHardwareWindow);
	}

	mBackend->getHardwareManager()->setConfigurations(mHardwareWindow->getConfiguration().keys());

	return true;
}

//----------------------------------------------------------------------------
void HardwareWizardPage::onDetectionStarted()
{
	GUI::MessageBox::wait(tr("#detecting_devices"), true);
	GUI::MessageBox::subscribe(this);

	QVariantMap params;
	params[SDK::GUI::CMessageBox::ButtonType] = SDK::GUI::MessageBoxParams::Text;
	params[SDK::GUI::CMessageBox::ButtonText] = tr("#stop_search");

	GUI::MessageBox::update(params);
}

//----------------------------------------------------------------------------
void HardwareWizardPage::onDetectionFinished()
{
	mBackend->getHardwareManager()->setConfigurations(mHardwareWindow->getConfiguration().keys());
	
	// Обновляем статусы найденных железок
	mBackend->getHardwareManager()->updateStatuses();

	GUI::MessageBox::hide();
}

//----------------------------------------------------------------------------
void HardwareWizardPage::onEditSlot(DeviceSlot * aSlot, EditorPane * aPane)
{
	connect(aPane, SIGNAL(finished()), SLOT(onEditFinished()));

	QStackedLayout * layout = qobject_cast<QStackedLayout *>(this->layout());
	if (layout)
	{
		layout->setCurrentWidget(mEditorWindow);
		mEditorWindow->layout()->addWidget(aPane->getWidget());

		emit pageEvent("#main_form", false);
	}

	if (aSlot->getType() == SDK::Driver::CComponents::Modem)
	{
		mBackend->getNetworkManager()->closeConnection();
	}
}

//----------------------------------------------------------------------------
void HardwareWizardPage::onRemoveSlot(DeviceSlot * aSlot)
{
	mHardwareWindow->removeDeviceSlot(aSlot, true);
}

//----------------------------------------------------------------------------
void HardwareWizardPage::onEditFinished()
{
	QStackedLayout * layout = qobject_cast<QStackedLayout *>(this->layout());
	if (layout)
	{
		layout->setCurrentWidget(mHardwareWindow);

		EditorPane * editor = qobject_cast<EditorPane *>(sender());
		if (editor)
		{
			mEditorWindow->layout()->removeWidget(editor->getWidget());
			QString deviceType(editor->getSlot()->getType());

			if (editor->isChanged())
			{
				editor->getSlot()->setParameterValues(editor->getParameterValues());
				mHardwareWindow->checkDeviceSlot(editor->getSlot());
				mBackend->getHardwareManager()->updateStatuses();
			}
			else if (editor->getSlot()->getModel().isEmpty())
			{
				mHardwareWindow->removeDeviceSlot(editor->getSlot());
			}

			if (deviceType == SDK::Driver::CComponents::Modem)
			{
				mBackend->getNetworkManager()->openConnection();
			}

			emit pageEvent("#main_form", true);
		}
	}

	disconnect(sender(), SIGNAL(finished()), this, SLOT(onEditFinished()));
}

//----------------------------------------------------------------------------
void HardwareWizardPage::onApplyingStarted()
{
	GUI::MessageBox::wait(tr("#applying_configuration"));
}

//----------------------------------------------------------------------------
void HardwareWizardPage::onApplyingFinished()
{
	GUI::MessageBox::hide();

	// Для переинициализации свежедобавленного устройства. В противном случае не работает тест купюроприемника.
	mHardwareWindow->setConfiguration(mBackend->getHardwareManager()->getConfiguration());
}

//------------------------------------------------------------------------
void HardwareWizardPage::onCurrentFormChanged(int aIndex)
{
	emit pageEvent("#main_form", !aIndex);
}

//------------------------------------------------------------------------
void HardwareWizardPage::onClicked(const QVariantMap & /*aParameters*/)
{
	GUI::MessageBox::hide();
	GUI::MessageBox::wait(tr("#waiting_stop_search"));
	
	mHardwareWindow->abortDetection();
}


//------------------------------------------------------------------------
