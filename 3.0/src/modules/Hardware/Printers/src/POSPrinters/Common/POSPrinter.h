/* @file POS-принтер. */

#pragma once

#include "Hardware/Printers/PortPrintersBase.h"
#include "Hardware/Printers/POSPrinterData.h"
#include "POSParameters.h"

//--------------------------------------------------------------------------------
template <class T>
class POSPrinter : public T
{
	SET_SERIES("POS")

public:
	POSPrinter();

protected:
	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Получить Id модели.
	virtual bool getModelId(QByteArray & aAnswer);

	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Печать строки.
	virtual bool printLine(const QByteArray & aString);

	/// Печать штрих-кода.
	virtual bool printBarcode(const QString & aBarcode);

	/// Напечатать картинку.
	virtual bool printImage(const QImage & aImage, const Tags::TTypes & aTags);

	/// Запросить и сохранить параметры устройства.
	virtual void processDeviceData();

	/// Получить ответ на запрос статуса.
	bool readStatusAnswer(QByteArray & aAnswer, int aTimeout, int aBytesCount) const;

	/// Проверка верифицированности модели.
	void checkVerifying();

	/// Выполнение предварительных операций перед печатью штрих-кода.
	QByteArray prepareBarcodePrinting();

	/// В плагине только дефолтные модели?
	bool isOnlyDefaultModels();

	/// Установить ID модели.
	void setModelID(char aModelId);

	/// Данные моделей.
	POSPrinters::ModelData mModelData;

	/// ID модели.
	QByteArray mModelID;

	/// Флаг переполнения буфера.
	bool mOverflow;

	/// Русская кодовая страница.
	char mRussianCodePage;

	/// Таймаут печати строки.
	int mPrintingStringTimeout;

	/// Параметры.
	POSPrinters::SParameters mParameters;
};

//--------------------------------------------------------------------------------
