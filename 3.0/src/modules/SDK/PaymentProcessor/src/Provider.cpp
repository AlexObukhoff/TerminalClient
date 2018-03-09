/* @file Описание платёжного оператора. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QDebug>
#include <Common/QtHeadersEnd.h>

// Project
#include "Provider.h"

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
SProvider::SProcessingTraits::SRequest::SResponseField::SResponseField()
{
	required = true;
	encoding = Text;
	codepage = Windows1251;
}

//------------------------------------------------------------------------------
void SProvider::SProcessingTraits::SRequest::SResponseField::setCodepage(const QString & aCodepage)
{
	if (aCodepage.contains("utf8", Qt::CaseInsensitive) || aCodepage.contains("utf-8", Qt::CaseInsensitive))
	{
		codepage = Utf8;
	}
	else
	{
		codepage = Windows1251;
	}
}

//------------------------------------------------------------------------------
void SProvider::SProcessingTraits::SRequest::SResponseField::setEncoding(const QString & aEncoding)
{
	if (aEncoding.contains("url", Qt::CaseInsensitive))
	{
		encoding = Url;
	}
	else if (aEncoding.contains("base64", Qt::CaseInsensitive))
	{
		encoding = Base64;
	}
	else 
	{
		encoding = Text;
	}
}

//------------------------------------------------------------------------------
QJsonObject enumToJson(const SProviderField::SEnumItem & aEnumItem)
{
	QJsonObject item;

	item["name"] = aEnumItem.title;
	item["value"] = aEnumItem.value;
	item["sort"] = aEnumItem.sort;

	QJsonArray subitems;
	foreach (auto item, aEnumItem.subItems)
	{
		subitems << enumToJson(item);
	}

	item["subItems"] = subitems;

	return item;
}

//------------------------------------------------------------------------------
QString SProvider::fields2Json(const TProviderFields & aFields)
{
	QJsonArray jsonFields;

	foreach (auto field, aFields)
	{
		QJsonObject jsonField;

		jsonField["type"] = field.type;
		jsonField["id"] = field.id;
		jsonField["keyboardType"] = field.keyboardType;
		jsonField["letterCase"] = field.letterCase;
		jsonField["language"] = field.language;
		jsonField["sort"] = field.sort;
		jsonField["minSize"] = field.minSize;
		jsonField["maxSize"] = field.maxSize;
		jsonField["isRequired"] = field.isRequired;
		jsonField["title"] = field.title;
		jsonField["comment"] = field.comment;
		jsonField["extendedComment"] = field.extendedComment;
		jsonField["mask"] = field.mask;
		jsonField["isPassword"] = field.isPassword;
		jsonField["behavior"] = field.behavior;
		jsonField["format"] = field.format;
		jsonField["defaultValue"] = field.defaultValue;
		jsonField["dependency"] = field.dependency;
		jsonField["url"] = field.url;
		jsonField["html"] = field.html;
		jsonField["back_button"] = field.backButton;
		jsonField["forward_button"] = field.forwardButton;

		if (!field.security.isEmpty())
		{
			QJsonObject security;

			foreach(auto subsystem, field.security.keys())
			{
				switch (subsystem)
				{
				case SProviderField::Default:
					security["default"] = field.security.value(subsystem);
					break;
				case SProviderField::Display:
					security["display"] = field.security.value(subsystem);
					break;
				case SProviderField::Log:
					security["log"] = field.security.value(subsystem);
					break;
				case SProviderField::Printer:
					security["printer"] = field.security.value(subsystem);
					break;
				}
			}

			jsonField["security"] = security;
		}

		QJsonArray enumItems;
		foreach (auto item, field.enumItems)
		{
			enumItems << enumToJson(item);
		}
		QJsonObject o;
		o.insert("values", enumItems);
		jsonField["enumItems"] = o;

		jsonFields << jsonField;
	}

	return QString::fromUtf8(QJsonDocument(jsonFields).toJson(QJsonDocument::Compact));
}

//------------------------------------------------------------------------------
bool json2Enum(const QJsonObject & aEnumItem, SProviderField::SEnumItem & aItem)
{
	if (aEnumItem.isEmpty())
	{
		return false;
	}

	aItem.title = aEnumItem["name"].toString();
	aItem.value = aEnumItem["value"].toString();
	aItem.sort = aEnumItem["sort"].toInt();

	foreach (auto jsonSubitem, aEnumItem["subItems"].toArray())
	{
		SProviderField::SEnumItem subItem;
		if (json2Enum(jsonSubitem.toObject(), subItem))
		{
			aItem.subItems << subItem;
		}
	}

	return true;
}

//------------------------------------------------------------------------------
TProviderFields SProvider::json2Fields(const QString & aJson)
{
	TProviderFields fields;

	QJsonParseError error;
	QJsonArray result = QJsonDocument::fromJson(aJson.toUtf8(), &error).array();

	if (QJsonParseError::NoError == error.error)
	{
		foreach (auto jsonFieldVariant, result)
		{
			auto jsonField = jsonFieldVariant.toObject();

			SProviderField field;

			field.type = jsonField["type"].toString();
			field.id = jsonField["id"].toString();
			field.keyboardType = jsonField["keyboardType"].toString();
			field.letterCase = jsonField["letterCase"].toString();
			field.language = jsonField["language"].toString();
			field.sort = jsonField["sort"].toInt();
			field.minSize = jsonField["minSize"].toInt();
			field.maxSize = jsonField["maxSize"].toInt();
			field.isRequired = jsonField["isRequired"].toBool();
			field.title = jsonField["title"].toString();
			field.comment = jsonField["comment"].toString();
			field.extendedComment = jsonField["extendedComment"].toString();
			field.mask = jsonField["mask"].toString();
			field.isPassword = jsonField["isPassword"].toBool();
			field.behavior = jsonField["behavior"].toString();
			field.format = jsonField["format"].toString();
			field.defaultValue = jsonField["defaultValue"].toString();
			field.url = jsonField["url"].toString();
			field.html = jsonField["html"].toString();
			field.backButton = jsonField["back_button"].toString();
			field.forwardButton = jsonField["forward_button"].toString();
			field.dependency = jsonField["dependency"].toString();

			foreach (auto jsonItem, jsonField["enumItems"].toObject()["values"].toArray())
			{
				SProviderField::SEnumItem item;
				if (json2Enum(jsonItem.toObject(), item))
				{
					field.enumItems << item;
				}
			}

			if (jsonField.contains("security"))
			{
				auto security = jsonField["security"].toObject();

				if (security.contains("default")) { field.security.insert(SProviderField::Default, security.value("default").toString()); }
				if (security.contains("display")) { field.security.insert(SProviderField::Display, security.value("display").toString()); }
				if (security.contains("log")) { field.security.insert(SProviderField::Log, security.value("log").toString()); }
				if (security.contains("printer")) { field.security.insert(SProviderField::Printer, security.value("printer").toString()); }
			}

			fields << field;
		}
	}
	else
	{
		qDebug() << QString("Error parse provider json(%1): %2").arg(error.offset).arg(error.errorString());
	}

	return fields;
}


//------------------------------------------------------------------------------
}} // PaymentProcessor::SDK
