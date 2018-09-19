/* @file Прокси класс для работы с платежами в скриптах. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QVariantMap>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/Encashment.h>
#include <SDK/PaymentProcessor/Settings/Provider.h>
#include <SDK/PaymentProcessor/Scripting/ScriptArray.h>
#include <SDK/PaymentProcessor/Payment/Amount.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

namespace SDK {
namespace PaymentProcessor {
class ICore;
class IPaymentService;
class Directory;
class DealerSettings;

namespace Scripting {
class PaymentService;

//------------------------------------------------------------------------------
class EnumItem: public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString name READ getName CONSTANT)
	Q_PROPERTY(QString value READ getValue CONSTANT)
	Q_PROPERTY(QString id READ getID CONSTANT)

public:
	EnumItem(const SProviderField::SEnumItem & aItem, QObject * aParent)
		: QObject(aParent), mItem(aItem)
	{}

private:
	QString getName() { return mItem.title; }
	QString getValue() { return mItem.value; }
	QString getID() { return mItem.id; }

private:
	SProviderField::SEnumItem mItem;
};

//------------------------------------------------------------------------------
class ProviderField : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString type READ getType CONSTANT)
	Q_PROPERTY(QString id READ getId CONSTANT)

	Q_PROPERTY(QString keyboardType READ getKeyboardType CONSTANT)
	Q_PROPERTY(QString language READ getLanguage CONSTANT)
	Q_PROPERTY(QString letterCase READ getLetterCase CONSTANT)

	Q_PROPERTY(int minSize READ getMinSize CONSTANT)
	Q_PROPERTY(int maxSize READ getMaxSize CONSTANT)

	Q_PROPERTY(bool isRequired READ isRequired CONSTANT)

	Q_PROPERTY(QString title READ getTitle CONSTANT)
	Q_PROPERTY(QString comment READ getComment CONSTANT)
	Q_PROPERTY(QString extendedComment READ getExtendedComment CONSTANT)

	Q_PROPERTY(QString mask READ getMask CONSTANT)
	Q_PROPERTY(QString format READ getFormat CONSTANT)
	Q_PROPERTY(bool isPassword READ isPassword CONSTANT)

	Q_PROPERTY(QString behavior READ getBehavior CONSTANT)
	Q_PROPERTY(QString defaultValue READ getDefaultValue CONSTANT)
	Q_PROPERTY(QObject * enumItems READ getEnumItems CONSTANT)

	Q_PROPERTY(QString url READ getUrl CONSTANT)
	Q_PROPERTY(QString html READ getHtml CONSTANT)
	Q_PROPERTY(QString backButton READ getBackButton CONSTANT)
	Q_PROPERTY(QString forwardButton READ getForwardButton CONSTANT)
	Q_PROPERTY(QString dependency READ getDependency CONSTANT)

public:
	ProviderField(const SProviderField & aField, QObject * aParent = 0)
		: QObject(aParent), mField(aField)
	{
	}

private:
	QString getType() { return mField.type; }
	QString getId() { return mField.id; }

	QString getKeyboardType() { return mField.keyboardType; }
	QString getLanguage() { return mField.language; }
	QString getLetterCase() { return mField.letterCase; }

	int getMinSize() { return mField.minSize; }
	int getMaxSize() { return mField.maxSize; }

	bool isRequired() { return mField.isRequired; }

	QString getTitle() { return mField.title; }
	QString getComment() { return mField.comment; }
	QString getExtendedComment() { return mField.extendedComment; }

	QString getMask() { return mField.mask; }
	QString getFormat() { return mField.format; }
	bool isPassword() { return mField.isPassword; }

	QString getBehavior() { return mField.behavior; }
	QString getDefaultValue() { return mField.defaultValue; }
	ScriptArray * getEnumItems();

	QString getUrl() { return mField.url; }
	QString getHtml() { return mField.html; }
	QString getBackButton() { return mField.backButton; }
	QString getForwardButton() { return mField.forwardButton; }
	
	QString getDependency() { return mField.dependency; }

	SProviderField mField;
};

//------------------------------------------------------------------------------
class Provider : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString id READ getID CONSTANT)
	Q_PROPERTY(QString gateway READ getCID CONSTANT)
	Q_PROPERTY(QString type READ getType CONSTANT)
	Q_PROPERTY(QString processorType READ getProcessorType CONSTANT)
	Q_PROPERTY(QString name READ getName CONSTANT)
	Q_PROPERTY(QString comment READ getComment CONSTANT)
	Q_PROPERTY(QString minLimit READ getMinLimit CONSTANT)
	Q_PROPERTY(QString maxLimit READ getMaxLimit CONSTANT)
	Q_PROPERTY(QString systemLimit READ getSystemLimit CONSTANT)
	Q_PROPERTY(QVariant fields READ getFields CONSTANT)
	Q_PROPERTY(bool skipCheck READ getSkipCheck CONSTANT)
	Q_PROPERTY(bool payOnline READ getPayOnline CONSTANT)
	Q_PROPERTY(bool askForRetry READ getAskForRetry CONSTANT)
	Q_PROPERTY(bool requirePrinter READ getRequirePrinter CONSTANT)
	Q_PROPERTY(bool showAddInfo READ getShowAddInfo CONSTANT)
	Q_PROPERTY(QString clientCard READ getClientCard CONSTANT)
	Q_PROPERTY(QString externalDataHandler READ getExternalDataHandler CONSTANT)
	Q_PROPERTY(QVariantMap receipts READ getReceipts CONSTANT)
	Q_PROPERTY(QVariantMap receiptParameters READ getReceiptParameters CONSTANT)

public:
	Provider(const SProvider & aProvider, QObject * aParent);

public slots:
	bool isNull() const { return mProvider.id == -1 || mProvider.fields.isEmpty(); }
	
	/// Проверить согласование проверки номера и лимитов, получаемых с сервера
	bool isCheckStepSettingsOK();
	
	QString applySecurityFilter(const QString aId, const QString & aValueRaw, const QString & aValueDisplay) const;
	
	QString xmlFields2Json(const QString & aXmlFields);

private:
	QString getID() const { return QString::number(mProvider.id); }
	QString getCID() const { return QString::number(mProvider.cid); }
	QString getType() const { return mProvider.type; }
	QString getProcessorType() const { return mProvider.processor.type; }
	QString getName() const { return mProvider.name; }
	QString getComment() const { return mProvider.comment; }
	QString getMinLimit() const { return mProvider.limits.min; }
	QString getMaxLimit() const { return mProvider.limits.max; }
	QString getSystemLimit() const { return mProvider.limits.system; }
	QVariant getFields();
	bool getSkipCheck() const { return mProvider.processor.skipCheck; }
	bool getPayOnline() const { return mProvider.processor.payOnline; }
	bool getAskForRetry() const { return mProvider.processor.askForRetry; }
	bool getRequirePrinter() const { return mProvider.processor.requirePrinter; }
	bool getShowAddInfo() const { return mProvider.processor.showAddInfo; }
	QString getClientCard() const { return QString::number(mProvider.processor.clientCard); }
	QString getExternalDataHandler() const { return mProvider.externalDataHandler; }
	QVariantMap getReceipts() const { return mProvider.receipts; }
	QVariantMap getReceiptParameters() const { return mProvider.receiptParameters; }

private:
	SProvider mProvider;
	QMap<QString, QObjectList> mFields;
};

#define PROPERTY_GET(type, name, holder) type name() const { return holder.name; }

//------------------------------------------------------------------------------
class Note : public QObject
{
	Q_OBJECT

	Q_PROPERTY(double nominal READ nominal CONSTANT)
	Q_PROPERTY(QString serial READ serial CONSTANT)
	Q_PROPERTY(int currency READ currency CONSTANT)
	Q_PROPERTY(int type READ type CONSTANT)

public:
	Note(const SNote & aNote, QObject * aParent) : QObject(aParent), mNote(aNote)
	{}

private:
	PROPERTY_GET(double, nominal, mNote)
	PROPERTY_GET(QString, serial, mNote)
	PROPERTY_GET(int, currency, mNote)
	PROPERTY_GET(int, type, mNote)

private:
	SNote mNote;
};

//------------------------------------------------------------------------------
class PaymentService : public QObject
{
	Q_OBJECT
	Q_ENUMS(EProcessResult)

public:
	/// Результат попытки проведения платежа.
	enum EProcessResult
	{
		OK = 0,                    /// Ошибок нет.
		LowMoney = 1,              /// Недостаточно средств для проведения платежа.
		OfflineIsNotSupported = 2, /// Попытка провести в оффлайне платёж, который поддерживает только онлайн.
		BadPayment = 3             /// Платеж не найден или не может быть проведён
	};

	PaymentService(ICore * aCore);

public slots:
	/// Создание платежа по номеру оператора aProvider. Возвращает номер платежа или код ошибки (меньше 0).
	qint64 create(qint64 aProvider);

	/// Возвращает номер активного платежа.
	qint64 getActivePaymentID();

	/// Возвращает номер последнего платежа.
	qint64 getLastPaymentID();

	/// Возвращает описание оператора активного платежа.
	QObject * getProvider();

	/// Возвращает описание реального оператора активного платежа.
	QObject * getMNPProvider();

	/// Возвращает описание оператора с идентификатором aID.
	QObject * getProvider(qint64 aID);

	/// Возвращает описание первого попавшегося оператора с номером шлюза aСID.
	QObject * getProviderByGateway(qint64 aCID);

	/// Возвращает провайдеров для данного номера в соответствии с номерной ёмкостью.
	QObject * getProviderForNumber(qint64 aNumber);

	/// Возвращает значение поля aName для платежа aPayment.
	QVariant getParameter(const QString & aName);

	/// Возвращает список полей платежа.
	QVariantMap getParameters();

	/// Вычислить размер комиссии для активного платежа, без записи результата в параметры платежа
	QVariantMap calculateCommission(const QVariantMap & aParameters);

	/// Вычислить для активного платежа, без записи результата в параметры платежа, лимиты комиссию для произвольной суммы 
	QVariantMap calculateLimits(const QString & aAmount, bool aFixedAmount = false);

	/// Возвращает список купюр текущего платежа.
	QObject * getPaymentNotes();

	/// Возвращает список не инкассированных купюр.
	QObject * getBalanceNotes();

	/// Возвращает список купюр из последней инкассации.
	QObject * getLastEncashmentNotes();

	/// Обновляет указанное свойство у платежа.
	void setExternalParameter(const QString & aName, const QVariant & aValue);

	/// Возвращает имя алиаса, к которому привязан параметр из запроса
	QString findAliasFromRequest(const QString & aParamName, const QString & aRequestName = QString("CHECK"));

	/// Обновляет указанное свойство у платежа.
	void setParameter(const QString & aName, const QVariant & aValue, bool aCrypted = false);

	/// Обновление нескольких свойств платежа.
	void setParameters(const QVariantMap & aParameters);

	/// Установить для провайдера лимиты платежа
	void updateLimits(qint64 aProviderId, double aExternalMin, double aExternalMax);

	/// Возвращает true, если платёж можно провести в оффлайне.
	bool canProcessOffline();

	/// Сконвертировать активный платёж в новый процессинг
	bool convert(const QString & aTargetType);

	/// Проверка введённых данных.
	void check();

	/// Выполенение шага платежа в онлайне.
	void processStep(int aStep);

	/// Проведение платежа.
	EProcessResult process(bool aOnline);

	/// Остановка платежа с текстом ошибки
	bool stop(int aError, const QString & aErrorMessage);

	/// Отмена платежа.
	bool cancel();

	/// Сбросить состояние платёжной логики
	void reset();

	/// Получение суммы сдачи, доступной для использования.
	double getChangeAmount();

	/// Использование сдачи от предыдущего платежа.
	void useChange();

	/// Переместить всю сумму из платежа обратно в сдачу.
	void useChangeBack();

	/// Сброс счётчика наполненой сдачи.
	void resetChange();

public slots:
	/// Поиск списка платежей по номеру телефона/счета
	QStringList findPayments(const QDate & aDate, const QString & aPhoneNumber);

	/// Получение списка параметров платежа по его ID
	QVariantMap getParameters(qint64 aPaymentId);

	/// Возвращает статистику по кол-ву платежей
	QVariantMap getStatistic();

	void checkStatus();

public slots:
#pragma region Multistage
	/// Отправка запроса для перехода на следующий шаг
	void stepForward();

	/// Вернутся на шаг назад
	void stepBack();

	/// Получение ID текущего шага
	QString currentStep();

	/// Признак, что текущий шаг - последний
	bool isFinalStep();

public:
	/// Получение списка полей для интерфейса в многошаговом шлюзе
	bool loadFieldsForStep(TProviderFields & aFields);
#pragma endregion

	/// Включить возможность проведения платежей при отсутствии связи
	void setForcePayOffline(bool aPayOffline);

private slots:
	/// Получение сигнала об окончании проверки платежа
	void onStepCompleted(qint64 aPayment, int aStep, bool aError);

signals:
	/// Завевершён шаг для платежа aPayment.
	void stepCompleted(qint64 aPayment, int aStep, bool aError);

	/// Обновлены суммы указанного платежа.
	void amountUpdated(qint64 aPayment);

	/// Обновление суммы сдачи
	void changeUpdated(double aAmount);

private:
	ICore * mCore;
	IPaymentService * mPaymentService;
	SDK::PaymentProcessor::Directory * mDirectory;
	SDK::PaymentProcessor::DealerSettings * mDealerSettings;
	SDK::PaymentProcessor::SCommonSettings mCommonSettings;
	bool mForcePayOffline;

private:
	/// Получить список купюр из баланса
	QObject * notesFromBalance(const SDK::PaymentProcessor::SBalance & aBalance);

	/// Модифицируем провайдера в соответствии с настройками
	SProvider updateSkipCheckFlag(SProvider aProvider);

	/// Провайдер, которому меняли лимиты платежа из сценария
	qint64 mProviderWithExternalLimits;

private:
	QList<SDK::PaymentProcessor::SBlockByNote> mBlockNotesList;
};

//------------------------------------------------------------------------------
}}} // Scripting::PaymentProcessor::SDK
