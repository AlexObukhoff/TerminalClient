/* @file Виртуальный купюроприемник. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QReadLocker>
#include <QtCore/QVector>
#include <Common/QtHeadersEnd.h>

// Project
#include "VirtualCashAcceptor.h"

using namespace SDK::Driver;

//---------------------------------------------------------------------------------
VirtualCashAcceptor::VirtualCashAcceptor() : mNotesPerEscrow(1)
{
	mDeviceName = "Virtual cash acceptor";
}

//--------------------------------------------------------------------------------
bool VirtualCashAcceptor::updateParameters()
{
	mReady = true;
	setEnable(false);

	mCurrencyError = processParTable();

	return mCurrencyError == ECurrencyError::OK;
}

//---------------------------------------------------------------------------------
void VirtualCashAcceptor::setDeviceConfiguration(const QVariantMap & aConfiguration)
{
	TVirtualCashAcceptor::setDeviceConfiguration(aConfiguration);

	mNotesPerEscrow = aConfiguration.value(CHardware::VirtualCashAcceptor::NotesPerEscrow, mNotesPerEscrow).toUInt();
}

//--------------------------------------------------------------------------------
void VirtualCashAcceptor::testStack(double aAmount)
{
	SDK::Driver::TParList pars;

	pars << SDK::Driver::SPar(aAmount, getConfigParameter(CHardwareSDK::CashAcceptor::SystemCurrencyId).toInt());

	emit stacked(pars);

	emit status(EWarningLevel::OK, QString("Test stacked %1").arg(aAmount, 0, 'f', 2), ECashAcceptorStatus::OK);
}

//--------------------------------------------------------------------------------
bool VirtualCashAcceptor::loadParTable()
{
	int currencyId = getConfigParameter(CHardwareSDK::CashAcceptor::SystemCurrencyId).toInt();

	mEscrowParTable.add(Qt::Key_F1, SPar(10, currencyId));
	mEscrowParTable.add(Qt::Key_F2, SPar(50, currencyId));
	mEscrowParTable.add(Qt::Key_F3, SPar(100, currencyId));
	mEscrowParTable.add(Qt::Key_F4, SPar(500, currencyId));
	mEscrowParTable.add(Qt::Key_F5, SPar(1000, currencyId));
	mEscrowParTable.add(Qt::Key_F6, SPar(5000, currencyId));

	//TODO: при необходимости кастомизировать для евро, доллара и тенге

	return true;
}

//--------------------------------------------------------------------------------
bool VirtualCashAcceptor::setEnable(bool aEnabled)
{
	if (!mReady && aEnabled)
	{
		return false;
	}

	using namespace BillAcceptorStatusCode::Normal;

	mStatusCodes.remove(Enabled);
	mStatusCodes.remove(Disabled);
	mStatusCodes.insert(aEnabled ? Enabled : Disabled);

	if (aEnabled)
	{
		emit status(EWarningLevel::OK, "Enabled", ECashAcceptorStatus::Enabled);
	}
	else
	{
		emit status(EWarningLevel::OK, "Disabled", ECashAcceptorStatus::Disabled);
	}

	return true;
}

//--------------------------------------------------------------------------------
bool VirtualCashAcceptor::leaveEscrow(int aStatusCode)
{
	bool escrow = mStatusCodes.contains(BillAcceptorStatusCode::BillOperation::Escrow);
	mStatusCodes.remove(BillAcceptorStatusCode::BillOperation::Escrow);

	if (!mStackedStatusCodes.isEmpty())
	{
		onPoll();
		mStackedStatusCodes.clear();

		return false;
	}

	if (!mReady || !escrow || !mStatusCodes.contains(BillAcceptorStatusCode::Normal::Enabled))
	{
		onPoll();

		return false;
	}

	blinkStatusCode(aStatusCode);

	return true;
}

//--------------------------------------------------------------------------------
bool VirtualCashAcceptor::stack()
{
	if (!mStackedStatusCodes.isEmpty())
	{
		mStatusCodes += mStackedStatusCodes;
	}

	return leaveEscrow(BillAcceptorStatusCode::BillOperation::Stacked);
}

//--------------------------------------------------------------------------------
bool VirtualCashAcceptor::reject()
{
	return leaveEscrow(BillAcceptorStatusCode::Busy::Returned);
}

//--------------------------------------------------------------------------------
void VirtualCashAcceptor::filterKeyEvent(int aKey, const Qt::KeyboardModifiers & aModifiers)
{
	if (aModifiers & Qt::ControlModifier)
	{
		switch (aKey)
		{
			case Qt::Key_F1:
			case Qt::Key_F2:
			case Qt::Key_F3:
			case Qt::Key_F4:
			case Qt::Key_F5:
			case Qt::Key_F6:
			{
				if (mEscrowParTable.data().contains(aKey))
				{
					mEscrowPars = QVector<SPar>(mNotesPerEscrow, mEscrowParTable[aKey]).toList();
					mStatusCodes.insert(BillAcceptorStatusCode::BillOperation::Escrow);
				}

				break;
			}
			case Qt::Key_F7:  { blinkStatusCode(BillAcceptorStatusCode::Reject::Unknown);                     break; }    // режект
			case Qt::Key_F8:  { changeStatusCode(BillAcceptorStatusCode::MechanicFailure::JammedInValidator); break; }    // купюра замята
			case Qt::Key_F9:  { changeStatusCode(DeviceStatusCode::Error::NotAvailable);                      break; }    // недоступен питания
			case Qt::Key_F10: { changeStatusCode(BillAcceptorStatusCode::MechanicFailure::StackerOpen);       break; }    // стекер снят
			case Qt::Key_F11: { changeStatusCode(BillAcceptorStatusCode::MechanicFailure::StackerFull);       break; }    // стекер полон
			case '*':         { blinkStatusCode(BillAcceptorStatusCode::Warning::Cheated);                    break; }    // мошенство
		}
	}
	else if (aModifiers & Qt::AltModifier)
	{
		switch (aKey)
		{
			case Qt::Key_F8:  { mStackedStatusCodes.insert(BillAcceptorStatusCode::MechanicFailure::JammedInStacker); break; }    // Купюра замята
			case Qt::Key_F9:  { mStackedStatusCodes.insert(DeviceStatusCode::Error::NotAvailable);                    break; }    // Недоступен питания
			case Qt::Key_F10: { mStackedStatusCodes.insert(BillAcceptorStatusCode::MechanicFailure::StackerOpen);     break; }    // Стекер снят
			case Qt::Key_F11: { mStackedStatusCodes.insert(BillAcceptorStatusCode::MechanicFailure::StackerFull);     break; }    // Стекер полон
		}
	}
}

//---------------------------------------------------------------------------------
