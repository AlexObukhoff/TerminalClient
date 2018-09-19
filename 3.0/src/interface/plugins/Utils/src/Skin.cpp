/* @file Класс для конвертирования json-описаний интерфейса пользователя в объекты QML */

#include <Common/QtHeadersBegin.h>
#include <QtCore/QScopedPointer>
#include <QtCore/QSettings>
#include <QtCore/QDir>
#include <QtCore/QUrl>
#include <QtCore/QDirIterator>
#include <QtGui/QFontDatabase>
#include <qjson.h>
#include <Common/QtHeadersEnd.h>


#include "Skin.h"
#include "Log.h"

//------------------------------------------------------------------------------
namespace CSkin
{
	const char DefaultSkinName[] = "default";
	const char ParamSkinName[] = "skin_name";
	const char ParamProviderId[] = "provider_id";
}

//------------------------------------------------------------------------------
Skin::Skin(const QObject * aApplication, const QString & aInterfacePath, const QString & aUserPath)
	: mName(CSkin::DefaultSkinName),
	  mInterfacePath(aInterfacePath),
	  mGuiService(aApplication->property("graphics").value<QObject *>())
{
	auto skinExist = [&](const QString & aName) -> bool
	{
		return QFile::exists(skinConfigFileName(aName));
	};

	auto getSkinName = [](const QString & aIniFile) -> QString
	{
		QSettings settings(aIniFile, QSettings::IniFormat);
		settings.setIniCodec("UTF-8");
		settings.beginGroup("ui");

		return settings.value("skin", "").toString();
	};

	// Имя скина из дистрибутива
	QString interfaceSkinName = getSkinName(mInterfacePath + QDir::separator() + "interface.ini");	
	
	// Имя скина, установленного пользователем
	QString userSkinName = getSkinName(aUserPath + QDir::separator() + "user.ini");
	
	// Приоритеты: пользовательский, дистрибутив, дефолтный
	mName = skinExist(userSkinName) ? userSkinName : (skinExist(interfaceSkinName) ? interfaceSkinName : CSkin::DefaultSkinName);
	mPrevName = mName;
	
	loadSkinConfig();	

	Log(Log::Normal) << QString("SET SKIN '%1'.").arg(mName);

	// Настройки брендирования
	auto getProviderSkinName = [](const QString & aIniFile) -> QVariantMap
	{
		QSettings settings(aIniFile, QSettings::IniFormat);
		settings.setIniCodec("UTF-8");
		settings.beginGroup("skin");

		// Читаем конфигурацию скинов для конкретных операторов(оператор=имя_скина)
		QVariantMap skins;
		foreach(auto const key, settings.allKeys())
		{
			foreach(QString provider, settings.value(key).toStringList())
			{
				skins.insert(provider, key);
			}
		}

		return skins;
	};

	mProviderSkinConfig = getProviderSkinName(mInterfacePath + QDir::separator() + "interface.ini");
	QVariantMap userProviderSkinConfig = getProviderSkinName(aUserPath + QDir::separator() + "user.ini");
	mProviderSkinConfig = userProviderSkinConfig.isEmpty() ? mProviderSkinConfig : userProviderSkinConfig;
	mProviderSkinConfig.insert("-1", mName);

	// Зарегистрируем шрифты всех скинов
	QDirIterator dirEntry(QString("%1/skins").arg(mInterfacePath), QString("*.ttf*;*.otf").split(";"), QDir::Files, QDirIterator::Subdirectories);
	while(dirEntry.hasNext())
	{
		dirEntry.next();
		
		if (QFontDatabase::addApplicationFont(dirEntry.filePath()) == -1)
		{
			Log(Log::Error) << QString("Failed to add font '%1'.").arg(dirEntry.fileName());
		}
	}
}

