/* @file Типы данных купюроприемников на протоколе SSP. */

#pragma once

//--------------------------------------------------------------------------------
namespace CSSP
{
	/// Таймаут для ожидания ответа по умолчанию, [мс].
	const int DefaultTimeout = 200;

	namespace Commands
	{
		struct SData
		{
			int timeout;
			bool setSync;

			SData(): timeout(DefaultTimeout), setSync(false) {}
			SData(int aTimeout, bool aSetSync): timeout(aTimeout), setSync(aSetSync) {}
		};
	}
}

//--------------------------------------------------------------------------------
