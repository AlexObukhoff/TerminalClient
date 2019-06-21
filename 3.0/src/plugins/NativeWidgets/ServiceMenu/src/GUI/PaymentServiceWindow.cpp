/* @file Окно платежей. */

// boost
#include <boost/bind.hpp>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTimer>
#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QFuture>
#include <QtCore/QSettings>
#include <QtCore/QItemSelectionModel>
#include <QtCore/QSortFilterProxyModel>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLayout>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QButtonGroup>
#include <Common/QtHeadersEnd.h>

// SDK
#include<SDK/PaymentProcessor/Payment/Step.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>
#include <SDK/PaymentProcessor/Core/IPaymentService.h>
#include <SDK/PaymentProcessor/Core/ReceiptTypes.h>

// Проект
#include "ServiceTags.h"
#include "MessageBox/MessageBox.h"
#include "Backend/PaymentManager.h"
#include "Backend/ServiceMenuBackend.h"
#include "PaymentServiceWindow.h"

namespace PPSDK = SDK::PaymentProcessor;
namespace CPayment = SDK::PaymentProcessor::CPayment::Parameters;

//----------------------------------------------------------------------------
namespace CPaymentServiceWindow
{
	const QString ColumnVisibility = "columnVisibility";
};

//----------------------------------------------------------------------------
PaymentServiceWindow::PaymentServiceWindow(ServiceMenuBackend * aBackend, QWidget * aParent)
	: QFrame(aParent),
	ServiceWindowBase(aBackend),
	mBackend(aBackend),
	mFiscalMode(false)
{
	setupUi(this);

	mPaymentManager = mBackend->getPaymentManager();
	mModel = new PaymentTableModel(mFiscalMode, mPaymentManager, this);
	mProxyModel = new PaymentProxyModel(this);
	mProxyModel->setDynamicSortFilter(true);
	mProxyModel->setSourceModel(mModel);
	tvPayments->setModel(mProxyModel);

	tvPayments->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	tvPayments->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	connect(mProxyModel, SIGNAL(layoutChanged()), tvPayments, SLOT(resizeRowsToContents()), Qt::QueuedConnection);

	createColumnWidgets();
	setupWidgets();
	setupConnections();

	wPaymentSummary->hide();
}

