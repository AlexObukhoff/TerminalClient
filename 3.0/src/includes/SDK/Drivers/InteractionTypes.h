/* @file Способы взаимодействия драйвера с устройством. */

#pragma once

namespace SDK {
namespace Driver {

#define ADD_IT(aType) const char aType[] = #aType; class It##aType {};

namespace CInteractionTypes
{
	ADD_IT(COM)
	ADD_IT(USB)
	ADD_IT(TCP)
	ADD_IT(OPOS)
	ADD_IT(System)
	ADD_IT(External)
}

//---------------------------------------------------------------------------
}} // SDK::Driver

//---------------------------------------------------------------------------
