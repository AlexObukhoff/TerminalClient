/* @file Модель для отображения списка провайдеров. */
#pragma once

// stl
#include <algorithm>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QHash>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QTextCodec>
#include <QtCore/QScopedPointer>
#include <QtCore/QSettings>
#include <Common/QtHeadersEnd.h>

// Project
#include "Log.h"
#include "GroupModel.h"
#include "ProviderConstants.h"

namespace CGroupModel
{
	const QString Group = "group";
	const QString Operator = "operator";
	const QString GroupLink = "group_link";

	const QString RootGroupType = "root";

	namespace Attributes
	{
		const char Id[] = "id";
		const char ExtId[] = "ext_id";
		const char Name[] = "name";
		const char Title[] = "title";
		const char Description[] = "descr";
		const char Type[] = "type";
		const char Image[] = "image";
		const char IsGroup[] = "isGroup";
		const char JSON[] = "json";
	}
}

//------------------------------------------------------------------------------
GroupModel::GroupModel() :
mRootElement(-1),
mCurrentCategory(0)
{
	mRoles[IdRole] = CGroupModel::Attributes::Id;
	mRoles[NameRole] = CGroupModel::Attributes::Name;
	mRoles[TitleRole] = CGroupModel::Attributes::Title;
	mRoles[DescriptionRole] = CGroupModel::Attributes::Description;
	mRoles[TypeRole] = CGroupModel::Attributes::Type;
	mRoles[ImageRole] = CGroupModel::Attributes::Image;
	mRoles[IsGroupRole] = CGroupModel::Attributes::IsGroup;
	mRoles[JSONRole] = CGroupModel::Attributes::JSON;
}

//------------------------------------------------------------------------------
QHash<int, QByteArray> GroupModel::roleNames() const
{
	return mRoles;
}

//------------------------------------------------------------------------------
QObject * GroupModel::get(int aIndex)
{
	if (mNodes.isEmpty() || aIndex < 0)
	{
		return nullptr;
	}

	ItemObject * iObject = new ItemObject(*mNodes.at(aIndex), this);

	mNodesObject << iObject;

	return iObject;
}

//------------------------------------------------------------------------------
int GroupModel::getMaxNameLength() const
{
	int result = 0;

	// Настройками можно выставить  ширину группы принудительно
	if (mGroupsWidth.contains(mRootElement))
	{
		// Ширина одной колонки 60 символов, максимальная ширина - 240 символов
		return (int)(241.f / mGroupsWidth.value(mRootElement));
	}

	foreach(auto item, mNodes)
	{
		int length = item->getName().length();
		result = result > length ? result : length;
	}

	return result;
}

//------------------------------------------------------------------------------
int GroupModel::rowCount(const QModelIndex &) const
{
	return mNodes.count();
}

//------------------------------------------------------------------------------
QString GroupModel::getSource() const
{
	return mSource;
}

//------------------------------------------------------------------------------
bool GroupModel::loadContent(const QString & aFileName, QDomDocument & aDocument)
{
	aDocument.clear();

	QFileInfo fileInfo(aFileName);

	// Загрузка или догрузка контента в дерево групп
	auto loadXml = [this](const QString & aFileName, QDomDocument & aDocument) -> bool
	{
		QFile file(aFileName);
		if (!file.open(QIODevice::ReadOnly))
		{
			Log(Log::Error) << QString("GroupModel: Error open file %1.").arg(aFileName);
			return false;
		}

		QByteArray sourceContent = file.readAll();
		file.close();

		QString errorMessage;
		int line, column;
		if (!aDocument.setContent(sourceContent, &errorMessage, &line, &column))
		{
			Log(Log::Error) << QString("GroupModel: %1 in %2:%3").arg(errorMessage).arg(line).arg(column);
			return false;
		}

		// Загружаем настройки размеров для групп
		{
			QSettings ini(QString(aFileName).replace(".xml", ".ini"), QSettings::IniFormat);
			ini.setIniCodec("UTF-8");
			ini.beginGroup("columns");

			foreach(QString key, ini.allKeys())
			{
				mGroupsWidth.insert(key.toLongLong(), ini.value(key).toInt());
			}
		}

		return true;
	};

	// Грузим для начала основной groups.xml
	if (!loadXml(aFileName, aDocument))
	{
		return false;
	}

	mSource = aFileName;

	// Затем грузим все остальные пользовательские группы и встраиваем их в текущее дерево хитрой функцией
	QStringList filters;
	filters << fileInfo.fileName().replace(".xml", "*.xml", Qt::CaseInsensitive);

	foreach(auto file, fileInfo.dir().entryInfoList(filters, QDir::Files, QDir::Name))
	{
		QDomDocument localDoc;

		if (!file.fileName().compare(fileInfo.fileName(), Qt::CaseInsensitive))
		{
			continue;
		}

		if (loadXml(file.filePath(), localDoc))
		{
			mergeGroups(aDocument.elementsByTagName("groups").at(0).toElement(), localDoc.elementsByTagName("groups").at(0).toElement());
		}
		else
		{
			return false;
		}
	}

	return true;
}

