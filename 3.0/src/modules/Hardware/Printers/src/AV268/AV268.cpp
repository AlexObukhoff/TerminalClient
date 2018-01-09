/* @file Принтер AV-268. */

#include "AV268.h"
#include "AV268Constants.h"

using namespace SDK::Driver::IOPort::COM;
using namespace PrinterStatusCode;

//--------------------------------------------------------------------------------
AV268::AV268()
{
	// данные устройства
	mDeviceName = "SysFuture AV-268";

	// данные порта
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR115200);
	mPortParameters[EParameters::Parity].append(EParity::No);

	// теги
	mTagEngine = Tags::PEngine(new CAV268::TagEngine());

	// данные устройства
	setConfigParameter(CHardware::Printer::Commands::Cutting, "\x1B\x69");
	mLineSize  = CAV268::LineSize;
	mOverflow  = false;
	mModelType = Unknown;
	mInitialize = false;
}

//--------------------------------------------------------------------------------
bool AV268::isConnected()
{
	TStatusCodes statusCodes;
	bool statusOK = getStatus(statusCodes);
	bool modelTypeOK = (mModelType != Unknown) && (mInitialize || (mModelType != Simple));

	if (mModelType == Plus)
	{
		mDeviceName = "SysFuture AV-268 Plus";
	}

	return statusOK && modelTypeOK;
}

//--------------------------------------------------------------------------------
QStringList AV268::getModelList()
{
	return QStringList()
		<< "SysFuture AV-268"
		<< "SysFuture AV-268 Plus";
}

//--------------------------------------------------------------------------------
void AV268::initialize()
{
	mInitialize = true;

	TSerialPrinterBase::initialize();
}

//--------------------------------------------------------------------------------
bool AV268::processCommand(const QByteArray & aCommand, QByteArray * aAnswer)
{
	return mIOPort->write(aCommand) && !(aAnswer && !getAnswer(*aAnswer));
}

//--------------------------------------------------------------------------------
bool AV268::updateParameters()
{
	bool result = processCommand(CAV268::Commands::Initialize);
	SleepHelper::msleep(CAV268::Timeouts::Initialize);

	return result;
}

//--------------------------------------------------------------------------------
bool AV268::printLine(const QByteArray & aString)
{
	if (!processCommand(aString))
	{
		return false;
	}

	QByteArray answer;
	getAnswer(answer);

	return !mOverflow || waitBufferClearing();
}

//--------------------------------------------------------------------------------
bool AV268::waitBufferClearing()
{
	QTime clockTimer;
	clockTimer.start();

	do
	{
		toLog(LogLevel::Normal, "AV268: buffer overflow, wait...");
		QByteArray answer;
		SleepHelper::msleep(CAV268::Timeouts::Wait);
		getAnswer(answer);

		if (!mOverflow)
		{
			return true;
		}
	}
	while(clockTimer.elapsed() < CAV268::Timeouts::Full);

	toLog(LogLevel::Normal, "AV268: Timeout for waiting buffer clearing");

	return false;
}

//--------------------------------------------------------------------------------
bool AV268::getAnswer(QByteArray & aAnswer, bool aNeedDelay)
{
	if (aNeedDelay)
	{
		SleepHelper::msleep(CAV268::Timeouts::Default);
	}

	if (!TSerialPrinterBase::getAnswer(aAnswer, 100))
	{
		return false;
	}

	for (int i = 0; i < aAnswer.size(); ++i)
	{
		if (aAnswer.contains(ASCII::XOn) || aAnswer.contains(ASCII::XOff))
		{
			mOverflow = aAnswer.contains(ASCII::XOff) == true;  //C4800, QBool::operator const void *() const
			aAnswer.remove(i, 1);
			--i;
		}
	}

	return !aAnswer.isEmpty();
}

