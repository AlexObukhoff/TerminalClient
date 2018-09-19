/* @file Принтеры Fujitsu на контроллере Trentino FTP-609. */

#pragma once

#include "Hardware/Printers/SerialPrinterBase.h"

//--------------------------------------------------------------------------------
class FujitsuPrinter : public TSerialPrinterBase
{
public:
	FujitsuPrinter();
	virtual ~FujitsuPrinter();

protected:
	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Инициализация устройства.
	virtual bool updateParameters();

private:
	/// Выполнить команду.
	bool processCommand(const QByteArray & aCommand, QByteArray * aAnswer = 0);

	/// Нужен ли ответ на команду.
	bool isNeedAnswer(const QByteArray & aCommand) const;
};

//--------------------------------------------------------------------------------
