/* @file Окно платежей. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QFutureWatcher>
#include <QtCore/QPointer>
#include <QtGui/QCheckBox>
#include <QtGui/QSortFilterProxyModel>
#include <QtCore/QList>
#include <QtCore/QMap>
#include "ui_PaymentServiceWindow.h"
#include <Common/QtHeadersEnd.h>

// Project
#include "IServiceWindow.h"
#include "PaymentInfo.h"

class ServiceMenuBackend;
class PaymentProxyModel;
class PaymentTableModel;
class PaymentManager;

//----------------------------------------------------------------------------
class PaymentServiceWindow : public QFrame, public ServiceWindowBase, protected Ui::PaymentServiceWindow
{
	Q_OBJECT

public:
	PaymentServiceWindow(ServiceMenuBackend * aBackend, QWidget * aParent = 0);
	virtual bool initialize();
	virtual bool shutdown();
	virtual bool activate();
	virtual bool deactivate();

private:
	void createColumnWidgets();
	void setupWidgets();
	void setupConnections();
	void loadPayments();
	QModelIndex getSelectedIndex();

private slots:
	void printCurrentReceipt();
	void processCurrentPayment();
	void onUpdatePayments(const QString & aMessage = QString());
	void onShowProcessWindow(bool aShow, const QString & aMessage);
	void onPaymentsUpdated();
	
	void printFilteredReceipts();

	void disableDateFilter(bool aEnabled);
	void enableLastEncashmentFilter(bool aEnabled);
	void enableDateRangeFilter(bool aEnabled);
	//void enableDateFilter(bool aEnabled);

	void updateDateRange();

	void showColumn(bool aShow);

private:
	enum DateRange
	{
		DayRange,
		WeekRange,
		MonthRange,
		ThreeMonthRange
	};

	ServiceMenuBackend * mBackend;
	PaymentManager * mPaymentManager;
	QFutureWatcher<void> mPaymentTaskWatcher;
	bool mFiscalMode;
	PaymentTableModel * mModel;
	PaymentProxyModel * mProxyModel;
	QList<PaymentInfo> mPaymentInfoList;
	QButtonGroup * mDateFilterButtonGroup;
	QButtonGroup * mPaymentsFilterButtonGroup;
	QMap<int, QPointer<QCheckBox>> mColumnCheckboxs;
};

//----------------------------------------------------------------------------
class PaymentTableModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	enum Column
	{
		Id,
		Provider,
		ProviderFields,
		Amount,
		AmountAll,
		CreationDate,
		LastUpdate,
		InitialSession,
		Session,
		TransId,
		Printed,
		Status,
		Processed
	};

	enum Role
	{
		DataRole = Qt::UserRole,
		IDRole
	};

	PaymentTableModel(bool aFiscalMode, PaymentManager * aPaymentMananger, QObject * aParent = 0);
	virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
	virtual Qt::ItemFlags flags(const QModelIndex & index) const;
	virtual QVariant headerData(int aSection, Qt::Orientation aOrientation, int aRole = Qt::DisplayRole ) const;
	void setPayments(QList<PaymentInfo> aPaymentInfoList);

signals:
	void updatePayments(const QString & message);
	void showProcessWindow(bool aShow, const QString & message);

public slots:
	void printReceipt(const QModelIndex & index);
	void printAllReceipts();
	void printFilteredReceipts(const QSet<qint64> & aPaymentsID);
	void proccessPayment(const QModelIndex & index);
	/// Провести все платежи
	void processAllPayments();
	/// Прервать проведение платежей
	void onClicked(const QVariantMap &);

private slots:
	void onReceiptPrinted(qint64 aPaymentId, bool aErrorHappened);
	void onUpdatePayment(qint64 aPaymentId);

	/// обрабатывает очередной платёж в очереди
	void proccessNextPayment();

private:
	PaymentManager * mPaymentManager;
	QList<PaymentInfo> mPaymentInfoList;
	QMap<qint64, int> mPaymentRowIndex;
	QSet<qint64> mPrintingQueue;
	
	/// очередь платежей на перепроводку
	struct ProcessPayments
	{
		QList<PaymentInfo> payments;
		int processed;

		ProcessPayments() : processed(0)
		{
		}
		
		void clear()
		{
			payments.clear();
			processed = 0;
		}
	} mProcessPayments;

	bool mFiscalMode;
	QHash<Column, QString> columnHeaders;
};

//---------------------------------------------------------------------------
class PaymentProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	enum PaymentFilter
	{
		AllPayments,
		PrintedPayments,
		ProcessedPayments
	};

	PaymentProxyModel(QObject * parent = 0);
	void showColumn(int aColumn, bool aShow);
	QVariantList getColumnVisibility() const;
	bool hiddenColumn(int aColumn) const;

public slots:
	void disableDateFilter();
	void setDateFilter(const QDateTime & aFrom, const QDateTime & aTo);
	void disablePaymentsFilter();
	void enablePrintedPaymentsFilter();
	void enableProcessedPaymentsFilter();
	
protected:
	bool filterAcceptsRow(int sourceRow, const QModelIndex & sourceParent) const;
	bool filterAcceptsColumn(int sourceColumn, const QModelIndex & sourceParent) const;

private:
	bool mDateFilterEnabled;
	QDateTime mStartDateTime;
	QDateTime mEndDateTime;
	PaymentFilter mPaymentFilter;
	QMap<int, bool> mColumns;
};

typedef QMap<int, bool> map_type;
Q_DECLARE_METATYPE(map_type)

//----------------------------------------------------------------------------