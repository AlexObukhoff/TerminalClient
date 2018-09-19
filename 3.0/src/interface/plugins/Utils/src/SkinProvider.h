/* @file Плагин для отрисовки изображений интерфейса пользователя */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QHash>
#include <QtDeclarative/QDeclarativeExtensionPlugin>
#include <QtDeclarative/QDeclarativeImageProvider>
#include <Common/QtHeadersEnd.h>

class Skin;

//------------------------------------------------------------------------------
class SkinProvider : public QObject, public QDeclarativeImageProvider
{
	Q_OBJECT

public:
	SkinProvider(const QString & aInterfacePath, const QString & aLogoPath, const QString & aUserLogoPath, const Skin * aSkin);
	~SkinProvider() {};

	virtual QImage requestImage(const QString & aId, QSize * aSize, const QSize & aRequestedSize);

private:
	QString getImagePath(const QString & aImageId) const;

private:
	QString mLogoPath;
	QString mUserLogoPath;
	const Skin * mSkin;	
	QString mInterfacePath;

	QHash<QString, QImage> mBackgrounds;
	QHash<QString, QImage> mLogos;
};

//------------------------------------------------------------------------------
