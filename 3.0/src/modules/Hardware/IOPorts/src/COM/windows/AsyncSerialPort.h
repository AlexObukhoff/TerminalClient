/* @file Асинхронная Windows-реализация COM-порта. */

#pragma once

// Windows
#include <windows.h>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QVector>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/IOPort/COMParameters.h>

// Project
#include "Hardware/IOPorts/IOPortBase.h"
#include "Hardware/Common/SerialDeviceUtils.h"

//--------------------------------------------------------------------------------
/// Буфер для чтения.
typedef QVector<char> TReadingBuffer;

//--------------------------------------------------------------------------------
/// Константы для работы с асинхронным COM-портом.
namespace CAsyncSerialPort
{
	/// Коэффициент надежности.
	const double KSafety = 1.8;

	/// Коэффициент надежности таймаута фактического времени открытия порта.
	const double KOpeningTimeout = 1.5;

	/// Признаки невозможности ожидания результата GetOverlappedResult.
	const QStringList CannotWaitResult = QStringList()
		<< "FTDI"
		<< "LPC USB VCom Port"
		<< "ATOL"
		<< "MSTAR"
		<< "CP210"
		<< "STMicroelectronics"
		<< "Honeywell"
		<< "ITL"
		<< "USB To Serial Port"
		<< "COM0COM";

	/// Ошибки не логгировать.
	const QVector<int> NoLogErrors = QVector<int>()
		<< ERROR_SUCCESS           /// Операция успешно завершена
		<< ERROR_IO_PENDING        /// Протекает наложенное событие ввода/вывода
		<< ERROR_IO_INCOMPLETE;    /// Наложенное событие ввода/вывода не находится в сигнальном состоянии

	/// Ошибки пропажи порта.
	const QVector<int> DisappearingErrors = QVector<int>()
		<< ERROR_FILE_NOT_FOUND          /// Не удается найти указанный файл
		<< ERROR_DEVICE_NOT_CONNECTED    /// Устройство не подключено
		<< ERROR_ACCESS_DENIED;          /// Отказано в доступе

	/// Таймаут единичного чтения из порта, [мс].
	const int ReadingTimeout = 50;

	/// Таймаут открытия порта, [мс].
	const int OpeningTimeout = 1500;

	/// Таймаут открытия порта в процессе подключения, [мс].
	const int OnlineOpeningTimeout = 5 * 1000;

	/// Пауза для VCOM-портов между ожиданием и чтением данных, [мс].
	const int VCOMReadingPause = 3;
}

#define BOOL_CALL(aFunctionName, ...) [&] () -> bool { if (!checkReady()) return false; \
	return process(std::bind(&::aFunctionName, mPortHandle, __VA_ARGS__), #aFunctionName); } ()

//--------------------------------------------------------------------------------
class AsyncSerialPort : public IOPortBase
{
	SET_SERIES("COM")

	typedef std::function<BOOL()> TBOOLMethod;

public:
	AsyncSerialPort();

	/// Возвращает список доступных в системе портов.
	static QStringList enumerateSystemNames();

	/// Опрашивает данные портов.
	virtual void initialize();

#pragma region SDK::Driver::IDevice
	/// Устанавливает конфигурацию устройству.
	virtual void setDeviceConfiguration(const QVariantMap & aConfiguration);

	/// Освобождает ресурсы, связанные с устройством, возвращается в состояние до вызова initialize().
	virtual bool release();
#pragma endregion

#pragma region SDK::Driver::IIOPort
	/// Открыть порт.
	virtual bool open();

	/// Закрыть порт.
	virtual bool close();

	/// Очистить буферы порта.
	virtual bool clear();

	/// Установить параметры порта.
	virtual bool setParameters(const SDK::Driver::TPortParameters & aParameters);

	/// Получить параметры порта.
	virtual void getParameters(SDK::Driver::TPortParameters & aParameters);

	/// Прочитать данные.
	virtual bool read(QByteArray & aData, int aTimeout = DefaultReadTimeout, int aMinSize = 1);

	/// Передать данные.
	virtual bool write(const QByteArray & aData);

	/// Подключено новое устройство?
	virtual bool deviceConnected();

	/// Открыт?
	virtual bool opened();
#pragma endregion

	/// Изменить таймаут выполнения зависоноопасной операции
	void changePerformingTimeout(const QString & aContext, int aTimeout, int aPerformingTime);

	/// Получить системные свойства устройств.
	TWinDeviceProperties getAsyncDeviceProperties(bool aForce, bool aQuick = false, TIOPortDeviceData * aData = nullptr);

protected:
	/// Идентификация.	
	virtual bool checkExistence();

	/// Открыть порт.
	virtual bool performOpen();

	/// Прочитать данные.
	virtual bool processReading(QByteArray & aData, int aTimeout);

	/// Выполнить команду.
	bool process(TBOOLMethod aMethod, const QString & aFunctionName);

	/// Обработка ошибки.
	void handleError(const QString & aFunctionName);

	/// Логгирование ошибки.
	void logError(const QString & aFunctionName);

	/// Проверить готовность порта.
	virtual bool checkReady();

	/// Ждет окончание асинхронной операции.
	bool waitAsyncAction(DWORD & aResult, int aTimeout);

	/// Управление структурой DCB.
	bool setBaudRate(SDK::Driver::IOPort::COM::EBaudRate::Enum aValue);
	bool setRTS(SDK::Driver::IOPort::COM::ERTSControl::Enum aValue);
	bool setDTR(SDK::Driver::IOPort::COM::EDTRControl::Enum aValue);
	bool setStopBits(SDK::Driver::IOPort::COM::EStopBits::Enum aValue);
	bool setByteSize(int aValue);
	bool setParity(SDK::Driver::IOPort::COM::EParity::Enum aValue);

	/// Запись настроек в порт.
	bool applyPortSettings();

	/// Инициализировать структуру для асинхронного обмена с портом.
	void initializeOverlapped(OVERLAPPED & aOverlapped);

	/// Проверить хэндл порта.
	bool checkHandle();

	/// Порт существует?
	virtual bool isExist();

	/// Служебная структура настроек.
	DCB mDCB;

	/// Хендл порта.
	HANDLE mPortHandle;

	/// Структуры и мьютексы для отслеживания асинхронных чтения/записи.
	QMutex mReadMutex;
	QMutex mWriteMutex;
	OVERLAPPED mReadOverlapped;
	OVERLAPPED mWriteOverlapped;
	DWORD mReadEventMask;

	/// Буфер для чтения.
	TReadingBuffer mReadingBuffer;

	/// Cуществует в системе.
	bool mExist;

	/// Последняя системная ошибка.
	DWORD mLastError;

	/// Последняя ошибка проверки порта.
	DWORD mLastErrorChecking;

	/// Максимальное количество байтов для чтения.
	int mMaxReadingSize;

	/// Системные имена портов.
	QStringList mSystemNames;

	/// Нефильтрованные системные данные портов.
	TWinDeviceProperties mWinProperties;

	/// Системное свойство для формирования пути для открытия порта.
	DWORD mPathProperty;

	/// GUID-ы для автопоиска.
	TUuids mUuids;

	/// Ждать окончания асинхронного чтения из порта, если результат - WAIT_TIMEOUT.
	bool mWaitResult;

	/// Количество прочитанных байтов.
	DWORD mReadBytes;

	/// Мьютекс для защиты статических пропертей портов.
	static QMutex mSystemAsyncPropertyMutex;
};

//--------------------------------------------------------------------------------
