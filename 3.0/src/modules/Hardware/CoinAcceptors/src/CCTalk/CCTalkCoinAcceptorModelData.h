/* @file Данные моделей монетоприемников на протоколе ccTalk. */

#pragma once

#include "Hardware/Acceptors/CCTalkModelData.h"
#include "Hardware/Acceptors/CCTalkAcceptorConstants.h"

//--------------------------------------------------------------------------------
namespace CCCTalk
{
	namespace CoinAcceptor
	{
		class CModelData: public CModelDataBase
		{
		public:
			CModelData()
			{
				add("MSC", "SP2", 6, 13);
				data()["MSC"]["SP2"].minVersions.insert(Currency::RUB, 5.38);

				add("MSC", "SP3", 6, 13);
				data()["MSC"]["SP3"].minVersions.insert(Currency::RUB, 5.42);

				add("NRI", "G13");
				data()["NRI"]["G13"].minVersions.insert(Currency::RUB, 52.03);
				data()["NRI"]["G13"].minVersions.insert(Currency::KZT, 52.05);
				data()["NRI"]["G13"].unsupported = CCCTalk::TUnsupported()
					<< CCCTalk::Command::BaseYear
					<< CCCTalk::Command::CreationDate
					<< CCCTalk::Command::SoftLastDate;

				add("NRI", "BV", "Pelicano");
				data()["NRI"]["BV"].unsupported = CCCTalk::TUnsupported()
					<< CCCTalk::Command::BaseYear
					<< CCCTalk::Command::CreationDate
					<< CCCTalk::Command::SoftLastDate;

				add("ICT", "SCA1");
				add("WHM", "EMP", 0, 0, "EMP-800");
			}
		};
	}
}

//--------------------------------------------------------------------------------
