/* @file Окно с модемным соединением. */

#include "DialupConnectionWindow.h"
#include "ListDelegate.h"

//---------------------------------------------------------------------------
DialupConnectionWindow::DialupConnectionWindow(QWidget * aParent)
	: QWidget(aParent)
{
	setupUi(this);

	lwModems->setItemDelegate(new ListDelegate(lwModems));

	connect(btnTest, SIGNAL(clicked()), SLOT(onTestConnection()));
	connect(btnNew, SIGNAL(clicked()), SLOT(switchToCreatePage()));
	connect(btnBackToList, SIGNAL(clicked()), SLOT(switchToListPage()));
	connect(btnCreate, SIGNAL(clicked()), SLOT(onCreateConnection()));
	connect(btnRemove, SIGNAL(clicked()), SLOT(onRemoveConnection()));

	connect(lwConnections, SIGNAL(currentTextChanged(const QString &)), SIGNAL(userSelectionChanged(const QString &)));

	connect(this, SIGNAL(updated()), SLOT(onUpdated()));
}

//---------------------------------------------------------------------------
DialupConnectionWindow::~DialupConnectionWindow()
{
}

//---------------------------------------------------------------------------
void DialupConnectionWindow::initialize()
{
	switchToListPage();
}

//---------------------------------------------------------------------------
QString DialupConnectionWindow::getUserSelection() const
{
	return lwConnections->count() ? lwConnections->currentItem()->text() : "";
}

//---------------------------------------------------------------------------
void DialupConnectionWindow::fillModemList(const QList<QPair<QString, QString> > & aModems)
{
	lwModems->clear();
	foreach (auto modem, aModems)
	{
		QListWidgetItem * item = new QListWidgetItem();
		item->setData(Qt::DisplayRole, modem.first);
		item->setData(Qt::UserRole + 1, modem.second);

		lwModems->addItem(item);
	}

	// Если список модемов пуст, блокируем кнопку теста соединения
	btnTest->setEnabled(lwModems->count() > 0);

	emit updated();
}

//---------------------------------------------------------------------------
void DialupConnectionWindow::fillConnectionList(const QStringList & aConnections, const QString & aCurrent)
{
	lwConnections->clear();
	lwConnections->addItems(aConnections);

	QList<QListWidgetItem *> items = lwConnections->findItems(aCurrent, Qt::MatchFixedString);

	if (items.size())
	{
		lwConnections->setCurrentItem(items.first(), QItemSelectionModel::ClearAndSelect);
		items.first()->setSelected(true);
	}
	else
	{
		lwConnections->setCurrentRow(0);
	}

	emit updated();
}

//---------------------------------------------------------------------------
void DialupConnectionWindow::fillTemplateList(const QStringList & aTemplates)
{
	lwTemplates->clear();
	lwTemplates->addItems(aTemplates);
	emit updated();
}

//---------------------------------------------------------------------------
void DialupConnectionWindow::onUpdated()
{
	btnTest->setEnabled(lwConnections->count() ? true : false);
	btnRemove->setEnabled(lwConnections->count() ? true : false);
	btnCreate->setEnabled(lwModems->count() && lwTemplates->count());
}

//---------------------------------------------------------------------------
void DialupConnectionWindow::switchToCreatePage()
{
	swPages->setCurrentWidget(wNewConnectionPage);
	lwModems->setCurrentRow(0);
	lwTemplates->setCurrentRow(0);
	onUpdated();
}

//---------------------------------------------------------------------------
void DialupConnectionWindow::switchToListPage()
{
	swPages->setCurrentWidget(wConnectionListPage);
	onUpdated();
}

//---------------------------------------------------------------------------
void DialupConnectionWindow::onCreateConnection()
{
	emit createConnection(lwTemplates->currentItem()->text(), lwModems->currentItem()->text());
}

//---------------------------------------------------------------------------
void DialupConnectionWindow::onRemoveConnection()
{
	emit removeConnection(lwConnections->currentItem()->text());
}


//---------------------------------------------------------------------------
void DialupConnectionWindow::onTestConnection()
{
	emit testConnection(lwConnections->currentItem()->text());
}

//---------------------------------------------------------------------------
