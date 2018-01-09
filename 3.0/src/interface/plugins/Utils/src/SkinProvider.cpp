/* @file Плагин для отрисовки изображений интерфейса пользователя */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtDeclarative/QtDeclarative>
#include <QtCore/QDir>
#include <QtCore/QSettings>
#include <Common/QtHeadersEnd.h>

// Проект
#include "Log.h"
#include "Skin.h"
#include "SkinProvider.h"

//------------------------------------------------------------------------------
SkinProvider::SkinProvider(const QString & aInterfacePath, const QString & aContentPath, const QString & aUserPath, const Skin * aSkin) :
	QDeclarativeImageProvider(QDeclarativeImageProvider::Image),
	mLogoPath(aContentPath + "/logo"),
	mUserLogoPath(aUserPath + "/logo"),
	mInterfacePath(aInterfacePath)
{
	mSkinName = aSkin->getName();
	mSkinConfig = aSkin->getConfiguration();
}

//------------------------------------------------------------------------------
QImage SkinProvider::requestImage(const QString & aId, QSize * aSize, const QSize & aRequestedSize)
{
	if (!aId.contains("logoprovider"))
	{
		QString path = mInterfacePath + QDir::separator() + "skins" + QDir::separator() + mSkinName + QDir::separator() + getImagePath(aId);

		if (!QFile::exists(path))
		{
			Log(Log::Warning) << QString("SkinProvider: failed to load texture '%1' from '%2'. Try loading texture from 'default'.").arg(aId).arg(path);
			path = mInterfacePath + QDir::separator() + "skins" + QDir::separator() + "default" + QDir::separator() + getImagePath(aId);
		}

		QImage image;

		if (image.load(path))
		{
			if (aRequestedSize.isValid())
			{
				//TODO
				image = image.scaled(aRequestedSize);
			}

			*aSize = image.size();
			
			return image;
		}
		else
		{
			Log(Log::Error) << QString("SkinProvider: failed to load texture %1 from '%2'.").arg(aId).arg(path);
		}
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
			QString path = mInterfacePath + QDir::separator() + "skins"+ QDir::separator() + mSkinName + QDir::separator() + getImagePath(background);
			QImage img;

			if (img.load(path))
			{
				bimage = mBackgrounds.insert(background, img);
			}
			else
			{
				Log(Log::Error) << QString("SkinProvider: failed to load logo background '%1' from '%2'.").arg(background).arg(path);
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
QString SkinProvider::getImagePath(const QString & aImageId) const
{
	QVariantMap::const_iterator it = mSkinConfig.find(aImageId);
	
	return it != mSkinConfig.end() ? it->toString() : "";
}

//------------------------------------------------------------------------------
