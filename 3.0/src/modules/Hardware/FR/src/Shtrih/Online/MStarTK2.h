/* @file ФР Multisoft MStar-TK2 на протоколе Штрих. */

#pragma once

#include "../Retractor/ShtrihRetractorFRLite.h"

//--------------------------------------------------------------------------------
namespace CMStarTK2FR
{
	/// Настройки печати.
	namespace Printing
	{
		const char All       = '\x00';    /// Печатать всё.
		const char NoFiscal  = '\x00';    /// Не печатать фисклаьный чек.
		const char NoZReport = '\x03';    /// Не печатать Z-отчёт.
	}
}

//--------------------------------------------------------------------------------
typedef ShtrihRetractorFRLite<ShtrihOnlineFRBase<ShtrihSerialFRBase>> TMStarTK2FR;

class MStarTK2FR : public TMStarTK2FR
{
	SET_SUBSERIES("MStarTK2")

public:
	MStarTK2FR();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

protected:
	/// Выполнить команду.
	virtual TResult processCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer = nullptr);

	/// Включить/выключить режим непечати документов.
	virtual bool setNotPrintDocument(bool aEnabled, bool aZReport = false);
};

//--------------------------------------------------------------------------------