//----------------------------------------------------------------------------
void PaymentServiceWindow::createColumnWidgets()
{
	if (!pageFields->layout() || 
		pageFields->layout()->count() >= mProxyModel->columnCount())
	{
		return;
	}

	QSet<int> defaultDisabledColumn;
	defaultDisabledColumn << PaymentTableModel::LastUpdate 
		<< PaymentTableModel::InitialSession
		<< PaymentTableModel::Session
		<< PaymentTableModel::TransId;

	for (int i = 0; i < mModel->columnCount(); i++)
	{
		if (mProxyModel->hiddenColumn(i))
		{
			mProxyModel->showColumn(i, false);
			continue;
		}

		QCheckBox * checkBox = new QCheckBox(this);
		QString header = mModel->headerData(i, Qt::Horizontal).toString();
		
		checkBox->setText(header);
		checkBox->setProperty("column", i);

		checkBox->setChecked(!defaultDisabledColumn.contains(i));
		mProxyModel->showColumn(i, !defaultDisabledColumn.contains(i));

		connect(checkBox, SIGNAL(toggled(bool)), this, SLOT(showColumn(bool)));
	
		pageFields->layout()->addWidget(checkBox);
		mColumnCheckboxs.insert(i, checkBox);
	}
	pageFields->layout()->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

//----------------------------------------------------------------------------
void PaymentServiceWindow::setupWidgets()
{
	mPaymentsFilterButtonGroup = new QButtonGroup(this);
	mPaymentsFilterButtonGroup->addButton(rbAllPayments);
	mPaymentsFilterButtonGroup->addButton(rbPrinted);
	mPaymentsFilterButtonGroup->addButton(rbProcessed);

	mDateFilterButtonGroup = new QButtonGroup(this);
	mDateFilterButtonGroup->addButton(rbAllDates);
	mDateFilterButtonGroup->addButton(rbLastEncashment);
	mDateFilterButtonGroup->addButton(rbDate);

	dateEdit->setDate(QDate::currentDate());

	QStringList dateRangeItems;
	dateRangeItems.insert(DayRange, tr("#day"));
	dateRangeItems.insert(WeekRange, tr("#week"));
	dateRangeItems.insert(MonthRange, tr("#month"));
	dateRangeItems.insert(ThreeMonthRange, tr("#three_month"));
	cbRange->addItems(dateRangeItems);
}

//----------------------------------------------------------------------------
void PaymentServiceWindow::setupConnections()
{
	connect(&mPaymentTaskWatcher, SIGNAL(finished()), SLOT(onPaymentsUpdated()));
	connect(mModel, SIGNAL(updatePayments(QString)), SLOT(onUpdatePayments(QString)));
	connect(mModel, SIGNAL(showProcessWindow(bool, QString)), SLOT(onShowProcessWindow(bool, QString)));
	connect(btnPrintCurrentReceipt, SIGNAL(clicked()), SLOT(printCurrentReceipt()));
	connect(btnPrintReceipts, SIGNAL(clicked()), mModel, SLOT(printAllReceipts()));
	connect(btnPrintFilteredReceipts, SIGNAL(clicked()), this, SLOT(printFilteredReceipts()));
	connect(btnProcessCurrentPayment, SIGNAL(clicked()), SLOT(processCurrentPayment()));
	connect(btnProcessPayments, SIGNAL(clicked()), mModel, SLOT(processAllPayments()));

	connect(rbAllDates, SIGNAL(toggled(bool)), this, SLOT(disableDateFilter(bool)));
	connect(rbLastEncashment, SIGNAL(toggled(bool)), this, SLOT(enableLastEncashmentFilter(bool)));
	connect(rbDate, SIGNAL(toggled(bool)), this, SLOT(enableDateRangeFilter(bool)));

	connect(rbDate, SIGNAL(toggled(bool)), cbRange, SLOT(setEnabled(bool)));
	connect(rbDate, SIGNAL(toggled(bool)), dateEdit, SLOT(setEnabled(bool)));

	connect(cbRange, SIGNAL(currentIndexChanged(int)), this, SLOT(updateDateRange()));
	connect(dateEdit, SIGNAL(dateChanged(QDate)), this, SLOT(updateDateRange()));

	connect(rbAllPayments, SIGNAL(toggled(bool)), mProxyModel, SLOT(disablePaymentsFilter()));
	connect(rbProcessed, SIGNAL(toggled(bool)), mProxyModel, SLOT(enableProcessedPaymentsFilter()));
	connect(rbPrinted, SIGNAL(toggled(bool)), mProxyModel, SLOT(enablePrintedPaymentsFilter()));
	
	connect(leSearch, SIGNAL(textChanged(QString)), mProxyModel, SLOT(setFilterWildcard(QString)));
}

//----------------------------------------------------------------------------
bool PaymentServiceWindow::initialize()
{
	ServiceMenuBackend::TAccessRights rights = mBackend->getAccessRights();

	// Право на просмотр суммарной информации по платежам
	wPaymentSummary->setEnabled(rights.contains(ServiceMenuBackend::ViewPaymentSummary));

	// Право на просмотр платежей
	tvPayments->setEnabled(rights.contains(ServiceMenuBackend::ViewPayments));

	return true;
}

//----------------------------------------------------------------------------
bool PaymentServiceWindow::shutdown()
{
	return true;
}

//----------------------------------------------------------------------------
bool PaymentServiceWindow::activate()
{
	// Обновим состояние кнопок печати
	bool canPrint = mBackend->getPaymentManager()->canPrint(PPSDK::CReceiptType::Payment);

	btnPrintCurrentReceipt->setEnabled(canPrint);
	btnPrintReceipts->setEnabled(canPrint);
	btnPrintFilteredReceipts->setEnabled(canPrint);
	
	// Обновляем все данные по платежам
	onUpdatePayments();

	connect(mPaymentManager, SIGNAL(receiptPrinted(qint64, bool)), mModel, SLOT(onReceiptPrinted(qint64, bool)));
	connect(mPaymentManager, SIGNAL(paymentChanged(qint64)), mModel, SLOT(onUpdatePayment(qint64)));

	QVariantMap parameters = mBackend->getConfiguration();
	QVariantList columns = parameters[CPaymentServiceWindow::ColumnVisibility].toList();
	for (int i = 0; i < columns.size(); i++)
	{
		mColumnCheckboxs.contains(i) ? mColumnCheckboxs[i]->setChecked(columns[i].toBool()) : mProxyModel->showColumn(i, columns[i].toBool());
	}

	return true;
}

//----------------------------------------------------------------------------
bool PaymentServiceWindow::deactivate()
{
	disconnect(mPaymentManager, SIGNAL(receiptPrinted(qint64, bool)), mModel, SLOT(onReceiptPrinted(qint64, bool)));
	disconnect(mPaymentManager, SIGNAL(paymentChanged(qint64)), mModel, SLOT(onUpdatePayment(qint64)));

	QVariantMap parameters;
	parameters.insert(CPaymentServiceWindow::ColumnVisibility, mProxyModel->getColumnVisibility());
	mBackend->setConfiguration(parameters);
	mBackend->saveConfiguration();

	return true;
}

//----------------------------------------------------------------------------
void PaymentServiceWindow::onUpdatePayments(const QString & aMessage)
{
	QVariantMap result;
		
	if (mPaymentManager->getPaymentsInfo(result))
	{
		lbLastRecievedPayment->setText(result[CServiceTags::LastPaymentDate].toString());
		lbLastProcessedPayment->setText(result[CServiceTags::LastProcessedPaymentDate].toString());
		lbSuccessfulPaymentCount->setText(result[CServiceTags::SuccessfulPaymentCount].toString());
		lbFailedPaymentCount->setText(result[CServiceTags::FailedPaymentCount].toString());
	}

	aMessage.isEmpty() 
		? GUI::MessageBox::wait(tr("#updating_payment_data")) 
		: GUI::MessageBox::wait(aMessage);

	mPaymentTaskWatcher.setFuture(QtConcurrent::run(this, &PaymentServiceWindow::loadPayments));
}

//----------------------------------------------------------------------------
void PaymentServiceWindow::onShowProcessWindow(bool aShow, const QString & aMessage)
{
	if (aShow)
	{
		GUI::MessageBox::wait(aMessage);
	}
	else
	{
		GUI::MessageBox::hide();
	}
}

//----------------------------------------------------------------------------
void PaymentServiceWindow::loadPayments()
{
	mPaymentInfoList = mPaymentManager->getPayments(true);
}

//----------------------------------------------------------------------------
void PaymentServiceWindow::onPaymentsUpdated()
{
	mModel->setPayments(mPaymentInfoList);
	
	rbLastEncashment->setChecked(true);
	rbAllPayments->setChecked(true);

	GUI::MessageBox::hide(true);
}

//----------------------------------------------------------------------------
void PaymentServiceWindow::printCurrentReceipt()
{
	mModel->printReceipt(getSelectedIndex());
}

//----------------------------------------------------------------------------
void PaymentServiceWindow::processCurrentPayment()
{
	mModel->proccessPayment(getSelectedIndex());
}

//----------------------------------------------------------------------------
QModelIndex PaymentServiceWindow::getSelectedIndex()
{
	QItemSelectionModel * selectionModel = tvPayments->selectionModel();
	QItemSelection itemSelection = selectionModel->selection();
	QItemSelection sourceSelection = mProxyModel->mapSelectionToSource(itemSelection);

	return sourceSelection.isEmpty() ? QModelIndex() : sourceSelection.indexes().first();
}

//----------------------------------------------------------------------------
void PaymentServiceWindow::disableDateFilter(bool aEnabled)
{
	if (aEnabled)
	{
		mProxyModel->disableDateFilter();
	}
}

//----------------------------------------------------------------------------
void PaymentServiceWindow::enableDateRangeFilter(bool aEnabled)
{
	if (aEnabled)
	{
		updateDateRange();
	}
}

//----------------------------------------------------------------------------
void PaymentServiceWindow::enableLastEncashmentFilter(bool aEnabled)
{
	if (aEnabled)
	{
		QVariantMap encashmentInfo = mBackend->getPaymentManager()->getBalanceInfo();
		if (encashmentInfo.isEmpty())
		{
			rbLastEncashment->setEnabled(false);
			rbAllDates->setChecked(true);
			return;
		}

		QDateTime lastEncashment = encashmentInfo[CServiceTags::LastEncashmentDate].toDateTime();

		mProxyModel->setDateFilter(lastEncashment, QDateTime::currentDateTime());
	}
}

//----------------------------------------------------------------------------
void PaymentServiceWindow::updateDateRange()
{
	QDateTime end;
	end.setDate(dateEdit->date());
	end = end.addDays(1);
	QDateTime start;

	switch (cbRange->currentIndex())
	{
	case DayRange:
		start = end.addDays(-1);
		break;
	case WeekRange:
		start = end.addDays(-7);
		break;
	case MonthRange:
		start = end.addMonths(-1);
		break;
	case ThreeMonthRange:
		start = end.addMonths(-3);
		break;
	default:
		start = end.addDays(-1);
	}

	mProxyModel->setDateFilter(start, end); 
}

//----------------------------------------------------------------------------
void PaymentServiceWindow::showColumn(bool aShow)
{
	if (sender())
	{
		int columnId = sender()->property("column").toInt();
		mProxyModel->showColumn(columnId, aShow);
	}
}

//----------------------------------------------------------------------------
PaymentTableModel::PaymentTableModel(bool aFiscalMode, PaymentManager * aPaymentManager, QObject * aParent)
	: QAbstractTableModel(aParent),
	mFiscalMode(aFiscalMode),
	mPaymentManager(aPaymentManager)
{
	columnHeaders.insert(Id, tr("#id"));
	columnHeaders.insert(ProviderFields, tr("#provider_fields"));
	columnHeaders.insert(Amount, tr("#amount_field"));
	columnHeaders.insert(AmountAll, tr("#amount_all_field"));
	columnHeaders.insert(Provider, tr("#provider"));
	columnHeaders.insert(CreationDate, tr("#create_date_field"));
	columnHeaders.insert(LastUpdate, tr("#last_update_field"));
	columnHeaders.insert(InitialSession, tr("#initial_session"));
	columnHeaders.insert(Session, tr("#session"));
	columnHeaders.insert(TransId, tr("#trans_id"));
	columnHeaders.insert(Status, tr("#status_field"));
	columnHeaders.insert(Printed, (mFiscalMode ? tr("#fiscal_receipt_printed_field") : tr("#receipt_printed_field")));
	columnHeaders.insert(Processed, "#processed");
}

//----------------------------------------------------------------------------
int PaymentTableModel::rowCount(const QModelIndex & /*parent*/) const
{
	return mPaymentInfoList.size();
}

//----------------------------------------------------------------------------
int PaymentTableModel::columnCount(const QModelIndex & /*parent*/) const
{
	return columnHeaders.size();
}

//----------------------------------------------------------------------------
QVariant PaymentTableModel::data(const QModelIndex & index, int role) const
{
	if (!index.isValid())
	{
		return QVariant();
	}

	int row = index.row();

	if (role == IDRole)
	{
		return mPaymentInfoList[row].getId();
	}

	if (role == DataRole)
	{
		switch (index.column())
		{
		case Id:
			return mPaymentInfoList[row].getId();
		case Provider:
			return mPaymentInfoList[row].getProvider();
		case ProviderFields:
			return mPaymentInfoList[row].getProviderFields();
		case Amount:
			return mPaymentInfoList[row].getAmount();
		case AmountAll:
			return mPaymentInfoList[row].getAmountAll();
		case CreationDate:
			return mPaymentInfoList[row].getCreationDate();
		case LastUpdate:
			return mPaymentInfoList[row].getLastUpdate();
		case InitialSession:
			return mPaymentInfoList[row].getInitialSession();
		case Session:
			return mPaymentInfoList[row].getSession();
		case TransId:
			return mPaymentInfoList[row].getTransId();
		case Printed:
			return mPaymentInfoList[row].getPrinted();
		case Status:
			return mPaymentInfoList[row].getStatus();
		case Processed:
			return mPaymentInfoList[row].isProccesed();
		default:
			return QVariant();
		}
	}

	if (role == Qt::DisplayRole)
	{
		switch (index.column())
		{
		case Id:
			return mPaymentInfoList[row].getId();
		case Provider:
			return mPaymentInfoList[row].getProvider();
		case ProviderFields:
			return mPaymentInfoList[row].getProviderFields();
		case Amount:
			return QString::number(mPaymentInfoList[row].getAmount(), 'f', 2);
		case AmountAll:
			return QString::number(mPaymentInfoList[row].getAmountAll(), 'f', 2);
		case CreationDate:
			return mPaymentInfoList[row].getCreationDate().toString("yyyy.MM.dd hh:mm:ss");
		case LastUpdate:
			return mPaymentInfoList[row].getLastUpdate().toString("yyyy.MM.dd hh:mm:ss:zzz");
		case InitialSession:
			return mPaymentInfoList[row].getInitialSession();
		case Session:
			return mPaymentInfoList[row].getSession();
		case TransId:
			return mPaymentInfoList[row].getTransId();
		case Printed:
			return mPaymentInfoList[row].getPrinted() ? tr("#yes") : tr("#no");
		case Status:
			return mPaymentInfoList[row].getStatusString();
		case Processed:
			return mPaymentInfoList[row].isProccesed();
		default:
			return QVariant();
		}
	}
	
	if (role == Qt::BackgroundRole)
	{
		auto status = mPaymentInfoList[row].getStatus();

		// Попытка мошенничества - фиолетовая строка.
		if (status == PPSDK::EPaymentStatus::Cheated)
		{
			return QBrush(QColor(255, 172, 255));
		}

		// Платёж проведён, удалён или отменён - зелёная строка.
		if (status == PPSDK::EPaymentStatus::Completed || 
			status == PPSDK::EPaymentStatus::Canceled || 
			status == PPSDK::EPaymentStatus::Deleted)
		{
			return QBrush(QColor(172, 255, 174));
		}

		// Платёж проводится - жёлтая строка.
		if (status == PPSDK::EPaymentStatus::ReadyForCheck)
		{
			return QBrush(QColor(255, 255, 172));
		}

		// Остальное - красные строки
		return QBrush(QColor(255, 172, 174));
	}

	return QVariant();
}

//----------------------------------------------------------------------------
Qt::ItemFlags PaymentTableModel::flags(const QModelIndex & /*index*/) const
{
	return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

//----------------------------------------------------------------------------
QVariant PaymentTableModel::headerData(int aSection, Qt::Orientation aOrientation, int aRole) const
{
	if ((aRole == Qt::DisplayRole) && (aOrientation == Qt::Horizontal))
	{
		if (aSection >= 0 && aSection < columnHeaders.size())
		{
			Column column = static_cast<Column>(aSection);
			return columnHeaders.value(column);
		}
		else
		{
			return QString();
		}
	}
	else
	{
		return QVariant();
	}
}

//----------------------------------------------------------------------------
void PaymentTableModel::printReceipt(const QModelIndex & index)
{
	if (index.row() >= 0 && index.row() < mPaymentInfoList.size())
	{
		PaymentInfo payment = mPaymentInfoList[index.row()];
	
		if (payment.canPrint())
		{
			if (mPaymentManager->printReceipt(payment.getId(), false))
			{
				mPrintingQueue.insert(payment.getId());
			}

			emit showProcessWindow(true, tr("printing_receipt"));
		}
		else
		{
			GUI::MessageBox::info(tr("#printed_before"));
		}
	}
	else
	{
		GUI::MessageBox::info(tr("#select_payment_to_print"));
	}
}

//----------------------------------------------------------------------------
void PaymentTableModel::printAllReceipts()
{
	foreach (const PaymentInfo & paymentInfo, mPaymentInfoList)
	{
		if (paymentInfo.canPrint() && !paymentInfo.getPrinted())
		{
			if (mPaymentManager->printReceipt(paymentInfo.getId(), true))
			{
				mPrintingQueue.insert(paymentInfo.getId());
			}
		}
	}

	if (!mPrintingQueue.isEmpty())
	{
		emit showProcessWindow(true, tr("#printing %1 receipts").arg(mPrintingQueue.size()));
	}
	else
	{
		GUI::MessageBox::info(tr("#nothing_to_print"));
	}
}

//----------------------------------------------------------------------------
void PaymentServiceWindow::printFilteredReceipts()
{
	QSet<qint64> payments;

	for (int i = 0; i < mProxyModel->rowCount(); ++i)
	{
		payments << mProxyModel->data(mProxyModel->index(i, 0), PaymentTableModel::IDRole).toLongLong();
	}

	mModel->printFilteredReceipts(payments);
}

//----------------------------------------------------------------------------
void PaymentTableModel::printFilteredReceipts(const QSet<qint64> & aPaymentsID)
{
	foreach (const PaymentInfo & paymentInfo, mPaymentInfoList)
	{
		if (paymentInfo.canPrint() && !paymentInfo.getPrinted() && aPaymentsID.contains(paymentInfo.getId()))
		{
			if (mPaymentManager->printReceipt(paymentInfo.getId(), true))
			{
				mPrintingQueue.insert(paymentInfo.getId());
			}
		}
	}

	if (!mPrintingQueue.isEmpty())
	{
		emit showProcessWindow(true, tr("#printing %1 receipts").arg(mPrintingQueue.size()));
	}
	else
	{
		GUI::MessageBox::info(tr("#nothing_to_print"));
	}
}

//----------------------------------------------------------------------------
void PaymentTableModel::onReceiptPrinted(qint64 aPaymentId, bool aErrorHappened)
{
	if (aErrorHappened)
	{
		GUI::MessageBox::critical(tr("#error_occurred_printing"));
		mPrintingQueue.clear();
		return;
	}

	if (mPrintingQueue.isEmpty())
	{
		return;
	}

	mPrintingQueue.remove(aPaymentId);


	emit layoutAboutToBeChanged();

	int row = mPaymentRowIndex[aPaymentId];
	mPaymentInfoList[row].setPrinted(true);

	emit dataChanged(index(row, 0), index(row, columnCount()));
	emit layoutChanged();

	if (mPrintingQueue.isEmpty())
	{
		if (!aErrorHappened)
		{
			emit showProcessWindow(false, "");
		}
	}
	else
	{
		emit showProcessWindow(true, tr("#printing %1 receipts").arg(mPrintingQueue.size()));
	}
}

//----------------------------------------------------------------------------
void PaymentTableModel::onUpdatePayment(qint64 aPaymentId)
{
	if (mPaymentRowIndex.contains(aPaymentId))
	{
		emit layoutAboutToBeChanged();
		int row = mPaymentRowIndex[aPaymentId];

		mPaymentInfoList[row] = mPaymentManager->getPayment(aPaymentId);

		emit dataChanged(index(row, 0), index(row, columnCount()));
		emit layoutChanged();
	}
}


//----------------------------------------------------------------------------
void PaymentTableModel::proccessPayment(const QModelIndex & index)
{
	if (index.row() >= 0 && index.row() < mPaymentInfoList.size())
	{
		PaymentInfo payment = mPaymentInfoList[index.row()];

		if (payment.canProcess())
		{
			mPaymentManager->processPayment(payment.getId());
			onUpdatePayment(payment.getId());

			GUI::MessageBox::info(tr("#process"));
		}
		else
		{
			GUI::MessageBox::info(tr("#bad_status"));
		}
	}
	else
	{
		GUI::MessageBox::info(tr("#select_payment_to_process"));
	}
}

//----------------------------------------------------------------------------
void PaymentTableModel::proccessNextPayment()
{
	if (mProcessPayments.payments.count())
	{
		auto payment = mProcessPayments.payments.takeFirst();

		if (payment.canProcess())
		{
			mPaymentManager->processPayment(payment.getId());

			onUpdatePayment(payment.getId());
			++mProcessPayments.processed;
		}
	}
	else
	{
		if (mProcessPayments.processed)
		{
			GUI::MessageBox::info(tr("#process %1 payments").arg(mProcessPayments.processed));
		}
		else
		{
			GUI::MessageBox::info(tr("#nothing_to_process"));
		}

		mProcessPayments.clear();
		return;
	}

	// Вызываем обработку следующего платежа
	QMetaObject::invokeMethod(this, "proccessNextPayment", Qt::QueuedConnection);
}

//----------------------------------------------------------------------------
void PaymentTableModel::processAllPayments()
{
	mProcessPayments.payments = mPaymentInfoList;
	mProcessPayments.processed = 0;

	GUI::MessageBox::wait(tr("#updating_payment_data"), true);
	GUI::MessageBox::subscribe(this);

	// Вызываем обработку следующего платежа
	QMetaObject::invokeMethod(this, "proccessNextPayment", Qt::QueuedConnection);
}

//----------------------------------------------------------------------------
void PaymentTableModel::onClicked(const QVariantMap &)
{
	// прерываем обработку платежей
	mProcessPayments.payments.clear();
}

//----------------------------------------------------------------------------
void PaymentTableModel::setPayments(QList<PaymentInfo> aPaymentInfoList)
{
	beginResetModel();

	mProcessPayments.clear();

	mPaymentInfoList = aPaymentInfoList;

	mPaymentRowIndex.clear();
	for (int i = 0; i < mPaymentInfoList.size(); ++i)
	{
		mPaymentRowIndex.insert(mPaymentInfoList[i].getId(), i);
	}

	endResetModel();
}

//----------------------------------------------------------------------------
PaymentProxyModel::PaymentProxyModel(QObject * parent) :
	QSortFilterProxyModel(parent),
	mPaymentFilter(AllPayments),
	mDateFilterEnabled(false)
{
	setFilterCaseSensitivity(Qt::CaseInsensitive);
	setSortRole(PaymentTableModel::DataRole);
}

//----------------------------------------------------------------------------
void PaymentProxyModel::showColumn(int aColumn, bool aShow)
{
	PaymentTableModel::Column column = static_cast<PaymentTableModel::Column>(aColumn);
	mColumns[column] = aShow;
	//QSettings settings;
	//settings.setValue(QString("ServiceMenu/column%1").arg(aColumn), aShow);
	invalidateFilter();
}

//----------------------------------------------------------------------------
QVariantList PaymentProxyModel::getColumnVisibility() const
{
	QVariantList columns;
	for (int i = 0; i < mColumns.size(); i++)
		columns << mColumns[i];
	return columns;
}

//----------------------------------------------------------------------------
bool PaymentProxyModel::hiddenColumn(int aColumn) const
{
	switch (aColumn)
	{
	case PaymentTableModel::Id:
	case PaymentTableModel::Processed:
		return true;
	default:
		return false;
	}
}

//----------------------------------------------------------------------------
void PaymentProxyModel::disableDateFilter()
{
	mDateFilterEnabled = false;
	invalidateFilter();
}

//----------------------------------------------------------------------------
void PaymentProxyModel::setDateFilter(const QDateTime & aFrom, const QDateTime & aTo)
{
	mStartDateTime = aFrom;
	mEndDateTime = aTo;
	mDateFilterEnabled = true;

	invalidateFilter();
}

//----------------------------------------------------------------------------
void PaymentProxyModel::disablePaymentsFilter()
{
	mPaymentFilter = AllPayments;
	invalidateFilter();
}

//----------------------------------------------------------------------------
void PaymentProxyModel::enablePrintedPaymentsFilter()
{
	mPaymentFilter = PrintedPayments;
	invalidateFilter();
}

//----------------------------------------------------------------------------
void PaymentProxyModel::enableProcessedPaymentsFilter()
{
	mPaymentFilter = ProcessedPayments;
	invalidateFilter();
}

//----------------------------------------------------------------------------
bool PaymentProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex & sourceParent) const
{
	QAbstractItemModel * sourceModel = this->sourceModel();
	QRegExp regExp = filterRegExp();

	QModelIndex providerFieldsIndex = sourceModel->index(sourceRow, PaymentTableModel::ProviderFields, sourceParent);
	QString providerFieldsValue = sourceModel->data(providerFieldsIndex, PaymentTableModel::DataRole).toString();
	bool providerFieldsFilter = providerFieldsValue.contains(regExp);

	QModelIndex initialSessionIndex = sourceModel->index(sourceRow, PaymentTableModel::InitialSession, sourceParent);
	QString initialSessionValue = sourceModel->data(initialSessionIndex, PaymentTableModel::DataRole).toString();
	bool initialSessionFilter = initialSessionValue.contains(regExp);

	QModelIndex sessionIndex = sourceModel->index(sourceRow, PaymentTableModel::Session, sourceParent);
	QString sessionValue = sourceModel->data(sessionIndex, PaymentTableModel::DataRole).toString();
	bool sessionFilter = sessionValue.contains(regExp);

	QModelIndex transIdIndex = sourceModel->index(sourceRow, PaymentTableModel::TransId, sourceParent);
	QString transIdValue = sourceModel->data(transIdIndex, PaymentTableModel::DataRole).toString();
	bool transIdFilter = transIdValue.contains(regExp);

	bool dateFilter = true;
	if (mDateFilterEnabled)
	{
		QModelIndex creationDateIndex = sourceModel->index(sourceRow, PaymentTableModel::CreationDate, sourceParent);
		QDateTime creationDateTimeValue = sourceModel->data(creationDateIndex, PaymentTableModel::DataRole).toDateTime();
		dateFilter = creationDateTimeValue >= mStartDateTime && creationDateTimeValue <= mEndDateTime;
	}
		
	bool processedFilter = true;
	if (mPaymentFilter == ProcessedPayments)
	{
		QModelIndex processedIndex = sourceModel->index(sourceRow, PaymentTableModel::Processed, sourceParent);
		processedFilter = !sourceModel->data(processedIndex, PaymentTableModel::DataRole).toBool();
	}

	bool printedFilter = true;
	if (mPaymentFilter == PrintedPayments)
	{
		QModelIndex printedIndex = sourceModel->index(sourceRow, PaymentTableModel::Printed, sourceParent);
		printedFilter = !sourceModel->data(printedIndex, PaymentTableModel::DataRole).toBool();
	}
		
	return 
		(providerFieldsFilter || initialSessionFilter || sessionFilter || transIdFilter)
		&& dateFilter
		&& processedFilter 
		&& printedFilter;
}

//----------------------------------------------------------------------------
bool PaymentProxyModel::filterAcceptsColumn(int sourceColumn, const QModelIndex & sourceParent) const
{
	Q_UNUSED(sourceParent);
	
	PaymentTableModel::Column column = static_cast<PaymentTableModel::Column>(sourceColumn);
	return mColumns.value(column, true);
}

//----------------------------------------------------------------------------
