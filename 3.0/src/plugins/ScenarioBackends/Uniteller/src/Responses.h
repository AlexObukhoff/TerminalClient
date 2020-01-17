/* @file Реализация классов-ответов от EFTPOS. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QDateTime>
#include <QtCore/QSharedPointer>
#include <Common/QtHeadersEnd.h>

// Project
#include "Uniteller.h"

namespace Uniteller
{
	//---------------------------------------------------------------------------
	class BaseResponse
	{
	public:
		char mClass;
		char mCode;
		QString mTerminalID;
		QByteArray mData;

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

	typedef QSharedPointer<ErrorResponse> ErrorResponsePtr;

	//---------------------------------------------------------------------------
	class LoginResponse : public BaseResponse
	{
	public:
		LoginResponse(const BaseResponse & aResponse) : BaseResponse(aResponse) {}
		virtual bool isValid() { return true; }
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
	class DeviceEventResponse : public BaseResponse
	{
	public:
		DeviceEventResponse(const BaseResponse & aResponse) : BaseResponse(aResponse) {}
		virtual bool isValid() { return true; }

		DeviceEvent::Enum event() const;
		char keyCode() const;
		KeyCode::Enum key() const;
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

} // namespace Uniteller

//---------------------------------------------------------------------------
