/* @file Прокси-класс для работы с рекламным контентом в скриптах. */

#pragma once

#include <SDK/GUI/IAdSource.h>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QVariantMap>
#include <Common/QtHeadersEnd.h>

namespace SDK {
namespace PaymentProcessor {

class ICore;

namespace Scripting {

namespace CAdService
{
	const char DefaultBanner[] = "banner";
}

//------------------------------------------------------------------------------
class AdService : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString receiptHeader READ getReceiptHeader CONSTANT)
	Q_PROPERTY(QString receiptFooter READ getReceiptFooter CONSTANT)

public:
	AdService(ICore * aCore);

public slots:
	// Добавить рекламное событие в статистику
	void addEvent(const QString & aEvent, const QVariantMap & aParameters);

	// Имя баннера
	QString getBanner(const QString & aBanner = QString(CAdService::DefaultBanner));

private:
	// Рекламный текст для шапки/подвала на чеке
	QString getReceiptHeader();
	QString getReceiptFooter();

	SDK::GUI::IAdSource * getAdSource();
	QString getContent(const QString & aName);

private:
	ICore * mCore;
	SDK::GUI::IAdSource * mAdSource;
};

//------------------------------------------------------------------------------
}}} // Scripting::PaymentProcessor::SDK
