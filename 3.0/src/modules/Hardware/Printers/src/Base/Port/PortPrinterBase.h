/* @file Базовый принтер с портовой реализацией протокола. */

#pragma once

// Modules
#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/Common/SerialDeviceBase.h"
#include "Hardware/Common/LibUSBDeviceBase.h"
#include "Hardware/Common/TCPDeviceBase.h"
#include "Hardware/Common/ProtoDevices.h"

// Project
#include "Hardware/Printers/PrinterBase.h"

//--------------------------------------------------------------------------------
namespace CPortPrinter
{
	/// Интервал поллинга при ожидании отмораживания по умолчанию, [мс].
	const int WaitingPollingInterval = 100;

	/// Таймаут печати 1 актуальной строки, [мс].
	const int PrintingStringTimeout = 100;
}

struct SPrintingOutData
{
	ILog * log;
	bool receiptProcessing;
	ELoggingType::Enum IOMessageLogging;
	QVariantMap configuration;
	QStringList receipt;

	SPrintingOutData() : log(nullptr), receiptProcessing(false), IOMessageLogging(ELoggingType::ReadWrite) {}
	SPrintingOutData(ILog * aLog, bool aReceiptProcessing, ELoggingType::Enum aIOMessageLogging, const QVariantMap & aConfiguration, const QStringList & aReceipt) :
		log(aLog), receiptProcessing(aReceiptProcessing), IOMessageLogging(aIOMessageLogging), configuration(aConfiguration), receipt(aReceipt) {}
};

//--------------------------------------------------------------------------------
template <class T>
class PortPrinterBase : public T
{
public:
	PortPrinterBase();

	/// Инициализация устройства.
	virtual bool updateParametersOut() { return true; };

	/// Напечатать массив строк из другого драйвера.
	virtual bool printOut(const SPrintingOutData & aPrintingOutData);

protected:
	/// Завершение инициализации.
	virtual void finaliseInitialization();

	/// Отпустить внешний ресурс (порт/драйвер).
	virtual bool releaseExternalResource();

	/// Напечатать чек.
	virtual bool printReceipt(const Tags::TLexemeReceipt & aLexemeReceipt);

	/// Получить ответ.
	virtual bool getAnswer(QByteArray & aAnswer, int aTimeout) const;

	/// Напечатать строку.
	virtual bool printLine(const QVariant & aLine);
	virtual bool printLine(const QByteArray & aString);

	/// Применить теги.
	virtual void execTags(Tags::SLexeme & aTagLexeme, QVariant & aLine);

	/// Отрезка.
	virtual bool cut();

	/// Презентация чека.
	virtual bool present();

	/// Вытолкнуть чек.
	virtual bool push();

	/// Забрать чек в ретрактор.
	virtual bool retract();

	/// Ожидать отвисания принтера до/после печати.
	bool waitAvailable();
};

//--------------------------------------------------------------------------------
