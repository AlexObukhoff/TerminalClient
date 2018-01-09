/* @file Общие константы диспенсеров. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QSet>
#include <QtCore/QList>
#include <Common/QtHeadersEnd.h>

//Modules
#include "Hardware/Common/Specifications.h"
#include "Hardware/Common/BaseStatusTypes.h"

// Project
#include "Hardware/Dispensers/DispenserStatusCodes.h"

//--------------------------------------------------------------------------------
namespace CDispenser
{
	/// Группы статус-кодов.
	namespace StatusCodes
	{
		using namespace DispenserStatusCode::Warning;
		using namespace DispenserStatusCode::Error;

		const TStatusCodes AllOpened = TStatusCodes() << Unit0Opened << Unit1Opened << Unit2Opened << Unit3Opened;

		typedef QList<int> TUnitGroup;
		const TUnitGroup AllEmpty = TUnitGroup() << Unit0Empty << Unit1Empty << Unit2Empty << Unit3Empty;
		const TUnitGroup AllNearEmpty = TUnitGroup() << Unit0NearEmpty << Unit1NearEmpty << Unit2NearEmpty << Unit3NearEmpty;

		//--------------------------------------------------------------------------------
		/// Описатель статус-кодов кассеты.
		struct SData
		{
			int empty;        /// Пустая.
			int nearEmpty;    /// Почти пустая.
			int opened;       /// Открыта.

			SData() : empty(-1), nearEmpty(-1), opened(-1) {}
			SData(int aEmpty, int aNearEmpty, int aOpened) : empty(aEmpty), nearEmpty(aNearEmpty), opened(aOpened) {}
		};

		#define ADD_UNIT_DATA(aUnit) append(aUnit, SData(Unit##aUnit##Empty, Unit##aUnit##NearEmpty, Unit##aUnit##Opened));

		/// Спецификация состояний кассет.
		class CData : public CSpecification<int, SData>
		{
		public:
			CData()
			{
				ADD_UNIT_DATA(0);
				ADD_UNIT_DATA(1);
				ADD_UNIT_DATA(2);
				ADD_UNIT_DATA(3);
			}
		};

		static CData Data;
	}
}

//--------------------------------------------------------------------------------
