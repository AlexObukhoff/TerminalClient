/* @file Верификатор данных по алгоритму MD5. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <Common/QtHeadersEnd.h>

#include "IVerifier.h"

//------------------------------------------------------------------------
namespace CHashVerifier
{
	const int MD5HashSize = 32;
	const int Sha256HashSize = 64;
}

class IHashVerifier : public IVerifier
{
public:
	virtual QString referenceHash() const = 0;
	virtual QString calculatedHash() const = 0;
};

//------------------------------------------------------------------------
class Md5Verifier : public IHashVerifier
{
public:
	Md5Verifier(const QString & aMD5);

	virtual bool verify(NetworkTask * aTask, const QByteArray & aData);

	QString referenceHash() const { return mMD5; }
	QString calculatedHash() const { return mCalculatedMD5; }

private:
	QString mMD5;
	QString mCalculatedMD5;
};

//------------------------------------------------------------------------
class Sha256Verifier : public IHashVerifier
{
public:
	Sha256Verifier(const QString & aSha256);

	virtual bool verify(NetworkTask * aTask, const QByteArray & aData);

	QString referenceHash() const { return mSha256; }
	QString calculatedHash() const { return mCalculatedSha256; }

private:
	QString mSha256;
	QString mCalculatedSha256;
};

//------------------------------------------------------------------------
