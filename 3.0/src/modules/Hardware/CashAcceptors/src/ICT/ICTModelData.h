/* @file Данные моделей устройств на протоколе ICT. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

// Project
#include "Hardware/Common/Specifications.h"

//--------------------------------------------------------------------------------
namespace CICT
{
	class ModelData : public CSpecification<QString, bool>
	{
	public:
		ModelData()
		{
			add("ICT A6");
			add("ICT V6");
			add("ICT N6");
			add("ICT S6");
			add("ICT A7");
			add("ICT V7", true);
			add("ICT P7");
			add("ICT N7");
			add("ICT S7");
			add("ICT PA7");
			add("ICT PV7");
			add("ICT P70");
			add("ICT P77", true);
			add("ICT P85");
			add("ICT L70");
			add("ICT L83");
			add("ICT TAOA");
			add("ICT TAOV");
			add("ICT TAOL");
			add("ICT J830");
			add("ICT B70");
			add("ICT U70", true);
		}

	private:
		void add(const QString & aModelName, bool aVerified = false)
		{
			append(aModelName, aVerified);
		}
	};
}

//--------------------------------------------------------------------------------
