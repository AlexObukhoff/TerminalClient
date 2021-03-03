/* @file Принтер Custom VKP-80 III. */

#pragma once

#include "CustomVKP80.h"

//--------------------------------------------------------------------------------
/// Константы и команды Custom VKP-80 III.
namespace CCustomVKP80III
{
	/// Команды.
	namespace Command
	{
		const char EjectorActivation[] = "\x1C\x50";    /// Неизменяемая часть команды активации эжектора.
	}

	/// Параметры работы эжектора.
	namespace EjectorParameters
	{
		const char Blinking   = '\x01';    /// Мигать светодиодами при презентации.
		const char Pushing    = 'E';       /// Выталкивание чека.
		const char Retraction = 'R';       /// Ретракция чека.
	}
}

//--------------------------------------------------------------------------------
template<class T>
class CustomVKP80III : public CustomVKP80<T>
{
	SET_SUBSERIES("CustomVKP80III")

public:
	CustomVKP80III();

protected:
	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Обработка чека после печати.
	virtual bool receiptProcessing();
};

//--------------------------------------------------------------------------------
typedef SerialPOSPrinter<CustomVKP80III<TSerialPrinterBase>> SerialCustomVKP80III;
typedef                  CustomVKP80III<TLibUSBPrinterBase>  LibUSBCustomVKP80III;

//--------------------------------------------------------------------------------
