/* @file Файл дистрибутива. */

#include <Common/QtHeadersBegin.h>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QCryptographicHash>
#include <Common/QtHeadersEnd.h>

// SDK
#include <NetworkTaskManager/HashVerifier.h>

// Проект 
#include "File.h"

//---------------------------------------------------------------------------
File::File()
{
	mSize = 0;
}

//---------------------------------------------------------------------------
File::File(const QString & aName, const QString & aHash, const QString & aUrl, qint64 aSize /*= 0*/) 
	: mName(aName), mHash(aHash), mUrl(aUrl), mSize(aSize)
{
}

//---------------------------------------------------------------------------
bool File::operator==(const File & aFile) const
{
	return (this->mName.compare(aFile.mName, Qt::CaseInsensitive) == 0 && this->mHash.compare(aFile.mHash, Qt::CaseInsensitive) == 0);
}

//---------------------------------------------------------------------------
File::Result File::verify(const QString & aTempFilePath)
{
	QFileInfo fInfo(aTempFilePath);

	if (!fInfo.exists())
	{
		return NotFullyDownloaded;
	}
	
	if (size() > 0)
	{
		if (fInfo.size() < size())
		{
			return NotFullyDownloaded;
		}
		else if (fInfo.size() > size())
		{
			// файл на диске больше файла на сервере - нужно удалить и качать заново
			return Error;
		}
	}

	QFile f(aTempFilePath);
	if (f.open(QIODevice::ReadOnly))
	{
		Sha256Verifier sha256V(hash());

		bool hashOK = sha256V.verify(nullptr, f.readAll());

		if (!hashOK && size() == 0)
		{
			return NotFullyDownloaded;
		}

		return  hashOK ? OK : Error;
	}
	else
	{
		return Error;
	}
}
