/* @file Системный принтер. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtPrintSupport/QPrinter>
#include <QtGui/QBitmap>
#include <QtGui/QTextDocument>
#include <Common/QtHeadersEnd.h>

// Modules
#include "SysUtils/ISysUtils.h"
#include "Hardware/Common/PollingDeviceBase.h"
#include "Hardware/Common/ProtoDevices.h"

// Project
#include "Hardware/Printers/PrinterBase.h"

//--------------------------------------------------------------------------------
/// Константы системного принтера.
namespace CSystemPrinter
{
	/// Тег конца строки.
	const char BRtag[] = "<br>";

	/// Отступ по умолчанию.
	const qreal DefaultMargin = 1.0;

	//----------------------------------------------------------------------------
	/// Теги.
	class TagEngine : public Tags::Engine
	{
	public:
		TagEngine()
		{
			appendSingle(Tags::Type::Italic,    "", "<i>", "</i>");
			appendSingle(Tags::Type::Bold,      "", "<b>", "</b>");
			appendSingle(Tags::Type::UnderLine, "", "<u>", "</u>");
			appendSingle(Tags::Type::Image,     "", "<img src='data:image/png;base64,", "'/>");
		}
	};
}

//--------------------------------------------------------------------------------
class SystemPrinter : public PrinterBase<PollingDeviceBase<ProtoPrinter>>
{
public:
	SystemPrinter();

protected:
	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Напечатать чек.
	virtual bool printReceipt(const Tags::TLexemeReceipt & aLexemeReceipt);

	/// Кутэшный принтер.
	QPrinter mPrinter;

	/// Битмап c картинкой.
	QBitmap mBitmap;

	/// Боковой отступ.
	qreal mSideMargin;

	/// Боковой отступ.
	TStatusGroupNames mLastStatusesNames;
};

//--------------------------------------------------------------------------------
