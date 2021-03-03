/* @file Интерфейс к SysUtils. Нужно для функций, специфичных для конкретной ОС. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QSet>
#include <QtCore/QDateTime>
#include <QtCore/QProcess>
#include <QtCore/QDebug>
#include <QtCore/QVariantMap>
#include <QtGui/QWidget>
#include <Common/QtHeadersEnd.h>

// Common
#include <Common/ILog.h>

// Modules
#include <Common/Exception.h>

//--------------------------------------------------------------------------------
class Ilog;

typedef QSet<int> TStatusCodes;
typedef QSet<QString> TStatusNames;
typedef QMap<QString, TStatusNames> TStatusGroupNames;

typedef QList<ulong> TJobStatus;

struct SPrinterData;

//--------------------------------------------------------------------------------
class ISysUtils
{
public:
	/// Перезагрузить систему. Возвращает код ошибки или 0 - в случае успеха.
	static int systemReboot();

	/// Выключить терминал. Возвращает код ошибки или 0 - в случае успеха.
	static int systemShutdown();

	/// Получить строку с полным описанием версии операционной системы
	static QString getOSVersionInfo();

	/// Выключить системный скринсейвер
	static void disableScreenSaver();

	/// Включение/выключение дисплея
	static void displayOn(bool aOn);

	/// Принудительно запустить системный скринсейвер
	static void runScreenSaver();

	/// Установить локальное время системы
	static void setSystemTime(QDateTime aDateTime) throw (...);

	/// Suspends the execution of the current thread until the time-out interval elapses.
	static void sleep(int aMs);

	/// Получение инфо о принтере
	static bool getPrinterInfo(ILog * aLog, const QString & aPrinterName, SPrinterData & aPrinterData);

	/// Получение данных параметров системного принтера
	static bool getPrinterStatusData(ILog * aLog, const QString & aPrinterName, TJobStatus & aJobStatus, ulong & aStatus, ulong & aAttributes);

	/// Получение параметров системного принтера
	static QVariantMap getPrinterData(ILog * aLog, const QString & aPrinterName);

	/// Установить режим печати через очередь
	static bool setPrintingQueuedMode(ILog * aLog, const QString & aPrinterName);

	/// Получить количество всех системных дескрипторов
	static bool getAllProcessHandleCount(quint64 & aCountOfHandles);

	/// Напечатать лог последней ошибки
	static void makeLog(ILog * aLog, const QString & aFunctionName);

	/// Сформировать ошибку для утилитарной логики. Возвращает всегда false
	static bool makeError(ILog * aLog, const QString & aFunctionName, void * aPrinter = 0);

	struct MemoryInfo
	{
		qint64 total;
		qint64 totalUsed;
		qint64 processUsed;

		MemoryInfo() : total(0), totalUsed(0), processUsed(0) { }
	};

	/// Получить количество памяти, используемое процессом
	static bool getProcessMemoryUsage(MemoryInfo & aMemoryInfo, const QProcess * aProcess = nullptr);

	struct SSignerInfo
	{
		QString name;
		QString issuer;
		QString serial;

		void clear() { name.clear(); issuer.clear(); serial.clear(); }
	};

	///
	static qlonglong verifyTrust(const QString & aFile);

	/// Получить из сертификата информацию о подписчике
	static bool getSignerInfo(const QString & aFile, SSignerInfo & aSigner);

	/// Переместить окно поверх всех окон
	static bool bringWindowToFront(WId aWindow);
	static bool bringWindowToFront(const QString & aWindowTitle);

	/// Информация о процессе
	struct SProcessInfo
	{
		quint64 id;
		QString path;
		quint64 memoryUsage;
		quint64 handlers;

		SProcessInfo() : id(0), memoryUsage(0), handlers(0) {}
	};

	/// Получить список работающих процессов
	typedef QList<SProcessInfo> TProcessInfo;
	static TProcessInfo getAllProcessInfo();

	/// Удалить сигнатуру BOM из файла
	/// WORKAROUND для QTBUG-23381
	static QString rmBOM(const QString & aFile);

	/// Получить описание последней системной ошибки.
	static QString getLastErrorMessage();

	/// Получить описание системной ошибки.
	static QString getErrorMessage(ulong aError, bool aNativeLanguage = true);
};

//--------------------------------------------------------------------------------
