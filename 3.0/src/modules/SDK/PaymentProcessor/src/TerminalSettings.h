/* @file  Настройки терминала (данной инсталляции ПО). */

#pragma once

// boost
#include <boost/optional.hpp>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <QtCore/QTime>
#include <Common/QtHeadersEnd.h>

// Common
#include <Common/ILogable.h>
#include <Common/PropertyTree.h>
#include <Common/Currency.h>

// SDK
#include <Connection/IConnection.h>
#include <SDK/PaymentProcessor/Core/EventTypes.h>
#include <SDK/PaymentProcessor/Settings/ISettingsAdapter.h>
#include <SDK/PaymentProcessor/Connection/Connection.h>

namespace SDK {
namespace PaymentProcessor {

//---------------------------------------------------------------------------
struct SDatabaseSettings
{
	QString host;
	QString name;
	QString user;
	QString password;
	int port;

	SDatabaseSettings() : port(0) {}
};

//---------------------------------------------------------------------------
struct SKeySettings
{
	bool isValid;
	int id;                   /// -1 - root key, -100 - invalid value
	int engine;
	QString sd;             /// Код диллера.
	QString ap;             /// Код точки.
	QString op;             /// Код оператора.
	ulong serialNumber;     /// Серийник ключа.
	ulong bankSerialNumber; /// Серийник банковского ключа.
	QString publicKeyPath;  /// Путь к открытому ключу терминала.
	QString secretKeyPath;  /// Путь к закрытому ключу терминала.
	QString secretPassword; /// Кодовая фраза.

	SKeySettings() : isValid(false), id(-100), engine(0), serialNumber(0), bankSerialNumber(0) {}
};

//----------------------------------------------------------------------------
struct SCurrencySettings
{
	int id;       /// Идентификатор валюты по ISO.
	QString code; /// Международное имя валюты ISO.
	QString name; /// Имя валюты в текущей локализации.
	QList<Currency::Nominal> coins; /// Список всех существующих номиналов монет
	QList<Currency::Nominal> notes; /// Список всех существующих номиналов купюр

	SCurrencySettings() : id(-1) {}
};

//----------------------------------------------------------------------------
struct SAppEnvironment
{
	QString userDataPath;
	QString contentPath;
	QString interfacePath;
	QString adPath;
	QString version;
};

//----------------------------------------------------------------------------
struct SBlockByNote
{
	quint32 nominal;
	quint32 interval;
	quint32 repeat;

	SBlockByNote() : nominal(0), interval(0), repeat(0) {}
	explicit SBlockByNote(quint32 aNominal, quint32 aInterval, quint32 aRepeat) : nominal(aNominal), interval(aInterval), repeat(aRepeat) {}
};

//----------------------------------------------------------------------------
struct SCommonSettings
{
	typedef enum
	{
		ValidatorError = 1,
		PrinterError,
		CardReaderError,
		AccountBalance,
		Penetration
	} BlockReason;

	QSet<BlockReason> _blockOn;
	QList<SBlockByNote> blockNotes; /// Блокировка по номиналам
	bool blockCheatedPayment; /// Блокировка при подозрении на манипуляции с устройством
	bool autoEncachement;
	Currency::Nominal minPar;
	QSet<Currency::Nominal> enabledParNotesList; /// Список разрешенных купюр.
	QSet<Currency::Nominal> enabledParCoinsList; /// Список разрешенных монет.
	boost::optional<int> timeZoneOffset;
	bool skipCheckWhileNetworkError; /// Принимать платежи offline, до блокировки по связи
	bool isValid;
	bool disableAmountOverflow; /// Не допускать появление сдачи путем отбраковки купюр сверх лимита.
	EEventType::Enum penetrationEventLevel;

	// Printer
	bool printFailedReceipts;  /// Распечатывать не напечатанные фискальные чеки при инкассации
	bool randomReceiptsID;     /// Номера чеков в рандомном порядке
	QTime autoZReportTime;     /// Автоматичекое закрытие смены ККТ в определенное время
	bool enableBlankFiscalData;/// Разрешать печатать фискальные чеки без фискальных данных

	SCommonSettings() : 
		blockCheatedPayment(false),
		autoEncachement(false),
		minPar(10),
		skipCheckWhileNetworkError(false),
		isValid(true),
		disableAmountOverflow(false),
		penetrationEventLevel(EEventType::OK),
		printFailedReceipts(true),
		randomReceiptsID(false),
		enableBlankFiscalData(false)
	{
		_blockOn
			<< ValidatorError
			<< PrinterError
			<< CardReaderError;
	}

