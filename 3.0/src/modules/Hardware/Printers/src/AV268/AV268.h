/* @file Принтер AV-268. */

#pragma once

#include "Hardware/Printers/SerialPrinterBase.h"

//--------------------------------------------------------------------------------
class AV268 : public TSerialPrinterBase
{
	SET_SERIES("SysFuture")

	enum Enum
	{
		Simple,
		Extended,
		Plus,
		Unknown
	};

public:
	AV268();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

	/// Подключает и инициализует устройство.
	virtual void initialize();

protected:
	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Получить статус;
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Печать строки.
	virtual bool printLine(const QByteArray & aString);

	/// Обработка чека после печати.
	virtual bool receiptProcessing();

	/// Получение ответа, задержка перед чтением ответа в мс.
	virtual bool getAnswer(QByteArray & aAnswer, bool aNeedDelay = false);

	/// Выполнение команды.
	bool processCommand(const QByteArray & aCommand, QByteArray * aAnswer = 0);

	/// Ожидание очистки буфера принтера.
	bool waitBufferClearing();

	/// Флаг переполнения буфера.
	bool mOverflow;

	/// Флаг инициализации, для понижения уровня надежности определения устройства.
	bool mInitialize;

	/// Допустимые настройки презентера и ретрактора.
	Enum mModelType;
};

//--------------------------------------------------------------------------------
