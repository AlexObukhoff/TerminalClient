/* @file Принтеры MStar. */

#pragma once

#include "Hardware/FR/SerialFRBase.h"
#include "Hardware/Protocols/FR/IncotexFR.h"

//--------------------------------------------------------------------------------
class MStarPrinters : public SerialFRBase
{
	SET_SERIES(ProtocolNames::FR::Incotex)

public:
	MStarPrinters();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

protected:
	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Получить статус;
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Напечатать [и выдать] чек.
	virtual bool processReceipt(const QStringList & aReceipt, bool aProcessing = true);

	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Напечатать строку.
	virtual bool printLine(const QByteArray & aString);

	/// Фискальная печать.
	virtual bool performFiscal(const QStringList & aReceipt, double aAmount);

	/// Печать Z отчета.
	virtual bool performZReport(bool aPrintDeferredReports);

	/// Печать инкассации.
	virtual bool performEncashment(const QStringList & aReceipt);

	/// Обработка чека после печати.
	virtual bool receiptProcessing();

	/// Отрезка.
	virtual bool cut();

	/// Локальная печать X-отчета.
	virtual bool processXReport();

	/// Печать выплаты.
	virtual bool processPayout();

	/// Идентифицировать модель.
	void identify(const CIncotexFR::SUnpackedData & aUnpackedData);

	/// Вернуть стандартный код ошибки.
	void standartCodeError(CIncotexFR::SUnpackedData & aUnpacketAnswer, TStatusCodes & aStatusCodes);

	/// Установить режим работы принтер/ФР.
	bool setMode(EFRMode::Enum aMode);

	/// Открыта ли сессия.
	virtual bool isSessionOpened();

	/// Режим работы.
	EFRMode::Enum mMode;
};

//--------------------------------------------------------------------------------
