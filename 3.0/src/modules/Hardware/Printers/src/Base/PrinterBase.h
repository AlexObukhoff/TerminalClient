/* @file Базовый принтер. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtGui/QImage>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/DeviceBase.h"

// Project
#include "Hardware/Printers/PrinterConstants.h"
#include "Hardware/Printers/PrinterStatusCodes.h"
#include "Hardware/Printers/Tags.h"

namespace CPrinter
{
	/// Спец-теги.
	const Tags::TTypes SpecialTags = Tags::TTypes() << Tags::Type::Image << Tags::Type::BarCode;
}

//--------------------------------------------------------------------------------
template <class T>
class PrinterBase : public T
{
public:
	PrinterBase();

	/// Напечатать массив строк.
	virtual bool print(const QStringList & aReceipt);

	/// Готов ли к печати.
	virtual bool isDeviceReady(bool aOnline);

	/// Устанавливает конфигурацию устройству.
	virtual void setDeviceConfiguration(const QVariantMap & aConfiguration);

protected:
	/// Идентифицирует устройство.
	virtual bool isConnected();

	/// Выполнить нереентерабельную команду.
	virtual bool processNonReentrant(TBoolMethod aCommand);

	/// Обработка чека после печати.
	virtual bool receiptProcessing();

	/// Фоновая логика при появлении определенных состояний устройства.
	virtual void postPollingAction(const TStatusCollection & aNewStatusCollection, const TStatusCollection & aOldStatusCollection);

	/// Возможна ли печать.
	virtual bool isPossible(bool aOnline, QVariant aCommand = QVariant());

	/// Напечатать чек.
	virtual bool printReceipt(const Tags::TLexemeReceipt & aLexemeReceipt);

	/// Напечатать [и выдать] чек.
	virtual bool processReceipt(const QStringList & aReceipt, bool aProcessing = true);

	/// Удалить строки, содержащие только ' ', \n, \r и \t.
	QStringList simplifyReceipt(const QStringList & aReceipt);

	/// Возможно ли принудительное включение буфера статусов после выполнения печатной операции.
	virtual bool canForceStatusBufferEnable() { return false; }

	/// Разбивает List строк на составные строки, используя разделители.
	void separate(QStringList & aReceipt) const;

	/// Анализирует коды статусов устройства и фильтрует лишние.
	virtual void cleanStatusCodes(TStatusCodes & aStatusCodes);

	/// Применить спец-тег.
	virtual bool execSpecialTag(const Tags::SLexeme & aTagLexeme);

	/// Разделить список лексем с учетом длины строки.
	void adjustToLineSize(Tags::TLexemesBuffer & aTagLexemes, Tags::TLexemesCollection & aLexemesCollection);

	/// Сформировать построчный список лексем с тегами из списка строк.
	void makeLexemeReceipt(const QStringList & aReceipt, Tags::TLexemeReceipt & aLexemeReceipt);

	/// Очистить диспенсер.
	bool clearDispenser(const QString & aCondition);

	/// Очистить квитанцию от пустых строк (или с пробелами, TAB), прочего.
	void cleanReceipt(QStringList & aReceipt);

	/// Применить теги.
	virtual void execTags(Tags::SLexeme & aTagLexeme, QVariant & aLine);

	/// Промотка.
	virtual bool feed();

	/// Напечатать строку.
	virtual bool printLine(const QVariant & /*aLine*/) { return true; }

	/// Напечатать картинку.
	virtual bool printImage(const QImage & /*aImage*/, const Tags::TTypes & /*aTags*/) { return true; }

	/// Обработка тега bc - печать штрих-кода.
	virtual bool printBarcode(const QString & /*aBarcode*/) { return true; }

	/// Отрезка.
	virtual bool cut()     { return true; }

	/// Презентация чека.
	virtual bool present() { return true; }

	/// Вытолкнуть чек.
	virtual bool push()    { return true; }

	/// Забрать чек в ретрактор.
	virtual bool retract() { return true; }

	/// Можно ли проверять готовность устройства.
	bool canCheckReady(bool aOnline);

	/// Время первого обнаружения бумаги в презентере.
	QDateTime mPaperInPresenter;

	/// Экземляр движка тегов.
	Tags::PEngine mTagEngine;

	/// Ошибки, при которых возможно выполнение определенных команд.
	typedef QMap<int, TStatusCodes> TUnnecessaryErrors;
	TUnnecessaryErrors mUnnecessaryErrors;

	/// Максимальное количество символов в строке, если известно и учитывается.
	int mLineSize;

	/// Необходимость перевода строки при построчной печати.
	bool mLineFeed;

	/// Теги для обработки отдельными командами.
	Tags::TTypes mLineTags;

	/// Количество фактически напечатанных строк.
	int mActualStringCount;

	/// Будет ли следующий документ распечатан сразу.
	bool mNextDocument;
};

//--------------------------------------------------------------------------------
