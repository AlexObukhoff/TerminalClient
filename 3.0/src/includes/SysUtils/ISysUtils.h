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

// Modules
#include <Common/Exception.h>

typedef QSet<int> TStatusCodes;
typedef QSet<QString> TStatusNames;
typedef QMap<QString, TStatusNames> TStatusGroupNames;

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

	/// Получение параметров системного принтера
	static QVariantMap getPrinterData(const QString & aPrinterName);

	/// Проверка принтера на ошибочное состояние
	static void getPrinterStatus(const QString & aPrinterName, TStatusCodes & aStatusCodes, TStatusGroupNames & aGroupNames);

	/// Установить режим печати через очередь
	static bool setPrinterQueuedMode(const QString & aPrinterName, QString & aErrorMessage);

	/// Получить количество всех системных дескрипторов
	static bool getAllProcessHandleCount(quint32 & aCountOfHandles);

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

};

//--------------------------------------------------------------------------------