//------------------------------------------------------------------------------
bool Skin::loadSkinConfig()
{
	// Загружаем настройки интерфейса.
	QFile json(skinConfigFileName(mName));

	if (json.open(QIODevice::ReadOnly))
	{
		QJsonParseError error;
		QJsonDocument result = QJsonDocument::fromJson(json.readAll(), &error);

		if (QJsonParseError::NoError == error.error)
		{
			QVariantMap config = result.object().toVariantMap();
			mConfig.clear();
			
			foreach(QString key, config.keys())
			{
				if (!key.startsWith("color.") && !key.startsWith("font."))
				{
					QString path = mInterfacePath + QDir::separator() + "skins" + QDir::separator() + mName + QDir::separator() + config.value(key).toString();

					if (!QFile::exists(path))
					{
						Log(Log::Debug) << QString("SkinProvider: failed to load texture '%1' from '%2'. Try loading texture from 'default'.").arg(key).arg(path);
						path = mInterfacePath + QDir::separator() + "skins" + QDir::separator() + "default" + QDir::separator() + config.value(key).toString();
					}

					QUrl url(path);
					url.setScheme("file");
					mConfig[key] = url.path();
				}
				else
				{
					mConfig[key] = config.value(key);
				}				
			}

			return true;
		}
		else
		{
			Log(Log::Error) << QString("Skin: failed to parse skin config '%1(%2)': %3.").arg(skinConfigFileName(mName)).arg(error.offset).arg(error.errorString());
		}
	}
	else
	{
		Log(Log::Error) << QString("Skin: failed to open skin config file '%1'.").arg(skinConfigFileName(mName));
	}

	return false;
}

//------------------------------------------------------------------------------
QFont Skin::font(const QString & aFontName) const
{
	QVariantMap::const_iterator it = mConfig.find(aFontName);
	if (it != mConfig.end())
	{
		QFont f;
		QVariantMap p = it->toMap();

		f.setBold(p["bold"].toBool());
		f.setPixelSize(p["pixelSize"].toInt());
		f.setFamily(p["family"].toString());
		f.setCapitalization(p["capitalization"] == "Font.AllUppercase" ? QFont::AllUppercase : QFont::MixedCase);

		return f;
	}

	Log(Log::Error) << QString("Failed to load font '%1'.").arg(aFontName);
	return QFont();
}

//------------------------------------------------------------------------------
QString Skin::color(const QString & aColorName) const
{
	QVariantMap::const_iterator it = mConfig.find(aColorName);
	if (it != mConfig.end())
	{
		return it->toString();
	}
	else
	{
		Log(Log::Error) << QString("Failed to load color '%1'.").arg(aColorName);
		return ("#FF00FF");
	} 
}

//------------------------------------------------------------------------------
QString Skin::image(const QString & aImageId) const
{
	QString scene = mGuiService->property("topScene").value<QString>();
	QString pathWithScene = QString("%1/%2").arg(scene).arg(aImageId);
	
	// Сначала ищем путь в виде текущая_сцена/имя_ресурса
	QVariantMap::const_iterator it = mConfig.find(pathWithScene);

	if (it != mConfig.end())
	{
		return it->toString();
	}

	it = mConfig.find(aImageId);

	return it != mConfig.end() ? it->toString() : "";
}

//------------------------------------------------------------------------------
QString Skin::getName() const
{
	return mName;
}

//------------------------------------------------------------------------------
QVariantMap Skin::getConfiguration() const
{
	return mConfig;
}

//------------------------------------------------------------------------------
void Skin::reload(const QVariantMap & aParams)
{
	bool result = false;
	
	if (!aParams.isEmpty())
	{
		mPrevName = mName;
		QString providerId = aParams.value(CSkin::ParamProviderId).toString();
		mName = providerId.isEmpty() ? mName : mProviderSkinConfig.value(providerId).toString();
		result = loadSkinConfig();
	}

	// Пустые параметры - вернуть предыдущий скин
	// Если не удалось - загружаем предыдущий скин
	if (aParams.isEmpty() || !result)
	{
		mName = mPrevName;				
		result = loadSkinConfig();
	}

	Log(Log::Normal) << QString("UPDATE SKIN '%1'. RESULT %2").arg(mName).arg(result);
}

//------------------------------------------------------------------------------
bool Skin::needReload(const QVariantMap & aParams) const
{
	QString pid = aParams.value(CSkin::ParamProviderId).toString();
	QString name = aParams.value(CSkin::ParamSkinName).toString();

	if (pid.toInt() == -1)
	{
		return false;
	}
	
	return
		(aParams.empty() && mPrevName != mName) ||
		mProviderSkinConfig.keys().contains(pid) &&
		mProviderSkinConfig.value(name).toString() != mName;
}

//------------------------------------------------------------------------------
QString Skin::skinConfigFileName(const QString & aName) const
{
	return mInterfacePath + "/skins/" + aName + "/config.json";
}

//------------------------------------------------------------------------------
