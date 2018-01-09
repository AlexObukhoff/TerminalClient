/* @file Принтеры Custom. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// Projects
#include "Hardware/Printers/POSPrinter.h"

//--------------------------------------------------------------------------------
/// Константы, команды и коды состояний принтеров Custom.
namespace CCustomPrinter
{
	namespace Models
	{
		const char TG2480[]  = "Custom TG2480";
		const char TG2480H[] = "Custom TG2480H";
		const char TG2460H[] = "Custom TG2460H";
		const char TL80[]    = "Custom TL80";
		const char TL60[]    = "Custom TL60";
	}

	/// Высота линии изображения в пикселях (1 пиксель == 1 бит).
	const int LineHeight = 24;

	/// Режим загрузки изображения в принтер - 24-х битовыми линиями.
	const char Image24BitMode = '\x21';

	/// Максимальная ширина печатаемой части изображения, [пикс] (1 пикс == 1 бит).
	const int MaxImageWidth = 319;

	//----------------------------------------------------------------------------
	/// Команды.
	namespace Commands
	{
		const char PrintImage[] = "\x1B\x2A";    /// Печать изображения.
	}

	//----------------------------------------------------------------------------
	/// GAM (Graphics advanced mode).
	namespace GAM
	{
		/// Максимальная ширина печатаемой части изображения, [пикс] (1 пикс == 1 бит).
		const int MaxImageWidth = 638;

		/// Получить время печати картинки, [мс].
		inline qint64 getImagePause(const QImage & aImage, const SDK::Driver::TPortParameters & aPortParameters)
		{
			int baudrate = aPortParameters[SDK::Driver::IOPort::COM::EParameters::BaudRate];
			int size = 21 + aImage.height() * qCeil(6.0 + qCeil(aImage.width() / 8.0));    // расчетный размер монохромной картинки в байтах

			return qCeil(1000.0 * size * getFrameSize(aPortParameters) / baudrate);
		}

		/// Команды.
		namespace Commands
		{
			const char SetPageLength[]    = "\x1B\x26\x6C\x30\x52";    /// Установить длину страницы = 0. По размеру?
			const char SetResolution204[] = "\x1B\x2A\x74\x32\x30\x34\x52";    /// Установить разрешение 240 dpi.
			const char SetNoCompression[] = "\x1B\x2A\x62\x30\x4D";    /// Отменить сжатие изображения.
			const char SetLeftMargin[]    = "\x1B\x2A\x70\x58";    /// Установить сдвиг слева.
			const char SendData[]   = "\x1B\x2A\x62\x57";    /// Послать картинку в принтер.
			const char PrintImage[] = "\x1B\x2A\x72\x42";    /// Печать изображения.
		}
	}
}

//--------------------------------------------------------------------------------
class CustomPrinter : public POSPrinter
{
	SET_SUBSERIES("Custom")

public:
	CustomPrinter();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

	/// Устанавливает конфигурацию устройству.
	virtual void setDeviceConfiguration(const QVariantMap & aConfiguration);

protected:
	/// Напечатать картинку протокольным методом по умолчанию.
	bool printImageDefault(const QImage & aImage, const Tags::TTypes & aTags);

	/// Напечатать картинку методом GAM.
	virtual bool printImage(const QImage & aImage, const Tags::TTypes & aTags);
};

//--------------------------------------------------------------------------------
