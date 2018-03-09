/* @file Реализация компоненты для редактирования профилей устройств. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtGui/QPainter>
#include <QtWidgets/QApplication>
#include <Common/QtHeadersEnd.h>

// Проект
#include "EditorPaneListItem.h"

//------------------------------------------------------------------------
EditorPaneListItemDelegate::EditorPaneListItemDelegate(QObject * aParent)
	: QStyledItemDelegate(aParent)
{
}

//------------------------------------------------------------------------
QVariant EditorPaneListItem::data(int aRole) const
{
	if (aRole == Qt::SizeHintRole)
	{
		return QSize(0, 40);
	}
	else if (aRole == ParameterName)
	{
		return mName;
	}
	else if (aRole == ParameterValue)
	{
		return mValue;
	}
	else if (aRole == Qt::DisplayRole)
	{
		return mName + ":\n" + mValue;
	}
	else
	{
		return QListWidgetItem::data(aRole);
	}
}

//------------------------------------------------------------------------
void EditorPaneListItem::setData(int aRole, const QVariant & aValue)
{
	if (aRole == ParameterName)
	{
		mName = aValue.toString();
	}
	else if (aRole == ParameterValue)
	{
		mValue = aValue.toString();
	}
	else
	{
		QListWidgetItem::setData(aRole, aValue);
	}
}

//------------------------------------------------------------------------
