/* @file Модель для отображения списка провайдеров. */
#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QAbstractListModel>
#include <QtCore/QPointer>
#include <QtCore/QSharedPointer>
#include <QtCore/QStringList>
#include <QtCore/QReadWriteLock>
#include <QtXml/QDomDocument>
#include <Common/QtHeadersEnd.h>

class Item;
class ItemObject;

//------------------------------------------------------------------------------
class GroupModel : public QAbstractListModel
{
	Q_OBJECT

	Q_PROPERTY(int count READ rowCount NOTIFY rowCountChanged)
	Q_PROPERTY(QString source READ getSource WRITE setSource)
	Q_PROPERTY(qint64 category READ getCategory NOTIFY categoryChanged)
	Q_PROPERTY(QString categoryName READ getCategoryName NOTIFY categoryNameChanged)
	Q_PROPERTY(qint64 rootElement READ getRootElement WRITE setRootElement)
	Q_PROPERTY(QStringList elementFilter READ getElementFilter WRITE setElementFilter)

	enum EntryRoles {
		IdRole = Qt::UserRole + 1,
		NameRole,
		TitleRole,
		DescriptionRole,
		TypeRole,
		ImageRole,
		IsGroupRole,
		JSONRole
	};

public:
	GroupModel();

	virtual int rowCount(const QModelIndex & aParent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex & aIndex, int aRole) const;

	/// Получить ID всех провайдеров
	QSet<qint64> allProviders() const;

	/// Получить имя провайдера
	QString getProviderName(qint64 aProviderId) const;

	/// Загрузить в модель данные статистики
	void setStatistic(QMap<qint64, quint32> & aStatistic);

public slots:
	QObject * get(int aIndex);

	int getMaxNameLength() const;

	qint64 getCategory() const;
	QString getCategoryName() const;
	qint64 findCategory(qint64 aProviderId) const;
	bool isProviderInCategory(qint64 aProvider, qint64 aCategory) const;

	QString getSource() const;
	void setSource(QString aSource);

	qint64 getRootElement() const;
	void setRootElement(qint64 aRootElement);

	QStringList getElementFilter();
	void setElementFilter(QStringList aFilter);

signals:
	void rowCountChanged();
	void categoryChanged();
	void categoryNameChanged();

public:
	typedef QSharedPointer<Item> ItemPtr;
	typedef QList<ItemPtr> ItemList;

private:
	bool loadContent(const QString & mFileName, QDomDocument & aDocument);
	
	/// Слияние двух groups
	void mergeGroups(QDomElement aTargetGroup, QDomElement aSourceGroup);

	void setRootElementInternal(qint64 aRootElement);

	/// Высчитываем максимальную индекс сортировки для содержимого группы
	quint32 getGroupOrder(qint64 aGroupID);

	qint64 getCategory(QDomNode aNode);

	void clearNodes();

	const ItemList & getItemList(qint64 aGroupID);

private:
	/// Имя xml файла с списком групп
	QString mSource;

	/// Весь документ
	QDomDocument mDocument;

	/// Список групп по их идентификаторам
	QHash<qint64, QDomNode> mGroups;
	
	/// Соответствие категории для каждой группы
	QHash<qint64, qint64> mCategories;

	/// Соответствие категории для каждого провайдера
	QHash<qint64, qint64> mProviderCategorys;

	/// Фильтр имён тегов xml
	QStringList mElementFilter;

	/// Id текущей корневой группы
	qint64 mRootElement;
	
	/// Id текущей категории
	qint64 mCurrentCategory;

	/// Список узлов внутри текущей корневой группы
	ItemList mNodes;
	QMap<qint64, ItemList> mNodesCache;
	QList<QPointer<ItemObject>> mNodesObject;

	/// Данные для сортировки кнопок, полученные по статистике платежей
	QMap<qint64, quint32> mProvidersStatistic;

	/// Группа - количество столбцов
	QMap<qint64, qint32> mGroupsWidth;
};

//------------------------------------------------------------------------------
class Item
{
public:
	Item(const QDomNode & aNode);

	virtual qint64 getId() const;
	virtual QString getName() const;
	virtual QString getTitle() const;
	virtual QString getDescription() const;
	virtual QString getType() const;
	virtual QString getImage() const;
	virtual QString getJSON() const;
	virtual bool isGroup() const;

	virtual QString getElementName() const;
	bool is(const QString & aElementName) const;

	virtual void setOrder(quint32 aOrder);
	virtual quint32 getOrder() const;

protected:
	QDomNamedNodeMap mAttributes;
	bool mIsGroup;
	QString mElementName;
	quint32 mOrder;
};

//------------------------------------------------------------------------------
class ItemObject : public QObject
{
	Q_OBJECT

	Q_PROPERTY(qint64 id READ getId)
	Q_PROPERTY(QString name READ getName)
	Q_PROPERTY(QString title READ getTitle)
	Q_PROPERTY(QString description READ getDescription)
	Q_PROPERTY(QString type READ getType)
	Q_PROPERTY(QString image READ getImage)
	Q_PROPERTY(QString json READ getJSON)
	Q_PROPERTY(bool isGroup READ isGroup)

public:
	ItemObject(const Item & aItem, QObject * aParent);

	qint64 getId() const;
	QString getName() const;
	QString getTitle() const;
	QString getDescription() const;
	QString getType() const;
	QString getImage() const;
	QString getJSON() const;
	bool isGroup() const;

private:
	const Item & mItem;
};

//------------------------------------------------------------------------------
