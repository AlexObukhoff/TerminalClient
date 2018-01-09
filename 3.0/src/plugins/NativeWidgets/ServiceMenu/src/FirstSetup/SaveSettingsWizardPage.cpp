/* @file Окно сохранения настроек. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTimer>
#include <QtCore/QtConcurrentRun>
#include <QtCore/QFuture>
#include <Common/QtHeadersEnd.h>

// Проект
#include "Backend/ServiceMenuBackend.h"
#include "GUI/ServiceTags.h"
#include "SaveSettingsWizardPage.h"

//----------------------------------------------------------------------------
SaveSettingsWizardPage::SaveSettingsWizardPage(ServiceMenuBackend * aBackend, QWidget * aParent)
	: WizardPageBase(aBackend, aParent)
{
	setupUi(this);
	
	connect(btnRepeat, SIGNAL(clicked()), SLOT(onSave()));
	connect(btnFinish, SIGNAL(clicked()), SLOT(onFinish()));
}

//------------------------------------------------------------------------
bool SaveSettingsWizardPage::initialize()
{
	return true;
}

//------------------------------------------------------------------------
bool SaveSettingsWizardPage::shutdown()
{
	return true;
}

//------------------------------------------------------------------------
bool SaveSettingsWizardPage::activate()
{
	swPages->setCurrentWidget(wProgressPage);

	QTimer::singleShot(0, this, SLOT(onSave()));

	return true;
}

//------------------------------------------------------------------------
bool SaveSettingsWizardPage::deactivate()
{
	return true;
}

//----------------------------------------------------------------------------
void SaveSettingsWizardPage::onSave()
{
	lbStatus->setText(tr("#saving_configs"));

	// TODO Надо получать описание ошибки
	if (!mBackend->saveConfiguration())
	{
		showError(tr("#when_saving_configs"), tr("#save_configuration_error"));
		return;
	}

	lbStatus->setText(tr("#saved_successfully"));
	swPages->setCurrentWidget(wFinishPage);
}

//----------------------------------------------------------------------------
void SaveSettingsWizardPage::onFinish()
{
	QVariantMap parameters;
	parameters["signal"] = "exit";

	// Завершаем сценарий.
	mBackend->sendEvent(SDK::PaymentProcessor::EEventType::UpdateScenario, parameters);

	mBackend->sendEvent(SDK::PaymentProcessor::EEventType::CloseApplication);
}

//----------------------------------------------------------------------------
void SaveSettingsWizardPage::showError(const QString & aContext, const QString & aError)
{
	swPages->setCurrentWidget(wRepeatPage);

	lbStatus->setText(aContext + "\n" + aError);
}

//----------------------------------------------------------------------------
