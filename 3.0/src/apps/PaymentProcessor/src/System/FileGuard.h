/* @file Сторож бэкап-файлов. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QFile>
#include <QtCore/QList>
#include <QtCore/QPair>
#include <Common/QtHeadersEnd.h>

namespace System
{

//----------------------------------------------------------------------------
class FileGuard
{
	typedef QPair<QString, QString> TStringPair;

public:
	FileGuard()
	{
		mFileList.clear();
	}

	~FileGuard()
	{
		for (int i = 0; i < mFileList.size(); i++)
		{
			TStringPair curPair = mFileList.at(i);

			if (QFile::exists(curPair.first))
			{
				if (!QFile::remove(curPair.first))
				{
					continue;
				}
			}

			QFile::rename(curPair.second, curPair.first);
		}
	}

	/// Добавляет пару имён файла, для восстановления.
	void addFile(const QString& aOldName, const QString& aNewName)
	{
		mFileList.append(TStringPair(aOldName, aNewName));
	}

	/// Очищает список файлов для восстановления.
	void release()
	{
		mFileList.clear();
	}

private:
	QList<TStringPair> mFileList;
};

//----------------------------------------------------------------------------

} // System
