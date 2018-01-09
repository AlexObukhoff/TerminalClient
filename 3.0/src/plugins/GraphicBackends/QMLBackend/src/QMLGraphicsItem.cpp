/* @file Графический элемент QML. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QUrl>
#include <QtCore/QDir>
#include <QtDeclarative/QDeclarativeItem>
#include <Common/QtHeadersEnd.h>

// Модули
#include <Common/ILog.h>

// Проект
#include "QMLGraphicsItem.h"

//---------------------------------------------------------------------------
namespace CQMLGraphicsItem
{
	const char ItemKey[] = "item";

	const char ShowHandlerName[] = "showHandler";
	const char ShowHandlerSignature[] = "showHandler()";

	const char HideHandlerName[] = "hideHandler";
	const char HideHandlerSignature[] = "hideHandler()";

	const char ResetHandlerName[] = "resetHandler";
	const char ResetHandlerSignature[] = "resetHandler(const QVariant &)";

	const char NotifyHandlerName[] = "notifyHandler";
	const char NotifyHandlerSignature[] = "notifyHandler(const QVariant &, const QVariant &)";
}

//---------------------------------------------------------------------------
QMLGraphicsItem::QMLGraphicsItem(const SDK::GUI::GraphicsItemInfo & aInfo, QDeclarativeEngine * aEngine, ILog * aLog)
	: mLog(aLog),
	  mEngine(aEngine),
	  mItem(0),
	  mInfo(aInfo)
{
	QString qmlPath = QDir::toNativeSeparators(QDir::cleanPath(aInfo.directory + QDir::separator() + aInfo.parameters[CQMLGraphicsItem::ItemKey]));
	QDeclarativeComponent component(mEngine, qmlPath.startsWith("qrc") ? QUrl(qmlPath) : QUrl::fromLocalFile(qmlPath));

	QObject * object = component.create();
	if (object)
	{
		mItem = QSharedPointer<QDeclarativeItem>(qobject_cast<QDeclarativeItem *>(object));
	}
	else
	{
		foreach (QDeclarativeError error, component.errors())
		{
			mError += error.toString() + "\n";
		}
	}
}

//---------------------------------------------------------------------------
void QMLGraphicsItem::show()
{
	if (mItem->metaObject()->indexOfMethod(CQMLGraphicsItem::ShowHandlerSignature) != -1)
	{
		QMetaObject::invokeMethod(mItem.data(), CQMLGraphicsItem::ShowHandlerName, Qt::DirectConnection);
	}
}

//---------------------------------------------------------------------------
void QMLGraphicsItem::hide()
{
	if (mItem->metaObject()->indexOfMethod(CQMLGraphicsItem::HideHandlerSignature) != -1)
	{
		QMetaObject::invokeMethod(mItem.data(), CQMLGraphicsItem::HideHandlerName, Qt::DirectConnection);
	}
}

//---------------------------------------------------------------------------
void QMLGraphicsItem::reset(const QVariantMap & aParameters)
{
	if (mItem->metaObject()->indexOfMethod(QMetaObject::normalizedSignature(CQMLGraphicsItem::ResetHandlerSignature)) != -1)
	{
		QVariant error;
		QMetaObject::invokeMethod(mItem.data(), CQMLGraphicsItem::ResetHandlerName, Qt::DirectConnection,
			Q_RETURN_ARG(QVariant, error), Q_ARG(const QVariant &, QVariant::fromValue(aParameters)));

		if (!error.isNull())
		{
			mLog->write(LogLevel::Error, translateError(error));
		}
	}
}

//---------------------------------------------------------------------------
void QMLGraphicsItem::notify(const QString & aEvent, const QVariantMap & aParameters)
{
	if (mItem->metaObject()->indexOfMethod(QMetaObject::normalizedSignature(CQMLGraphicsItem::NotifyHandlerSignature)) != -1)
	{
		QVariant error;
		QMetaObject::invokeMethod(mItem.data(), CQMLGraphicsItem::NotifyHandlerName, Qt::DirectConnection,
			Q_RETURN_ARG(QVariant, error), Q_ARG(const QVariant &, QVariant::fromValue(aEvent)),
			Q_ARG(const QVariant &, QVariant::fromValue(aParameters)));

		if (!error.isNull())
		{
			mLog->write(LogLevel::Error, translateError(error));
		}
	}
}

//---------------------------------------------------------------------------
QGraphicsItem * QMLGraphicsItem::getWidget() const
{
	return mItem.data();
}

//---------------------------------------------------------------------------
QVariantMap QMLGraphicsItem::getContext() const
{
	return mInfo.context;
}

//---------------------------------------------------------------------------
bool QMLGraphicsItem::isValid() const
{
	return !mItem.isNull();
}

//---------------------------------------------------------------------------
QString QMLGraphicsItem::getError() const
{
	return mError;
}

//---------------------------------------------------------------------------
QString QMLGraphicsItem::translateError(const QVariant & aError) const
{
	QVariantMap e(aError.value<QVariantMap>());

	return QString("%1:%2 %3").
		arg(e["fileName"].toString()).
		arg(e["lineNumber"].toString()).
		arg(e["message"].toString());
}

//---------------------------------------------------------------------------
