/* @file Принтеры Custom модификации III. */

#pragma once

#include "CustomPrinters.h"

//--------------------------------------------------------------------------------
namespace ECustomPrinterEmulation
{
	enum Enum
	{
		None = 0,
		POS,
		TGH 
	};
}

//--------------------------------------------------------------------------------
/// Константы и команды принтеров Custom модификации III.
namespace CCustomPrinterIII
{
	namespace Models
	{
		const char TG2480HIII[]    = "Custom TG2480HIII";
		const char TG2460HIII[]    = "Custom TG2460HIII";
		const char TG2460HIIIEJC[] = "Custom TG2460HIII EJC";
		const char TG1260HIII[]    = "Custom TG1260HIII";
		const char TL80III[]       = "Custom TL80III";
		const char TL60III[]       = "Custom TL60III";
	}

	/// Команды.
	namespace Command
	{
		const char GetModelId[]               = "\x1D\x49\xFF";    /// Получение идентификатора модели.
		const char SetPOSEmulation[]          = "\x1C=EPOS=";      /// Изменить эмуляцию на POS.
		const char SetTGHEmulation[]          = "\x1C=TGHE=";      /// Изменить эмуляцию на TGH.
		const char GetSerialNumber[]          = "\x1C\xEA\x52";    /// Получение серийного номера.
		const char CutBackFeed[]              = "\x1C\xC0\x34";    /// Промотка и отрезка с возможностью обратной промотки.
		const char PerformAntiJammingAction[] = "\x1D\xDC";        /// Выполнить операцию антизамятия.
		const char EnableJammingSensor[]      = "\x1C\xC1\x01";    /// Включить определение замятия бумаги внутри принтера.
	}
}

//--------------------------------------------------------------------------------
template<class T>
class CustomPrinterIII : public CustomPrinter<T>
{
	SET_SUBSERIES("CustomPrinterIII")

public:
	CustomPrinterIII();

protected:
	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Запросить и сохранить параметры устройства.
	virtual void processDeviceData();

	/// Обработка чека после печати.
	virtual bool receiptProcessing();
};

//--------------------------------------------------------------------------------
typedef SerialPOSPrinter<CustomPrinterIII<TSerialPrinterBase>> SerialCustomPrinterIII;
typedef                  CustomPrinterIII<TLibUSBPrinterBase>  LibUSBCustomPrinterIII;

//--------------------------------------------------------------------------------
