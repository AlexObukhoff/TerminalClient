/* @file Устройство приема денег на протоколе ccTalk с 2-ступенчатой схемой включения на прием денег. */

#pragma once

#include "CCTalkAcceptorBase.h"

//--------------------------------------------------------------------------------
template <class T>
class CCTalkComplexEnableAcceptor : public T
{
	SET_SUBSERIES("ComplexEnabling")

protected:
	/// Фоновая логика при появлении определенных состояний устройства.
	virtual void postPollingAction(const TStatusCollection & aNewStatusCollection, const TStatusCollection & aOldStatusCollection);

	/// Применить таблицу номиналов.
	virtual bool applyParTable();
};

//--------------------------------------------------------------------------------
