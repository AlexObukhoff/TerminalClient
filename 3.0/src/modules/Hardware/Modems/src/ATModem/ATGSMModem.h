/* @file AT-совместимый модем. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QCoreApplication>
#include <Common/QtHeadersEnd.h>

// Project
#include "ATModemBase.h"
#include "ATData.h"

//--------------------------------------------------------------------------------
namespace ENetworkAccessability
{
	enum Enum
	{
		NotRegistered,
		RegisteredHomeNetwork,
		SearchingOperator,
		RegistrationDenied,
		Unknown,
		RegisteredRoaming
	};
}

//--------------------------------------------------------------------------------
class ATGSMModem : public ATModemBase
{
public:
	ATGSMModem();

#pragma region IDevice interface
	/// Устанавливает конфигурацию устройству.
	virtual void setDeviceConfiguration(const QVariantMap & aConfiguration);
#pragma endregion

#pragma region IModem interface
	/// Сброс.
	virtual bool reset();

	/// Получение оператора.
	virtual bool getOperator(QString & aOperator);

	/// Получение качество сигнала.
	virtual bool getSignalQuality(int & aSignalQuality);

	/// Получение информации об устройстве
	virtual bool getInfo(QString & aInfo);

	/// Выполнить USSD-запрос.
	virtual bool processUSSD(const QString & aMessage, QString & aAnswer);

	/// Послать SMS.
	virtual bool sendMessage(const QString & aPhone, const QString & aMessage);

	/// Забрать из модема полученные SMS
	virtual bool takeMessages(TMessages & aMessages);
#pragma endregion

protected:
	/// Получение статуса
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Получить состояние регистрации в сети
	bool getNetworkAccessability(ENetworkAccessability::Enum & aNetworkAccessability);

	/// Функция ожидания доступности GSM сети
	bool waitNetworkAccessability(int aTimeout);

	/// Декодирование ответа +CUSD команды
	bool getCUSDMessage(const QByteArray & aBuffer, QString & aMessage);

	/// Разбор имени модема
	virtual void setDeviceName(const QByteArray & aFullName);

	/// Получить инфо о SIM-карте
	void getSIMData(const QByteArray & aCommand);

	bool parseFieldInternal(const QByteArray & aBuffer, const QString & aFieldName, QString & aValue);
	bool getSiemensCellList(QString & aValue);
	bool getSimCOMCellList(QString & aValue);

	/// Определение диалекта модема
	AT::EModemDialect::Enum mGsmDialect;
};

//--------------------------------------------------------------------------------
