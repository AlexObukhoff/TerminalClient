/* @file Настройки дилера: операторы, комиссии, персональная инфа и т.п. */

#pragma once

// Boost
#pragma push_macro("foreach")
#undef foreach
#include <boost/noncopyable.hpp>
#pragma pop_macro("foreach")

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QReadWriteLock>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/ILogable.h>
#include <Common/PropertyTree.h>

// SDK
#include <SDK/PaymentProcessor/Settings/Range.h>
#include <SDK/PaymentProcessor/Settings/Provider.h>
#include <SDK/PaymentProcessor/Settings/ISettingsAdapter.h>

// Project
#include "Commissions.h"

class ILog;

namespace SDK {
namespace PaymentProcessor {

//---------------------------------------------------------------------------
/// Структура с персональными данными дилера.
struct SPersonalSettings
{
	bool isValid()
	{
		return !name.isEmpty() || !inn.isEmpty();
	}

	QString pointName;
	QString pointAddress;
	QString pointExternalID;
	QString name;
	QString address;
	QString businessAddress;
	QString inn;
	QString kbk;
	QString phone;
	QString isBank;

	QString operatorName;
	QString operatorAddress;
	QString operatorInn;
	QString operatorPhone;
	QString operatorContractNumber;

	QString bankName;
	QString bankAddress;
	QString bankBik;
	QString bankInn;
	QString bankPhone;
	QString bankContractNumber;

	QMap<QString, QString> mPrintingParameters;
};

//---------------------------------------------------------------------------
class DealerSettings : public ISettingsAdapter, public ILogable, private boost::noncopyable
{
	struct SCustomer
	{
		bool blocked;
		QSet<QString> values;
		Commissions commissions;

		void addValue(const QString & aValue) { if (!aValue.isEmpty()) { values.insert(aValue); } }
		bool isEmpty() const { return values.isEmpty(); }
		bool contains(QSet<QString> aValues) const { return !aValues.intersect(values).isEmpty(); }
	};

public:
	typedef QMap<qint64, SProvider> TProviderList;

	DealerSettings(TPtree & aProperties);
	virtual ~DealerSettings();

	/// Валидация загруженных данных.
	virtual bool isValid() const;

	/// Получить имя адаптера.
	static QString getAdapterName();

	/// Инициализация настроек.
	void initialize();

	/// Возвращает персональные данные дилера.
	const SPersonalSettings & getPersonalSettings() const;

	/// Возвращает оператора по идентификатору.
	SProvider getProvider(qint64 aId);

	/// Возвращает оператора по идентификатору.
	SProvider getMNPProvider(qint64 aId, qint64 aCidIn, qint64 aCidOut);

	/// Возвращает оператора по номеру шлюза.
	QList<SProvider> getProvidersByCID(qint64 aCid);

	/// Возвращает оператора(ов) для данного диапазона(ов).
	QList<SProvider> getProvidersByRange(QList<SRange> aRanges, QSet<qint64> aExclude = QSet<qint64>());

	/// Возвращает список провайдеров по конкретному типу процессинга.
	const QList<qint64> getProviders(const QString & aProcessingType);

	/// Возвращает список типов процессинга 
	QStringList getProviderProcessingTypes();

	void setExternalLimits(qint64 aProviderId, double aMinExternalLimit, double aMaxExternalLimit);

	/// Возвращает false, если реквизиты платежа в чёрном списке.
	bool isCustomerAllowed(const QVariantMap & aParameters);

	/// Возвращает список комиссий по номеру оператора и заполненным полям.
	TCommissions getCommissions(qint64 aProvider, const QVariantMap & aParameters);

	/// Возвращает объект для подсчёта комиссии по оператору, заполненным полям и сумме платежа.
	Commission getCommission(qint64 aProvider, const QVariantMap & aParameters, double aSum);

	/// Получение комиссии процессинга за платёж по указанному оператору. Используется для разбивки комиссии
	/// на платёжном чеке. Все подсчёты производятся только методом getCommission.
	ProcessingCommission getProcessingCommission(qint64 aProvider);

	/// Получить размер НДС для провайдера 
	int getVAT(qint64 aProvider);

	/// Удаляет провайдера из списка
	void disableProvider(qint64 aId);

private:
	typedef QList<SCustomer> TCustomers;

private:
	/// Загружает список операторов.
	bool loadProviders();

	/// Загружает список операторов из xml файла.
	bool loadOperatorsXML(const QString & aFileName);

	/// Загружает оператора из буфера
	bool loadProvidersFromBuffer(const std::string & aBuffer, SProvider & aProvider);

	/// Загружает комисии.
	bool loadCommissions();

	/// Загружает пресональные данные дилера.
	bool loadPersonalSettings();

	/// Предикат для поиска в списке клиентов.
	TCustomers::iterator findCustomer(const QVariantMap & aParameters);

	/// Перезаписать настройки комиссии 
	void setExternalCommissions(const Commissions & aCommissions);

	/// Сбросить настройки комиссий до начальных
	void resetExternalCommissions();

private:
	TPtree & mProperties;

	QReadWriteLock mProvidersLock;
	QMap<qint64, std::string> mProviderRawBuffer;
	TProviderList mProviders;
	
	QMultiMap<qint64, qint64> mProviderGeteways;
	QMultiMap<QString, qint64> mProvidersProcessingIndex;
	SPersonalSettings mPersonalSettings;

	Commissions mCommissions;
	Commissions mExternalCommissions;

	/// Чёрно-белый список клиентов.
	TCustomers mCustomers;

	/// Флаг состояния.
	bool mIsValid;
};

//---------------------------------------------------------------------------
}} // SDK::PaymentProcessor

//---------------------------------------------------------------------------
