/* @file Базовый класс устройств на порту. */

#pragma once

// SDK
#include <SDK/Drivers/IIOPort.h>

// Modules
#include "Hardware/IOPorts/IOPortStatusesDescriptions.h"

// Project
#include "Hardware/Common/LoggingType.h"
#include "Hardware/Common/DeviceBase.h"
#include "Hardware/Common/CommandResults.h"

//--------------------------------------------------------------------------------
template <class T>
class PortDeviceBase : public T
{
public:
	PortDeviceBase();

#pragma region SDK::Driver::IDevice interface
	/// Освобождает ресурсы, связанные с устройством, возвращается в состояние до вызова initialize().
	virtual bool release();

	/// Задаёт лог.
	virtual void setLog(ILog * aLog);

	/// Устанавливает конфигурацию устройству.
	virtual void setDeviceConfiguration(const QVariantMap & aConfiguration);
#pragma endregion

protected:
	/// Идентификация.
	virtual bool checkExistence();

	/// Завершение инициализации.
	virtual void finaliseInitialization();

	/// Запрос статуса.
	virtual bool processStatus(TStatusCodes & aStatusCodes);

	/// Проверить порт.
	virtual bool checkPort() { return false; }

	/// Полл.
	virtual void doPoll(TStatusCodes & aStatusCodes);

	/// Отправить статус-коды.
	virtual void emitStatusCodes(TStatusCollection & aStatusCollection, int aExtendedStatus = SDK::Driver::EStatus::Actual);

	/// Выполнить команду.
	TResult processCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer = nullptr);
	TResult processCommand(char aCommand, QByteArray * aAnswer = nullptr);
	TResult processCommand(const QByteArray & aCommand, QByteArray * aAnswer = nullptr);
	TResult processCommand(char aCommand, const QByteArray & aCommandData, QByteArray * aAnswer = nullptr);

	/// Выполнить команду.
	virtual TResult execCommand(const QByteArray & /*aCommand*/, const QByteArray & /*aCommandData*/, QByteArray * /*aAnswer*/ = nullptr) { return CommandResult::Driver; }

	/// Получить переводы новых статус-кодов для последующей их обработки.
	virtual QString getTrOfNewProcessed(const TStatusCollection & aStatusCollection, SDK::Driver::EWarningLevel::Enum aWarningLevel);

	/// Получить спецификацию статуса.
	virtual SStatusCodeSpecification getStatusCodeSpecification(int aStatusCode);

	/// Проверка возможности применения буфера статусов.
	virtual bool canApplyStatusBuffer();

	/// Получить уровень тревожности по буферу статус-кодов.
	virtual SDK::Driver::EWarningLevel::Enum getWarningLevel(const TStatusCollection & aStatusCollection);

	/// Добавить данные порта в данные устройства.
	void addPortData();

	/// Проверка наличия ошибки связи с устройством.
	bool checkError(int aError, TBoolMethod aChecking, const QString & aErrorLog);

	/// Порт ввода-вывода.
	SDK::Driver::IIOPort * mIOPort;

	/// Ошибки порта.
	TStatusCodes mIOPortStatusCodes;

	/// Логгирование посылок в классе порта.
	ELoggingType::Enum mIOMessageLogging;

	/// Экземпляр класса-описателя статусов портов.
	IOPortStatusCode::CSpecifications mIOPortStatusCodesSpecification;

	/// Контролировать включение/выключение.
	bool mControlRemoving;
};

//---------------------------------------------------------------------------
