/* @file Компонент дистрибутива - архив. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QList>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

#include "Component.h"

//---------------------------------------------------------------------------
namespace CPackage
{
	const int PostActionTimeout = 120; // Таймаут работы post-action
}

//---------------------------------------------------------------------------
/// Компонент дистрибьютива - набор файлов.
class Package : public Component
{
	Q_OBJECT

public:
	Package(const QString & aName, const QString & aVersion, const TFileList & aFiles, const QStringList & aPostActions, const QString & aURL, const QString & aHash, int aSize);

	/// Выполнить действие.
	virtual void applyPostActions(const QString & aWorkingDir) throw (std::exception);

	/// Произвести установку файлов aFiles в каталог aDestination.
	virtual void deploy(const TFileList & aFiles, const QString & aDestination) throw (std::exception);

	/// Производит закачку компонента, находящего по aBaseURL во временную папку.
	virtual QList<NetworkTask *> download(const QString & aBaseURL, const TFileList & aExceptions);

	/// Собрать URL компоненты
	QString getURL(const QString & aFileName, const QString & aDefaultUrl) const;

private:
	QString mHash;
	int mSize;
};

//---------------------------------------------------------------------------
