/* @file Реализация классов-ответов от EFTPOS. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QDateTime>
#include <QtCore/QSharedPointer>
#include <Common/QtHeadersEnd.h>

// Project
#include "Ucs.h"


namespace Ucs
{
//---------------------------------------------------------------------------
class BaseResponse
{
public:
	char mClass;
	char mCode;
	QString mTerminalID;
	QByteArray mData;
	/// Статус объекта API в момент получения ответа
	APIState::Enum mAPIState;

public:
	BaseResponse(const BaseResponse & aResponse) :
		mClass(aResponse.mClass), mCode(aResponse.mCode), mTerminalID(aResponse.mTerminalID), mData(aResponse.mData) { }
	virtual ~BaseResponse() {}
	static QSharedPointer<BaseResponse> createResponse(QByteArray & aResponseBuffer);

	virtual bool isValid() { return false; }

protected:
	BaseResponse() { mClass = mCode = 0; }
	BaseResponse(const QByteArray & aResponseBuffer);
};

typedef QSharedPointer<BaseResponse> BaseResponsePtr;

//---------------------------------------------------------------------------
class ErrorResponse : public BaseResponse
{
public:
	ErrorResponse(const BaseResponse & aResponse);
	virtual bool isValid() { return true; }

	QString getError() const;
	QString getErrorMessage() const;
};

//---------------------------------------------------------------------------
class ConsoleResponse : public BaseResponse
{
public:
	ConsoleResponse(const BaseResponse & aResponse);
	virtual bool isValid() { return true; }

	QString getMessage() const;

private:
	QString mMessage;
};

//---------------------------------------------------------------------------
class LoginResponse : public BaseResponse
{
public:
	LoginResponse(const BaseResponse & aResponse);
	virtual bool isValid() { return true; }
		
	//0	EFTPOS устройство полностью готово к обработке транзакций по картам
	//1	EFTPOS устройству требуется инкассация
	//2	В EFTPOS устройстве закончилась бумага для печати
	QString getStatusCode() const;
	QString getTerminalID() const;
	bool needEncashment() const;

private:
	QString mStatusCode;
};

//---------------------------------------------------------------------------
class PrintLineResponse : public BaseResponse
{
public:
	PrintLineResponse(const BaseResponse & aResponse);
	virtual bool isValid() { return true; }

	bool isLast() const;
	QString getText() const;
};

//---------------------------------------------------------------------------
class GetStateResponse : public BaseResponse
{
public:
	GetStateResponse(const BaseResponse & aResponse) : BaseResponse(aResponse) {}
	virtual bool isValid() { return true; }

	bool isLast() const;
	int state() const;
	QString getName() const;
};

//---------------------------------------------------------------------------
class InitialResponse : public BaseResponse
{
public:
	InitialResponse(const BaseResponse & aResponse) : BaseResponse(aResponse) {}
	virtual bool isValid() { return true; }
};

//---------------------------------------------------------------------------
class BreakResponse : public BaseResponse
{
public:
	BreakResponse(const BaseResponse & aResponse) : BaseResponse(aResponse) {}
	virtual bool isValid() { return true; }

	bool isComplete() const;
};

//---------------------------------------------------------------------------
class PINRequiredResponse : public BaseResponse
{
public:
	PINRequiredResponse(const BaseResponse & aResponse) : BaseResponse(aResponse) {}
	virtual bool isValid() { return true; }
};

//---------------------------------------------------------------------------
class OnlineRequiredResponse : public BaseResponse
{
public:
	OnlineRequiredResponse(const BaseResponse & aResponse) : BaseResponse(aResponse) {}
	virtual bool isValid() { return true; }
};

//---------------------------------------------------------------------------
class AuthResponse : public BaseResponse
{
public:
	Operation::Enum mOperation;
	quint32 mTransactionSumm;
	quint32 mCurrency;
	QDateTime mStamp;
	QString mMerchant;
	QString mRRN;
	QString mResponse;
	QString mConfirmation;
	QString mCardNumber;
	QString mCardLabel;
	QString mMessage;

public:
	AuthResponse(const BaseResponse & aResponse);
	virtual bool isValid() { return true; }

	bool isOK() const;
	QString toString() const;
};

//---------------------------------------------------------------------------
class EncashmentResponse : public BaseResponse
{
public:
	EncashmentResponse(const BaseResponse & aResponse) : BaseResponse(aResponse) {}
	virtual bool isValid() { return true; }
};

//---------------------------------------------------------------------------
class HoldResponse : public BaseResponse
{
public:
	HoldResponse(const BaseResponse & aResponse) : BaseResponse(aResponse) {}
	virtual bool isValid() { return true; }
};

//---------------------------------------------------------------------------
class MessageResponse : public BaseResponse
{
public:
	MessageResponse(const BaseResponse & aResponse);
	virtual bool isValid() { return true; }

	bool needEncashment() const;

private:
	QString mStatusCode;
	QString mCurrentCountTxns;
	QString mTimeUpload;
};

} // namespace Uniteller

//---------------------------------------------------------------------------
