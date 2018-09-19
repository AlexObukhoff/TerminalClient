/* @file Данные моделей купюроприемников на протоколе ccTalk. */

#pragma once

#include "Hardware/Acceptors/CCTalkModelData.h"
#include "Hardware/Acceptors/CCTalkAcceptorConstants.h"

//--------------------------------------------------------------------------------
namespace CCCTalk
{
	namespace CashAcceptor
	{
		namespace Models
		{
			const char NV200Spectral[] = "NV200 Spectral";
		}

		class CModelData: public CModelDataBase
		{
		public:
			CModelData()
			{
				add("ITL", "NVS200", Models::NV200Spectral);
				data()["ITL"]["NVS200"].minVersions.insert(Currency::RUB, 0.00);
				data()["ITL"]["NVS200"].unsupported = CCCTalk::TUnsupported()
					<< CCCTalk::Command::DBVersion
					<< CCCTalk::Command::CreationDate
					<< CCCTalk::Command::SoftLastDate;
			}
		};
	}
}

//--------------------------------------------------------------------------------
