/* @file Окно визарда. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtGui/QCheckBox>
#include <Common/QtHeadersEnd.h>

// Project
#include "Backend/MessageBox.h"
#include "Backend/ServiceMenuBackend.h"
#include "Backend/KeysManager.h"

#include "WizardContext.h"
#include "WizardFrame.h"
#include "WelcomeWizardPage.h"
#include "HardwareWizardPage.h"
#include "NetworkWizardPage.h"
#include "DialupWizardPage.h"
#include "UnmanagedWizardPage.h"
#include "TokenWizardPage.h"
#include "KeysWizardPage.h"
#include "SaveSettingsWizardPage.h"

//----------------------------------------------------------------------------
namespace CWizardFrame
{
	char * ContextProperty = "cyberContext";
}

//----------------------------------------------------------------------------
WizardFrame::WizardFrame(ServiceMenuBackend * aBackend, QWidget * aParent)
	: QWidget(aParent),
	  mBackend(aBackend),
	  mCurrentPage(0)
{
	setupUi(this);
	wPage->setLayout(new QGridLayout);

	mSignalMapper.connect(btnBack, SIGNAL(clicked()), SLOT(map()));
	mSignalMapper.connect(btnForward, SIGNAL(clicked()), SLOT(map()));

	connect(this, SIGNAL(changePage(const QString &)), this, SLOT(onChangePage(const QString &)));
	connect(&mSignalMapper, SIGNAL(mapped(const QString &)), SLOT(onControlEvent(const QString &)));
	connect(btnExit, SIGNAL(clicked()), SLOT(onExit()));

	mBackend->toLog("Welcome to the First Setup Wizard!");
}

//----------------------------------------------------------------------------
WizardFrame::~WizardFrame()
{
}

//----------------------------------------------------------------------------
void WizardFrame::initialize()
{
	emit changePage(CWizardContext::StartPage);
}

//----------------------------------------------------------------------------
void WizardFrame::shutdown()
{
	hidePage(mCurrentContext, mCurrentPage);

	foreach (CacheItem item, mPages.values())
	{
		item.page->deactivate();
	}
}

//----------------------------------------------------------------------------
void WizardFrame::setStatus(const QString & aStatus)
{
	lbStatus->setText(aStatus);
}

//----------------------------------------------------------------------------
void WizardFrame::setPage(const QString & aContext, WizardPageBase * aPage, bool aCanCache)
{
	if (aCanCache)
	{
		CacheItem item;
		item.page = aPage;
		mPages[aContext] = item;
	}

	connect(aPage, SIGNAL(pageEvent(const QString &, bool)), SLOT(onPageEvent(const QString &, bool)));
	hidePage(mCurrentContext, mCurrentPage);
	showPage(aContext, aPage);

	mBackend->saveConfiguration();
}

//----------------------------------------------------------------------------
void WizardFrame::commitPageChanges()
{
	if (mCurrentPage)
	{
		mCurrentPage->initialize();
	}
}

//----------------------------------------------------------------------------
void WizardFrame::showPage(const QString & aContext, WizardPageBase * aPage)
{
	wPage->layout()->addWidget(aPage);
	aPage->show();
	aPage->activate();

	mCurrentContext = aContext;
	mCurrentPage = aPage;

	mBackend->toLog(QString("Show page %1").arg(aContext));
}

//----------------------------------------------------------------------------
void WizardFrame::hidePage(const QString & aContext, WizardPageBase * aPage)
{
	if (aPage)
	{
		aPage->deactivate();
		aPage->hide();
		wPage->layout()->removeWidget(aPage);
	}

	TPageMap::iterator page = mPages.find(aContext);
	
	if (page != mPages.end())
	{
		for (QList<CacheItem::ControlItem>::iterator i = page->controls.begin(); i != page->controls.end(); ++i)
		{
			switch (i->control)
			{
			case BackButton:
				i->enabled = btnBack->isEnabled();
				break;

			case ForwardButton:
				i->enabled = btnForward->isEnabled();
				break;
			}
		}
	}

	btnBack->hide();
	btnForward->hide();

	mSignalMapper.removeMappings(btnBack);
	mSignalMapper.removeMappings(btnForward);
}

//----------------------------------------------------------------------------
void WizardFrame::setupDecoration(const QString & aStage, const QString & aStageName, const QString & aStageHowto)
{
	lbStage->setText(aStage);
	lbStageName->setText(aStageName);
	lbStageHowto->setText(aStageHowto);

	TPageMap::iterator page = mPages.find(mCurrentContext);

	if (page != mPages.end())
	{
		page->stage = aStage;
		page->stageName = aStageName;
		page->stageHowto = aStageHowto;
	}
}

//----------------------------------------------------------------------------
void WizardFrame::setupControl(Control aControl, bool aEnabled, 
	const QString & aTitle, const QString & aContext, bool aCanCache)
{
	switch (aControl)
	{
		case BackButton:
			btnBack->show();
			btnBack->setEnabled(aEnabled);
			btnBack->setText(aTitle);
			btnBack->setProperty(CWizardFrame::ContextProperty, aContext);
			mSignalMapper.setMapping(btnBack, aContext);
			break;

		case ForwardButton:
			btnForward->show();
			btnForward->setEnabled(aEnabled);
			btnForward->setText(aTitle);
			btnForward->setProperty(CWizardFrame::ContextProperty, aContext);
			mSignalMapper.setMapping(btnForward, aContext);
			break;

		case ExitButton:
			btnExit->setVisible(aEnabled);
			break;
	}

	if (aCanCache)
	{
		TPageMap::iterator page = mPages.find(mCurrentContext);
		
		if (page != mPages.end())
		{
			CacheItem::ControlItem control;
			control.control = aControl;
			control.context = aContext;
			control.title = aTitle;
			control.enabled = aEnabled;

			page->controls << control;
		}
	}
}

//----------------------------------------------------------------------------
void WizardFrame::onPageEvent(const QString & aContext, bool aFlag)
{
	if (aContext == "#can_proceed")
	{
		// Сигнализируем о возможности двигаться дальше
		btnForward->setEnabled(aFlag);
	}
	else if (aContext == "#main_form")
	{
		// Показ кнопок навигации
		wButtonBar->setVisible(aFlag);
	}
	else if (aFlag) 
	{
		// Иначе переход по заданному контексту
		TPageMap::iterator page = mPages.find(aContext);
		
		if (page != mPages.end())
		{
			hidePage(mCurrentContext, mCurrentPage);
			showPage(aContext, page->page);

			foreach (CacheItem::ControlItem control, page->controls)
			{
				setupControl(control.control, control.enabled, control.title, control.context, false);
			}

			setupDecoration(page->stage, page->stageName, page->stageHowto);
		}
		else
		{
			emit changePage(aContext);
		}
	}
}

//----------------------------------------------------------------------------
void WizardFrame::onControlEvent(const QString & aContext)
{
	onPageEvent(aContext, true);
}

//---------------------------------------------------------------------------
QString WizardFrame::stageIndex(const QString & aContext)
{
#ifdef TC_USE_TOKEN
	bool hasRuToken = true;
#else
	bool hasRuToken = false;
#endif

	int index = 0;

	if (aContext == CWizardContext::StartPage)
	{
		index = 0;
	}
	else if (aContext == CWizardContext::SetupHardware)
	{
		index = 1;
	}
	else if (aContext == CWizardContext::SetupNetwork || aContext == CWizardContext::SetupDialup || aContext == CWizardContext::SetupUnmanaged)
	{
		index = 2;
	}
	else if (aContext == CWizardContext::SetupToken)
	{
		index = 3;
	}
	else if (aContext == CWizardContext::SetupKeys)
	{
		index = hasRuToken ? 4 : 3;
	}
	else
	{
		index = hasRuToken ? 5 : 4;
	}

	return QString("%1/%2").arg(index).arg(hasRuToken ? 5 : 4);
}

//---------------------------------------------------------------------------
void WizardFrame::onChangePage(const QString & aContext)
{
#ifdef TC_USE_TOKEN
	bool hasToken = true;
#else
	bool hasToken = false;
#endif
	
	if (aContext == CWizardContext::StartPage)
	{
		WelcomeWizardPage * wwp = new WelcomeWizardPage(mBackend, this);
		
		setPage(aContext, wwp);
		setupDecoration("", "", "");

		connectAllAbstractButtons(wwp);
	}
	else if (aContext == CWizardContext::SetupHardware)
	{
		HardwareWizardPage * hwp = new HardwareWizardPage(mBackend, this);

		setPage(aContext, hwp);
		setupDecoration(stageIndex(aContext), tr("#hardware_setup_stage"), tr("#hardware_setup_howto"));
		setupControl(WizardFrame::BackButton, true,
			tr("#to_start_page"), CWizardContext::StartPage);
		setupControl(WizardFrame::ForwardButton, true,
			tr("#to_network_setup"), CWizardContext::SetupNetwork);

		connectAllAbstractButtons(hwp);
	}
	else if (aContext == CWizardContext::SetupNetwork)
	{
		NetworkWizardPage * nwp = new NetworkWizardPage(mBackend, this);
		
		setPage(aContext, nwp);
		setupDecoration(stageIndex(aContext), tr("#network_setup_stage"), tr("#network_setup_howto"));
		setupControl(WizardFrame::BackButton, true,
			tr("#to_hardware_setup"), CWizardContext::SetupHardware);

		connectAllAbstractButtons(nwp);
	}
	else if (aContext == CWizardContext::SetupDialup)
	{
		DialupWizardPage * dwp = new DialupWizardPage(mBackend, this);
		
		setPage(aContext, dwp);
		setupDecoration(stageIndex(aContext), tr("#dialup_setup_stage"), tr("#dialup_setup_howto"));
		setupControl(WizardFrame::BackButton, true, tr("#to_network_setup"), CWizardContext::SetupNetwork);

		if (hasToken)
		{
			setupControl(WizardFrame::ForwardButton, true, tr("#to_token_setup"), CWizardContext::SetupToken);
		}
		else
		{
			setupControl(WizardFrame::ForwardButton, true, tr("#to_keys_setup"), CWizardContext::SetupKeys);
		}

		connectAllAbstractButtons(dwp);
	}
	else if (aContext == CWizardContext::SetupUnmanaged)
	{
		UnmanagedWizardPage * uwp = new UnmanagedWizardPage(mBackend, this);
		
		setPage(aContext, uwp);
		setupDecoration(stageIndex(aContext), tr("#unmanaged_setup_stage"), tr("#unmanaged_setup_howto"));
		setupControl(WizardFrame::BackButton, true, tr("#to_network_setup"), CWizardContext::SetupNetwork);

		if (hasToken)
		{
			setupControl(WizardFrame::ForwardButton, true, tr("#to_token_setup"), CWizardContext::SetupToken);
		}
		else
		{
			setupControl(WizardFrame::ForwardButton, true, tr("#to_keys_setup"), CWizardContext::SetupKeys);
		}

		connectAllAbstractButtons(uwp);
	}
	else if (aContext == CWizardContext::SetupToken)
	{
		TokenWizardPage * rwp = new TokenWizardPage(mBackend, this);

		setPage(aContext, rwp);
		setupDecoration(stageIndex(aContext), tr("#token_setup_stage"), tr("#token_setup_howto"));
		setupControl(WizardFrame::BackButton, true, tr("#to_network_setup"), CWizardContext::SetupNetwork);
		setupControl(WizardFrame::ForwardButton, true, tr("#to_keys_setup"), CWizardContext::SetupKeys);

		connectAllAbstractButtons(rwp);
	}
	else if (aContext == CWizardContext::SetupKeys)
	{
		KeysWizardPage * kwp = new KeysWizardPage(mBackend, this);

		setPage(aContext, kwp);
		setupDecoration(stageIndex(aContext), tr("#keys_setup_stage"), tr("#keys_setup_howto"));

		if (hasToken)
		{
			setupControl(WizardFrame::BackButton, true, tr("#to_token_setup"), CWizardContext::SetupToken);
		}
		else
		{
			setupControl(WizardFrame::BackButton, true, tr("#to_network_setup"), CWizardContext::SetupNetwork);
		}

		setupControl(WizardFrame::ForwardButton, true, tr("#to_save_settings"), CWizardContext::SaveSettings);

		connectAllAbstractButtons(kwp);
	}
	else if (aContext == CWizardContext::SaveSettings)
	{
		SaveSettingsWizardPage * swp = new SaveSettingsWizardPage(mBackend, this);
		
		setPage(aContext, swp);
		setupDecoration(stageIndex(aContext), tr("#save_settings_stage"), tr("#save_settings_howto"));

		connectAllAbstractButtons(swp);
	}

	commitPageChanges();
}

//----------------------------------------------------------------------------
void WizardFrame::onExit()
{
	if (MessageBox::question(tr("#question_exit")))
	{
		QVariantMap parameters;
		parameters["signal"] = "exit";

		// Завершаем сценарий.
		mBackend->sendEvent(SDK::PaymentProcessor::EEventType::UpdateScenario, parameters);

		// Останавливаем ПО.
		mBackend->sendEvent(SDK::PaymentProcessor::EEventType::StopSoftware);

		mBackend->toLog("Bye-bye.");
	}
}

//------------------------------------------------------------------------
void WizardFrame::connectAllAbstractButtons(QWidget * aParentWidget)
{
	foreach (QAbstractButton * b, aParentWidget->findChildren<QAbstractButton *>())
	{
		connect(b, SIGNAL(clicked()), this, SLOT(onAbstractButtonClicked()));
	}
}

//------------------------------------------------------------------------
void WizardFrame::onAbstractButtonClicked()
{
	QAbstractButton * button = qobject_cast<QAbstractButton *>(sender());

	QString message(QString("Button clicked: %1").arg(button->text()));

	QCheckBox * checkBox = qobject_cast<QCheckBox *>(sender());
	if (checkBox)
	{
		checkBox->isChecked() ? message += " (checked)" : message += " (unchecked)";
	}

	message += ".";

	mBackend->toLog(message);
}

//----------------------------------------------------------------------------
