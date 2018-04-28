/* @file Плагин для отрисовки изображений интерфейса пользователя */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtQuick/QQuickImageProvider>
#include <QtCore/QDir>
#include <QtCore/QSettings>
#include <QtGui/QPainter>
#include <QtGui/QTextLayout>
#include <Common/QtHeadersEnd.h>

// Проект
#include "Log.h"
#include "Skin.h"
#include "SkinProvider.h"

//------------------------------------------------------------------------------
SkinProvider::SkinProvider(
	const QObject * aApplication, 
	const QString & aInterfacePath, 
	const QString & aContentPath, 
	const QString & aUserPath, 
	const Utils::TSkinConfig & aSkinConfig
) :
	QQuickImageProvider(QQmlImageProviderBase::Image),
	mApplication(aApplication),
	mLogoPath(aContentPath + "/logo"),
	mUserLogoPath(aUserPath + "/logo"),
	mInterfacePath(aInterfacePath),
	mSkinConfig2(aSkinConfig),
	mCurrentSkin(nullptr)
{
	mPaymentService = mApplication->property("payment").value<QObject *>();
	mGuiService = mApplication->property("graphics").value<QObject *>();

	// Зарегистрируем шрифты всех скинов
	QDirIterator dirEntry(QString("%1/skins").arg(mInterfacePath), QString("*.ttf*;*.otf").split(";"), QDir::Files, QDirIterator::Subdirectories);
	while (dirEntry.hasNext())
	{
		dirEntry.next();

		if (QFontDatabase::addApplicationFont(dirEntry.filePath()) == -1)
		{
			Log(Log::Debug) << QString("Failed to add font '%1'.").arg(dirEntry.fileName());
		}
	}

	foreach (qint64 o, mSkinConfig2.keys())
	{
		QString name = mSkinConfig2.value(o);
		mSkins.insert(name, QSharedPointer<Skin>(new Skin(aInterfacePath, name)));
	}
}

//------------------------------------------------------------------------------
QImage SkinProvider::requestImage(const QString & aId, QSize * aSize, const QSize & aRequestedSize)
{
	if (!aId.contains("logoprovider"))
	{
		QString path = getImagePath(aId);

		if (path.isEmpty())
		{
			return QImage();
		}

		QImage i;
		i.load(path);
		*aSize = i.size();

		return aRequestedSize.isValid() ? i.scaled(aRequestedSize) : i;

		/*Log(Log::Debug) << QString("requestImage [%1] %2").arg(aId).arg(path);

		QImage image = mImageCache.value(path);
		
		if (image.isNull())
		{
			

			QImage i;

			if (i.load(path))
			{
				image = aRequestedSize.isValid() ? i.scaled(aRequestedSize) : i;

				Log(Log::Debug) << QString("IMAGE insert in cache [%1] %2").arg(aId).arg(path);

				mImageCache.insert(path, image);

				return image;
			}
			else
			{
				Log(Log::Debug) << QString("SkinProvider: failed to load texture %1 from '%2'.").arg(aId).arg(path);
			}
		}

		return image;*/
	}
	// Логотипы предварительно обработаем перед выдачей
	else
	{
		QString id = aId.section("/", 1, 1);
		QString background = aId.section("/", 2, 2);
		QString label = aId.section("/", 3);

		QHash<QString, QImage>::iterator ilogo = mLogos.find(aId);
		if (ilogo != mLogos.end())
		{
			return *ilogo;
		}

		QHash<QString, QImage>::iterator bimage = background.isEmpty() ? mBackgrounds.end() : mBackgrounds.find(background);

		// Загружаем фон
		if (bimage == mBackgrounds.end() && !background.isEmpty())
		{
			QString path = getImagePath(background);
			QImage img;

			if (img.load(path))
			{
				bimage = mBackgrounds.insert(background, img);
			}
			else
			{
				Log(Log::Debug) << QString("SkinProvider: failed to load logo background '%1' from '%2'.").arg(background).arg(path);
			}
		}

		// Сперва логотипы киберплата, затем пользователские
		QImage logo(mLogoPath + QDir::separator() + id + ".png");
		if (logo.isNull())
		{
			logo = QImage(mUserLogoPath + QDir::separator() + id + ".png");
		}

		QImage image(aRequestedSize.isValid() ? aRequestedSize : logo.size(), QImage::Format_ARGB32);

		if (bimage != mBackgrounds.end())
		{
			image = *bimage;
		}
		else
		{
			image.fill(qRgba(0, 0, 0, 0));
		}

		QPainter painter(&image);

		if (!logo.isNull())
		{
			painter.drawImage(QPoint((image.width() - logo.width()) / 2, (image.height() - logo.height()) / 2), logo);
		}
		else
		{
			QTextOption option(Qt::AlignHCenter);
			option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);

			QFont font("Roboto Condensed");
			font.setPixelSize(16);
			font.setBold(true);

			QString title = label.isEmpty() ? id : label;
			title.replace(title.indexOf('"'), 1, QChar(0x00AB));
			title.replace(title.lastIndexOf('"'), 1, QChar(0x00BB));
			title = title.toUpper();

			QTextLayout layout(title);
			layout.setTextOption(option);
			layout.setFont(font);
			layout.beginLayout();

			// Нарезаем строки
			qreal y = 0;
			QTextLine line = layout.createLine();

			while (line.isValid())
			{
				line.setLineWidth(image.width() - 44);
				line.setPosition(QPointF(0, y));
				y += line.height();

				if (layout.lineCount() == 3)
				{
					break;
				}

				line = layout.createLine();
			}

			layout.endLayout();
			layout.draw(&painter, QPointF(22, (image.rect().height() - layout.boundingRect().height()) / 2));
		}

		painter.end();
		*aSize = image.size();

		mLogos.insert(aId, image);

		return image;
	}

	return QImage();
}

