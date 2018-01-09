/* @file Многошаговый платёж через процессинг Киберплат. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtXml/QDomDocument>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Payment/Step.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>

// Project
#include "MultistagePayment.h"
#include "MultistagePaymentGetStepRequest.h"
#include "MultistagePaymentGetStepResponse.h"

//---------------------------------------------------------------------------
namespace CMultistage
{
	const QString Step = "MULTISTAGE_STEP";            /// текущий шаг 
	const QString StepFields = "MULTISTAGE_FIELDS_%1"; /// список полей для конкретного шага
	const QString History = "MULTISTAGE_HISTORY";      /// история шагов
}

namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
MultistagePayment::MultistagePayment(PaymentFactory * aFactory)
	: Payment(aFactory)
{
}

//---------------------------------------------------------------------------
bool MultistagePayment::canProcessOffline() const
{
	return false;
}

//---------------------------------------------------------------------------
bool MultistagePayment::performStep(int aStep)
{
	if (aStep == PPSDK::EPaymentStep::GetStep)
	{
		// Для проверки номера генерируем временную сессию.
		if (getSession().isEmpty())
		{
			setParameter(SParameter(PPSDK::CPayment::Parameters::Session, createPaymentSession(), true));
		}

		return getNextStep();
	}
	else
	{
		return Payment::performStep(aStep);
	}
}

//---------------------------------------------------------------------------
bool MultistagePayment::getNextStep()
{
	toLog(LogLevel::Normal, QString("Payment %1. %2, operator: %3 (%4), session: %5, step: %6.")
		.arg(getID())
		.arg(CPayment::Requests::GetStep)
		.arg(mProviderSettings.id)
		.arg(mProviderSettings.name)
		.arg(getSession())
		.arg(currentStep()));

	QScopedPointer<Request> request(createRequest(CPayment::Requests::GetStep));
	if (!request)
	{
		toLog(LogLevel::Error, QString("Payment %1. Failed to create request for operation.").arg(getID()));

		return false;
	}

	QUrl url(mProviderSettings.processor.requests[CPayment::Requests::GetStep].url);
	if (!url.isValid())
	{
		toLog(LogLevel::Error, QString("Payment %1. Can't find url for operation.").arg(getID()));

		setParameter(SParameter(PPSDK::CPayment::Parameters::ServerError, ELocalError::BadProvider, true));

		return false;
	}

	QScopedPointer<Response> response(sendRequest(url, *request));
	if (response)
	{
		if (response->isOk())
		{
			MultistagePaymentGetStepResponse * getStepResponse = dynamic_cast<MultistagePaymentGetStepResponse *>(response.data());

			toLog(LogLevel::Normal, QString("Payment %1. Get next step OK. New step %2.").arg(getID()).arg(getStepResponse->getMultistageStep()));

			SDK::PaymentProcessor::TProviderFields fields = parseFieldsXml(getStepResponse->getStepFields());
			
			nextStep(getStepResponse->getMultistageStep(), PPSDK::SProvider::fields2Json(fields));

			return true;
		}
	}
	else
	{
		setParameter(SParameter(PPSDK::CPayment::Parameters::ServerError, ELocalError::NetworkError, true));
	}

	return false;
}

//---------------------------------------------------------------------------
void MultistagePayment::nextStep(const QString & aStep, const QString & aFields)
{
	QStringList history = getHistory() << currentStep();

	setParameter(SParameter(CMultistage::History, history.join(";"), true));
	setParameter(SParameter(CMultistage::Step, aStep, true));
	setParameter(SParameter(CMultistage::StepFields.arg(aStep), aFields, true));
}
	
//---------------------------------------------------------------------------
Request * MultistagePayment::createRequest(const QString & aStep)
{
	if (aStep == CPayment::Requests::GetStep)
	{
		return new MultistagePaymentGetStepRequest(this);
	}

	return Payment::createRequest(aStep);
}

//------------------------------------------------------------------------------
Response * MultistagePayment::createResponse(const Request & aRequest, const QString & aResponseString)
{
	if (dynamic_cast<const MultistagePaymentGetStepRequest *>(&aRequest))
	{
		return new MultistagePaymentGetStepResponse(aRequest, aResponseString);
	}
	
	return Payment::createResponse(aRequest, aResponseString);
}

//---------------------------------------------------------------------------
QString MultistagePayment::currentStep() const
{
	const SParameter & stepParam = getParameter(CMultistage::Step);

	QString res = stepParam.value.toString();

	// 0 - default value
	return res.isEmpty() ? "0" : res;
}

//---------------------------------------------------------------------------
bool MultistagePayment::isFirstStep() const
{
	return (currentStep() == "0");
}

//---------------------------------------------------------------------------
bool MultistagePayment::isFinalStep() const
{
	return (currentStep() == CMultistage::Protocol::FinalStepValue);
}

//---------------------------------------------------------------------------
QStringList MultistagePayment::getHistory()
{
	PPSDK::IPayment::SParameter history = getParameter(CMultistage::History);
	if (history.value.isValid())
	{
		return history.value.toString().split(";", QString::SkipEmptyParts);
	}

	return QStringList();
}

//---------------------------------------------------------------------------
PPSDK::TProviderFields MultistagePayment::getFieldsForStep(const QString & aStep) const
{
	if (aStep.isEmpty() || aStep == "0")
	{
		return getProviderSettings().fields;
	}

	QString fieldsParam = getParameter(CMultistage::StepFields.arg(aStep)).value.toString();

	return PPSDK::SProvider::json2Fields(fieldsParam);
}

//---------------------------------------------------------------------------
void loadProviderEnumItems(PPSDK::SProviderField::TEnumItems & aItemList, QDomNode aNode)
{
	QDomElement itemElement = aNode.firstChildElement("item");

	while (!itemElement.isNull())
	{
		PPSDK::SProviderField::SEnumItem item;

		item.title = itemElement.attributeNode("name").nodeValue();
		item.value = itemElement.attributeNode("value").nodeValue();
		
		QString sortValue = itemElement.attributeNode("sort").nodeValue();
		item.sort  = sortValue.isEmpty() ? 65535 : sortValue.toInt();

		loadProviderEnumItems(item.subItems, itemElement);

		aItemList << item;

		itemElement = itemElement.nextSiblingElement("item");
	}

	qStableSort(aItemList.begin(), aItemList.end(), [](const PPSDK::SProviderField::SEnumItem & a, const PPSDK::SProviderField::SEnumItem & b){return a.sort < b.sort;});
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::TProviderFields MultistagePayment::parseFieldsXml(const QString & aFieldsXml) const
{
	auto toBool = [](QDomNode aNode, bool aDefault) -> bool
	{
		QString value = aNode.nodeValue();
		
		return (value == "true") ? true : (value == "false" ? false : aDefault);
	};

	auto toInt = [](QDomNode aNode, int aDefault) -> int
	{
		QString value = aNode.nodeValue();
		
		return value.isEmpty() ? aDefault : value.toInt();
	};

	SDK::PaymentProcessor::TProviderFields resultFields;

	QDomDocument doc("mydocument");
	if (doc.setContent(aFieldsXml))
	{
		QDomNodeList fieldsList = doc.elementsByTagName("field");

		for (int i = 0; i < fieldsList.size(); ++i)
		{
			QDomNode node = fieldsList.item(i);

			PPSDK::SProviderField field;
				
			field.id = node.attributes().namedItem("id").nodeValue();
			field.type = node.attributes().namedItem("type").nodeValue();
			field.keyboardType = node.attributes().namedItem("keyboard_type").nodeValue();
			field.isRequired = toBool(node.attributes().namedItem("required"), true);
			field.sort = toInt(node.attributes().namedItem("sort"), 65535);
			field.minSize = toInt(node.attributes().namedItem("min_size"), -1);
			field.maxSize = toInt(node.attributes().namedItem("max_size"), -1);
			field.language = node.attributes().namedItem("lang").nodeValue();
			field.letterCase = node.attributes().namedItem("case").nodeValue();

			field.title = node.firstChildElement("name").nodeValue();
			field.comment = node.firstChildElement("comment").nodeValue();
			field.extendedComment = node.firstChildElement("extended_comment").nodeValue();
			field.behavior = node.firstChildElement("behavior").nodeValue();
			field.mask = node.firstChildElement("mask").nodeValue();
			field.format = node.firstChildElement("format").nodeValue();

			field.dependency = node.firstChildElement("dependency").nodeValue();
			field.defaultValue = node.firstChildElement("default").nodeValue();

			loadProviderEnumItems(field.enumItems, node.firstChildElement("enum"));

			resultFields << field;
		}

		qStableSort(resultFields.begin(), resultFields.end(), [](const PPSDK::SProviderField & a, const PPSDK::SProviderField & b){return a.sort < b.sort;});
	}

	return resultFields;
}

//---------------------------------------------------------------------------
