/* @file Плагин для отрисовки изображений интерфейса пользователя */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMap>
#include <QtQuick/QQuickImageProvider>
#include <Common/QtHeadersEnd.h>

class Skin;

//------------------------------------------------------------------------------
class SkinProvider : public QQuickImageProvider
{
public:
	SkinProvider(const QString & aInterfacePath, const QString & aLogoPath, const QString & aUserLogoPath, const Skin * aSkin);
	~SkinProvider() {};

	virtual QImage requestImage(const QString & aId, QSize * aSize, const QSize & aRequestedSize);

private:
	QString getImagePath(const QString & aImageId) const;

private:
	QString mLogoPath;
	QString mUserLogoPath;
	QString mSkinName;
	QVariantMap mSkinConfig;
	QString mInterfacePath;

	QHash<QString, QImage> mBackgrounds;
	QHash<QString, QImage> mLogos;
};

//------------------------------------------------------------------------------
