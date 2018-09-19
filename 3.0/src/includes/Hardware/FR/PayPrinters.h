/* @file Модели принтеров для ФР Pay. */

#pragma once

#include "Hardware/Common/Specifications.h"

//--------------------------------------------------------------------------------
namespace CPayPrinters
{
	struct SModelData
	{
		QString name;
		bool hasPNESensor;

		SModelData(): hasPNESensor(false) {}
		SModelData(const QString & aName, bool aHasPNESensor): name(aName), hasPNESensor(aHasPNESensor) {}
	};

	const uchar Custom80 = 1;     /// Custom VKP-80 80 мм.
	const uchar Virtual = 100;    /// Печать в никуда.

	const char Default[] = "Default";    /// По умолчанию.

	class CModels : public CSpecification<uchar, SModelData>
	{
	public:
		CModels()
		{
			add( 1, "Custom VKP-80/80II/80III");
			add( 2, "Custom TG-2460");
			add( 3, "Custom TG-2480");
			add( 4, "Citizen PPU-700", false);
			add( 5, "Citizen CT-S2000", false);
			add( 6, "Axiohm KPSL1200-00H");
			add( 7, "SANEI SK1-21");
			add( 8, "Nippon NCR-F309");
			add( 9, "Custom VKP80/80II/80III 60mm");
			add(10, "Custom TG-02/02H");
			add(11, "Nippon NP-F209");
			add(12, "SEWOO SLK-TL100");
			add(13, "KIOSKO K300");
			add(14, "Custom VKP-80II SX");
			add(15, "Custom VKP-80II SX 60mm");
			add(16, "SNBC BT-T080R");
			add(17, "Custom TG-2480 56mm");
			add(Virtual, "Virtual", false);
		}

		QStringList getNames()
		{
			QStringList result;

			for (auto it = data().begin(); it != data().end(); ++it)
			{
				if ((it.key() != Virtual))
				{
					result << it->name;
				}
			}

			result.prepend(Default);

			return result;
		}

	private:
		void add(uchar aId, const QString & aName, bool aHasPNESensor = true)
		{
			append(aId, SModelData(aName, aHasPNESensor));
		}
	};

	static CModels Models;
}

//--------------------------------------------------------------------------------
