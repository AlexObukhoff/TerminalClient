/* @file Базовый класс устройств. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QSet>
#include <Common/QtHeadersEnd.h>

// Project
#include "Hardware/Common/PollingExpector.h"
#include "Hardware/Common/BaseStatusDescriptions.h"
#include "Hardware/Common/BaseStatusTypes.h"
#include "Hardware/Common/HystoryList.h"
#include "Hardware/Common/MetaDevice.h"

//--------------------------------------------------------------------------------
typedef QList<int> TStatusCodesBuffer;

//--------------------------------------------------------------------------------
/// Общие константы устройств.
namespace CDevice
{
	/// Размер истории статусов.
	const int StatusCollectionHistoryCount = 10;

	/// Имя устройства по умолчанию.
	const char DefaultName[] = "Unknown device";

	/// Разделитель статусов.
	const char StatusSeparator[] = "; ";
}

//--------------------------------------------------------------------------------
#define START_IN_WORKING_THREAD(aFunction) \
	if ((!mOperatorPresence || (getConfigParameter(CHardware::CallingType) != CHardware::CallingTypes::Internal)) && !isWorkingThread()) { \
	if (mThread.isRunning()) { QMetaObject::invokeMethod(this, #aFunction, Qt::QueuedConnection); } \
	else { connect(&mThread, SIGNAL(started()), this, SLOT(aFunction()), Qt::UniqueConnection); mThread.start(); } \
	return; }

//--------------------------------------------------------------------------------
template <class T>
class DeviceBase : public T
{
public:
	DeviceBase();

#pragma region SDK::Driver::IDevice interface
	/// Подключает и инициализует устройство. Обертка для вызова функционала в рабочем потоке.
	virtual void initialize();

	/// Освобождает ресурсы, связанные с устройством, возвращается в состояние до вызова initialize().
	virtual bool release();

	/// Соединяет сигнал данного интерфейса со слотом приёмника.
	virtual bool subscribe(const char * aSignal, QObject * aReceiver, const char * aSlot);

	/// Отсоединяет сигнал данного интерфейса от слота приёмника.
	virtual bool unsubscribe(const char * aSignal, QObject * aReceiver);
#pragma endregion

#pragma region SDK::Driver::IDetectingIterator interface
	/// Поиск устройства на текущих параметрах.
	virtual bool find();
#pragma endregion

	/// Полл без пост-действий.
	void simplePoll();

protected:
	/// Идентификация.	
	virtual bool checkExistence();

	/// Обработчик сигнала полла.
	virtual void onPoll();

	/// Полл.
	virtual void doPoll(TStatusCodes & aStatusCodes);

	/// Получить и обработать статус.
	virtual bool processStatus(TStatusCodes & aStatusCodes);

	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Применение буфера статусов для блокирования "мигающих" ошибок.
	void applyStatusBuffer(TStatusCodes & aStatusCodes);

	/// Анализирует коды статусов устройства и фильтрует несуществующие статусы для нижней логики.
	virtual void cleanStatusCodes(TStatusCodes & aStatusCodes);

	/// Есть ли ошибка инициализации при фильтрации статусов.
	bool isInitializationError(TStatusCodes & aStatusCodes);

	/// Исправить исправимые ошибки.
	void recoverErrors(TStatusCodes & aStatusCodes);

	/// Получить переводы новых статус-кодов для последующей их обработки.
	virtual QString getTrOfNewProcessed(const TStatusCollection & aStatusCollection, SDK::Driver::EWarningLevel::Enum aWarningLevel);

	/// Посылка статусов, если необходимо, в зависимости от типа устройства.
	void processStatusCodes(const TStatusCodes & aStatusCodes);

	/// Фоновая логика при появлении определенных состояний устройства.
	virtual void postPollingAction(const TStatusCollection & aNewStatusCollection, const TStatusCollection & aOldStatusCollection);

	/// Переинициализация в рамках фоновой логики пост-поллинга.
	virtual void reInitialize();

	/// Отправка статусов.
	virtual void sendStatuses(const TStatusCollection & aNewStatusCollection, const TStatusCollection & aOldStatusCollection);

	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Установить начальные параметры.
	virtual void setInitialData();

	/// Завершение инициализации.
	virtual void finaliseInitialization();

	/// Идентифицирует устройство.
	virtual bool isConnected();

	/// Проверка возможности выполнения функционала, предполагающего связь с устройством.
	virtual bool checkConnectionAbility();

	/// Проверка возможности применения буфера статусов.
	virtual bool isStatusesReplaceable(TStatusCodes & aStatusCodes);

	/// Проверка возможности применения буфера статусов.
	virtual bool canApplyStatusBuffer();

	/// Есть ли несовпадение плагина и ПП.
	bool isPluginMismatch();

	/// Получить переводы статусов.
	QString getStatusTranslations(const TStatusCodes & aStatusCodes, bool aLocale) const;

	/// Получить спецификацию статуса.
	virtual SStatusCodeSpecification getStatusCodeSpecification(int aStatusCode) const;

	/// Получить статус-коды.
	TStatusCodes getStatusCodes(const TStatusCollection & aStatusCollection);

	/// Получить коллекцию статусов.
	TStatusCollection getStatusCollection(const TStatusCodes & aStatusCodes, TStatusCodeSpecification * aStatusCodeSpecification = nullptr);

	/// Получить уровень лога.
	LogLevel::Enum getLogLevel(SDK::Driver::EWarningLevel::Enum aLevel);

	/// Отправить статус-коды.
	void emitStatusCode(int aStatusCode, int aExtendedStatus = SDK::Driver::EStatus::Actual);
	virtual void emitStatusCodes(TStatusCollection & aStatusCollection, int aExtendedStatus = SDK::Driver::EStatus::Actual);

	/// Устройство было перезагружено по питанию?
	bool isPowerReboot();

	/// Состояние окружения устройства изменилось.
	virtual bool environmentChanged();

	/// Подождать готовность.
	bool waitReady(const SWaitingData & aWaitingData, bool aReady = true);

	/// Получить уровень тревожности по буферу статус-кодов.
	virtual SDK::Driver::EWarningLevel::Enum getWarningLevel(const TStatusCollection & aStatusCollection);

	/// Счетчик отсутствия ответа на полловые посылки.
	int mBadAnswerCounter;

	/// Максимальное число демпфируемых полловых запросов c некорректным ответом.
	int mMaxBadAnswers;

	/// Признак принудительного включения буфера статусов.
	bool mForceStatusBufferEnabled;

	/// Версия драйвера.
	QString mVersion;

	/// После полла устройства эмитить статусы и выполнять device-specific действия.
	bool mPostPollingAction;

	/// Теоретически исправимые после переинициализации ошибки.
	TStatusCodes mRecoverableErrors;

	/// Неустойчивые пограничные состояния (не-ошибки), в которых нельзя исправлять исправимые ошибки.
	TStatusCodes mUnsafeStatusCodes;

	/// Устройство протестировано на совместимость.
	bool mVerified;

	/// Mодель соответствует своему плагину.
	bool mModelCompatibility;

	/// Мьютекс для блокировки поллинга при выполнении внешних операций.
	QMutex mExternalMutex;

	/// Мьютекс для блокировки запросов к логическим ресурсам (контейнеры и т.п.).
	mutable QMutex mResourceMutex;

	/// Экземпляр класса-описателя статусов устройства.
	DeviceStatusCode::PSpecifications mStatusCodesSpecification;

	/// Кэш состояний устройства. Фильтрованы только несуществующие, но не несущественные статусы, применен буфер статусов.
	TStatusCollection mStatusCollection;

	/// История состояний устройства. Фильтрованы несуществующие статусы, применен буфер статусов.
	typedef HystoryList<TStatusCollection> TStatusCollectionList;
	TStatusCollectionList mStatusCollectionHistory;

	/// Уровень тревожности для последнего отправленного в пп набора статусов.
	SDK::Driver::EWarningLevel::Enum mLastWarningLevel;

	/// Устройство подключено.
	bool mConnected;

	/// Статус-коды для фильтрации 3-го уровня.
	TStatusCollection mExcessStatusCollection;

	/// Флаг старой прошивки.
	bool mOldFirmware;

	/// Флаг необходимости перезагрузки устройства по питанию.
	bool mNeedReboot;

	/// Количество повторов при инициализации.
	int mInitializeRepeatCount;

	/// Может находиться автопоиском.
	bool mAutoDetectable;

	/// Статусы, наличие которых в ответе на статус запускает работу буфера статусов.
	TStatusCodes mReplaceableStatuses;
};

//---------------------------------------------------------------------------
