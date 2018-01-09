/* @file ФР ПРИМ c презентером. */

#pragma once

#include "../PrimFRBase.h"
#include "../Online/PrimOnlineFRBase.h"

//--------------------------------------------------------------------------------
template <class T>
class PrimPresenterFR : public T
{
	SET_SUBSERIES("Presenter")

public:
	PrimPresenterFR();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

protected:
	/// Напечатать [и выдать] чек.
	virtual bool performReceipt(const QStringList & aReceipt, bool aProcessing = true);

	typedef QSharedPointer<TSerialPrinterBase> PPrinter;
	PPrinter mPrinter;
};

//--------------------------------------------------------------------------------
typedef PrimPresenterFR<PrimFRBase> PrimPresenterFRBase;

//--------------------------------------------------------------------------------
