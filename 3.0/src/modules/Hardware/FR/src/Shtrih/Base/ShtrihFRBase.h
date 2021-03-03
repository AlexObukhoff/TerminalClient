/* @file ФР семейства Штрих на порту. */

#pragma once

#include "Hardware/FR/ProtoShtrihFR.h"

//--------------------------------------------------------------------------------
template <class T>
class ShtrihFRBase : public ProtoShtrihFR<T>
{
public:
	ShtrihFRBase();

protected:
	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Печать Z отчета.
	virtual bool performZReport(bool aPrintDeferredReports);

	/// Обработка ответа на предыдущей команды. Автоисправление некоторых ошибок.
	virtual bool processAnswer(const QByteArray & aCommand, char aError);

	/// Снять Z-отчет.
	virtual bool execZReport(bool aAuto);

	/// Проверка готовности фискальника к Z-отчету.
	virtual bool prepareZReport(bool aAuto, QVariantMap & aData);

	/// Распечатать отложенные Z-отчеты.
	bool printDeferredZReports();

	/// Может ли получить количество Z-отчетов в буфере?
	bool canGetZReportQuantity();

	/// Получить количество Z-отчетов в буфере.
	bool getZReportQuantity();

	/// После подачи команды Z-отчета ждем окончания формирования Z-отчета.
	bool waitForChangeZReportMode();
};

//--------------------------------------------------------------------------------
