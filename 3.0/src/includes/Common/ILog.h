/* @file Интерфейс логгера. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

//---------------------------------------------------------------------------
/// Уровень логгирования.
namespace LogLevel
{
	enum Enum
	{
		Off = 0,
		Fatal,
		Error,
		Warning,
		Normal,
		Debug,
		Trace,
		Max = Trace
	};
}

//---------------------------------------------------------------------------
/// Устройство вывода лога.
namespace LogType
{
	enum Enum
	{
		File,
		Console,
		Debug
	};
}

//---------------------------------------------------------------------------
class ILog
{
public:
	/// Возвращает экземпляр лога с направлением по умолчанию.
	static ILog * getInstance();

	/// Возвращает экземпляр лога с заданным направлением.
	static ILog * getInstance(const QString & aName);

	/// Возвращает экземпляр лога с заданными направлением, устройством вывода и минимальным уровнем логирования
	static ILog * getInstance(const QString & aName, LogType::Enum aType);

	/// Переоткрыть все файлы логов
	static void logRotateAll();

	/// Установить уровень логирования для всех логов
	static void setGlobalLevel(LogLevel::Enum aMaxLogLevel);

	/// Возвращает имя экземпляра лога.
	virtual const QString & getName() const = 0;

	/// Возвращает тип вывода данного экземпляра лога.
	virtual LogType::Enum getType() const = 0;

	/// Возвращает направление вывода.
	virtual const QString & getDestination() const = 0;

	/// Устанавливает направление вывода.
	virtual void setDestination(const QString & aDestination) = 0;

	/// Устанавливает минимальный уровень, ниже которого логгирование игнорируется.
	virtual void setLevel(LogLevel::Enum aLevel) = 0;

	/// Возвращает минимальный уровень, ниже которого логгирование игнорируется.
	virtual LogLevel::Enum getLevel() = 0;

	/// Устанавливает уровень отступа для древовидных логов.
	virtual void adjustPadding(int aStep) = 0;

	/// Производит запись в лог.
	virtual void write(LogLevel::Enum aLevel, const QString & aMessage) = 0;

	/// Производит запись в лог c форматированием данных.
	virtual void write(LogLevel::Enum aLevel, const QString & aMessage, const QByteArray & aData) = 0;

	/// Принудительно закрыть журнал. Функция write заново его откроет.
	virtual void logRotate() = 0;

protected:
	virtual ~ILog() {}
};

//---------------------------------------------------------------------------
// Запись в лог с проверкой указателя на лог.
#define LOG(log, level, message)            \
	{                                       \
		if (log != 0)                       \
			log->write(level, message);     \
		else                                \
			qCritical("Log pointer is empty. Message:%s.", qPrintable(message));\
	}

//---------------------------------------------------------------------------
// Запись в лог с именем функции и проверкой указателя на лог.
#define LOGF(log, level, message)                         \
	{                                                     \
		if (log != 0)                                     \
			log->write(level, message + QString(" (%1)")  \
				.arg(Q_FUNC_INFO));                       \
		else                                              \
			qCritical("Log pointer is empty. Message:%s.", qPrintable(message));\
	}

//---------------------------------------------------------------------------
// Запись в лог с именем функции и проверкой указателя на лог.
#define LOGB(log, level, message, binaryData)        \
	{                                          \
		if (log != 0)                          \
			log->write(level, message, binaryData);  \
		else                                   \
			qCritical("Log pointer is empty. Message:%s.", qPrintable(message));\
	}

//---------------------------------------------------------------------------
