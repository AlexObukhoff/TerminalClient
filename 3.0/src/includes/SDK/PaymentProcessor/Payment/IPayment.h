/* @file Интерфейс платежа. */

#pragma once

// stl
#include <algorithm>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QDateTime>
#include <QtCore/QList>
#include <Common/QtHeadersEnd.h>

namespace SDK {
namespace PaymentProcessor {

class IPaymentFactory;

//------------------------------------------------------------------------------
class IPayment
{
public:
	/// Приоритет проведения платежа.
	enum Priority
	{
		Online = 0,
		Offline,
		Low
	};

	/// Параметр платежа.
	struct SParameter
	{
		SParameter()
			: crypted(false),
			  updated(false),
			  external(false)
		{
		}

		SParameter(QString aName, QVariant aValue, bool aUpdated = false, bool aCrypted = false, bool aExternal = false)
			: name(aName),
			  value(aValue),
			  crypted(aCrypted),
			  updated(aUpdated),
			  external(aExternal)
		{
		}

		inline bool isNull() const
		{
			return name.isEmpty();
		}

		/// Название.
		QString name;

		/// Значение.
		QVariant value;

		/// Признак того, что параметр должен храниться в зашифрованном виде.
		bool crypted;

		/// Признак того, что параметр надо обновить в БД.
		bool updated;

		/// Признак передачи данного параметра во все запросы платежа как есть.
		bool external;
	};

	/// Если предикат возвращает true, то платёж не сформирован должным образом и может быть удалён.
	virtual bool isNull() const = 0;

	/// Возвращает фабрику, которой был создан платёж.
	virtual IPaymentFactory * getFactory() const = 0;

	/// Установка параметра в платёж.
	virtual void setParameter(const SParameter & aParameter) = 0;

	/// Получение параметра по имени.
	virtual SParameter getParameter(const QString & aName) const = 0;

	/// Получение всех параметров платежа.
	virtual QList<SParameter> getParameters() const = 0;

	/// Вычислить размер комиссии, без записи результата в параметры платежа
	virtual QList<SParameter> calculateCommission(const QList<SParameter> & aParameters) = 0;

	/// Возвращает список параметров, достаточных для последющего восстановления платежа.
	virtual QList<SParameter> serialize() const = 0;

	/// Восстановление платежа из списка параметров.
	virtual bool restore(const QList<SParameter> & aParameters) = 0;

	/// Идентификатор платежа.
	virtual qint64 getID() const = 0;

	/// Тип платежа.
	virtual QString getType() const = 0;

	/// Возвращает активную сессию платежа.
	virtual QString getSession() const = 0;

	/// Возвращает начальную сессию платежа.
	virtual QString getInitialSession() const = 0;

	/// Провайдер платежа.
	virtual qint64 getProvider(bool aMNP) const = 0;

	/// Дата создания платежа.
	virtual QDateTime getCreationDate() const = 0;

	/// Дата завершения формирования платежа.
	virtual QDateTime getCompleteDate() const = 0;

	/// Дата последнего изменения платежа.
	virtual QDateTime getLastUpdateDate() const = 0;

	/// Статус платежа.
	virtual int getStatus() const = 0;

	/// Устанавливает платежу статус aStatus.
	virtual void setStatus(int aStatus) = 0;

	/// Возвращает приоритет платежа.
	virtual Priority getPriority() const = 0;

	/// Устанавливает приоритет проведения платежу.
	virtual void setPriority(Priority aPriority) = 0;

	/// Устанавливает дату следующей попытки проведения платежа.
	virtual void setNextTryDate(const QDateTime & aDate) = 0;

	/// Устанавливает дату завершения формирования платежа.
	virtual void setCompleteDate(const QDateTime & aDate) = 0;

	/// Возвращает true, если платёж можно провести в оффлайне.
	virtual bool canProcessOffline() const = 0;

	/// Выполнение шага с идентификатором aStep.
	virtual bool performStep(int aStep) = 0;

	/// Обновление статуса платежа.
	virtual void process() = 0;

	/// Отмена платёжа. В случае успеха возвращает true.
	virtual bool cancel() = 0;

	/// Отметка платежа как удаленного. В случае успеха возвращает true.
	virtual bool remove() = 0;

	/// Вспомогательные метод возвращающий параметр из списка по имени.
	static SParameter parameterByName(const QString & aName, const QList<SParameter> & aParameters)
	{
		QList<SParameter>::const_iterator it = std::find_if(aParameters.begin(), aParameters.end(),
			[aName](const SParameter & aP) -> bool { return aP.name == aName; });

		return (it == aParameters.end()) ? SParameter() : *it;
	}

protected:
	virtual ~IPayment() {}
};

//------------------------------------------------------------------------------
}} // SDK::PaymentProcessor

