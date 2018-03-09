/* @file Item, отрисовывающий flash */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtWebKitWidgets/QWebFrame>
#include <Common/QtHeadersEnd.h>

// Project
#include "FlashPlayerItem.h"

//--------------------------------------------------------------------------
FlashPlayerItem::FlashPlayerItem(QDeclarativeItem * aParent)
	: QDeclarativeItem(aParent)
{
	setFlag(QGraphicsItem::ItemHasNoContents, false);

	QWebSettings *defaultSettings = QWebSettings::globalSettings();
	defaultSettings->setAttribute(QWebSettings::JavascriptEnabled, true);
	defaultSettings->setAttribute(QWebSettings::PluginsEnabled, true);

	mWebView = new QGraphicsWebView(this);

	connect(mWebView->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
		this, SLOT(populateJavaScriptWindowObject()));

	mHtmlWrapper = "<body marginwidth='0' marginheight='0'> \
								 <embed width='%1' height='%2' name='plugin' src='%3' wmode='transparent' type='application/x-shockwave-flash'> \
								 <script>function click(aParameters) { handler.onClicked(aParameters); }</script></body>";
}

//--------------------------------------------------------------------------
FlashPlayerItem::~FlashPlayerItem()
{
}

//--------------------------------------------------------------------------
QString FlashPlayerItem::getMovie() const
{
	return mMovie;
}

//--------------------------------------------------------------------------
void FlashPlayerItem::setMovie(const QString & aMovie)
{
	mMovie = aMovie;

	QString html = QString(mHtmlWrapper)
		.arg(QString::number(mGeometry.width()))
		.arg(QString::number(mGeometry.height()))
		.arg(mMovie);

	mWebView->setHtml(html);
}

//--------------------------------------------------------------------------
void FlashPlayerItem::geometryChanged(const QRectF& aNewGeometry, const QRectF& aOldGeometry)
{
	if (aNewGeometry != aOldGeometry)
	{
		mWebView->setGeometry(QRectF(0., 0., aNewGeometry.width(), aNewGeometry.height()));
		mGeometry = aNewGeometry;

		if (!mMovie.isEmpty())
		{
			QString html = QString(mHtmlWrapper)
				.arg(QString::number(mGeometry.width()))
				.arg(QString::number(mGeometry.height()))
				.arg(mMovie);

			mWebView->page()->mainFrame()->setHtml(html);
		}
	}

	QDeclarativeItem::geometryChanged(aNewGeometry, aOldGeometry);
}

//--------------------------------------------------------------------------
void FlashPlayerItem::populateJavaScriptWindowObject()
{
	mWebView->page()->mainFrame()->addToJavaScriptWindowObject("handler", this);
}

//--------------------------------------------------------------------------
