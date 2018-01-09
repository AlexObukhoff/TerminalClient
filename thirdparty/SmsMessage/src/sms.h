/* @file Смс сообщение. */

#pragma once

// Qt
#include <QtCore/QString>
#include <QtCore/QDebug>
#include <QtCore/QSharedDataPointer>

class SmsPart;
class SmsData;

//------------------------------------------------------------------------------
class Sms
{
public:
	Sms();
	Sms(const Sms & aOther);
#ifdef Q_COMPILER_RVALUE_REFS
    inline Sms(Sms && aOther);
    inline Sms & operator=(Sms && aOther);
#endif
	virtual ~Sms();

	//Функция добавляет часть сообщения part к списку частей.
	void addPart(const SmsPart & aPart);

	//Возвращает часть сообщения по индексу id.
	SmsPart getPart(int aId) const;
	
	//Возвращает количество частей.
	int getSize() const;

	//Возвращает адрес отправителя. Может быть номером телефона в международном формате, может быть текстом.
	QString getSenderAddress() const;

	// Возвращает дату и время отправки сообщения. Если
	// сообщение состоит из нескольких частей, возвращается
	// дата и время отправки первой части.
	QDateTime getDateTime() const;

	//Возвращает полный текст сообщения.
	QString getText() const;

	//Возвращает true при валидном объекте.
	bool isValid() const;

	QString getErrorString() const;

	//Возвращает часть по индексу.
    SmsPart operator[](int aId) const;

	static SmsPart decode(const QString & aData);
	static QList<Sms> join(QList<SmsPart> aParts);

private:
    QSharedDataPointer<SmsData> d;
};

//------------------------------------------------------------------------------

QDebug operator<<(QDebug aDebug, const Sms & aSms);
QDebug operator<<(QDebug aDebug, Sms * aSms);
