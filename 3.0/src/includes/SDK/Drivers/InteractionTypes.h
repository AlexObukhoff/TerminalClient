/* @file Способы взаимодействия драйвера с устройством. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

namespace SDK {
namespace Driver {

#define ADD_IT(aType) const char aType[] = #aType; class It##aType {};

namespace CInteractionTypes
{
	ADD_IT(COM)
	ADD_IT(USB)
	ADD_IT(LibUSB)
	ADD_IT(TCP)
	ADD_IT(OPOS)
	ADD_IT(System)
	ADD_IT(External)
	ADD_IT(ExternalCOM)
	ADD_IT(ExternalVCOM)
}

/// Все типы взаимодействия.
const QStringList InteractionTypes = QStringList()
	<< CInteractionTypes::COM
	<< CInteractionTypes::USB
	<< CInteractionTypes::LibUSB
	<< CInteractionTypes::TCP
	<< CInteractionTypes::OPOS
	<< CInteractionTypes::System
	<< CInteractionTypes::External
	<< CInteractionTypes::ExternalCOM
	<< CInteractionTypes::ExternalVCOM;

/// Типы взаимодействия, для которых требуется отдельное логгирование средствами ТК.
const QStringList LoggedInteractionTypes = QStringList()
	<< CInteractionTypes::COM
	<< CInteractionTypes::USB
	<< CInteractionTypes::LibUSB;

/// Типы взаимодействия, для которых требуется выделять порт под внешний драйвер.
const QStringList ExternalWithRRTypes = QStringList()
	<< CInteractionTypes::ExternalCOM
	<< CInteractionTypes::ExternalVCOM;

}} // SDK::Driver

//---------------------------------------------------------------------------
