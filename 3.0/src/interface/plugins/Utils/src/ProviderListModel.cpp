/* @file Модель со списком провайдеров, хагруженных клиентом. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QSet>
#include <QtCore/QTimer>
#include <Common/QtHeadersEnd.h>

// Project
#include "ProviderListModel.h"
#include "ProviderConstants.h"
#include "GroupModel.h"

//------------------------------------------------------------------------------
ProviderListModel::ProviderListModel(QObject * aParent, QSharedPointer<GroupModel> aGroupModel) :
	QAbstractListModel(aParent),
	mGroupModel(aGroupModel)
{
	mRoles[IdRole] = "id";
	mRoles[NameRole] = "name";
	mRoles[InfoRole] = "info";
	mRoles[ImageRole] = "image";
}

//------------------------------------------------------------------------------
ProviderListModel::~ProviderListModel()
{

}

//------------------------------------------------------------------------------
QHash<int, QByteArray> ProviderListModel::roleNames() const
{
	return mRoles;
}

//------------------------------------------------------------------------------
int ProviderListModel::rowCount(const QModelIndex & /*parent = QModelIndex()*/) const
{
	return mProviderList.size();
}

//------------------------------------------------------------------------------
QVariant ProviderListModel::data(const QModelIndex & aIndex, int aRole) const
{
	if (aIndex.row() >= 0 && aIndex.row() < mProviderList.count())
	{
		switch(aRole)
		{
		case IdRole:
		case ImageRole:
			return mProviderList[aIndex.row()].id;
		case NameRole:
			return mProviderList[aIndex.row()].name;
		case InfoRole:
			return mProviderList[aIndex.row()].info;
		}
	}
	
	return QVariant();
}

//------------------------------------------------------------------------------
void ProviderListModel::groupsUpdated()
{
	if (!mProvidersId.empty() || !mProviderList.empty())
		return;

	if (mGroupModel)
	{
		auto providersId = mGroupModel->allProviders().toList();

		// Исключаем повторную загрузку и индексацию
		if (providersId.size() > mProviderList.size() + mProvidersId.size())
		{
			mProviderList.clear();
			mProvidersId = providersId;

			QMetaObject::invokeMethod(this, "getNextProviderInfo", Qt::QueuedConnection);
		}
	}
}

//------------------------------------------------------------------------------
void ProviderListModel::getNextProviderInfo()
{
	if (!mProvidersId.isEmpty())
	{
		SProvider provider(mProvidersId.first());

		if (provider.id > 0 && provider.id != Providers::AutodetectID)
		{
			QObject * providerObject = nullptr;

			if (QMetaObject::invokeMethod(mPaymentService.data(), "getProvider", Q_RETURN_ARG(QObject *, providerObject), Q_ARG(qint64, provider.id)) && providerObject)
			{
				// Проверка - если провайдера нет в operators
				if (providerObject->property("id").value<qint64>() == provider.id)
				{
					provider.name = providerObject->property("name").value<QString>();
					provider.info = provider.name.toLower(); // +  providerObject->property("comment").value<QString>().toLower();

					if (provider.info.size() < 256)
					{
						provider.info += QString().fill(' ', 256 - provider.info.size());
					}

					foreach (auto receiptString, providerObject->property("receiptParameters").value<QVariantMap>().values())
					{
						provider.info += receiptString.toString().toLower();
					}
				}
				else
				{
					// Если не смогли добыть полное описание провайдера берем его имя из модели групп
					provider.name = mGroupModel->getProviderName(provider.id);
					provider.info = provider.name.toLower();
				}

				providerObject->deleteLater();
			}

			emit beginInsertRows(QModelIndex(), mProviderList.size(), mProviderList.size());

			mProviderList << provider;

			emit endInsertRows();
		}

		mProvidersId.removeAll(provider.id);
	}

	if (!mProvidersId.isEmpty())
	{
		QMetaObject::invokeMethod(this, "getNextProviderInfo", Qt::QueuedConnection);
	}
}

//------------------------------------------------------------------------------
void ProviderListModel::setPaymentService(QObject * aPaymentService)
{
	mPaymentService = aPaymentService;
}