//------------------------------------------------------------------------------
void GroupModel::mergeGroups(QDomElement aTargetGroup, QDomElement aSourceGroup)
{
	//auto getAttr = [](const QDomElement & aElement, )
	auto ID = [](QDomElement aElement) -> qint64 {
		return aElement.attribute("id", "0").toLongLong();
	};

	auto findGroup = [&](const QDomElement & aElement, qint64 aID, QDomElement & aGroupElement) -> bool {
		for (QDomNode n = aElement.firstChild(); !n.isNull(); n = n.nextSibling())
		{
			if (n.nodeName() == "group")
			{
				QDomElement element = n.toElement();
				if (ID(element) == aID)
				{
					aGroupElement = element;
					return true;
				}
			}
		}
		return false;
	};

	auto copyAttr = [](const QDomElement & aSource, QDomElement & aDestination) {
		for (int i = 0; i < aSource.attributes().size(); i++)
		{
			QDomAttr attr = aSource.attributes().item(i).toAttr();
			aDestination.setAttribute(attr.name(), attr.value());
		}
	};

	QDomElement element = aSourceGroup.lastChildElement();
	while (!element.isNull())
	{
		if (element.nodeName() == "group")
		{
			QDomElement target;
			if (findGroup(aTargetGroup, ID(element), target))
			{
				copyAttr(element, target);
				mergeGroups(target, element);
				element = element.previousSiblingElement();
			}
			else
			{
				// вставляем в начало списка
				auto nextElement = element.previousSiblingElement();
				aTargetGroup.insertBefore(element, QDomNode());
				element = nextElement;
			}
		}
		else
		{
			// вставляем в начало списка
			auto nextElement = element.previousSiblingElement();
			aTargetGroup.insertBefore(element, QDomNode());
			element = nextElement;
		}
	}
}

//------------------------------------------------------------------------------
void GroupModel::setSource(QString aSource)
{
	if (mSource == aSource)
	{
		return;
	}

	if (!loadContent(aSource, mDocument))
	{
		return;
	}

	emit beginResetModel();

	mGroups.clear();

	// Корневая группа
	mGroups[0] = mDocument.documentElement();

	QDomNodeList groups = mDocument.elementsByTagName(CGroupModel::Group);
	for (int i = 0; i < groups.count(); i++)
	{
		Item item(groups.at(i));

		qint64 id = item.getId();

		mGroups[id] = groups.at(i);
		mCategories[id] = getCategory(groups.at(i));
	}

	// Заполняем категории для каждого провайдера
	mProviderCategorys[Providers::AutodetectID] = 101;

	QDomNodeList providers = mDocument.elementsByTagName(CGroupModel::Operator);
	for (int i = 0; i < providers.count(); i++)
	{
		Item item(providers.at(i));

		qint64 id = item.getId();

		qint64 category = getCategory(providers.at(i));

		if (category)
		{
			mProviderCategorys.insert(id, category);
		}
	}

	setRootElementInternal(0);

	emit endResetModel();
}

//------------------------------------------------------------------------------
QStringList GroupModel::getElementFilter()
{
	return mElementFilter;
}

//------------------------------------------------------------------------------
void GroupModel::setElementFilter(QStringList aFilter)
{
	mElementFilter = aFilter;

	emit beginResetModel();

	setRootElementInternal(mRootElement);

	emit endResetModel();
}

//------------------------------------------------------------------------------
qint64 GroupModel::getCategory(QDomNode aNode)
{
	if (aNode.isNull())
	{
		// Не нашли вышестоящей корневой группы
		return 0;
	}

	Item i(aNode);

	if (i.is(CGroupModel::Group) && i.getType() == CGroupModel::RootGroupType)
	{
		return i.getId();
	}

	return getCategory(aNode.parentNode());
}

//------------------------------------------------------------------------------
qint64 GroupModel::getRootElement() const
{
	return mRootElement;
}

