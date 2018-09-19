/* @file Сканер Honeywell Metrologic на OPOS-драйвере. */

#pragma once

// OPOS
#include <Common/QtHeadersBegin.h>
#pragma warning(disable: 4100) // warning C4100: 'identifier' : unreferenced formal parameter
#include <OPOS/QtWrappers/Scanner.h>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/PollingDeviceBase.h"
#include "Hardware/Common/OPOSPollingDeviceBase.h"
#include "Hardware/HID/ProtoOPOSScanner.h"
#include "Hardware/HID/HIDBase.h"

//--------------------------------------------------------------------------------
/// Константы OPOS-cканеров.
namespace COPOSScanners
{
	/// Величина префикса удаляемых данных.
	const int Prefix = 4;

	/// Таймаут Claim-а, [мс]. Связи с реальностью почти не имеет.
	const int ClaimTimeout = 20 * 60 * 1000;

	/// Количество попыток для Claim-а при определенных ошибках.
	const int ClaimAttempts = 10;
}

//--------------------------------------------------------------------------------
typedef HIDBase<OPOSPollingDeviceBase<ProtoOPOSScanner, OPOS::OPOSScanner>> TPollingOPOSScanner;

class OPOSMetrologicScanner : public TPollingOPOSScanner
{
	SET_SERIES("Honeywell")

	Q_OBJECT

public:
	OPOSMetrologicScanner();

	/// Возвращает список сконфигурированных OPOS устройств.
	static QStringList getProfileNames();

	/// Включает/выключает устройство на чтение штрих-кодов. Пикать все равно будет.
	virtual bool enable(bool aReady);

protected slots:
	/// Вызывается по приходу данных от сканера.
	void onGotData(const QString & aName, int aArgumentsCount, void * aArgumentsValues);

protected:
	/// Инициализировать ресурсы.
	virtual void initializeResources();

	/// Идентифицирует устройство.
	virtual bool isConnected();

	/// Проверка возможности выполнения функционала, предполагающего связь с устройством.
	virtual bool checkConnectionAbility();

	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Получить и обработать статус.
	virtual bool processStatus(TStatusCodes & aStatusCodes);

	/// Включает/выключает устройство.
	bool setAvailable(bool aEnable);

	/// Мьютекс для операций со считанными данными.
	QMutex mDataMutex;

	/// Был включен извне.
	bool mExEnabled;
};

//--------------------------------------------------------------------------------
