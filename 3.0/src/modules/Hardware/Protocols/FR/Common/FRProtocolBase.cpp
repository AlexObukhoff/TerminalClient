/* @file Базовая реализация протоколов фискальников. */

#include <cmath>

#include "FRProtocolBase.h"

//--------------------------------------------------------------------------------
using namespace SDK::Driver::IOPort::COM;

//--------------------------------------------------------------------------------
FRProtocolBase::FRProtocolBase() :
	mSessionOpened(true),
	mNeedCloseSession(false)
{
	// данные порта
	mPortParameters[EParameters::Parity].append(EParity::No);

	// кодек
	mCodec = CodecByName[CHardware::Codepages::CP866];
}

//--------------------------------------------------------------------------------
bool FRProtocolBase::isSessionOpened()
{
	return mSessionOpened;
}

//--------------------------------------------------------------------------------
