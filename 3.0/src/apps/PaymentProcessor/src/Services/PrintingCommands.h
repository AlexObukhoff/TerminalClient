/* @file Команды печати и формирования чеков. */
#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QCoreApplication>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/DatabaseConstants.h>
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>
#include <SDK/Drivers/FR/FiscalPrinterCommand.h>
#include <SDK/Drivers/IFiscalPrinter.h>

// Project
#include "PrintConstants.h"

namespace FiscalCommand = SDK::Driver::EFiscalPrinterCommand;
namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
namespace CPrintCommands
{
	/// Постфикс имени файла нераспечатанных чеков.
	const char NotPrintedPostfix[] = "_not_printed";

	/// Шаблон имени файла фискального чека.
	const char ReceiptNameTemplate[] = "hhmmsszzz";

	/// Данные фискальных тегов.
	const QStringList FFDataList = QStringList()
		<< CPrintConstants::OpPhone
		<< CPrintConstants::DealerSupportPhone
		<< CPrintConstants::BankPhone
		<< CPrintConstants::BankAddress
		<< CPrintConstants::BankInn
		<< CPrintConstants::BankName;
}

//---------------------------------------------------------------------------
class PrintingService;

//---------------------------------------------------------------------------
/// Комманда для печати чеков определенного типа.
class PrintCommand
{
	Q_DECLARE_TR_FUNCTIONS(PrintCommand)

public:
	PrintCommand(const QString & aReceiptType):
		mReceiptType(aReceiptType) {}
	virtual ~PrintCommand() {}

	/// Проверка возможности печати.
	virtual bool canPrint(SDK::Driver::IPrinter * aPrinter, bool aRealCheck)
	{
		return aPrinter && aPrinter->isDeviceReady(aRealCheck);
	}

	/// Печать.
	virtual bool print(SDK::Driver::IPrinter * aPrinter, const QVariantMap & aParameters) = 0;

	/// Установка шаблона чека.
	void setReceiptTemplate(const QString & aTemplateName);

	/// Возвращает тип чека.
	QString getReceiptType() const { return mReceiptType; }

	/// Получить параметры принтера для печати.
	QVariantMap getPrintingParameters(SDK::Driver::IPrinter * aPrinter);

protected:
	QString mReceiptType;
	QString mReceiptTemplate;
};

//---------------------------------------------------------------------------
/// Печать фискального чека.
class PrintFiscalCommand : public PrintCommand
{
	Q_DECLARE_TR_FUNCTIONS(PrintFiscalCommand)

public:
	PrintFiscalCommand(const QString & aReceiptType, FiscalCommand::Enum aFiscalCommand, PrintingService * aService);

	/// Получить данные для фискальной печати
	static SDK::Driver::SPaymentData getPaymentData(const QVariantMap & aParameters);

protected:
	/// Проверка возможности печати фискального документа.
	bool canFiscalPrint(SDK::Driver::IPrinter * aPrinter, bool aRealCheck);

	/// Получить строки с фискальной информацией чека
	bool getFiscalInfo(QVariantMap & aParameters, QStringList & aReceiptLines, bool aWaitResult);

	FiscalCommand::Enum mFiscalCommand;

	PrintingService * mService;
};

//---------------------------------------------------------------------------
/// Печать платежного чека.
class PrintPayment : public PrintFiscalCommand
{
	Q_DECLARE_TR_FUNCTIONS(PrintPayment)

	SDK::Driver::TFiscalFieldData mFiscalFieldData;

public:
	PrintPayment(const QString & aReceiptType, PrintingService * aService):
		PrintFiscalCommand(aReceiptType, FiscalCommand::Sale, aService) {}

	/// Проверка возможности печати.
	virtual bool canPrint(SDK::Driver::IPrinter * aPrinter, bool aRealCheck);

	/// Печать.
	virtual bool print(SDK::Driver::IPrinter * aPrinter, const QVariantMap & aParameters);

	/// Сформировать фискальный чек через фискальный сервер.
	bool makeFiscalByFR(const QVariantMap & aParameters);

private:
	/// Добавить данные платежа.
	void addFiscalPaymentData(const SDK::Driver::TFiscalPaymentData & aFPData, QStringList & aData);

	bool isFiscal(SDK::Driver::IPrinter * aPrinter);

};

//---------------------------------------------------------------------------
// Команды, реализующие проверку принтера и печать определенного вида чека. В зависимости от типа чека, также могут выполняться вспомогательные
// действия (например сохранение чека в текстовом файле, добавление определенных полей и т.д.)

/// Печать баланса.
class PrintBalance : public PrintFiscalCommand
{
public:
	PrintBalance(const QString & aReceiptType, PrintingService * aService) :
		PrintFiscalCommand(aReceiptType, FiscalCommand::XReport, aService), mFiscalMode(true) {}

	/// Установить признак фискальной печати
	void setFiscal(bool aFiscal) { mFiscalMode = aFiscal; }

	/// Печать.
	virtual bool print(SDK::Driver::IPrinter * aPrinter, const QVariantMap & aParameters);

protected:
	/// Добавляет специфические параметры.
	QVariantMap expandFields(const QVariantMap & aParameters);

	/// Признак фискальной печати
	bool mFiscalMode;
};

//---------------------------------------------------------------------------
/// Печать чека инкассации.
class PrintEncashment : public PrintBalance
{
public:
	PrintEncashment(const QString & aReceiptType, PrintingService * aService);

	/// Печать.
	virtual bool print(SDK::Driver::IPrinter * aPrinter, const QVariantMap & aParameters);
};

//---------------------------------------------------------------------------
/// Печать Z-отчета.
class PrintZReport : public PrintFiscalCommand
{
public:
	PrintZReport(const QString & aReceiptType, PrintingService * aService, bool aFull) :
	  PrintFiscalCommand(aReceiptType, FiscalCommand::ZReport, aService), mFull(aFull) {}

	virtual bool canPrint(SDK::Driver::IPrinter * aPrinter, bool aRealCheck);

	/// Печать.
	virtual bool print(SDK::Driver::IPrinter * aPrinter, const QVariantMap & /*aParameters*/);

private:
	bool mFull;
};

//---------------------------------------------------------------------------
/// Печать нетипизированного чека.
class PrintReceipt : public PrintCommand
{
public:
	PrintReceipt(const QString & aReceiptType, PrintingService * aService):
		PrintCommand(aReceiptType), mService(aService) {}

	/// Печать.
	virtual bool print(SDK::Driver::IPrinter * aPrinter, const QVariantMap & aParameters);

private:
	PrintingService * mService;
};
