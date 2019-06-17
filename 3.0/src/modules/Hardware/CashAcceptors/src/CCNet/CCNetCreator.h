/* @file Купюроприемник Creator на протоколе CCNet. */

#pragma once

#include "CCNetCashAcceptorBase.h"

//--------------------------------------------------------------------------------
class CCNetCreator : public CCNetCashAcceptorBase
{
	SET_SUBSERIES("Creator")

public:
	CCNetCreator();

protected:
	/// Запросить и сохранить параметры устройства.
	virtual void processDeviceData(QByteArray & aAnswer);

	/// Обновить прошивку.
	virtual bool performUpdateFirmware(const QByteArray & aBuffer);

	/// Изменить скорость работы.
	virtual bool performBaudRateChanging(const SDK::Driver::TPortParameters & aPortParameters);

	/// Записать головную часть прошивки.
	bool writeHead(const QByteArray & aBuffer);

	/// Записать блок данных.
	bool writeBlock(const QByteArray & aBuffer, int aIndex, bool aLast);

	/// Распарсить данные устройства.
	QString parseDeviceData(const QByteArray & aData, const QString & aPattern);
};

//--------------------------------------------------------------------------------
