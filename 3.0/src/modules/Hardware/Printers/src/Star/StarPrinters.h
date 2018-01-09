/* @file Принтеры семейства Star. */

#pragma once

#include "Hardware/Printers/SerialPrinterBase.h"
#include "StarMemorySwitches.h"

//--------------------------------------------------------------------------------
class StarPrinter : public TSerialPrinterBase
{
	SET_SERIES("STAR")

public:
	StarPrinter();

	/// Задаёт лог.
	virtual void setLog(ILog * aLog);

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

protected:
	/// Анализирует коды статусов устройства и фильтрует лишние.
	virtual void cleanStatusCodes(TStatusCodes & aStatusCodes);

	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Инициализация регистров устройства.
	bool initializeRegisters();

	/// Напечатать чек.
	virtual bool printReceipt(const Tags::TLexemeReceipt & aLexemeReceipt);

	/// После подачи команды, связанной с печатью ждем окончания печати.
	bool waitForPrintingEnd();

	/// Сброс.
	bool reset();

	/// Установить параметры мем-свича.
	bool setMemorySwitch(int aSwitch, ushort aValue);

	/// Получить параметры мем-свича.
	bool getMemorySwitch(int aSwitch, ushort & aValue);

	/// Прочитать мем-свичей.
	void getMemorySwitches();

	/// Обновить мем-свичи.
	bool updateMemorySwitches(const CSTAR::TMemorySwitches & aMemorySwitches);

	/// Получить ответ на команду мем-свича.
	bool readMSWAnswer(QByteArray & aAnswer);

	/// Получить ответ на ASB статус.
	bool readASBAnswer(QByteArray & aAnswer, int & aLength);

	/// Получить ответ на запрос идентификации.
	bool readIdentificationAnswer(QByteArray & aAnswer);

	/// Напечатать картинку.
	virtual bool printImage(const QImage & aImage, const Tags::TTypes & aTags);

	/// Подождать состояния эжектора.
	bool waitEjectorState(bool aBusy);

	/// Находится ли бумага в презентере.
	bool isPaperInPresenter();

	/// Наложить маску на 1 байт и сдвинуть.
	inline int shiftData(const QByteArray aAnswer, int aByteNumber, int aSource, int aShift, int aDigits) const;

	/// Мем-свичи.
	CSTAR::TMemorySwitches mMemorySwitches;

	/// Экземпляр утилитного класса для работы с мем-свичами.
	CSTAR::MemorySwitches::Utils mMemorySwitchUtils;

	/// Модели данной реализации.
	QStringList mModels;

	/// Флаг ошибки инициализации.
	bool mInitializationError;

	/// Время работы буфера статусов после печати.
	QDateTime mStartPrinting;

	/// Необходимо забрать чек из презентера.
	bool mNeedPaperTakeOut;

	/// Флаг полного поллинга.
	bool mFullPolling;
};

//--------------------------------------------------------------------------------
