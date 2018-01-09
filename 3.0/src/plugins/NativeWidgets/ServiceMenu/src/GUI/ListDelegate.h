/* @file Делегат отображения элемента с комментарием в QListWidget */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtGui/QPainter>
#include <QtGui/QItemDelegate>
#include <Common/QtHeadersEnd.h>

//---------------------------------------------------------------------------
class ListDelegate : public QItemDelegate
{
public:
	ListDelegate(QObject * aParent = 0) 
	{
		Q_UNUSED(aParent);
	}

	virtual void paint (QPainter * aPainter, const QStyleOptionViewItem & aOption, const QModelIndex & aIndex) const
	{
		if (aOption.state & QStyle::State_Selected)
		{
			aPainter->fillRect(aOption.rect, aOption.palette.color(QPalette::Highlight));
		}

		QString title = aIndex.data(Qt::DisplayRole).toString();
		QString description = aIndex.data(Qt::UserRole + 1).toString();

		QRect r = aOption.rect;
		aPainter->drawText(r.left(), r.top(), r.width(), r.height(), Qt::AlignLeft | Qt::AlignTop, title, &r);

		if (!description.isEmpty())
		{
			QFont font;
			QFont oldFont = aPainter->font();

			font = oldFont;
			font.setPointSize(oldFont.pointSize() - 3);

			aPainter->setFont(font);
			r = aOption.rect.adjusted(10, 0, 0, 0);
			aPainter->drawText(r.left(), r.top(), r.width(), r.height(), Qt::AlignLeft | Qt::AlignBottom, description, &r);

			aPainter->setFont(oldFont);
		}
	}
	
	virtual QSize sizeHint(const QStyleOptionViewItem & aOption, const QModelIndex & aIndex) const
	{
		QString description = aIndex.data(Qt::UserRole + 1).toString();
		QSize size = QItemDelegate::sizeHint(aOption, aIndex);

		if (!description.isEmpty())
		{
			size.setHeight(static_cast<int>(size.height() * 1.6));
		}

		return size;
	}
};

//---------------------------------------------------------------------------
