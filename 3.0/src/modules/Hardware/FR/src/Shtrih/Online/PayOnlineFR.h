/* @file ФР Пэй на протоколе Штрих. */

#pragma once

#include "../Retractor/ShtrihRetractorFRLite.h"

namespace CPayOnlineFR
{
	struct SPrinterData
	{
		QString name;
		bool hasPNESensor;

		SPrinterData(): hasPNESensor(false) {}
		SPrinterData(const QString & aName, bool aHasPNESensor): name(aName), hasPNESensor(aHasPNESensor) {}
	};

	class CPrinterModels : public CSpecification<uchar, SPrinterData>
	{
	public:
		CPrinterModels()
		{
			add(  1, "Custom VKP-80");
			add(  2, "Custom TG-2460");
			add(  3, "Custom TG-2480");
			add(  4, "Citizen PPU-700", false);
			add(  5, "Citizen CT-S2000", false);
			add(  6, "Axiohm KPSL1200-00H");
			add(  7, "SANEI SK1-21");
			add(  8, "Nippon NCR-F309");
			add(  9, "Custom VKP-80 44mm");
			add(100, "Virtual", false);
		}

	private:
		void add(uchar aId, const QString & aName, bool aHasPNESensor = true)
		{
			append(aId, SPrinterData(aName, aHasPNESensor));
		}
	};

	static CPrinterModels PrinterModels;
}

//--------------------------------------------------------------------------------
template<class T>
class PayOnlineFR : public ShtrihRetractorFRLite<T>
{
public:
	PayOnlineFR();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

protected:
	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Запросить и вывести в лог критичные параметры ФР.
	virtual void processDeviceData();

	/// Снять Z-отчет.
	virtual bool execZReport(bool aAuto);

	/// Добавить общие статусы.
	virtual void appendStatusCodes(ushort aFlags, TStatusCodes & aStatusCodes);

	/// Id модели подключенного принтера.
	uchar mPrinterModelId;
};

typedef PayOnlineFR<ShtrihOnlineFRBase<ShtrihTCPFRBase>> PayOnlineTCPFR;
typedef PayOnlineFR<ShtrihOnlineFRBase<ShtrihSerialFRBase>> PayOnlineSerialFR;

//--------------------------------------------------------------------------------
