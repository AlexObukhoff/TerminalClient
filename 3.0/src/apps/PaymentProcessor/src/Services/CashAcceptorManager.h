/* @file Обработчик команд работы с устройствами/сервисами получения денег. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QSharedPointer>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ICashAcceptorManager.h>
#include <SDK/PaymentProcessor/Core/IDeviceService.h>
#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/Plugins/IPlugin.h>
#include <SDK/Drivers/ICashAcceptor.h>

// Modules
#include <Common/ILogable.h>

// PP
#include "DatabaseUtils/IPaymentDatabaseUtils.h"

class IApplication;
class IHardwareDatabaseUtils;

namespace SDK { namespace PaymentProcessor {
	class IDeviceService;
	class IChargeProvider;
}}

//---------------------------------------------------------------------------
class CashAcceptorManager : public SDK::PaymentProcessor::ICashAcceptorManager, public ILogable
{
	Q_OBJECT

public:
	CashAcceptorManager(IApplication * aApplication);
	virtual ~CashAcceptorManager();

	/// Инициализация.
	virtual bool initialize(IPaymentDatabaseUtils * aDatabase);

	/// Остановка.
	virtual bool shutdown();

	/// Возвращает список доступных методов оплаты.
	virtual QStringList getPaymentMethods();

	/// Начать приём денег для указанного платежа.
	virtual bool enable(qint64 aPayment, const QString & aPaymentType, SDK::PaymentProcessor::TPaymentAmount aMaxAmount);

	/// Завершить приём денег для указанного платежа.
	virtual bool disable(qint64 aPayment);

	/// Значение счетчика непринятых купюр.
	int getRejectCount() const;

private:
	/// Увеличить значение счетчика непринятых купюр.
	void incrementRejectCount();

	/// Загрузка таблицы разрешённых номиналов
	void initWorkingParList();

	/// Разрешить ли принимать купюру/монету данного номинала в счет активного платежа
	bool allowMoreMoney(SDK::PaymentProcessor::TPaymentAmount aAmount);

	/// Определяет, имеет ли платеж фиксированную сумму
	bool isFixedAmountPayment(qint64 aPayment);

private slots:
	/// Валидатор распознал купюру.
	void onEscrow(SDK::Driver::SPar aPar);

	/// Валидатор распознал купюру (с контролем переполнения суммы платежа)
	void onEscrowChangeControl(SDK::Driver::SPar aPar);

	/// Изменение статуса валидатора.
	void onStatusChanged(SDK::Driver::EWarningLevel::Enum aWarningLevel, const QString & aTranslation, int aStatus);

	/// Валидатор успешно уложил купюру.
	void onStacked(SDK::Driver::TParList aPar);

	/// Обновление списка устройств.
	void updateHardwareConfiguration();

	/// Провайдер денег получил указанную сумму.
	void onChargeProviderStacked(SDK::PaymentProcessor::SNote aNote);

private:
	IApplication * mApplication;
	IPaymentDatabaseUtils * mDatabase;
	SDK::PaymentProcessor::IDeviceService * mDeviceService;
	bool mDisableAmountOverflow;

	struct SPaymentData
	{
		qint64 paymentId;
		SDK::PaymentProcessor::TPaymentAmount currentAmount;
		SDK::PaymentProcessor::TPaymentAmount maxAmount;
		QSet<SDK::Driver::ICashAcceptor *> validators;
		QSet<SDK::PaymentProcessor::IChargeProvider *> chargeProviders;

		explicit SPaymentData(qint64 aPaymentId) :
			paymentId(aPaymentId), 
			currentAmount(0.0), 
			maxAmount(0.0) {}

		bool maxAmountReached() const;
		bool chargeSourceEmpty() const;
	};

	/// Список всех валидаторов.
	typedef QList<SDK::Driver::ICashAcceptor *> TCashAcceptorList;
	TCashAcceptorList mDeviceList;

	QSharedPointer<SPaymentData> mPaymentData;

	/// Набор разрешенных номиналов.
	SDK::Driver::TParList mWorkingParList;

	/// Список всех провайдеров денежных средств
	QList<SDK::PaymentProcessor::IChargeProvider *> mChargeProviders;
};

//---------------------------------------------------------------------------

