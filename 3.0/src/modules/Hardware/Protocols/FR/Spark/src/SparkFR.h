/* @file Протокол ФР SPARK. */

#pragma once

#include "Hardware/Common/ProtocolBase.h"

//--------------------------------------------------------------------------------
/// Класс протокола KKM.
class SparkFRProtocol: public ProtocolBase
{
public:
	/// Выполнить команду.
	TResult processCommand(const QByteArray & aCommandData, QByteArray & aAnswerData, int aTimeout);
	TResult processCommand(char aCommand, QByteArray & aAnswerData, int aTimeout);

private:
	/// Выполнить команду.
	TResult performCommand(const QByteArray & aCommandData, QByteArray & aAnswerData, int aTimeout);

	/// Подсчет контрольной суммы пакета данных.
	char calcCRC(const QByteArray & aData);

	/// Распаковка пришедших из порта данных.
	bool unpack(QByteArray & aAnswerData);

	/// Получить данные из порта.
	bool readData(QByteArray & aAnswerData, int aTimeout);
};

//--------------------------------------------------------------------------------