	void setBlockOn(BlockReason aReason, bool aBlock)
	{
		if (aBlock)
		{
			_blockOn << aReason;
		}
		else
		{
			_blockOn.remove(aReason);
		}
	}

	bool blockOn(BlockReason aReason) const
	{
		return _blockOn.contains(aReason);
	}
};

//----------------------------------------------------------------------------
struct SMonitoringSettings
{
	QUrl url;
	QUrl restUrl;
	int heartbeatTimeout;
	int restCheckTimeout;
	int restLimit;

	QStringList cleanupItems;
	QStringList cleanupExclude;

	SMonitoringSettings() :
		heartbeatTimeout(10),
		restCheckTimeout(30),
		restLimit(0)
	{
	}

	bool isBlockByAccountBalance() const
	{
		return restLimit > 0 && restUrl.isValid();
	}
};

//----------------------------------------------------------------------------
struct SServiceMenuPasswords
{
	SServiceMenuPasswords() : operatorId(0) {}

	int operatorId;
	QString phone;
	QMap<QString, QString> passwords;
};

//----------------------------------------------------------------------------
namespace CServiceMenuPasswords
{
	const char Service[] = "service_password";
	const char Screen[] ="screen_password";
	const char Collection[] = "collection_password";
	const char Technician[] = "technician_password";
}

//----------------------------------------------------------------------------
class TerminalSettings : public ISettingsAdapter, public ILogable
{
public:
	TerminalSettings(TPtree & aProperties);
	~TerminalSettings();

	/// Инициализация настроек.
	void initialize();

	/// Проверка настроек.
	virtual bool isValid() const;

	/// Получить имя адаптера.
	static QString getAdapterName();

	/// Получить настройки соединения.
	SConnection getConnection() const;

	/// Сохранить настройки соединения.
	void setConnection(const SConnection & aConnection);

	/// Получить список хостов для проверки соединения.
	QList<IConnection::CheckUrl> getCheckHosts() const;

	/// Получить настройки БД.
	SDatabaseSettings getDatabaseSettings() const;

	/// Получить список устройств.
	QStringList getDeviceList() const;

	/// Получить имя конфигурации принтера, настроенного для печати заданного типа чеков.
	QString getPrinterForReceipt(const QString & aReceiptType);

	/// Сохранить список устройств.
	void setDeviceList(const QStringList & aHardware);

	/// Мониторинг.
	SMonitoringSettings getMonitoringSettings() const;

	/// Ключи.
	QMap<int, SKeySettings> getKeys() const;
	void setKey(const SKeySettings & aKey, bool aReplaceIfExists = true);
	void cleanKeys();

	/// Валюта.
	SCurrencySettings getCurrencySettings() const;

	/// URL для генерации ключей.
	QString getKeygenURL() const;

	/// URL для генерации письма с электронной копией чека
	QString getReceiptMailURL() const;

	/// URL для отправки фидбека в Киберплат
	QString getFeedbackURL() const;

	/// Возвращает мапу из типов процессинга и разрешенных ChargeProvider для каждого соотвтественно.
	QVariantMap getChargeProviderAccess() const;

	/// Общие настройки.
	virtual SCommonSettings getCommonSettings() const;

	/// Пароли для доступа к сервисному меню.
	SServiceMenuPasswords getServiceMenuPasswords() const;

	/// Пути к данным.
	SAppEnvironment getAppEnvironment() const;
	void setAppEnvironment(const SAppEnvironment & aEnv);

	/// Максимальный размер log файлов
	int getLogsMaxSize() const;

	/// Урлы апдейтера
	QStringList getUpdaterUrls() const;

	/// Реклама.
	QString getAdProfile() const;

	/// возвращает валидное время если необходимо проверять обновление ПО терминала
	QTime autoUpdate() const;

	/// возвращает диапазон времени с какого по какое мы держим монитор в выключенном состоянии
	QString energySave() const;

	/// Получить список критичных ошибок процессинга
	const QSet<int> & getCriticalErrors() const;

private:
	TerminalSettings(const TerminalSettings &);
	void operator =(const TerminalSettings &);

	TPtree & mProperties;

	/// Список критичных ошибок процессинга
	QSet<int> mCriticalErrors;
};

}} // SDK::PaymentProcessor

//---------------------------------------------------------------------------
