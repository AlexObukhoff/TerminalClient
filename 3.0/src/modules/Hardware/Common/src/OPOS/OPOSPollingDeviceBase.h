/* @file Базовый класс устройств на OPOS-функционале. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QWaitCondition>
#include <QtCore/QSharedPointer>
#include <QtCore/QSet>

// OPOS
#pragma warning(disable: 4146) // warning C4146: unary minus operator applied to unsigned type, result still unsigned
#pragma warning(disable: 4100) // warning C4100: 'identifier' : unreferenced formal parameter
#include <OPOS/QtWrappers/Constants.h>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/PollingDeviceBase.h"

// Project
#include "Hardware/Common/WorkingThreadProxy.h"

/// Базовые константы OPOS
namespace COPOS
{
	typedef QSet<QString> TFunctionNames;

	/// Имя функции запроса статуса.
	const char Status[] = "CheckHealth";

	/// Число попыток в случае непроходимости Claim.
	const int ClaimAttempts = 1;

	template <class T>
	inline std::function<T()> getFunction(std::function<T()> aFunction, std::function<void()> aLogFunction)
	{
		aLogFunction();

		return aFunction;
	}
}

/// Результат выполнения int-OPOS-операции
struct SOPOSResult
{
	int error;
	bool extended;

	SOPOSResult() : error(OPOS::OPOS_SUCCESS), extended(false) {}
	SOPOSResult(int aResult) : error(aResult), extended(false) {}

	bool operator!=(const SOPOSResult & aResult) const
	{
		return (error != aResult.error) || (extended != aResult.extended);
	}
};

//--------------------------------------------------------------------------------
#define OPOS_SUCCESS(aResult) aResult.error == OPOS::OPOS_SUCCESS
#define OPOS_DEBUG_LOG(aMethod) std::bind(&MetaDevice::toLog, this, LogLevel::Debug, QString("    call IOPOSFiscalPrinter::%1,%2line %3").arg(#aMethod).arg(' ', 25 - QString(#aMethod).size()).arg(__LINE__))
#define OPOS_BIND(aMethod, aType, ...) COPOS::getFunction<aType>(std::bind(&TOPOSDriver::aMethod, mDriver.data(), __VA_ARGS__), OPOS_DEBUG_LOG(aMethod))

#define     INT_CALL_OPOS(aMethod, ...) processIntMethod(OPOS_BIND(aMethod, int, __VA_ARGS__), #aMethod##"("##""#__VA_ARGS__##")")
#define    VOID_CALL_OPOS(aMethod, ...) mThreadProxy.invokeMethod<  void >(OPOS_BIND(aMethod,   void , __VA_ARGS__))
#define    BOOL_CALL_OPOS(aMethod, ...) mThreadProxy.invokeMethod<  bool >(OPOS_BIND(aMethod,   bool , __VA_ARGS__))
#define  STRING_CALL_OPOS(aMethod, ...) mThreadProxy.invokeMethod<QString>(OPOS_BIND(aMethod, QString, __VA_ARGS__))

#define    INT_OPOS_PROPERTY(aProperty) mThreadProxy.invokeMethod<  int  >(OPOS_BIND(aProperty, int))
#define STRING_OPOS_PROPERTY(aProperty) mThreadProxy.invokeMethod<QString>(OPOS_BIND(aProperty, QString))

#define toOPOS_LOG(aLevel, aMessage) if (mConnected || !mPollingActive || (mInitialized == ERequestStatus::InProcess)) toLog(aLevel, aMessage)

//--------------------------------------------------------------------------------
template <class T, class T2>
class OPOSPollingDeviceBase : public PollingDeviceBase<T>
{
	SET_INTERACTION_TYPE(OPOS)

public:
	typedef T2 TOPOSDriver;

	OPOSPollingDeviceBase();

#pragma region SDK::Driver::IDevice interface
	/// Подключает и инициализует устройство.
	virtual void initialize();

	/// Освобождает ресурсы, связанные с устройством, возвращается в состояние до вызова initialize().
	virtual bool release();

	/// Переформировывает список параметров для автопоиска и устанавливает 1-й набор параметров из этого списка.
	virtual SDK::Driver::IDevice::IDetectingIterator * getDetectingIterator();
#pragma endregion

#pragma region SDK::Driver::IDetectingIterator interface
	/// Переход к следующим параметрам устройства.
	virtual bool moveNext();

	/// Поиск устройства на текущих параметрах.
	virtual bool find();
#pragma endregion

protected:
	/// Освобождает ресурсы, связанные с устройством, возвращается в состояние до вызова initialize().
	virtual bool performRelease();

	/// Инициализировать ресурсы.
	virtual void initializeResources();

	/// Проверка возможности выполнения функционала, предполагающего связь с устройством.
	virtual bool checkConnectionAbility();

	/// Идентификация.	
	virtual bool checkExistence();

	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Получить статус;
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Вызывает int-метод в рабочем потоке, возвращает и обработывает результат.
	virtual SOPOSResult processIntMethod(TIntMethod aMethod, const QString & aFunctionData);

	/// Закрыть OPOS-драйвер.
	bool close();

	/// Запросить статус;
	bool checkHealth(TStatusCodes & aStatusCodes, SOPOSResult & aResult);

	/// Определяет, что используется определенная фукнция по ее сигнатуре.
	bool functionUse(const COPOS::TFunctionNames & aFunctions, const QString & aFunctionData);

	/// Получить описание ошибки.
	virtual QString getErrorDescription();

	/// OPOS-драйвер.
	typedef QSharedPointer<T2> POPOSDriver;
	POPOSDriver mDriver;

	/// Мьютекс для операций при старте потока.
	QMutex mStartMutex;

	/// Wait-condition для операций при старте потока.
	QWaitCondition mStartCondition;

	/// Флаг ошибки инициализации.
	bool mCOMInitialized;

	/// Последний OPOS-статус.
	SOPOSResult mLastStatus;

	/// Имена OPOS-профилей.
	QStringList mProfileNames;

	/// Имя OPOS-профиля.
	QString mProfileName;

	/// Прослойка для вызова OPOS-функционала в рабочем потоке.
	WorkingThreadProxy mThreadProxy;

	/// Для данных функций не логгировать результаты.
	COPOS::TFunctionNames mNotLogResult;

	/// Таймаут Claim-а.
	int mClaimTimeout;

	/// Пытался поключиться к устройству.
	bool mTriedToConnect;

	/// Признак открытия OPOS-драйвера.
	bool mOpened;
};

//---------------------------------------------------------------------------
