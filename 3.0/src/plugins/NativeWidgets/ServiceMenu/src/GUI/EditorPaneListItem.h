/* @file Реализация компоненты для редактирования профилей устройств. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtWidgets/QStyledItemDelegate>
#include <QtWidgets/QListWidgetItem>
#include <Common/QtHeadersEnd.h>

//------------------------------------------------------------------------
class EditorPaneListItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public: 
	EditorPaneListItemDelegate(QObject * aParent = 0);
};

//------------------------------------------------------------------------
class EditorPaneListItem : public QListWidgetItem
{
public:
	enum DataRole
	{
		ParameterName  = Qt::UserRole + 1,
		ParameterValue
	};

	EditorPaneListItem() {}
	virtual ~EditorPaneListItem() {}

	virtual QVariant data(int aRole) const;
	virtual void setData(int aRole, const QVariant & aValue);

private:
	QString mValue;
	QString mName;
};

//---------------------------------------------------------------------------
