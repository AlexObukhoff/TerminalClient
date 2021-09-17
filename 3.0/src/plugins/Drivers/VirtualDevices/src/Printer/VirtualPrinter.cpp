/* @file Виртуальный диспенсер. */

// STL
#include <algorithm>

// Modules
#include "Hardware/Dispensers/DispenserStatusCodes.h"
#include "Hardware/Dispensers/DispenserData.h"

// Project
#include "VirtualPrinter.h"

//---------------------------------------------------------------------------
VirtualPrinter::VirtualPrinter()
{
	mDeviceName = "Virtual printer";
	mMaxBadAnswers = 0;
}

//--------------------------------------------------------------------------------
bool VirtualPrinter::isDeviceReady(bool aOnline)
{
	if (aOnline)
	{
		SleepHelper::msleep(CVirtualPrinter::Delay::OnlineReadyChecking);
	}

	MutexLocker locker(&mExternalMutex);

	return mStatusCollection.isEmpty(SDK::Driver::EWarningLevel::Error);
}

//--------------------------------------------------------------------------------
bool VirtualPrinter::print(const QStringList & aReceipt)
{
	if (!isDeviceReady(false))
	{
		toLog(LogLevel::Normal, mDeviceName + ": Failed to print receipt");
		return false;
	}

	Tags::TLexemeReceipt lexemeReceipt;
	makeLexemeReceipt(aReceipt, lexemeReceipt);
	QStringList receipt;

	for (int i = 0; i < lexemeReceipt.size(); ++i)
	{
		QString line;

		for (int j = 0; j < lexemeReceipt[i].size(); ++j)
		{
			for (int k = 0; k < lexemeReceipt[i][j].size(); ++k)
			{
				line += lexemeReceipt[i][j][k].data;
			}
		}

		receipt << line;
	}

	if (!receipt.isEmpty())
	{
		toLog(LogLevel::Normal, "Printing receipt:\n" + receipt.join("\n"));
		SleepHelper::msleep(CVirtualPrinter::Delay::Printing);
		toLog(LogLevel::Normal, "Receipt has been printed successfully");
	}
	else
	{
		toLog(LogLevel::Normal, mDeviceName + ": receipt is empty");
	}

	return true;
}

//--------------------------------------------------------------------------------
void VirtualPrinter::filterKeyEvent(int aKey, const Qt::KeyboardModifiers & aModifiers)
{
	if (!(aModifiers ^ (Qt::ControlModifier | Qt::ShiftModifier)) && !isKeyModifier(aKey))
	{
		switch (aKey)
		{
			case Qt::Key_F9: { changeStatusCode(DeviceStatusCode::Error::NotAvailable); break; }
		}
	}
}

//--------------------------------------------------------------------------------