//------------------------------------------------------------------------------
QString SkinProvider::getImagePath(const QString & aImageId)
{
	QPointer<QObject> userProps(mApplication->property("userProperties").value<QObject*>());	
	QVariantMap all = userProps->property("clone").value<QVariantMap>();
	qint64 providerId = all.value("operator_id").toString().toInt();
	if (!mSkinConfig2.keys().contains(providerId))
	{
		providerId = -1;
	}

	QString newImageId = aImageId.split("$").first();

	QString scene = mGuiService->property("topScene").value<QString>();
	QString pathWithScene = QString("%1/%2").arg(scene).arg(newImageId);
	QString skinName = mSkinConfig2.value(providerId);

	QVariantMap skinConfig = mSkins.value(skinName)->getConfiguration();
	QVariantMap::const_iterator it = skinConfig.find(pathWithScene);
	mCurrentSkin = mSkins.value(skinName).data();
		
	QString result;

	if (it != skinConfig.end())
	{
		result = mInterfacePath + QDir::separator() + "skins" + QDir::separator() + skinName + QDir::separator() + it->toString();
	}
	else
	{
		it = skinConfig.find(pathWithScene.split("/").last());

		if (it != skinConfig.end())
		{
			result = mInterfacePath + QDir::separator() + "skins" + QDir::separator() + skinName + QDir::separator() + it->toString();
		}
	}

	if (!QFile::exists(result))
	{
		Log(Log::Debug) << QString("SkinProvider: failed to load texture '%1' from '%2'. Try loading from 'default'.").arg(it->toString()).arg(result);
		result = mInterfacePath + QDir::separator() + "skins" + QDir::separator() + "default" + QDir::separator() + it->toString();

		if (!QFile::exists(result))
		{
			Log(Log::Debug) << QString("SkinProvider: failed to load texture '%1' from 'default'").arg(it->toString());
			return QString();
		}
	}
	
	return  result;
}

//------------------------------------------------------------------------------
QObject * SkinProvider::getSkin() const
{
	return mCurrentSkin ? mCurrentSkin : mSkins.value(mSkinConfig2.value(-1)).data();
}

//------------------------------------------------------------------------------
