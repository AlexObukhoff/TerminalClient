/* @file Прокси-класс для работы со сканером в скриптах. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <QtCore/QBuffer>
#include <QtGui/QImage>
#include <QtScript/QScriptEngine>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IHIDService.h>
#include <SDK/PaymentProcessor/Core/IPaymentService.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>
#include <SDK/PaymentProcessor/Scripting/HIDService.h>
#include <SDK/Drivers/HardwareConstants.h>

namespace PPSDK = SDK::PaymentProcessor;

namespace SDK {
namespace PaymentProcessor {
namespace Scripting {

//------------------------------------------------------------------------------
HIDService::HIDService(ICore * aCore)
	: mCore(aCore),
	  mService(mCore->getHIDService())
{
	connect(mService, SIGNAL(error()), SIGNAL(error()));
	
	// Сигналы из ядра завернем в общий скртптовый сигнал hiddata
	connect(mService, SIGNAL(ejected()), this, SLOT(onEjected()));
	connect(mService, SIGNAL(inserted(QVariantMap)), this, SLOT(onInserted(QVariantMap)));

	connect(mService, SIGNAL(data(const QVariantMap &)), this, SLOT(onData(const QVariantMap &)), Qt::QueuedConnection);
}

//------------------------------------------------------------------------------
void HIDService::enable(const QString & aName)
{
	mService->setEnable(true, aName);
}

//------------------------------------------------------------------------------
void HIDService::disable(const QString & aName)
{
	mService->setEnable(false, aName);
}

//------------------------------------------------------------------------------
void HIDService::updateParameters(const QVariantMap & aParameters)
{
	mParameters = aParameters;
}

//------------------------------------------------------------------------------
QString HIDService::getExternalData()
{
	qint64 providerId = mParameters[SDK::PaymentProcessor::CPayment::Parameters::Provider].toLongLong();
	
	return mCore->getPaymentService()->getProvider(providerId).externalDataHandler;
}

//------------------------------------------------------------------------------
QScriptValue JS_setField(QScriptContext * aContext, QScriptEngine *, void * aParameters)
{
	reinterpret_cast<QVariantMap *>(aParameters)->insert(aContext->argument(0).toString(), aContext->argument(1).toString());

	return QScriptValue();
}

//------------------------------------------------------------------------------
void HIDService::onData(const QVariantMap & aDataMap)
{
	QVariantMap parameters;
	QString value;

	if (aDataMap.contains(CHardwareSDK::HID::Text))
	{
		value = mService->valueToString(aDataMap.value(CHardwareSDK::HID::Text));

		parameters.insert(HID::SOURCE, HID::SOURCE_SCANNER);
		parameters.insert(HID::STRING, value);
		parameters.insert(HID::RAW, value);
		parameters.insert(HID::RAW_BASE64, QString(value.toLatin1().toBase64()));

		parameters.insert(HID::EXTERNAL_DATA, false);

		QString externalDataHandler = getExternalData();
		if (!externalDataHandler.trimmed().isEmpty() && !value.isEmpty())
		{
			QScriptEngine script;

			script.globalObject().setProperty("value", value);
			QScriptValue setFunc = script.newFunction(JS_setField, &parameters);
			script.globalObject().setProperty("setField", setFunc);

			if (!script.canEvaluate(externalDataHandler))
			{
				toLog(LogLevel::Warning, QString("Can't parse expression: %1").arg(externalDataHandler));
				return;
			}

			script.evaluate(externalDataHandler);
			if (script.hasUncaughtException())
			{
				toLog(LogLevel::Error, QString("An exception occured while calling (line %1): %2\nBacktrace:\n%3.")
					.arg(script.uncaughtExceptionLineNumber())
					.arg(script.uncaughtException().toString())
					.arg(script.uncaughtExceptionBacktrace().join("\n")));

				return;
			}

			parameters.insert(HID::EXTERNAL_DATA, true);
		}
	}

	if (aDataMap.contains(CHardwareSDK::HID::Image))
	{
		bool faceDetected = aDataMap.value(CHardwareSDK::HID::FaceDetected, false).value<bool>();

		QImage image = aDataMap.value(CHardwareSDK::HID::Image).value<QImage>();
		QBuffer buffer;
		buffer.open(QIODevice::WriteOnly);
		image.convertToFormat(QImage::Format_RGB16).save(&buffer, "jpg");

		parameters.insert(HID::SOURCE, HID::SOURCE_CAMERA);
		parameters.insert(HID::RAW, image);
		parameters.insert(HID::RAW_BASE64, QString(buffer.data().toBase64()));
		parameters.insert(HID::CAMERA_FACE_DETECTED, faceDetected);

		if (faceDetected)
		{
			QBuffer faceBuffer;
			faceBuffer.open(QIODevice::WriteOnly);
			aDataMap.value(CHardwareSDK::HID::ImageWithFaceArea).value<QImage>().convertToFormat(QImage::Format_RGB16).save(&faceBuffer, "jpg");
			parameters.insert(HID::CAMERA_FACE_DETECTED_IMAGE, QString(faceBuffer.data().toBase64()));
		}
	}

	emit HIDData(parameters);
}

//------------------------------------------------------------------------------
void HIDService::onInserted(const QVariantMap & aData)
{
	QVariantMap data;

	data.insert(HID::SOURCE, HID::SOURCE_CARD);
	data.insert(HID::SIGNAL, HID::SIGNAL_INSERT);

	foreach(QString name, aData.keys())
	{
		data.insert(name, aData[name]);
	}

	emit HIDData(data);
}

//------------------------------------------------------------------------------
void HIDService::onEjected()
{
	QVariantMap data;

	data.insert(HID::SOURCE, HID::SOURCE_CARD);
	data.insert(HID::SIGNAL, HID::SIGNAL_EJECT);

	emit HIDData(data);
}

//------------------------------------------------------------------------------
void HIDService::executeExternalHandler(const QVariantMap & aExpression)
{
	emit externalHandler(aExpression);
}

//------------------------------------------------------------------------------

}}} // Scripting::PaymentProcessor::SDK
