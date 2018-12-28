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
		const char NoFiscal  = '\x00';    /// Не печатать фискальный чек.
		const char NoZReport = '\x03';    /// Не печатать Z-отчёт.
	}

	/// Таймаут незабранного чека. Валики почти сразу отпускают распечатанный чек. 0 - не отпустят совсем.
	const int LeftReceiptTimeout = 1;
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

	/// Устанавливает конфигурацию устройству.
	virtual void setDeviceConfiguration(const QVariantMap & aConfiguration);

protected:
	/// Выполнить команду.
	virtual TResult execCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer = nullptr);

	/// Включить/выключить режим непечати документов.
	virtual bool setNotPrintDocument(bool aEnabled, bool aZReport = false);
};

//--------------------------------------------------------------------------------
