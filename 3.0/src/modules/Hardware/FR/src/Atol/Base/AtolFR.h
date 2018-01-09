/* @file ПД АТОЛ. */

#pragma once

#include "AtolSerialFR.h"

//--------------------------------------------------------------------------------
class PayCTS2000 : public AtolSerialFR
{
	SET_SUBSERIES("PayCTS2000")

public:
	PayCTS2000::PayCTS2000()
	{
		mDeviceName = CAtolFR::Models::PayCTS2000K;
		mSupportedModels = QStringList() << mDeviceName;
	}
};

//--------------------------------------------------------------------------------
class AtolFR : public AtolSerialFR
{
public:
	AtolFR::AtolFR()
	{
		mDeviceName = "ATOL FR";
		mSupportedModels = getModelList();
	}

	static QStringList getModelList() { return CAtolFR::CModelData().getModelList(EFRType::EKLZ, true);}
};

//--------------------------------------------------------------------------------
class AtolFRSingle : public AtolFR
{
	SET_SUBSERIES("Single")

public:
	AtolFRSingle::AtolFRSingle()
	{
		mDeviceName = "ATOL single FR";
		mSupportedModels = getModelList();
	}

	static QStringList getModelList() { return CAtolFR::CModelData().getModelList(EFRType::EKLZ, false);}
};

//--------------------------------------------------------------------------------
class AtolDP : public AtolSerialFR
{
	SET_DEVICE_TYPE(DocumentPrinter)

public:
	AtolDP::AtolDP()
	{
		mDeviceName = "ATOL DP";
		mSupportedModels = getModelList();
	}

	static QStringList getModelList() { return CAtolFR::CModelData().getModelList(EFRType::NoEKLZ, true);}
};

//--------------------------------------------------------------------------------
class AtolDPSingle : public AtolDP
{
	SET_SUBSERIES("Single")

public:
	AtolDPSingle::AtolDPSingle()
	{
		mDeviceName = "ATOL single DP";
		mSupportedModels = getModelList();
	}

	static QStringList getModelList() { return CAtolFR::CModelData().getModelList(EFRType::NoEKLZ, false);}
};

//--------------------------------------------------------------------------------