//--------------------------------------------------------------------------------
bool AV268::getStatus(TStatusCodes & aStatusCodes)
{
	QByteArray answer;

	if (!processCommand(CAV268::Commands::GetStatus, &answer))
	{
		return false;
	}

	char status = answer[0];

	if (status & CAV268::Statuses::NotConnected)
	{
		toLog(LogLevel::Error, "AV268: wrong byte returned");
		return false;
	}

	if (status & CAV268::Statuses::HeadOverheat)
	{
		aStatusCodes.insert(Error::Temperature);
	}

	if (status & CAV268::Statuses::NoPaper)
	{
		aStatusCodes.insert(Error::PaperEnd);
	}

	if (status & CAV268::Statuses::HeadDoorOpened)
	{
		aStatusCodes.insert(DeviceStatusCode::Error::MechanismPosition);
	}

	if (status & CAV268::Statuses::UnknownError)
	{
		aStatusCodes.insert(DeviceStatusCode::Error::Unknown);
	}

	if (mModelType != Simple)
	{
		bool result = processCommand(CAV268::Commands::GetSettings, &answer) &&
		              answer.startsWith(CAV268::Answers::GetSettings) &&
		              (answer.size() == 3);

		if (mModelType == Unknown)
		{
			mModelType = result ? Extended : Simple;
		}

		if (!result)
		{
			if (mModelType != Simple)
			{
				toLog(LogLevel::Error, "AV268: Wrong answer for settings request");
				return false;
			}

			return true;
		}

		char DIPSettings = answer[2];

		if (((DIPSettings & CAV268::DIPSwitches::HalfHeight) &&
		    (~DIPSettings & CAV268::DIPSwitches::DoubleHeight)) ||
		    (~DIPSettings & CAV268::DIPSwitches::Cutter) ||
		(bool(DIPSettings & CAV268::DIPSwitches::Presenter) != (mModelType == Plus)))
		{
			aStatusCodes.insert(DeviceStatusCode::Warning::WrongSwitchesConfig);
		}

		int factor = (DIPSettings & CAV268::DIPSwitches::DoubleWidth) ? 2 : 1;
		mLineSize = CAV268::LineSize / factor;

		if (processCommand(CAV268::Commands::GetPresenterStatus, &answer) && (answer.size() <= 10) && (answer.size() >= 2) &&
			answer.startsWith(CAV268::Answers::GetPresenterStatus))
		{
			bool isAVPlus = (DIPSettings & CAV268::DIPSwitches::Presenter) && (answer.size() == 10) &&
				((answer[1] == CAV268::Answers::Presenter::Enable) ||
				 (answer[1] == CAV268::Answers::Presenter::Disable));

			if (isAVPlus)
			{
				mDeviceName = "SysFuture AV-268 Plus";
				mModelType = Plus;
			}
			else
			{
				mDeviceName = "SysFuture AV-268";
				mModelType = Extended;
			}

			if ((answer[1] == CAV268::Answers::Presenter::Enable)  ||
				(answer[1] == CAV268::Answers::Presenter::Disable) ||
				(answer[1] == CAV268::Answers::Presenter::NotAvaiable))
			{
				ushort errorCode = answer[3] | answer[2] << 8;

				if (errorCode & CAV268::Statuses::PaperJam)
				{
					aStatusCodes.insert(Error::PaperJam);
				}

				if (errorCode & CAV268::Statuses::Presenter)
				{
					aStatusCodes.insert(Error::Presenter);
				}

				if (errorCode & CAV268::Statuses::PowerSupply)
				{
					aStatusCodes.insert(DeviceStatusCode::Error::PowerSupply);
				}
			}
			else
			{
				toLog(LogLevel::Error, "AV268: Wrong answer for presenter request");

				if (mModelType == Plus)
				{
					aStatusCodes.insert(Error::Presenter);
				}
			}
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
bool AV268::receiptProcessing()
{
	bool result = TSerialPrinterBase::receiptProcessing();
	SleepHelper::msleep(CPortPrinter::PrintingStringTimeout * mActualStringCount);
	waitAvailable();

	return result;
}

//--------------------------------------------------------------------------------
