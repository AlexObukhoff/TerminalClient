/* @file Окно настроек. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTime>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/Application.h>
#include <SDK/PaymentProcessor/Core/ICore.h>

// Проект
#include "MessageBox/MessageBox.h"
#include "Backend/KeysManager.h"
#include "Backend/ServiceMenuBackend.h"
#include "KeysServiceWindow.h"

//------------------------------------------------------------------------
KeysServiceWindow::KeysServiceWindow(ServiceMenuBackend * aBackend, QWidget * aParent)
	: QFrame(aParent),
	  ServiceWindowBase(aBackend)
{
	setupUi(this);

	mWindow = new KeysWindow(aBackend, this);

	connect(mWindow, SIGNAL(beginGenerating()), SLOT(onBeginGenerating()));
	connect(mWindow, SIGNAL(endGenerating()), SLOT(onEndGenerating()));
	connect(mWindow, SIGNAL(error(QString)), SLOT(onError(QString)));

	mWindow->setParent(this);
	wContainer->setLayout(new QHBoxLayout);
	wContainer->layout()->setSpacing(0);
	wContainer->layout()->setMargin(0);
	wContainer->layout()->addWidget(mWindow);
}

//------------------------------------------------------------------------
bool KeysServiceWindow::activate()
{
	auto tokenStatus = mBackend->getKeysManager()->tokenStatus();

	mWindow->initialize(tokenStatus.available, tokenStatus.initialized);

	return true;
}

//------------------------------------------------------------------------
bool KeysServiceWindow::deactivate()
{
	return true;
}

//------------------------------------------------------------------------
bool KeysServiceWindow::initialize()
{
	auto tokenStatus = mBackend->getKeysManager()->tokenStatus();

	mWindow->initialize(tokenStatus.available, tokenStatus.initialized);

	return true;
}

//------------------------------------------------------------------------
bool KeysServiceWindow::shutdown()
{
	return true;
}

//------------------------------------------------------------------------
void KeysServiceWindow::onBeginGenerating()
{
	if (GUI::MessageBox::question(tr("#question_generate_keys_warning")))
	{
		GUI::MessageBox::hide();
		GUI::MessageBox::wait(tr("#creating_keys"));
		
		mWindow->doGenerate();
	}
}

//------------------------------------------------------------------------
void KeysServiceWindow::onEndGenerating()
{
	GUI::MessageBox::hide();

	QString generateResult;
	generateResult = "\n" + tr("#sd") + " " + mBackend->getKeysManager()->getSD() + "\n";
	generateResult += tr("#ap") + " " + mBackend->getKeysManager()->getAP() + "\n";
	generateResult += tr("#op") + " " + mBackend->getKeysManager()->getOP();

	if (GUI::MessageBox::question(tr("#question_save_and_register_keys") + generateResult))
	{
		if (mWindow->save())
		{
			mBackend->saveConfiguration();

			if (mBackend->getKeysManager()->isDefaultKeyOP(mBackend->getKeysManager()->getOP()))
			{
				if (MessageBox::question(tr("#question_need_new_config")))
				{
					mBackend->needUpdateConfigs();
				}
			}

			QVariantMap params;
			params["signal"] = "close";
			mBackend->sendEvent(SDK::PaymentProcessor::EEventType::UpdateScenario, params);
			mBackend->sendEvent(SDK::PaymentProcessor::EEventType::CloseApplication);
		}
	}
	else
	{
		auto tokenStatus = mBackend->getKeysManager()->tokenStatus();

		mWindow->initialize(tokenStatus.available, tokenStatus.initialized);
	}
}

//------------------------------------------------------------------------
void KeysServiceWindow::onError(QString aError)
{
	GUI::MessageBox::hide();
	GUI::MessageBox::critical(aError);
}

//------------------------------------------------------------------------
