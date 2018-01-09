/* @file Плагин сценария главного меню платёжной книжки */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMap>
#include <QtCore/QSharedPointer>
#include <Common/QtHeadersEnd.h>

// Modules
#include <ScenarioEngine/Scenario.h>

// SDK
#include <SDK/Plugins/IFactory.h>

// Project
#include "RemoteService.h"

class IApplication;
class ReportBuilder;

namespace SDK { 
	namespace PaymentProcessor {
		class ICore;
		
		namespace Scripting
		{
			class PaymentService;
		}
	}

	namespace Plugin {
		class IEnvironment;
	}

	namespace Driver {
		class IDevice;
	}
}

//---------------------------------------------------------------------------
class FirmwareUploadScenario : public GUI::Scenario
{
	Q_OBJECT

public:
	FirmwareUploadScenario(IApplication * mApplication);
	virtual ~FirmwareUploadScenario();

public:
	/// Запуск сценария.
	virtual void start(const QVariantMap & aContext);

	/// Остановить сценарий.
	virtual void stop();
	
	/// Приостановить сценарий с возможностью последующего возобновления.
	virtual void pause();

	/// Продолжение после паузы.
	virtual void resume(const QVariantMap & aContext);

	/// Инициализация сценария.
	virtual bool initialize(const QList<GUI::SScriptObject> & aScriptObjects);

	/// Обработка сигнала из активного состояния с дополнительными аргументами.
	virtual void signalTriggered(const QString & aSignal, const QVariantMap & aArguments = QVariantMap());

	/// Обработчик таймаута
	virtual void onTimeout();

	/// Возвращает false, если сценарий не может быть остановлен в текущий момент.
	virtual bool canStop();

protected:
	void timerEvent(QTimerEvent * aEvent);

	/// Заблокировать GUI
	void lockGUI();

	/// Оборвать с ошибкой выполнение сценария
	void abortScenario(const QString & aErrorMessage);

	/// Почистить за собой файл прошивки
	void cleanFirmwareArtifacts();

private slots:
	/// Обработчик события прошивки устройства
	void onUpdated(bool aSuccess);

	/// Запуск прошивки устройства
	void acquireDevice();

	/// Получение сигнала что устройство успешно инициализированно
	void onDeviceInitialized();

public slots:
	/// Текущее состояние.
	virtual QString getState() const;

private:
	IApplication * mApplication;
	ReportBuilder * mReportBuilder;
	RemoteService::UpdateCommand mCommand;
	QByteArray mFirmware;
	int mRetryCount;

	SDK::Driver::IDevice * mDevice;

	/// Таймер ожидания инициализации устройства
	int mDeviceInitializedTimer;
};

//--------------------------------------------------------------------------

