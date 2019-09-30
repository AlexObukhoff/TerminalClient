/* @file Монетоприемник на протоколе ccTalk. */

#pragma once

// Modules
#include "Hardware/Acceptors/CCTalkAcceptorBase.h"

// Project
#include "Hardware/CoinAcceptors/CoinAcceptorBase.h"
#include "CCTalkCoinAcceptorModelData.h"

//--------------------------------------------------------------------------------
typedef CCTalkAcceptorBase<CoinAcceptorBase> TCCTalkCoinAcceptorBase;

class CCTalkCoinAcceptorBase : public TCCTalkCoinAcceptorBase
{
public:
	CCTalkCoinAcceptorBase();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

protected:
	/// Получить буферизованные статусы.
	virtual bool getBufferedStatuses(QByteArray & aAnswer);

	/// Распарсить данные о купюре.
	virtual void parseCreditData(uchar aCredit, uchar aError, TStatusCodes & aStatusCodes);

	/// Можно ли применить простые статус-код (включен/отключен).
	virtual bool canApplySimpleStatusCodes(const TStatusCodes & aStatusCodes);

	/// Локальный сброс.
	virtual bool processReset();

	/// Загрузка таблицы номиналов из устройства.
	virtual bool loadParTable();
};

//--------------------------------------------------------------------------------
