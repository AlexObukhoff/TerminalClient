/* @file Протокол ФР ПРИМ. */

#pragma once

#include "Hardware/Common/ProtocolBase.h"
#include "Hardware/Protocols/FR/PrimFRRealTime.h"

/// Условия выполнения команд.
namespace EPrimFRCommandConditions
{
	enum Enum
	{
		None = 0,       /// Нет дополнительных условий
		PrinterMode,    /// Команда протокола выполняется из режима принтера
		NAKRepeat       /// Повторный запрос ответа с помощью NAK-а
	};
}

//--------------------------------------------------------------------------------
/// Класс протокола PrimFR.
class PrimFRProtocol : public ProtocolBase
{
public:
	PrimFRProtocol();

	/// Выполнить команду протокола.
	TResult processCommand(const QByteArray & aCommandData, QByteArray & aAnswer, int aTimeout);

	/// Выполнить команду протокола без запаковки ответа.
	TResult execCommand(const QByteArray & aRequest, QByteArray & aAnswer, int aTimeout, EPrimFRCommandConditions::Enum aConditions = EPrimFRCommandConditions::None);

	/// Получить результат выполнения последней команды.
	TResult getCommandResult(char & aAnswer, bool aOnline = false);

protected:
	/// Подсчет контрольной суммы пакета данных.
	ushort calcCRC(const QByteArray & aData);

	/// Распаковка пришедших из порта данных.
	TResult check(const QByteArray & aRequest, const QByteArray & aAnswer, bool aPrinterMode);

	/// Считываем данные из порта.
	bool readData(QByteArray & aData, int aTimeout);

	/// Отличительный байт.
	uchar mDifferential;

	/// Протокол реал-тайм запросов.
	PrimFRRealTimeProtocol mRTProtocol;

	/// Ответ на запрос выполнения последней команды.
	char mLastCommandResult;
};

//--------------------------------------------------------------------------------
