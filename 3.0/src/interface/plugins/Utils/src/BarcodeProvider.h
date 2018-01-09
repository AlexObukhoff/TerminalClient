/* @file Плагин для отрисовки штрихкодов в интерфейсе пользователя */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMap>
#include <QtDeclarative/QDeclarativeExtensionPlugin>
#include <QtDeclarative/QDeclarativeImageProvider>
#include <Common/QtHeadersEnd.h>

/*------------------------------------------------------------------------------
 Пример использования в QML

 Image {
	 id: code128
	 source: "image://barcode/fgcolor=black&bgcolor=white&text=01234567890&type=code128&whitespace=20&height=100"
	 width: 500
	 height: 100
 }
 Image {
	 id: qrcode
	 source: "image://barcode/fgcolor=black&bgcolor=white&text=panel.secondary&whitespace=2"
	 width: 300
	 height: 300
 }

 Параметры провайдера:

	 fgcolor - цвет штрих-кода
	 bgcolor - цвет фона 
	 type - ['code128','qr'] - тип штрих-кода ('qr' по умолчанию)
	 whitespace - размер отступов в относительных единицах (1-3 достаточно)
	 height - высота штрихов в пикселах для code128

*/

//------------------------------------------------------------------------------
class BarcodeProvider : public QDeclarativeImageProvider
{
public:
	BarcodeProvider();
	~BarcodeProvider() {};

	virtual QImage requestImage(const QString & aId, QSize * aSize, const QSize & aRequestedSize);

private:
	QSize mDefaultBarcodeSize;
	QHash<QString, QImage> mBarcodeCache;
};

//------------------------------------------------------------------------------