//------------------------------------------------------------------------------
void GroupModel::clearNodes()
{
	while (!mNodesObject.isEmpty())
	{
		auto item = mNodesObject.takeFirst();
		if (item)
		{
			item->deleteLater();
		}
	}

	mNodes.clear();
}

//------------------------------------------------------------------------------
const GroupModel::ItemList & GroupModel::getItemList(qint64 aGroupID)
{
	if (mNodesCache.contains(aGroupID))
	{
		return mNodesCache[aGroupID];
	}

	ItemList result;
	QDomNodeList nodes = mGroups[aGroupID].childNodes();

	for (int i = 0; i < nodes.count(); i++)
	{
		result << QSharedPointer<Item>(new Item(nodes.at(i)));
	}

	mNodesCache.insert(aGroupID, result);

	// заполняем order сразу и больше не трогаем
	if (!mProvidersStatistic.isEmpty())
	{
		for (int i = 0; i < mNodesCache[aGroupID].size(); i++)
		{
			ItemPtr & item = mNodesCache[aGroupID][i];

			if (item->is(CGroupModel::Operator))
			{
				item->setOrder(mProvidersStatistic.value(item->getId(), 0));
			}
			else if (item->is(CGroupModel::Group) || item->is(CGroupModel::GroupLink))
			{
				item->setOrder(getGroupOrder(item->getId()));
			}
		}
	}

	return mNodesCache[aGroupID];
}

//------------------------------------------------------------------------------
void GroupModel::setRootElementInternal(qint64 aRootElement)
{
	clearNodes();

	QStringList filter = mElementFilter;

	if (filter.isEmpty())
	{
		if (!aRootElement)
		{
			filter << CGroupModel::Operator << CGroupModel::GroupLink;
		}
		else
		{
			// перечислим все валидные теги
			filter << CGroupModel::Operator << CGroupModel::Group << CGroupModel::GroupLink;
		}
	}

	int currentCount = mNodes.count();

	foreach(auto item, getItemList(aRootElement))
	{
		if (filter.contains(item->getElementName()))
		{
			mNodes.push_back(item);
		}
	}

	if (aRootElement && !mProvidersStatistic.isEmpty())
	{
		qStableSort(mNodes.begin(), mNodes.end(), [](const ItemPtr & aItemA, const ItemPtr & aItemB) -> bool { return aItemA->getOrder() > aItemB->getOrder(); });
	}

	if (mNodes.count() > currentCount)
	{
		emit rowCountChanged();
	}

	mRootElement = aRootElement;
}

//------------------------------------------------------------------------------
quint32 GroupModel::getGroupOrder(qint64 aGroupID)
{
	auto lessOrder = [](const ItemPtr & aItemA, const ItemPtr & aItemB) -> bool {
		return aItemA->getOrder() < aItemB->getOrder(); };

	auto items = getItemList(aGroupID);

	auto it = std::max_element(items.begin(), items.end(), lessOrder);

	return it != items.end() ? it->data()->getOrder() : 0;
}

//------------------------------------------------------------------------------
void GroupModel::setRootElement(qint64 aRootElement)
{
	if (aRootElement != mRootElement && mGroups.contains(aRootElement))
	{
		emit beginResetModel();

		setRootElementInternal(aRootElement);

		emit endResetModel();

		if (mGroups.contains(aRootElement) && mCurrentCategory != mCategories[aRootElement])
		{
			mCurrentCategory = mCategories[aRootElement];
			emit categoryChanged();
		}
	}
}

//------------------------------------------------------------------------------
qint64 GroupModel::getCategory() const
{
	return mCurrentCategory;
}

//------------------------------------------------------------------------------
QString GroupModel::getCategoryName() const
{
	return mGroups[mCurrentCategory].toElement().attribute("name");
}

//------------------------------------------------------------------------------
QVariant GroupModel::data(const QModelIndex & aIndex, int aRole) const
{
	if (aIndex.row() >= 0 && aIndex.row() < mNodes.count())
	{
		switch (aRole)
		{
		case IdRole:
			return mNodes[aIndex.row()]->getId();
		case NameRole:
			return mNodes[aIndex.row()]->getName();
		case TitleRole:
			return mNodes[aIndex.row()]->getTitle();
		case DescriptionRole:
			return mNodes[aIndex.row()]->getDescription();
		case TypeRole:
			return mNodes[aIndex.row()]->getType();
		case ImageRole:
			return mNodes[aIndex.row()]->getImage();
		case IsGroupRole:
			return mNodes[aIndex.row()]->isGroup();
		case JSONRole:
			return mNodes[aIndex.row()]->getJSON();
		}
	}

	return QVariant();
}

