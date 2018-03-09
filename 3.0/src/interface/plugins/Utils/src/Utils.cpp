/* @file Набор вспомогательных функций для qml. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QSettings>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonValue>
#include <QtCore/QJsonObject>
#include <QtCore/QByteArray>
#include <QtCore/QScopedPointer>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtGui/QKeyEvent>
#include <QtQml/QQmlEngine>
#include <QtQml/QQmlContext>
#include <QtMultimedia/QSound>
#include <Common/QtHeadersEnd.h>

// Проект
#include "Log.h"
#include "Utils.h"

//------------------------------------------------------------------------------
namespace CUtils
{
	const QString MaskPlaceholders = "AaNnXxDdHhBb09#";
	const QString MaskModifiers = "<!>";

	const QString DefaultConf = "interface.ini";
	const QString UserConf = "user.ini";

	const QString UseCommonSounds = "sound/use_common";
	const QString UseNarratorSounds = "sound/use_narrator";
	const QString UseAutoOrderProviders = "ui/auto_order_operator";
}

//------------------------------------------------------------------------------
Utils::Utils(QQmlEngine * aEngine, const QString & aInterfacePath, const QString & aUserPath) :
	mEngine(aEngine),
	mInterfacePath(aInterfacePath),
	mUserPath(aUserPath),
	mUseCommonSounds(true),
	mUseNarratorSounds(false),
	mUseAutoOrderProviders(false)
{
	// Получаем директорию с файлами интерфейса.
	QObject * application = mEngine->rootContext()->contextProperty("Core").value<QObject *>();

	Q_ASSERT(application);

	loadConfiguration();

	mTranslator = QSharedPointer<Translator>(new Translator(mInterfacePath));
	connect(mTranslator.data(), SIGNAL(languageChanged()), this, SIGNAL(updateTranslator()));

	// Модель для интерфейса
	mGroupModel = QSharedPointer<GroupModel>(new GroupModel());
	mGroupModel->setStatistic(getStatistic());

	mRootGroupModel = QSharedPointer<GroupModel>(new GroupModel());
	mRootGroupModel->setElementFilter(QStringList() << "group");

	// Модели и фильтры для поиска
	mProviderListModel = QSharedPointer<ProviderListModel>(new ProviderListModel(this, mGroupModel));
	connect(mGroupModel.data(), SIGNAL(modelReset()), mProviderListModel.data(), SLOT(groupsUpdated()));
	mProviderListModel->setPaymentService(application ? application->property("payment").value<QObject *>() : nullptr);
	mProviderListFilter = QSharedPointer<ProviderListFilter>(new ProviderListFilter(this));
	mProviderListFilter->setSourceModel(mProviderListModel.data());
	mProviderListFilter->setDynamicSortFilter(true);

	mSkin = QSharedPointer<Skin>(new Skin(mInterfacePath, mUserPath));
}

//------------------------------------------------------------------------------
void Utils::generateKeyEvent(int aKey, int aModifiers, const QString & aText) const
{
	QApplication::postEvent(QApplication::focusWidget(), new QKeyEvent(QEvent::KeyPress, aKey, Qt::KeyboardModifiers(aModifiers), aText));
}

//------------------------------------------------------------------------------
// подсчет количества позиций в маске ввода
int maskLength(QString mask)
{
	int len = 0;

	mask.remove(QRegExp("(;.)$"));
	
	for (int i=0; i < mask.size(); ++i)
	{
		if (mask[i] == '\\')
		{
			++i;
		}
		else if (mask[i] == '>' || mask[i] == '<' || mask[i] == '!')
		{
			continue;
		}
		++len;
	}

	return len;
}

//------------------------------------------------------------------------------
QString Utils::stripMask(const QString & aSource, const QString & aMask) const
{
	int pos = 0;
	bool escaped = false;
	QString result;

	if (!aSource.length() || !aMask.length())
	{
		return aSource;
	}

	QChar userPlaceholder = aMask[aMask.length() - 1];

	foreach (QChar c, aMask)
	{
		if (!escaped)
		{
			if (c == ';')
			{
				break;
			}
			else if (CUtils::MaskPlaceholders.contains(c) && pos < aSource.length() && aSource[pos] != userPlaceholder)
			{
				result += aSource[pos];
			}
			else if (c == '\\')
			{
				escaped = true;
			}
		}
		else
		{
			escaped = false;
		}

		if (!escaped && !CUtils::MaskModifiers.contains(c))
		{
			++pos;
		}
	}

	return aMask.isEmpty() ? aSource : result;
}

//------------------------------------------------------------------------------
// Маска                                 : +7 (999) 999-99-99;*
// Абсолютно пустое свойство displayValue: +7 (***) ***-**-**
// Свойство displayValue после ввода     : +7 (903) ***-**-**
// Позиция первого различия справа       :       ^
// Нужная позиция курсора                :          ^  (1-й placeholder справа от различия)

int Utils::getCursorPosition(const QString & aMask, const QString & aBefore, const QString & aAfter) const
{
	if (aMask.isEmpty())
	{
		return aAfter.length();
	}
	else if (aBefore.length() != aAfter.length())
	{
		Log(Log::Error) << "Utils::getCursorPosition(): string lengths differ.";

		return 0;
	}

	// Находим первое справа отличие пустого displayValue от заполненного
	int mismatchIndex;

	for(mismatchIndex = aBefore.length() - 1; mismatchIndex >= 0; --mismatchIndex)
	{
		if (aBefore[mismatchIndex] != aAfter[mismatchIndex])
		{
			break;
		}
	}

	// Теперь находим первый справа от различия незаполненный символ
	++mismatchIndex;

	int placeholderIndex = 0;
	bool escaped = false;

	foreach (QChar c, aMask)
	{
		if (!escaped)
		{
			if (c == ';')
			{
				mismatchIndex = aAfter.length();
				break;
			}
			else if (placeholderIndex >= mismatchIndex && CUtils::MaskPlaceholders.contains(c))
			{
				break;
			}
			else if (c == '\\')
			{
				escaped = true;
			}
		}
		else
		{
			escaped = false;
		}

		if (!escaped && !CUtils::MaskModifiers.contains(c))
		{
			++placeholderIndex;
		}
	}

	return placeholderIndex;
}


//------------------------------------------------------------------------------
QString Utils::format(const QString & aSource, const QString & aFormat) const
{
	QString result;

	if (!aFormat.isEmpty())
	{
		QRegExp rx("\\[(\\d+)\\]");

		QString outFormat = aFormat;
		int pos;

		while ((pos = rx.indexIn(outFormat, 0)) >= 0)
		{
			int srcIndex = rx.cap(1).toInt() - 1;

			if (srcIndex < aSource.size())
			{
				outFormat.replace(pos, rx.matchedLength(), aSource.at(srcIndex));
			}
			else
			{
				outFormat.remove(pos, rx.matchedLength());
			}
		}

		result = outFormat;
	}

	return result;
}

//------------------------------------------------------------------------------
QString Utils::readFile(const QString & aPath) const
{
	// TODO если путь отнсительный, то считать от каталога скина
	QFile file(aPath);
	QString result;

	if (file.open(QIODevice::ReadOnly))
	{
		QByteArray contents = file.readAll();
		QMap<QString, QTextCodec *>::iterator codec = mCodecCache.find(aPath);

		if (codec == mCodecCache.end())
		{
			codec = mCodecCache.insert(aPath, QTextCodec::codecForHtml(contents));
		}

		result = (*codec)->toUnicode(contents);
	}
	else
	{
		Log(Log::Error) << "Failed to read file " << aPath << ": " << file.errorString();
	}

	return result;
}

//------------------------------------------------------------------------------
bool Utils::fileExist(const QString & aPath) const
{
	return QFileInfo(aPath).exists();
}

//------------------------------------------------------------------------------
QString Utils::toHtml(const QString & aSource) const
{
	return QString(aSource)
		.replace("[br]", " ")
		.replace(QRegExp("\\s+"), " ")
		.replace(QRegExp("\\[(/*\\w+)\\]"), "<" + QString("\\1") + ">");
}

//------------------------------------------------------------------------------
QString Utils::toPlain(const QString & aSource) const
{
	return QString(aSource)
		.replace("[br]", "")
		.replace(QRegExp("\\s+"), " ")
		.replace(QRegExp("\\[(/*\\w+)\\]"), "")
		.replace(QRegExp("<(/*\\w+)>"), "");
}

//------------------------------------------------------------------------------
QObject * Utils::getTranslator()
{
	return mTranslator.data();
}

//------------------------------------------------------------------------------
QObject * Utils::getGroupModel()
{
	return mGroupModel.data();
}

//------------------------------------------------------------------------------
QObject * Utils::getRootGroupModel()
{
	return mRootGroupModel.data();
}

//------------------------------------------------------------------------------
QObject * Utils::getProviderList()
{
	return mProviderListFilter.data();
}

//------------------------------------------------------------------------------
QObject * Utils::getSkin()
{
	return mSkin.data();
}

// Файлы для воспроизведения должны лежать в соответствующих папках: common, narrator, etc
// Файл из произвольного места воспроизведен не будет
//------------------------------------------------------------------------------
void Utils::playSound(const QString & aFileName) const
{
	QStringList path = aFileName.split("/");
	path.removeLast();

	QString key = QString("sound/use_%1").arg(path.last());
	QString filePath;
	
	if (key == CUtils::UseCommonSounds && mUseCommonSounds)
	{
		filePath = mInterfacePath + QDir::separator() + "sounds" + QDir::separator() + aFileName;
	}
	else if (key == CUtils::UseNarratorSounds && mUseNarratorSounds)
	{
		filePath = mInterfacePath + QDir::separator() + "sounds" + QDir::separator() + mTranslator->getLanguage() + QDir::separator() + aFileName;
	}
	else
	{
		// Звук отключен
		return;
	}

	if (filePath.isEmpty() || !QFile::exists(filePath))
	{
		Log(Log::Warning) << QString("Audio file %1 not found.").arg(filePath);
		return;
	}		

	QSound::play(filePath);
}

//------------------------------------------------------------------------------
QString Utils::fromBase64(const QString & aSource) const
{
	return QTextCodec::codecForName("UTF-8")->toUnicode(QByteArray::fromBase64(aSource.toLatin1()));
}

//------------------------------------------------------------------------------
QString Utils::fromUrlEncoding(const QString & aSource) const
{
	return QTextCodec::codecForName("windows-1251")->toUnicode(QByteArray::fromPercentEncoding(aSource.toLatin1()));
}

//------------------------------------------------------------------------------
void Utils::click()
{
	QMetaObject::invokeMethod(this, "onClicked", Qt::QueuedConnection);
}

//------------------------------------------------------------------------------
void Utils::onClicked()
{
	emit clicked();
}

//------------------------------------------------------------------------------
QVariantMap Utils::str2json(const QString & aString) const
{
	if (aString.isEmpty())
	{
		return QVariantMap();
	}
	
	QJsonParseError ok;
	QString str = aString;
	QJsonDocument result = QJsonDocument::fromJson(str.replace("'", "\"").toUtf8(), &ok);

	if (ok.error == QJsonParseError::NoError)
	{
		return result.object().toVariantMap();
	}

	Log(Log::Error) << QString("Utils: failed to parsed JSON string %1").arg(aString);
	
	return QVariantMap();
}

//------------------------------------------------------------------------------
QMap<qint64, quint32> Utils::getStatistic()
{
	QMap<qint64, quint32> result;

	if (mUseAutoOrderProviders)
	{
		QObject * application = mEngine->rootContext()->contextProperty("Core").value<QObject *>();
		QObject * paymentService = application ? application->property("payment").value<QObject *>() : nullptr;

		QVariantMap statistic;

		if (paymentService && QMetaObject::invokeMethod(paymentService, "getStatistic", Q_RETURN_ARG(QVariantMap, statistic)))
		{
			QMapIterator<QString, QVariant> i(statistic);

			while (i.hasNext())
			{
				i.next();

				result.insert(i.key().toLongLong(), i.value().toUInt());
			}
		}
	}

	return result;
}

//------------------------------------------------------------------------------
void Utils::loadConfiguration()
{
	// Настройки
	QVariantMap configuration;

	// Загружаем значения по умолчанию
	{
		QSettings defaultSettings(mInterfacePath + QDir::separator() + CUtils::DefaultConf, QSettings::IniFormat);
		defaultSettings.setIniCodec("UTF-8");

		foreach(QString key, defaultSettings.allKeys())
		{
			configuration.insert(key, defaultSettings.value(key));
		}
	}

	// Загружаем пользовательские настройки
	{
		QSettings userSettings(mUserPath + QDir::separator() + CUtils::UserConf, QSettings::IniFormat);
		userSettings.setIniCodec("UTF-8");

		foreach(QString key, userSettings.allKeys())
		{
			configuration.insert(key, userSettings.value(key));
		}
	}

	mUseCommonSounds = configuration.value(CUtils::UseCommonSounds, mUseCommonSounds).toBool();
	mUseNarratorSounds = configuration.value(CUtils::UseNarratorSounds, mUseNarratorSounds).toBool();
	mUseAutoOrderProviders = configuration.value(CUtils::UseAutoOrderProviders, mUseAutoOrderProviders).toBool();
}

//------------------------------------------------------------------------------
