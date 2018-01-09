/* @file Верификатор данных по алгоритму MD5. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QCryptographicHash>
#include <Common/QtHeadersEnd.h>

// Thirdparty
#if QT_VERSION < 0x050000
#include <Qt5Port/qt5port.h>
#endif

#include "hashVerifier.h"

//------------------------------------------------------------------------
Md5Verifier::Md5Verifier(const QString & aMd5)
{
	mMD5 = aMd5;
}

//------------------------------------------------------------------------
bool Md5Verifier::verify(NetworkTask * /*aTask*/, const QByteArray & aData)
{
	mCalculatedMD5 = QCryptographicHash::hash(aData, QCryptographicHash::Md5).toHex();

	return (mMD5.compare(mCalculatedMD5, Qt::CaseInsensitive) == 0);
}

//------------------------------------------------------------------------
Sha256Verifier::Sha256Verifier(const QString & aSha256)
{
	mSha256 = aSha256;
}

//------------------------------------------------------------------------
bool Sha256Verifier::verify(NetworkTask * /*aTask*/, const QByteArray & aData)
{
#if QT_VERSION >= 0x050000
	mCalculatedSha256 = QCryptographicHash::hash(aData, QCryptographicHash::Sha256).toHex();
#else
	mCalculatedSha256 = CCryptographicHash::hash(aData, CCryptographicHash::Sha256).toHex();
#endif

	return (mSha256.compare(mCalculatedSha256, Qt::CaseInsensitive) == 0);
}
//------------------------------------------------------------------------
