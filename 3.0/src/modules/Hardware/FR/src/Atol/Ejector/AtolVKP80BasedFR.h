/* @file ФР на базе Custom VKP-80 на протоколе АТОЛ. */

#pragma once

#include "AtolEjectorFR.h"

//--------------------------------------------------------------------------------
/// Константы ФР PayVKP-80.
namespace CAtolVKP80BasedFR
{
	/// Максимальное количество попыток переинициализации принтера в случае возможности такого выхода из ошибки.
	const int MaxReInitPrinterCount = 3;

	/// Переинициализация принтера Custom VKP-80.
	namespace ReInitialization
	{
		/// Данные комманды.
		const char Data[] = "\x8F\x1B\x40";

		/// Таймаут, [мс].
		const int Timeout = 2000;
	}

	/// Минимальная длина презентации
	const char MinPresentationLength = 2;

	/// Максимальная длина презентации
	const char MaxPresentationLength = 14;
}

//--------------------------------------------------------------------------------
template <class T>
class AtolVKP80BasedFR : public AtolEjectorFR<T>
{
public:
	AtolVKP80BasedFR();

	/// Устанавливает конфигурацию устройству.
	virtual void setDeviceConfiguration(const QVariantMap & aConfiguration);

protected:
	/// Получить общие для всех ФР статусы.
	virtual bool getCommonStatus(TStatusCodes & aStatusCodes);

	/// Напечатать [и выдать] чек.
	virtual bool processReceipt(const QStringList & aReceipt, bool aProcessing = true);

	/// Печать фискального чека.
	virtual bool performFiscal(const QStringList & aReceipt, const SDK::Driver::SPaymentData & aPaymentData, quint32 * aFDNumber = nullptr);

	/// Печать Z отчета.
	virtual bool performZReport(bool aPrintDeferredReports);

	/// Печать выплаты.
	virtual bool processPayout(double aAmount);

	/// Локальная печать X-отчета.
	virtual bool processXReport();

	/// Аварийно завершить фискальный(?) документ в случае ошибки фискальной части.
	virtual void cancelDocument(bool aDocumentIsOpened);

	/// Открыть чек.
	virtual bool openDocument(SDK::Driver::EPayOffTypes::Enum aPayOffType);

	/// Установить режим снятия Z-отчета.
	virtual bool setEjectorMode(char aEjectorMode);

	/// Переинициализировать принтер.
	bool reInitPrinter();

	/// Параметры презентера и ретрактора.
	CEjectorAtolFR::SData mEjectorSettings;

	/// Параметры презентера и ретрактора.
	char mEjectorMode;
};

//--------------------------------------------------------------------------------
