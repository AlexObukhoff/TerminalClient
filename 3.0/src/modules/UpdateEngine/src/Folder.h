/* @file Компонент дистрибутива - набор файлов. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QIODevice>
#include <QtCore/QList>
#include <QtCore/QPair>
#include <Common/QtHeadersEnd.h>

#include "Component.h"

//---------------------------------------------------------------------------
/// Компонент для загрузки - индексный файл.
class Folder : public Component
{
	Q_OBJECT

public:
	Folder(const QString & aName, const QString & aVersion, const TFileList & aFiles, const QStringList & aPostActions, const QString & aURL);

	/// Выполнить действие.
	virtual void applyPostActions(const QString & aWorkingDir) throw (std::exception);

	/// Произвести установку файлов aFiles в каталог aDestination.
	virtual void deploy(const TFileList & aFiles, const QString & aDestination) throw (std::exception);

	/// Производит закачку компонента, находящего по aBaseURL во временную папку.
	virtual QList<NetworkTask *> download(const QString & aBaseURL, const TFileList & aExceptions);

	/// Проверить что папка для будущего файла существует
	void ensureTargetDirectory(const QString & targetDir, const QString & targetFileName);
};

//---------------------------------------------------------------------------
