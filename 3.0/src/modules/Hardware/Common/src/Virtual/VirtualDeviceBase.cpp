/* @file Базовый класс виртуальных устройств. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QCoreApplication>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/CashAcceptors/ProtoCashAcceptor.h"
#include "Hardware/CashAcceptors/CashAcceptorBase.h"
#include "Hardware/Dispensers/ProtoDispenser.h"
#include "Hardware/Dispensers/DispenserBase.h"

// Project
#include "VirtualDeviceBase.h"

//-------------------------------------------------------------------------------
template class VirtualDeviceBase<CashAcceptorBase<DeviceBase<ProtoCashAcceptor>>>;
template class VirtualDeviceBase<DispenserBase<DeviceBase<ProtoDispenser>>> ;

//---------------------------------------------------------------------------------
template<class T>
VirtualDeviceBase<T>::VirtualDeviceBase()
{
	mDeviceName = "Virtual";
}

//---------------------------------------------------------------------------------
template<class T>
void VirtualDeviceBase<T>::initialize()
{
	START_IN_WORKING_THREAD(initialize)

	T::initialize();

	// Меняем поток на главный, иначе фильтр событий не будет работать.
	moveToThread(qApp->thread());

	// Подписываемся на уведобления о событиях от приложения.
	qApp->installEventFilter(this);
}

//---------------------------------------------------------------------------------
template<class T>
bool VirtualDeviceBase<T>::release()
{
	qApp->removeEventFilter(this);

	return T::release();
}

//---------------------------------------------------------------------------------
template<class T>
bool VirtualDeviceBase<T>::getStatus(TStatusCodes & aStatusCodes)
{
	aStatusCodes += mStatusCodes;

	return true;
}

//--------------------------------------------------------------------------------
template<class T>
void VirtualDeviceBase<T>::blinkStatusCode(int aStatusCode)
{
	mStatusCodes.insert(aStatusCode);
	onPoll();

	mStatusCodes.remove(aStatusCode);
	onPoll();
}

//--------------------------------------------------------------------------------
template<class T>
void VirtualDeviceBase<T>::changeStatusCode(int aStatusCode)
{
	if (mStatusCodes.contains(aStatusCode))
	{
		mStatusCodes.remove(aStatusCode);
	}
	else
	{
		mStatusCodes.insert(aStatusCode);
	}
}

//---------------------------------------------------------------------------------
template<class T>
bool VirtualDeviceBase<T>::eventFilter(QObject * /*aWatched*/, QEvent * aEvent)
{
	if ((aEvent->type() == QEvent::KeyPress) && aEvent->spontaneous())
	{
		QKeyEvent * keyEvent = static_cast<QKeyEvent *>(aEvent);

		filterKeyEvent(keyEvent->key(), keyEvent->modifiers());
		onPoll();
	}

	return false;
}

//---------------------------------------------------------------------------------
