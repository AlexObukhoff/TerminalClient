#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtGui/QValidator>
#include <QtCore/QCryptographicHash>
#include <Common/QtHeadersEnd.h>

namespace
{
	const int Md5ValidatorMinLength = 5;
}

//------------------------------------------------------------------------------
class Md5Validator : public QValidator
{
	Q_OBJECT
		Q_PROPERTY(QString hash READ getHash WRITE setHash)

public:
	Md5Validator(QObject * aParent = nullptr)
		: QValidator(aParent)
	{
	}

public:
	virtual QValidator::State validate(QString & aInput, int & aPos) const
	{
		Q_UNUSED(aPos)

		if (mHash.isEmpty())
		{
			return QValidator::Intermediate;
		}

		if (aInput.length() < Md5ValidatorMinLength)
		{
			return QValidator::Intermediate;
		}

		return QCryptographicHash::hash(aInput.toLatin1(), QCryptographicHash::Md5).toHex() == mHash ? QValidator::Acceptable : QValidator::Intermediate;
	}

	private slots:
	QString getHash() const
	{
		return mHash;
	}

	void setHash(const QString & aHash)
	{
		mHash = aHash.toLower();
	}

private:
	QString mHash;
};