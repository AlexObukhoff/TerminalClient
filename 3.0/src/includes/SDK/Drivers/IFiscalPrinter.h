/* @file Интерфейс фискального регистратора. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QList>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/IPrinter.h>
#include <SDK/Drivers/FR/FiscalPrinterCommand.h>
#include <SDK/Drivers/FR/FiscalDataTypes.h>

namespace SDK {
namespace Driver {

class IFiscalPrinter: public IPrinter
{
public:
	/// Сигнал об закрытии смены.
	static const char * FRSessionClosedSignal; // = SIGNAL(FRSessionClosed(const QVariantMap &));

public:
	/// Печать фискального чека.
	virtual bool printFiscal(const QStringList & aStrings, const SPaymentData & aPaymentData, TFiscalPaymentData & aFPData, SDK::Driver::TComplexFiscalPaymentData & aPSData) = 0;

	/// Выполнить Z-отчет [и распечатать отложенные Z-отчеты.
	virtual bool printZReport(bool aPrintDeferredReports) = 0;

	/// Выполнить X-отчет [и распечатать нефискальный чек - баланс].
	virtual bool printXReport(const QStringList & aStrings) = 0;

	/// Выполнить выплату [и распечатать нефискальный чек - инкассацию].
	virtual bool printEncashment(const QStringList & aStrings) = 0;
	virtual bool printEncashment(const QStringList & aStrings, double aAmount) = 0;

	/// Готов ли к обработке данной фискальной команды.
	virtual bool isFiscalReady(bool aOnline, EFiscalPrinterCommand::Enum aCommand = EFiscalPrinterCommand::Sale) = 0;

	/// Открыта ли сессия.
	virtual bool isSessionOpened() = 0;

	/// Находится ли в фискальном режиме.
	virtual bool isFiscal() const = 0;

	/// Является ли онлайновым.
	virtual bool isOnline() const = 0;

protected:
	virtual ~IFiscalPrinter() {}
};

}} // namespace SDK::Driver

//--------------------------------------------------------------------------------

