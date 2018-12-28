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

/// Состояние сессии.
namespace ESessionState
{
	enum Enum
	{
		Error,     /// Ошибка определения.
		Opened,    /// Открыта.
		Closed,    /// Закрыта.
		Expired    /// Истекла.
	};
}

/// Состояние документа.
namespace EDocumentState
{
	enum Enum
	{
		Error,     /// Ошибка определения.
		Opened,    /// Открыт.
		Closed     /// Закрыт.
	};
}

//--------------------------------------------------------------------------------
class IFiscalPrinter: public IPrinter
{
public:
	/// Сигнал об закрытии смены.
	static const char * FRSessionClosedSignal; // = SIGNAL(FRSessionClosed(const QVariantMap &));

public:
	/// Печать фискального чека.
	virtual bool printFiscal(const QStringList & aStrings, const SPaymentData & aPaymentData, quint32 * aFDNumber = nullptr) = 0;

	/// Получить фискальные теги по номеру документа.
	virtual bool checkFiscalFields(quint32 aFDNumber, TFiscalPaymentData & aFPData, SDK::Driver::TComplexFiscalPaymentData & aPSData) = 0;

	/// Выполнить Z-отчет [и распечатать отложенные Z-отчеты.
	virtual bool printZReport(bool aPrintDeferredReports) = 0;

	/// Выполнить X-отчет [и распечатать нефискальный чек - баланс].
	virtual bool printXReport(const QStringList & aStrings) = 0;

	/// Выполнить выплату [и распечатать нефискальный чек - инкассацию].
	virtual bool printEncashment(const QStringList & aStrings) = 0;
	virtual bool printEncashment(const QStringList & aStrings, double aAmount) = 0;

	/// Готов ли к обработке данной фискальной команды.
	virtual bool isFiscalReady(bool aOnline, EFiscalPrinterCommand::Enum aCommand = EFiscalPrinterCommand::Sale) = 0;

	/// Получить состояние смены.
	virtual ESessionState::Enum checkSessionState() = 0;

	/// Получить состояние документа.
	virtual EDocumentState::Enum checkDocumentState() = 0;

	/// Находится ли в фискальном режиме.
	virtual bool isFiscal() const = 0;

	/// Является ли онлайновым.
	virtual bool isOnline() const = 0;

	/// Может работать с буфером Z-отчетов?
	virtual bool canProcessZBuffer() = 0;

protected:
	virtual ~IFiscalPrinter() {}
};

}} // namespace SDK::Driver

//--------------------------------------------------------------------------------
