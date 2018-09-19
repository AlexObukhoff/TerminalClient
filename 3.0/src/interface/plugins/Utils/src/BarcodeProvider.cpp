/* @file Плагин для отрисовки штрихкодов в интерфейсе пользователя */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtDeclarative/QtDeclarative>
#include <QtCore/QDir>
#include <QtCore/QSettings>
#include <Common/QtHeadersEnd.h>

// Thirdparty
#include <qzint.h>

// Проект
#include "Log.h"
#include "BarcodeProvider.h"

//------------------------------------------------------------------------------
BarcodeProvider::BarcodeProvider() :
	QDeclarativeImageProvider(QDeclarativeImageProvider::Image),
	mDefaultBarcodeSize(200, 200)
{
}

//------------------------------------------------------------------------------
QImage BarcodeProvider::requestImage(const QString & aId, QSize * aSize, const QSize & aRequestedSize)
{
	QSize barcodeSize = aRequestedSize.isValid() && !aRequestedSize.isEmpty() ? aRequestedSize : mDefaultBarcodeSize;
	QString path = QString("%1|%2x%3").arg(aId).arg(barcodeSize.width()).arg(barcodeSize.height());

	if (mBarcodeCache.contains(path))
	{
		return mBarcodeCache.value(path);
	}

	QColor fgColor = QColor("black");
	QColor bgColor = QColor("transparent");
	QString text;
	int symbol = BARCODE_QRCODE;
	int whitespace = 0;
	int height = barcodeSize.height();

	// Парсим параметры
	foreach (auto param, aId.split("&", QString::SkipEmptyParts))
	{
		QStringList nameValue = param.split("=");
		if (nameValue.size() != 2)
		{
			continue;
		}
		QString name = nameValue[0].toLower();
		QString value = QByteArray::fromPercentEncoding(nameValue[1].toLatin1());

		if (name == "bgcolor")
		{
			bgColor = QColor(value);
		}
		else if (name == "fgcolor")
		{
			fgColor = QColor(value);
		}
		else if (name == "text")
		{
			text = value;
		}
		else if (name == "type")
		{
			if (value == "code128")
			{
				symbol = BARCODE_CODE128;
			}
		}
		else if (name == "whitespace")
		{
			whitespace = value.toInt();
		}
		else if (name == "height")
		{
			height = value.toInt();
		}
	}

	QImage image(barcodeSize, QImage::Format_ARGB32);
	image.fill(QColor("transparent"));

	Zint::QZint zint;

	zint.setWidth(0);
	zint.setHeight(height);
	zint.setText(text);
	zint.setWhitespace(whitespace);
	zint.setBorderType(Zint::QZint::NO_BORDER);
	zint.setSecurityLevel(0);
	zint.setInputMode(DATA_MODE);
	zint.setHideText(true);
	zint.setSymbol(symbol);
	zint.setFgColor(fgColor);
	zint.setBgColor(bgColor);

	{
		QPainter painter(&image);
		zint.render(painter, QRectF(0, 0, image.width(), image.height()), Zint::QZint::KeepAspectRatio);
		painter.end();
	}

	if (zint.hasErrors())
	{
		Log(Log::Error) << QString("BarcodeProvider: failed render barcode '%1': %2.").arg(aId).arg(zint.lastError());
	}
	else
	{
		mBarcodeCache.insert(path, image);
	}

	*aSize = image.size();
	return image;
}

//------------------------------------------------------------------------------
