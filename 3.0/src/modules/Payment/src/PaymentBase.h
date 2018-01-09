/* @file Базовый класс для класса реализующего платёж. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QReadWriteLock>
#include <Common/QtHeadersEnd.h>

// Boost
#pragma push_macro("foreach")
#undef foreach
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#pragma pop_macro("foreach")

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>
#include <SDK/PaymentProcessor/Settings/Provider.h>
#include <SDK/PaymentProcessor/Payment/IPayment.h>

//------------------------------------------------------------------------------
namespace CPayment
{
	namespace Requests
	{
		const char FakeCheck[] = "FAKE_CHECK";
		const char Check[] = "CHECK";
		const char Pay[] = "PAYMENT";
		const char Status[] = "STATUS";
		const char GetStep[] = "GETSTEP";
	}

	namespace Steps
	{
		const int Init = 1;
		const int Pay = 2;
	}
}

//------------------------------------------------------------------------------
class PaymentBase : 
	public SDK::PaymentProcessor::IPayment,
	public ILogable
{
	struct SParameterModifier
	{
		SParameterModifier(const SParameter & aModifiedValue)
			: modifiedValue(aModifiedValue)
		{
		}

		void operator()(SParameter & aParameter)
		{
			aParameter = modifiedValue;
		}

	private:
		SParameter modifiedValue;
	};

public:
	PaymentBase(SDK::PaymentProcessor::IPaymentFactory * aFactory, SDK::PaymentProcessor::ICore * aCore);

#pragma region SDK::PaymentProcessor::IPayment interface

	/// Если предикат возвращает true, то платёж не сформирован должным образом и может быть удалён.
	virtual bool isNull() const;

	/// Возвращает фабрику, которой был создан платёж.
	virtual SDK::PaymentProcessor::IPaymentFactory * getFactory() const;

	/// Установка параметра в платёж.
	virtual void setParameter(const SParameter & aParameter);

	/// Получение параметра по имени.
	virtual SParameter getParameter(const QString & aName) const;

	/// Получение всех параметров платежа.
	virtual QList<SParameter> getParameters() const;

	/// Вычислить размер комиссии, без записи результата в параметры платежа
	virtual QList<SParameter> calculateCommission(const QList<SParameter> & aParameters);

	/// Возвращает список параметров, достаточных для последющего восстановления платежа.
	virtual QList<SParameter> serialize() const;

	/// Восстановление платежа из списка параметров.
	virtual bool restore(const QList<SParameter> & aParameters);

	/// Идентификатор платежа.
	virtual qint64 getID() const;

	/// Тип платежа.
	virtual QString getType() const;

	/// Возвращает активную сессию платежа.
	virtual QString getSession() const;

	/// Возвращает начальную сессию платежа.
	virtual QString getInitialSession() const;

	/// Провайдер платежа.
	virtual qint64 getProvider(bool aMNP) const;

	/// Дата создания платежа.
	virtual QDateTime getCreationDate() const;

	/// Дата завершения формирования платежа.
	virtual QDateTime getCompleteDate() const;

	/// Дата последнего изменения платежа.
	virtual QDateTime getLastUpdateDate() const;

	/// Статус платежа.
	virtual int getStatus() const;

	/// Устанавливает платежу статус aStatus.
	virtual void setStatus(int aStatus);

	///	Возвращает приоритет платежа.
	virtual Priority getPriority() const;

	/// Устанавливает приоритет проведения платежу.
	virtual void setPriority(Priority aPriority);

	/// Устанавливает дату следующей попытки проведения платежа.
	virtual void setNextTryDate(const QDateTime & aDate);

	/// Устанавливает дату завершения формирования платежа.
	virtual void setCompleteDate(const QDateTime & aDate);

	/// Возвращает true, если платёж можно провести в оффлайне.
	virtual bool canProcessOffline() const;

	/// Возвращает, можно ли обновлять лимиты платежа
	virtual bool getBlockUpdateLimits() const;

	/// Устанавливает блокировку обновления лимитов платежа
	virtual void setBlockUpdateLimits(bool aBlock);

	/// Отмена платёжа. В случае успеха возвращает true.
	virtual bool cancel();

	/// Отметка платежа как удаленного. В случае успеха возвращает true.
	virtual bool remove();

#pragma endregion

public:
	/// Возвращает сумму, которая будет зачислена на счёт плательщика.
	virtual QString getAmount() const;

	/// Возвращает сумму внесённых средств.
	virtual QString getAmountAll() const;

	/// Возвращает следующую дату проведения платежа.
	virtual QDateTime getNextTryDate() const;

	/// Возвращает описание провайдера для платежа.
	const SDK::PaymentProcessor::SProvider getProviderSettings(qint64 aProvider = -1) const;

	/// Возвращает описание провайдера для MNP платежа.
	const SDK::PaymentProcessor::SProvider getMNPProviderSettings() const;

	/// Возвращает настройки ключей для проведения платежа.
	const SDK::PaymentProcessor::SKeySettings & getKeySettings() const;

protected:
	/// Устанавливает значение параметра во внутреннем списке.
	virtual void updateParameter(const SParameter & aParameter);

	/// Возвращает true, если ограничения на сумму платежа зависят от переданного параметра.
	virtual bool limitsDependOnParameter(const SParameter & aParameter);

	/// Подсчёт верхней и нижней границ для сумм платежа.
	virtual bool calculateLimits();

	/// Получение лимитов для данного платежа
	virtual bool getLimits(double & aMinAmount, double & aMaxAmount);

	/// Рассчитывает сумму платежа по полю AMOUNT_ALL.
	virtual void calculateSums();

	/// Генерирует новую сесиию для проведения платежа.
	QString createPaymentSession() const;

	/// Критичекая ошибка, проведение платежа прекращается.
	virtual bool isCriticalError(int aError) const;

protected:
	struct NameTag;
	struct UpdateTag;

	typedef boost::multi_index_container <
		SParameter,
		boost::multi_index::indexed_by <
		boost::multi_index::ordered_unique <
		boost::multi_index::tag<NameTag>,
		boost::multi_index::member <SParameter, QString, &SParameter::name> >,
		boost::multi_index::ordered_non_unique <
		boost::multi_index::tag<UpdateTag>,
		boost::multi_index::member <SParameter, bool, &SParameter::updated> > > > TParameterList;

private:
	mutable QReadWriteLock mParametersLock;
	TParameterList mParameters;

protected:
	SDK::PaymentProcessor::IPaymentFactory * mFactory;
	SDK::PaymentProcessor::ICore * mCore;

	SDK::PaymentProcessor::SKeySettings mKeySettings;
	SDK::PaymentProcessor::SProvider mProviderSettings;

	/// Флаг - true, если идёт восстановление платежа из списка параметров.
	bool mIsRestoring;
};
