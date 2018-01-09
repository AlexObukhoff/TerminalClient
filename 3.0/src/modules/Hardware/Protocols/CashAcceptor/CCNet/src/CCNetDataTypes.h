/* @file Типы данных купюроприемников на протоколе CCNet. */

#pragma once

//--------------------------------------------------------------------------------
namespace CCCNet
{
	namespace Commands
	{
		struct SData
		{
			bool deviceACK;
			bool hostACK;
			int timeout;

			SData() : deviceACK(true), hostACK(false), timeout(0) {}
			SData(bool aDeviceACK, bool aHostACK, int aTimeout) : deviceACK(aDeviceACK), hostACK(aHostACK), timeout(aTimeout) {}
		};
	}
}

//--------------------------------------------------------------------------------
