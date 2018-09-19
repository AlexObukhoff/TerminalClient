/* @file ФР Pay PPU-700. */

#include "PayPPU700.h"

//--------------------------------------------------------------------------------
PayPPU700::PayPPU700()
{
	mDeviceName = CAtolFR::Models::PayPPU700K;
	mMaxBadAnswers = 4;
	mSupportedModels = QStringList() << mDeviceName;
}

//--------------------------------------------------------------------------------
void PayPPU700::setDeviceConfiguration(const QVariantMap & aConfiguration)
{
	QVariantMap configuration(aConfiguration);

	if (!configuration.contains(CHardware::FR::EjectorParameters) &&
		 configuration.contains(CHardware::Printer::Settings::Loop) &&
		 configuration.contains(CHardware::Printer::Settings::PreviousReceipt))
	{
		QString loop = configuration[CHardware::Printer::Settings::Loop].toString();
		QString previousReceipt = configuration[CHardware::Printer::Settings::PreviousReceipt].toString();

		QString ejectorParameters = CHardware::FR::Values::NoLoopAndPushNotTakenOnTimeout;

		if (previousReceipt == CHardware::Printer::Values::Retract)
		{
			ejectorParameters = CHardware::FR::Values::NoLoopAndRetractNotTakenOnTimeout;
		}
		else if (loop == CHardwareSDK::Values::Use)
		{
			ejectorParameters = CHardware::FR::Values::LoopAndPushNotTakenOnTimeout;
		}

		configuration.remove(CHardware::Printer::Settings::Loop);
		configuration.remove(CHardware::Printer::Settings::PreviousReceipt);
		configuration.insert(CHardware::FR::EjectorParameters, ejectorParameters);
	}

	AtolEjectorFR::setDeviceConfiguration(configuration);
}

//--------------------------------------------------------------------------------
bool PayPPU700::updateParameters()
{
	QString ejectorParameters = getConfigParameter(CHardware::FR::EjectorParameters).toString();
	char ejectorParametersValue = CPayPPU700::EjectorParameters::NoLoopAndRetractNotTakenOnTimeout;

	if (ejectorParameters == CHardware::FR::Values::LoopAndPushNotTakenOnTimeout)
	{
		ejectorParametersValue = CPayPPU700::EjectorParameters::LoopAndPushNotTakenOnTimeout;
	}
	else if (ejectorParameters == CHardware::FR::Values::NoLoopAndPushNotTakenOnTimeout)
	{
		ejectorParametersValue = CPayPPU700::EjectorParameters::NoLoopAndPushNotTakenOnTimeout;
	}

	int leftReceiptTimeout = getConfigParameter(CHardware::Printer::Settings::LeftReceiptTimeout).toInt();
	char leftReceiptTimeoutValue = (leftReceiptTimeout % ~CEjectorAtolFR::SpecialSettingMask) / 10;

	char ejectorMode = CPayPPU700::EjectorParameters::ZReportPush | ejectorParametersValue | leftReceiptTimeoutValue;

	return AtolEjectorFR::updateParameters() && setEjectorMode(ejectorMode);
}

//--------------------------------------------------------------------------------
