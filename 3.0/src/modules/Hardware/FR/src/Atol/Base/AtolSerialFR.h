/* @file ФР семейства АТОЛ на COM-порту. */

#pragma once

#include "Atol2FRBase.h"

//--------------------------------------------------------------------------------
class AtolSerialFR : public Atol2FRBase
{
public:
	AtolSerialFR();

protected:
	/// Проверить параметры налога.
	virtual bool checkTax(SDK::Driver::TVAT aVAT, CFR::Taxes::SData & aData);

	/// Получить общие для всех ФР статусы.
	virtual bool getCommonStatus(TStatusCodes & aStatusCodes);

	/// Получить короткий статус.
	virtual bool getShortStatus(TStatusCodes & aStatusCodes);

	/// Выполнить команду.
	virtual TResult performCommand(const QByteArray & aCommandData, QByteArray & aAnswer, int aTimeout);

	/// Обработка ответа на предыдущей команды. Автоисправление некоторых ошибок.
	virtual bool processAnswer(const QByteArray & aCommand, char aError);

	/// Запросить и вывести в лог критичные параметры ФР.
	virtual void processDeviceData();

	/// Войти в расширенный режим снятия Z-отчетов.
	virtual bool enterExtendedMode();

	/// Получить ключ модели для идентификации.
	virtual CAtolFR::TModelKey getModelKey(const QByteArray & aAnswer);

	/// Получить статус ЭКЛЗ.
	void getEKLZStatus(TStatusCodes & aStatusCodes);

	/// Можно ли смысл запрашивать статус ЭКЛЗ.
	bool canPerformEKLZRequest();

	/// Установить флаги по ошибке в ответе.
	virtual void setErrorFlags(const QByteArray & aCommand, char aError);

	/// Критична ли ошибка ЭКЛЗ.
	bool isEKLZErrorCritical(const char aError, bool aIsDirectRequest = false) const;
};

//--------------------------------------------------------------------------------