//------------------------------------------------------------------------------
qint64 GroupModel::findCategory(qint64 aProviderId) const
{
	if (mProviderCategorys.contains(aProviderId))
	{
		return mProviderCategorys[aProviderId];
	}

	return 0;
}

//------------------------------------------------------------------------------
bool GroupModel::isProviderInCategory(qint64 aProvider, qint64 aCategory) const
{
	return aCategory == findCategory(aProvider);
}

//------------------------------------------------------------------------------
QSet<qint64> GroupModel::allProviders() const
{
	return mProviderCategorys.keys().toSet();
}

//------------------------------------------------------------------------------
QString GroupModel::getProviderName(qint64 aProviderId) const
{
	QDomNodeList providers = mDocument.elementsByTagName(CGroupModel::Operator);

	for (int i = 0; i < providers.count(); i++)
	{
		Item item(providers.at(i));

		if (item.getId() == aProviderId)
		{
			return item.getName();
		}
	}

	return QString();
}

//------------------------------------------------------------------------------
void GroupModel::setStatistic(QMap<qint64, quint32> & aStatistic)
{
	mProvidersStatistic.swap(aStatistic);
}

//------------------------------------------------------------------------------
Item::Item(const QDomNode & aNode) :
mAttributes(aNode.attributes()),
mIsGroup(aNode.nodeName().contains(CGroupModel::Group, Qt::CaseInsensitive) == true),
mElementName(aNode.nodeName().toLower()),
mOrder(0)
{
}

//------------------------------------------------------------------------------
QString Item::getElementName() const
{
	return mElementName;
}

//------------------------------------------------------------------------------
qint64 Item::getId() const
{
	qint64 id = mAttributes.namedItem(CGroupModel::Attributes::Id).nodeValue().toLongLong();
	qint64 extId = mAttributes.namedItem(CGroupModel::Attributes::ExtId).nodeValue().toLongLong();

	return extId ? extId : id;
}

//------------------------------------------------------------------------------
QString Item::getName() const
{
	return mAttributes.namedItem(CGroupModel::Attributes::Name).nodeValue();
}

//------------------------------------------------------------------------------
QString Item::getTitle() const
{
	return mAttributes.namedItem(CGroupModel::Attributes::Title).nodeValue();
}

//------------------------------------------------------------------------------
QString Item::getDescription() const
{
	return mAttributes.namedItem(CGroupModel::Attributes::Description).nodeValue();
}

//------------------------------------------------------------------------------
QString Item::getType() const
{
	return mAttributes.namedItem(CGroupModel::Attributes::Type).nodeValue().toLower();
}

//------------------------------------------------------------------------------
QString Item::getImage() const
{
	return mAttributes.namedItem(CGroupModel::Attributes::Image).nodeValue();
}

//------------------------------------------------------------------------------
bool Item::isGroup() const
{
	return mIsGroup;
}

//------------------------------------------------------------------------------
QString Item::getJSON() const
{
	return mAttributes.namedItem(CGroupModel::Attributes::JSON).nodeValue();
}

//------------------------------------------------------------------------------
void Item::setOrder(quint32 aOrder)
{
	mOrder = aOrder;
}

//------------------------------------------------------------------------------
quint32 Item::getOrder() const
{
	return mOrder;
}

//------------------------------------------------------------------------------
bool Item::is(const QString & aElementName) const
{
	return mElementName == aElementName;
}

//------------------------------------------------------------------------------
ItemObject::ItemObject(const Item & aItem, QObject * aParent) : QObject(aParent), mItem(aItem)
{
}

//------------------------------------------------------------------------------
qint64 ItemObject::getId() const
{
	return mItem.getId();
}

//------------------------------------------------------------------------------
QString ItemObject::getName() const
{
	return mItem.getName();
}

//------------------------------------------------------------------------------
QString ItemObject::getTitle() const
{
	return mItem.getTitle();
}

//------------------------------------------------------------------------------
QString ItemObject::getDescription() const
{
	return mItem.getDescription();
}

//------------------------------------------------------------------------------
QString ItemObject::getType() const
{
	return mItem.getType();
}

//------------------------------------------------------------------------------
QString ItemObject::getImage() const
{
	return mItem.getImage();
}

//------------------------------------------------------------------------------
QString ItemObject::getJSON() const
{
	return mItem.getJSON();
}

//------------------------------------------------------------------------------
bool ItemObject::isGroup() const
{
	return mItem.isGroup();
}

//------------------------------------------------------------------------------
