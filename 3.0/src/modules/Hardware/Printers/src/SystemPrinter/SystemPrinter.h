/* @file Системный принтер. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtGui/QPrinter>
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

	/// Коэффициент масштабирования для изображений.
	const qreal ImageScalingFactor = 0.5;    //0.475 соответствует попиксельной печати 1:1 для Custom VKP-80

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
			appendSingle(Tags::Type::Image,     "", "<div align='center'><img src='data:image/png;base64,", "'/></div>");
		}
	};
}

typedef PrinterBase<PollingDeviceBase<ProtoPrinter>> TSystemPrinter;

//--------------------------------------------------------------------------------
class SystemPrinter : public TSystemPrinter
{
public:
	SystemPrinter();

	/// Подключает и инициализует устройство. Обертка для вызова функционала в рабочем потоке.
	virtual void initialize();

protected:
	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Проверка принтера на ошибочное состояние
	void getPrinterStatus(const QString & aPrinterName, TStatusCodes & aStatusCodes, TStatusGroupNames & aGroupNames) const;

	/// Напечатать чек.
	virtual bool printReceipt(const Tags::TLexemeReceipt & aLexemeReceipt);

	/// Qt-принтер.
	QPrinter mPrinter;

	/// Боковой отступ.
	qreal mSideMargin;

	/// Боковой отступ.
	TStatusGroupNames mLastStatusesNames;

	/// Отключенные/некорректно работающие системные атрибуты драйвера принтера.
	QSet<ulong> mAttributesDisabled;

	/// Идентификационное имя для автопоиска. Получается из .
	QString mIdName;
};

//--------------------------------------------------------------------------------
