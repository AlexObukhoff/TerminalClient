/* @file Окошко для отображения истории инкассаций. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtGui/QWidget>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <Common/QtHeadersEnd.h>

// Project
#include "Backend/PaymentManager.h"
#include "Backend/ServiceMenuBackend.h"
#include "ServiceTags.h"
#include "EncashmentHistoryWindow.h"

//------------------------------------------------------------------------
EncashmentHistoryWindow::EncashmentHistoryWindow(ServiceMenuBackend * aBackend, QWidget * aParent) : 
	QWidget(aParent),
	mBackend(aBackend)
{
	setupUi(this);

	mSignalMapper = new QSignalMapper(this);
	connect(mSignalMapper, SIGNAL(mapped(int)), this, SLOT(printEncashment(int)));
}

//------------------------------------------------------------------------
EncashmentHistoryWindow::~EncashmentHistoryWindow()
{
}

//------------------------------------------------------------------------
void EncashmentHistoryWindow::updateHistory()
{
	// clear all
	foreach(auto button, mWidgets)
	{
		gridHistoryLayout->removeWidget(button);
		button->deleteLater();
	}
	mWidgets.clear();

	auto paymentManager = mBackend->getPaymentManager();
	int count = paymentManager->getEncashmentsHistoryCount();

	for (int i = 0; i < count; i++)
	{
		QVariantMap encashment = paymentManager->getEncashmentInfo(i);

		QString text = QString("[%1] %2\n").arg(encashment[CServiceTags::EncashmentID].toString()).arg(encashment[CServiceTags::EncashmentDate].toString())
			+ tr("#total") + ": " + encashment[CServiceTags::CashAmount].toString();

		QPushButton * button = new QPushButton(text, this);
		button->setMinimumHeight(35);
		button->setMaximumWidth(250);

		connect(button, SIGNAL(clicked()), mSignalMapper, SLOT(map()));
		mSignalMapper->setMapping(button, i);

		gridHistoryLayout->addWidget(button, i % 5, i / 5);
		mWidgets << button;
	}

	gridHistoryLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding), 5, 0);
}

//------------------------------------------------------------------------
void EncashmentHistoryWindow::printEncashment(int aIndex)
{
	auto paymentManager = mBackend->getPaymentManager();

	paymentManager->printEncashment(aIndex);
}

//------------------------------------------------------------------------
