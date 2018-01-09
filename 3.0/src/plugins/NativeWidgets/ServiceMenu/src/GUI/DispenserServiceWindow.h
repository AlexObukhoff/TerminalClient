/* @file Окно редактирования купюр в диспенсере. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTimer>
#include <QtCore/QFutureWatcher>
#include <QtGui/QStringListModel>
#include "ui_DispenserServiceWindow.h"
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ICashDispenserManager.h>

// Проект
#include "IServiceWindow.h"

namespace PPSDK = SDK::PaymentProcessor;

//------------------------------------------------------------------------
class DispenserServiceWindow : public QFrame, public ServiceWindowBase, protected Ui::DispenserServiceWindow
{
	Q_OBJECT

public:
	DispenserServiceWindow(ServiceMenuBackend * aBackend, QWidget * aParent = 0);

public:
	virtual bool initialize();
	virtual bool shutdown();
	virtual bool activate();
	virtual bool deactivate();
};

//------------------------------------------------------------------------
