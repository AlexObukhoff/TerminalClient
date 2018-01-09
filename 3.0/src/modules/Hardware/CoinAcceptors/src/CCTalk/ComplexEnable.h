/* @file Монетоприемник на протоколе ccTalk с 2-ступенчатой схемой включения на прием денег. */

#pragma once

#include "CCTalkCoinAcceptorBase.h"

//--------------------------------------------------------------------------------
class ComplexEnableCCTalkCoinAcceptor : public CCTalkCoinAcceptorBase
{
	SET_SUBSERIES("ComplexEnabling")

public:
	ComplexEnableCCTalkCoinAcceptor();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

protected:
	/// Фоновая логика при появлении определенных состояний устройства.
	virtual void postPollingAction(const TStatusCollection & aNewStatusCollection, const TStatusCollection & aOldStatusCollection);

	/// Применить таблицу номиналов.
	virtual bool applyParTable();
};

//--------------------------------------------------------------------------------
