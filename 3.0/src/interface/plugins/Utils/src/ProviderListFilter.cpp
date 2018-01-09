/* @file Модель-фильтр  списка провайдеров. */

// std
#include <limits>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QSet>
#include <Common/QtHeadersEnd.h>

// Project
#include "ProviderListModel.h"
#include "ProviderListFilter.h"
#include "ProviderConstants.h"

//------------------------------------------------------------------------------
ProviderListFilter::ProviderListFilter(QObject * aParent)
	: QSortFilterProxyModel(aParent)
{
	connect(this, SIGNAL(layoutChanged()), this, SIGNAL(emptyChanged()));
	connect(this, SIGNAL(modelReset()), this, SIGNAL(emptyChanged()));

	setDynamicSortFilter(false);
}

//------------------------------------------------------------------------------
ProviderListFilter::~ProviderListFilter()
{

}

//------------------------------------------------------------------------------
bool ProviderListFilter::filterAcceptsRow(int aSourceRow, const QModelIndex & /*aSourceParent*/) const
{
	if (mFilterLexemList.isEmpty())
	{
		return false;
	}

	QModelIndex index = sourceModel()->index(aSourceRow, 0);
	QString info = sourceModel()->data(index, ProviderListModel::InfoRole).value<QString>();

	foreach (auto lexem, mFilterLexemList)
	{
		if (!info.contains(lexem))
		{
			return false;
		}
	}

	return true;
}

//------------------------------------------------------------------------------
bool ProviderListFilter::lessThan(const QModelIndex & aLeft, const QModelIndex & aRight) const
{
	if (mFilterLexemList.isEmpty())
	{
		return false;
	}

	return calcSortIndex(sourceModel()->data(aLeft, ProviderListModel::InfoRole).value<QString>()) 
		<  calcSortIndex(sourceModel()->data(aRight, ProviderListModel::InfoRole).value<QString>());
}

//------------------------------------------------------------------------------
inline int ProviderListFilter::calcSortIndex(const QString & aInfo) const
{
	int index = std::numeric_limits<int>::max();

	foreach(auto lexem, mFilterLexemList)
	{
		int pos = aInfo.indexOf(lexem);

		if (pos >= 0) 
		{
			index = qMin(index, pos);
		}
	}

	return index;
}

//------------------------------------------------------------------------------
bool ProviderListFilter::getEmpty() const
{
	return rowCount() == 0;
}

//------------------------------------------------------------------------------
QString ProviderListFilter::getFilter() const
{
	return mFilter;
}

//------------------------------------------------------------------------------
void ProviderListFilter::setFilter(const QString & aFilter)
{
	static QRegExp spaceRegExp("\\s+");

	beginResetModel();

	mFilter = aFilter;
	mFilterLexemList = aFilter.toLower().replace(spaceRegExp, " ").split(" ", QString::SkipEmptyParts);

	endResetModel();

	sort(0);

	emit emptyChanged();
}

//------------------------------------------------------------------------------
QObject * ProviderListFilter::get(int aIndex)
{
	return new ProviderObject(this, 
		sourceModel()->data(mapToSource(index(aIndex, 0)), ProviderListModel::IdRole).value<qint64>(),
		sourceModel()->data(mapToSource(index(aIndex, 0)), ProviderListModel::NameRole).value<QString>());
}

//------------------------------------------------------------------------------
