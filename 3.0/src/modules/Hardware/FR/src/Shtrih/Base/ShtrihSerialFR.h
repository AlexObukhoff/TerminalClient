/* @file ФР семейства Штрих на COM-порту. */

#pragma once

#include "ShtrihFRBase.h"

//--------------------------------------------------------------------------------
typedef ShtrihFRBase<ShtrihSerialFRBase> TShtrihSerialFRBase;

class ShtrihSerialFR : public TShtrihSerialFRBase
{
public:
	ShtrihSerialFR();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

protected:
	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Ошибка?
	virtual bool isNotError(char aCommand);

	/// Обработка ответа на предыдущей команды. Автоисправление некоторых ошибок.
	virtual bool processAnswer(const QByteArray & aCommand);

	/// Добавить общие статусы.
	virtual void appendStatusCodes(ushort aFlags, TStatusCodes & aStatusCodes);

	/// Определяет по ID модели, есть ли в ФР датчики, контролирующие перемещение печатающей матричной головки.
	bool isHeadSensorsExist() const;

	/// Определяет по ID модели, есть ли в ФР весовой датчик для контрольной ленты.
	bool isControlWeightSensor() const;

	/// Определяет по ID модели, есть ли в ФР оптический датчик для контрольной ленты.
	bool isControlOpticalSensor() const;

	/// Определяет по ID модели, есть ли в ФР рычаг, прижимающий контрольную ленту к голове.
	bool isControlLeverExist() const;

	/// Распарсить данные ФР, полученные из длинного статуса.
	virtual void parseDeviceData(const QByteArray & aData);

	/// Установить флаги по ошибке в ответе.
	virtual void setErrorFlags(const QByteArray & /*aCommand*/);

	/// Критична ли ошибка ЭКЛЗ (ошибки).
	bool isEKLZErrorCritical(char aError) const;

	/// Критична ли ошибка ФП (ошибки + статусы).
	bool isFMErrorCritical(char aError) const;
};

//--------------------------------------------------------------------------------
