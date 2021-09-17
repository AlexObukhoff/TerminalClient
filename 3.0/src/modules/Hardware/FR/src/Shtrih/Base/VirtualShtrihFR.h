/* @file Виртуальный ФР на протоколе Штрих на виртуальном COM-порту. */

#pragma once

#include "Hardware/FR/ProtoShtrihFR.h"
#include "ShtrihFRBaseConstants.h"

//--------------------------------------------------------------------------------
class VirtualShtrihFR : public ProtoShtrihFR<ShtrihSerialFRBase>
{
	SET_SERIES("ShtrihVirtual")
	SET_VCOM_DATA(Types::Manufacturer, ConnectionTypes::VCOMOnly, ManufacturerTags::FR::Virtual)

public:
	VirtualShtrihFR();

protected:
	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Получить параметры печати.
	virtual bool getPrintingSettings();
};

//--------------------------------------------------------------------------------
