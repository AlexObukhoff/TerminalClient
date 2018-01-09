/* @file Модель-фильтр  списка провайдеров. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtGui/QSortFilterProxyModel>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

//------------------------------------------------------------------------------
class ProviderListFilter : public QSortFilterProxyModel
{
	Q_OBJECT

	Q_PROPERTY(bool empty READ getEmpty NOTIFY emptyChanged);
	Q_PROPERTY(QString filter READ getFilter WRITE setFilter);

public:
	ProviderListFilter(QObject * aParent);
	~ProviderListFilter();

	bool getEmpty() const;
	QString getFilter() const;
	void setFilter(const QString & aFilter);

protected:
	virtual bool filterAcceptsRow(int aSourceRow, const QModelIndex & aSourceParent) const;
	virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

protected:
	inline int calcSortIndex(const QString & aInfo) const;

public slots:
	QObject * get(int aIndex);

signals:
	void emptyChanged();

private:
	QString mFilter;
	QStringList mFilterLexemList;
};

//------------------------------------------------------------------------------
class ProviderObject : public QObject
{
	Q_OBJECT

	Q_PROPERTY(qint64 id READ getId);
	Q_PROPERTY(QString name READ getName);

public:
	ProviderObject(QObject * aParent, qint64 aId, const QString & aName) :
	  QObject(aParent), mId(aId), mName(aName)
	{
	}

public:
	qint64 getId() const
	{
		return mId;
	}

	QString getName() const
	{
		return mName;
	}

private:
	qint64 mId;
	QString mName;
};