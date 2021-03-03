/* @file Прото-ФР семейства АТОЛ. */

#pragma once

#include "Hardware/FR/PortFRBase.h"
#include "../AtolFRConstants.h"
#include "../AtolModelData.h"

//--------------------------------------------------------------------------------
template <class T>
class ProtoAtolFR : public T
{
public:
	ProtoAtolFR();

	/// Готов ли к обработке данной фискальной команды.
	virtual bool isFiscalReady(bool aOnline, SDK::Driver::EFiscalPrinterCommand::Enum aCommand = SDK::Driver::EFiscalPrinterCommand::Sale);

	/// Печать выплаты.
	virtual bool performEncashment(const QStringList & aReceipt, double aAmount);

protected:
	/// Установить начальные параметры.
	virtual void setInitialData();

	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Получить ключ модели для идентификации.
	virtual bool processModelKey(CAtolFR::TModelKey & aModeKey);

	/// Напечатать [и выдать] чек.
	virtual bool processReceipt(const QStringList & aReceipt, bool aProcessing = true);

	/// Данные устройства.
	CAtolFR::SModelData mModelData;

	/// Номер сборки прошивки ПО ФР.
	int mFRBuild;

	/// Текущий режим.
	char mMode;

	/// Текущий подрежим.
	char mSubmode;

	/// Заблокирован ли ФР.
	bool mLocked;

	/// Необнуляемая сумма;
	double mNonNullableAmount;
};

//--------------------------------------------------------------------------------
