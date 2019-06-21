/* @file Окно логов. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QFutureWatcher>
#include <QtCore/QStringListModel>
#include "ui_LogsServiceWindow.h"
#include <Common/QtHeadersEnd.h>

// Проект
#include "IServiceWindow.h"

//------------------------------------------------------------------------
class LogFileModel : public QStringListModel
{
	Q_OBJECT

	QStringList mList;

public:
	LogFileModel(){ }

	virtual void setStringList(const QStringList & aStrings)
	{
		mList = aStrings;
		QStringListModel::setStringList(mList);
	}

	virtual QVariant data(const QModelIndex & aIndex, int aRole) const
	{
		int row = aIndex.row();

		if (row >= 0 && row < mList.size())
		{
			switch (aRole)
			{
			case Qt::DisplayRole:
			case Qt::EditRole:
				return mList.at(row);

			case Qt::TextColorRole:
				{
					const QString & line = mList.at(row);

					if (line.size() > 14)
					{
						switch (line.at(14).toLatin1())
						{
						case 'W': return QColor(0xff, 0xaa, 0x00);
						case 'E':
						case 'C': return QColor(0xff, 0x00, 0x00);
						case 'D': return QColor(0xaa, 0xbb, 0xcc);
						default:  return QColor(0x00, 0x00, 0x00);
						}
					}
				}
			}
		}

		return QVariant();
	}
};

//------------------------------------------------------------------------
class LogsServiceWindow : public QFrame, public ServiceWindowBase, protected Ui::LogsServiceWindow
{
	Q_OBJECT

public:
	LogsServiceWindow(ServiceMenuBackend * aBackend, QWidget * aParent = 0);

public:
	virtual bool initialize();
	virtual bool shutdown();
	virtual bool activate();
	virtual bool deactivate();

private slots:
	/// Открыть лог файл
	void onShowLogButtonClicked();

	/// Изменился выделенный файл в списке логов
	void logsSelectionChanged();

	/// Скролирование содержимого журнального файла
	void onScrollUpClicked();
	void onScrollDownClicked();

	/// Закрыть журнальный файл
	void onCloseLogClicked();

	/// Скроллирование списка лог-файлов
	void onScrollUpLogListClicked();
	void onScrollDownLogListClicked();

private:
	template <class T>
	void scrollPgUp(T * aListWidget)
	{
		QModelIndex firstVisibleIndex;
		QPoint point(10, 0);

		do 
		{
			firstVisibleIndex = aListWidget->indexAt(point);
			point += QPoint(0, 10);
		} while(!firstVisibleIndex.isValid() && point.ry() < aListWidget->height());

		if (firstVisibleIndex.isValid())
		{
			aListWidget->setCurrentIndex(firstVisibleIndex);

			QCoreApplication::postEvent(aListWidget, new QKeyEvent(QEvent::KeyPress, Qt::Key_PageUp, Qt::NoModifier));
		}
	}

	template <class T>
	void scrollPgDown(T * aListWidget)
	{
		QModelIndex lastVisibleIndex;
		QPoint point(10, aListWidget->height());

		do 
		{
			lastVisibleIndex = aListWidget->indexAt(point);
			point -= QPoint(0, 10);
		} while(!lastVisibleIndex.isValid() && point.ry() > 0);
		
		if (lastVisibleIndex.isValid())
		{
			aListWidget->setCurrentIndex(lastVisibleIndex);

			QCoreApplication::postEvent(aListWidget, new QKeyEvent(QEvent::KeyPress, Qt::Key_PageDown, Qt::NoModifier));
		}
	}

private:
	QMap<QString, QString> mLogs;
	LogFileModel mModel;
};

//------------------------------------------------------------------------
