/* @file Кардридер IDTech. */
#pragma once

// IDTech SDK
#pragma warning(push, 1)
	#include "libIDT_Device.h"
#pragma warning(pop)

// Modules
#include "Hardware/Common/PollingDeviceBase.h"
#include "Hardware/HID/ProtoHID.h"

namespace CIDTechReader
{
	// Имя dll SDK.
	const char DLLSDKName[] = "libIDTechSDK.dll.1.0.16";
}

//--------------------------------------------------------------------------------
typedef PollingDeviceBase<DeviceBase<ProtoHID>> TPollingHID;

class IDTechReader: public TPollingHID
{
	SET_INTERACTION_TYPE(External)
	SET_SERIES("IDTech")

public:
	IDTechReader();
	virtual ~IDTechReader();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

#pragma region SDK::Driver::IHID
	/// Включает/выключает устройство на чтение штрих-кодов. Пикать все равно будет.
	virtual bool enable(bool aEnabled);

	/// Готов ли к работе (инициализировался успешно, ошибок нет).
	virtual bool isDeviceReady();
#pragma endregion

	/// Получить результат чтения и данные MSR-карты.
	void getMSRCardData(int aType, IDTMSRData * aCardData1);

protected:
	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Инициализировать библиотеки.
	bool initializeLibraries();

	/// Проверить наличие и функциональноть библиотеки - получить её версию.
	template <class T>
	bool checkLibrary(const char * aName, const char * aFunctionName, std::function<QString(T)> aFunction);

	/// Зарегистрировать callback.
	template <class T>
	bool registerCallback(HMODULE aHandle, const char * aFunctionName, T aFunction);

	/// Установить callback.
	template <class T>
	bool setCallback(HMODULE aHandle, const char * aFunctionName, T aFunction);

	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Флаг корректной инициализации библиотек.
	bool mLibrariesInitialized;

	/// Id устройства.
	int mId;

private:
	/// Статический эксземпляр класса.
	static IDTechReader * mInstance;

	/// Счетчик эксземпляров класса.
	static int mInstanceCounter;
};

//------------------------------------------------------------------------------
