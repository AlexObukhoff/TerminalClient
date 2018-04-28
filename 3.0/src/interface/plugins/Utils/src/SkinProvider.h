/* @file Плагин для отрисовки изображений интерфейса пользователя */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTimer>
#include <QtCore/QMap>
#include <QtQuick/QQuickImageProvider>
#include <Common/QtHeadersEnd.h> 

#include "Utils.h"

//------------------------------------------------------------------------------
class SkinProvider : public QObject, public QQuickImageProvider
{
	Q_OBJECT

	Q_PROPERTY(QObject * ui READ getSkin CONSTANT);

public:
	SkinProvider(const QObject * aApplication, const QString & aInterfacePath, const QString & aLogoPath, const QString & aUserLogoPath, const Utils::TSkinConfig & aSkinConfig);
	~SkinProvider() {};

	virtual QImage requestImage(const QString & aId, QSize * aSize, const QSize & aRequestedSize);

private:
	QString getImagePath(const QString & aImageId);

private:
	QObject * getSkin() const;

private:
	QString mLogoPath;
	QString mUserLogoPath;
	QString mInterfacePath;
	Utils::TSkinConfig mSkinConfig2;
	QMap<QString, QSharedPointer<Skin>> mSkins;

	QHash<QString, QImage> mBackgrounds;
	QHash<QString, QImage> mLogos;
	const QObject * mApplication;
	QPointer<QObject> mPaymentService;
	QPointer<QObject> mGuiService;
	QObject * mCurrentSkin;
	QMap<QString, QImage> mImageCache;
};

//------------------------------------------------------------------------------
