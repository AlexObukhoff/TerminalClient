/* @file Базовый класс скачиваемого компонента. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QSet>
#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

#include "File.h"

class NetworkTask;

//---------------------------------------------------------------------------
namespace CComponent
{
	inline const char * OptionalTask()
	{ 
		return "OptionalTaskProperty";
	}
}

//---------------------------------------------------------------------------
class Component : public QObject
{
	Q_OBJECT

public:
	Component(const QString & aName, const QString & aVersion, const TFileList & aFiles, const QStringList & aPostActions, const QString & aURL);
	virtual ~Component();

	/// Получение полного списка файлов.
	virtual TFileList getFiles() const;

	/// Выполнить действие.
	virtual void applyPostActions(const QString & aWorkingDir) throw (std::exception) = 0;

	/// Произвести установку файлов aFiles в каталог aDestination.
	virtual void deploy(const TFileList & aFiles, const QString & aDestination) throw (std::exception) = 0;

	/// Производит закачку компонента, находящего по aBaseURL во временную папку.
	virtual QList<NetworkTask *> download(const QString & aBaseURL, const TFileList & aExceptions) = 0;

	/// Имя компонента.
	QString getId() const;

	/// Имя временной папки.
	QString getTemporaryFolder() const;

	/// Получить URL компонента, если задан.
	QString getURL(const File & aFile, const QString & aDefaultUrl) const;

	/// Собрать URL компоненты
	QString getURL(const QString & aFileName, const QString & aDefaultUrl) const;

	/// Версия.
	QString getVersion() const;

	/// Файлы, которые надо выполнить после установки.
	QStringList getPostActions() const;

	/// Установить флаг пропуска обновления существующих файлов
	void setSkipExisting(bool aSkipExisting);

	/// Пропуск обновления существующих файлов
	bool skipExisting() const;

	/// Необязательность обновления 
	void setOptional(bool aOptional);
	bool optional() const;

private:
	QString mId;
	TFileList mFiles;
	QStringList mPostActions;
	QString mURL;
	QString mVersion;
	bool mOptional;

protected:
	bool mSkipExisting;
};

//---------------------------------------------------------------------------
