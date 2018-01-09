/* @file Протокол SSP. */

#pragma once

#include "Hardware/Common/ProtocolBase.h"

//--------------------------------------------------------------------------------
class SSPProtocol : public ProtocolBase
{
public:
	SSPProtocol();

	/// Выполнить команду протокола.
	TResult processCommand(const QByteArray & aCommandData, QByteArray & aAnswerData, bool aSetSync = false);

	/// Установить ID адреса устройства.
	void setAddress(char aAddressID);

protected:
	/// Подсчет контрольной суммы пакета данных.
	ushort calcCRC(const QByteArray & aData);

	/// Получить ответ.
	bool getAnswer(QByteArray & aAnswer);

	/// Проверка пришедших из порта данных.
	bool check(const QByteArray & aAnswer);

	/// ID адреса устройства.
	char mAddress;

	/// Флаг последовательности.
	bool mSequenceFlag;
};

//--------------------------------------------------------------------------------
