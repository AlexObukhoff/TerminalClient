/* @file Менеджер устройств. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QSet>
#include <QtCore/QStringList>
#include <QtCore/QMap>
#include <QtCore/QMutex>
#include <Common/QtHeadersEnd.h>

// Plugin SDK
#include <SDK/Plugins/IPluginLoader.h>

// Driver SDK
#include <SDK/Drivers/IDevice.h>

#include <Common/ILogable.h>

typedef QMultiMap<SDK::Driver::IDevice *, SDK::Driver::IDevice *> TDeviceDependencyMap;
typedef QPair<QString, SDK::Driver::IDevice *> TNamedDevice;
typedef QList<TNamedDevice> TFallbackDevices;

struct SSimpleSearchDeviceData
{
	QString driverName;
	QStringList * result;
	TFallbackDevices * fallbackDevices;
	QStringList * nonMarketDetectedDriverNames;
	QVariantMap requiredDeviceData;

	SSimpleSearchDeviceData(): fallbackDevices(nullptr), nonMarketDetectedDriverNames(nullptr) {}
	SSimpleSearchDeviceData(QStringList * aResult, TFallbackDevices * aFallbackDevices, QStringList * aNonMarketDetectedDriverNames):
		result(aResult), fallbackDevices(aFallbackDevices), nonMarketDetectedDriverNames(aNonMarketDetectedDriverNames) {}
};

//--------------------------------------------------------------------------------
class DeviceManager : public QObject, public ILogable
{
	Q_OBJECT

public:
	DeviceManager(SDK::Plugin::IPluginLoader * aPluginLoader);
	~DeviceManager();

	/// Инициализирует DeviceManager.
	bool initialize();

	/// Освобождает ресурсы.
	void shutdown();

	/// Ищет все подключенные устройства без сохранения конфигуркции.
	QStringList detect(bool aFast, const QString & aDeviceType);

	/// Подключение/захват устройства, c заданной конфигурацией. По умолчаю конфигурация грузится из конфига плагина.
	SDK::Driver::IDevice * acquireDevice(const QString & aInstancePath, const QString & aConfigPath = "");

	/// Отключает/освобождает указанные устройства.
	void releaseDevice(SDK::Driver::IDevice * aDevice);

	/// Создаёт устройства. Возвращает имя новой конфигурации.
	Q_INVOKABLE TNamedDevice createDevice(const QString & aDriverPath, const QVariantMap & aConfig, bool aDetecting = false);

	/// Сохраняет конфигурацию устройства.
	void saveConfiguration(SDK::Driver::IDevice * aDevice);

	/// Получает список драверов (поддерживаемых устройств).
	QMap<QString, QStringList> getModelList(const QString & aFilter);

	/// Получает список доступных драйверов.
	QStringList getDriverList() const;

	/// Останавливает процесс поиска устройств.
	void stopDetection();

	/// Возвращает список созданных устройств.
	QStringList getAcquiredDevicesList() const;

	/// Устанавливает конфигурацию устройству.
	void setDeviceConfiguration(SDK::Driver::IDevice * aDevice, const QVariantMap & aConfig);

	/// Получает конфигурацию устройства.
	QVariantMap getDeviceConfiguration(SDK::Driver::IDevice * aDevice) const;

	/// Получает список возможных значений для параметров устройств.
	SDK::Plugin::TParameterList getDriverParameters(const QString & aDriverPath) const;

	/// Проверяет правильность путей плагинов драйверов. Workaround для обратной совместимости при накате обновления без автопоиска драйверов, при изменении путей.
	// TODO: убрать после реализации автопоиска через мониторинг.
	void checkInstancePath(QString & aInstancePath);
	void checkITInstancePath(QString & aInstancePath);

	/// Пробует изменить путь плагина драйвера. Workaround для обратной совместимости при накате обновления без автопоиска драйверов, при изменении путей.
	typedef QSet<QString> TNewPaths;
	typedef QMap<QString, TNewPaths> TPaths;
	void changeInstancePath(QString & aInstancePath, const QString & aConfigPath, const TPaths & aPaths);

private:
	/// Пробует найти устройство aDevicePath.
	TNamedDevice findDevice(SDK::Driver::IDevice * aRequired, const QStringList & aDevicesToFind, bool aVCOM = false);

	/// Пробует найти устройство aDriverName, не имеющее зависимых устройств
	void findSimpleDevice(const SSimpleSearchDeviceData & aDeviceData);

	/// Ищет все подключенные устройства, имеющие зависимые устройства, без сохранения конфигуркции.
	void findRRDevices(QStringList & aFoundDevices, bool aVCOM);

	/// Ищет все подключенные устройства, использующие зависимые устройства без сладения ими, без сохранения конфигуркции.
	void findExternalRRDevices(SSimpleSearchDeviceData & aDeviceData);

	/// Было ли устройство данного типа уже найдено?
	bool isDetected(const QString & aConfigName);

	/// Добавляет устройство в список найденных.
	void markDetected(const QString & aConfigName);

	/// Предикат для сортировки списка устройств при автодетекте.
	bool deviceSortPredicate(const QString & aLhs, const QString & aRhs) const;

	/// Устанавливает лог для устройства.
	void setDeviceLog(SDK::Driver::IDevice * aDevice, bool aDetecting);

	/// Получает имя лога простого устройства.
	QString getSimpleDeviceLogName(SDK::Driver::IDevice * aDevice, bool aDetecting);

	/// Получает имя лога устройства.
	QString getDeviceLogName(SDK::Driver::IDevice * aDevice, bool aDetecting);

	/// Логгирует данные зависимых устройств.
	void logRequiedDeviceData();

signals:
	/// Сигнал об обнаружении нового устройства.
	void deviceDetected(const QString & aConfigName, SDK::Driver::IDevice * device);

	/// Сигнал прогресса.
	void progress(int aCurrent, int aMax);

private slots:
	/// Обновляет конфигурацию устройства (без зависимых устройств).
	void onConfigurationChanged();

private:
	SDK::Plugin::IPluginLoader * mPluginLoader;

	/// Список свободных системных зависимых устройств
	QSet<QString> mFreeSystemNames;

	/// Таблица принадлежности драйвера и системного имени зависимого устройства
	QMultiMap<QString, QString> mRDSystemNames;

	/// Таблица имен требуемых ресурсов.
	QMap<QString, QString> mRequiredResources;

	/// Таблица зависимостей.
	TDeviceDependencyMap mDeviceDependencyMap;

	/// Таблица драйверок для конкретного устройства.
	QMap<QString, QStringList> mDriverList;

	/// Флаг остановки поиска устройств.
	char mStopFlag;

	/// Таблица всех возвожных значений параметров устройств.
	QMap<QString, SDK::Plugin::TParameterList> mDriverParameters;

	/// Список типов найденных устройств.
	QSet<QString> mDetectedDeviceTypes;
	QMutex mAccessMutex;

	/// Данные логов устройств.
	typedef QMap<bool, int> TLogData;
	typedef QMap<SDK::Driver::IDevice *, TLogData> TDevicesLogData;
	TDevicesLogData mDevicesLogData;

	/// Список путей всех использованных устройств.
	QStringList mAcquiredDevices;

	/// Список путей всех использованных устройств.
	bool mFastAutoSearching;
};

//--------------------------------------------------------------------------------
