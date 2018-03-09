/* @file Модель со списком провайдеров, хагруженных клиентом. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QAbstractListModel>
#include <QtCore/QPointer>
#include <QtCore/QSharedPointer>
#include <Common/QtHeadersEnd.h>

//------------------------------------------------------------------------------
class GroupModel;

//------------------------------------------------------------------------------
struct SProvider
{
	qint64 id;
	QString name;
	QString info;

	SProvider() { id = -1; }
	explicit SProvider(quint64 aId) : id(aId) { }

	bool isValid() const { return id != -1; }

	bool operator<(const SProvider & aProvider) const
	{
		return info < aProvider.info;
	}
};

//------------------------------------------------------------------------------
class ProviderListModel : public QAbstractListModel
{
	Q_OBJECT

public:
	enum EntryRoles {
		IdRole = Qt::UserRole + 1,
		NameRole,
		InfoRole,
		ImageRole
	};

public:
	ProviderListModel(QObject * aParent, QSharedPointer<GroupModel> aGroupModel);
	~ProviderListModel();

	virtual int rowCount(const QModelIndex & aParent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex & aIndex, int aRole = Qt::DisplayRole) const;

	/// Передать модели указатель на сервис платежей
	void setPaymentService(QObject * aPaymentService);

public slots:
	/// Обрабатывает сигнал об загрузке списка провайдеров в модель групп
	void groupsUpdated();

	/// Загрузка очередного провайдера
	void getNextProviderInfo();

private:
	QHash<int, QByteArray> mRoles;
	virtual QHash<int, QByteArray> roleNames() const;

private:
	QList<qint64> mProvidersId;
	QList<SProvider> mProviderList;
	QPointer<QObject> mPaymentService;
	QSharedPointer<GroupModel> mGroupModel;
};

