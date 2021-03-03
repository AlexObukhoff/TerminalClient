/* @file Обертка над оберткой АТОЛа над драйвером библиотеки fptr. */

#pragma once

// STL
#include <string>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QSharedPointer>
#include <Common/QtHeadersEnd.h>

// ATOL SDK
#include "atol5/wrappers/cpp/fptr10/fptr10.h"
#include "atol5/include/libfptr10.h"

// Common
#include <Common/ILogable.h>

// Project
#include "../Atol5DataTypes.h"

//--------------------------------------------------------------------------------
/// Логи.
namespace CAtol5Logs
{
	/// Имя файла шаблона параметров логов. LPT == log properties template.
	const char LPTFileName[] = "log_properties_template.txt";

	/// Имя файла параметров логов. LP == log properties.
	const char LPFileName[] = "fptr10_log.properties";

	/// Системная переменная АТОЛ DTO для логов.
	const char DTOLogEV[] = "DTO10_LOG_CONFIG_FILE";

	/// Теги.
	namespace Tags
	{
		const char Type[] = "%TYPE%";    /// Тип (уровень) логов.
		const char Path[] = "%PATH%";    /// Путь к папке логов.
		const char Date[] = "%DATE%";    /// Дата в названии файла лога.
	}

	/// Типы (уровни) логов.
	namespace Types
	{
		const char Error[] = "ERROR";    /// Только ошибки.
		const char Info[]  = "INFO";     /// Базовое.
		const char Debug[] = "DEBUG";    /// Расширенное.
	}

	/// Формат представления даты для имени файла лога.
	const char FileDateFormat[] = "yyyy.MM.dd";
}

//--------------------------------------------------------------------------------
class AtolDriverWrapper: public ILogable
{
public:
	AtolDriverWrapper();

	/// Инициализирует.
	bool initialize(const QString & aLibraryPath);

	/// Инициализирует данные логов.
	bool initializeLogData(const QString & aUserPath, const QString & aLogPath, bool aDebug);

	/// Освобождает.
	void release();

	/// Инициализирован?
	bool isInitialized();

	/// Готов?
	bool isReady();

	/// Предоставляет константный доступ к обертке драйвера АТОЛ.
	Atol::Fptr::Fptr * const operator()(bool aEnqueue = true);

	/// Установить соединение.
	bool connect();

	/// Разорвать соединение.
	bool disconnect();

	/// Устанавливает Single параметр.
	template <class T>
	void setSetting(const std::wstring & aKey, const T & aValue);

	/// Получает Single параметр.
	template <class T>
	T getSetting(const std::wstring & aKey);

	/// Проверяет Single параметр. Возвращает true, если параметр был изменен.
	template <class T>
	bool checkSetting(const std::wstring & aKey, const T & aValue);

	/// Устанавливает параметр драйвера.
	template <class T>
	void setDriverParameter(int aKey, const T & aValue);

	/// Получает параметр драйвера.
	template <class T>
	T getDriverParameter(int aKey);

	/// Устанавливает параметры драйвера для работы какого-либо метода.
	void setFRMethodParameters(const CAtol5OnlineFR::TMethodParameters & aParameters);

	/// Получает параметры драйвера для работы какого-либо метода.
	CAtol5OnlineFR::TMethodParameters getFRMethodParameters();

	/// Устанавливает параметр ФР.
	template <class T>
	bool setFRParameter(int aKey, const T & aValue);

	/// Получает параметр ФР.
	template <class T>
	bool getFRParameter(int aKey, T & aValue);

	/// Логгировать последнюю ошщибку.
	bool logLastError(const QString & aLog);

private:
	/// Очистить и проверить на слеши путь.
	QString cleanPath(const QString & aPath, bool aSeparator) const;

	/// Проверить наличие пути/файла.
	bool checkPath(const QString & aPath, const QString & aLog, const QString & aFileName = "") const;

	/// Обертка АТОЛа над драйвером библиотеки fptr.
	typedef QSharedPointer<Atol::Fptr::Fptr> PDriver;
	PDriver mDriver;

	/// Лог.
	ILog * mLog;

	/// Лог.
	CAtol5OnlineFR::MethodParameters mFRMethodParameters;
};

//--------------------------------------------------------------------------------
