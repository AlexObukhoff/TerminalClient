/* @file Реализация классов-ответов от EFTPOS Ucs. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QTextCodec>
#include <Common/QtHeadersEnd.h>

// Project
#include "Responses.h"

namespace
{
	const quint32 EncashmentThreshold = 2;
}



namespace Ucs
{

BaseResponsePtr BaseResponse::createResponse(QByteArray & aResponseBuffer)
{
	if (aResponseBuffer.size() < 14)
	{
		return QSharedPointer<BaseResponse>(nullptr);
	}

	if (aResponseBuffer.size() == aResponseBuffer.count('\0'))
	{
		aResponseBuffer.clear();
		return QSharedPointer<BaseResponse>(nullptr);
	}

	BaseResponse response;
	response.mClass = aResponseBuffer[0];
	response.mCode = aResponseBuffer[1];
	response.mTerminalID = aResponseBuffer.mid(2, 10);
		
	bool ok = true;
	int dataLength = QString::fromLatin1(aResponseBuffer.mid(12, 2)).toInt(&ok, 16);
	if (ok && dataLength)
	{
		response.mData = aResponseBuffer.mid(14, dataLength);
	}

	aResponseBuffer.remove(0, 14 + dataLength);
		
	switch (response.mClass)
	{
	case Ucs::Class::Service:
		switch (response.mCode)
		{
		case Ucs::Encashment::CodeResponse: return QSharedPointer<BaseResponse>(new EncashmentResponse(response));
		}
		break;
	case Ucs::Class::Session:
		switch (response.mCode)
		{
		case Ucs::Login::CodeResponse: return QSharedPointer<BaseResponse>(new LoginResponse(response));
		case Ucs::PrintLine::CodeRequest: return QSharedPointer<BaseResponse>(new PrintLineResponse(response));
		case Ucs::Break::CodeResponse: return QSharedPointer<BaseResponse>(new BreakResponse(response));
		case Ucs::Information::CodeResponse: return QSharedPointer<MessageResponse>(new MessageResponse(response));
		}
		break;

	case Ucs::Class::Accept:
		switch (response.mCode)
		{
		case Ucs::Initial::CodeResponse: return QSharedPointer<BaseResponse>(new InitialResponse(response));
		case Ucs::Sale::PinReqired: return QSharedPointer<BaseResponse>(new PINRequiredResponse(response));
		case Ucs::Sale::OnlineReqired: return QSharedPointer<BaseResponse>(new OnlineRequiredResponse(response));
		case Ucs::Error::Code: return QSharedPointer<BaseResponse>(new ErrorResponse(response));
		case Ucs::ConsoleMessage::CodeResponse: return QSharedPointer<BaseResponse>(new ConsoleResponse(response));
		case Ucs::Hold::CodeResponse: return QSharedPointer<BaseResponse>(new HoldResponse(response));
		}
		break;

	case Ucs::Class::AuthResponse:
		switch (response.mCode)
		{
		case Ucs::Auth::Response: return QSharedPointer<BaseResponse>(new AuthResponse(response));
		}
		break;
	}

	return QSharedPointer<BaseResponse>(new BaseResponse(response));
}

//---------------------------------------------------------------------------
ErrorResponse::ErrorResponse(const BaseResponse & aResponse) : BaseResponse(aResponse)
{
}

//---------------------------------------------------------------------------
QString ErrorResponse::getError() const
{
	if (mData.size() >= 2)
	{
		return QString::fromLatin1(mData.left(2));
	}

	return QString();
}

//---------------------------------------------------------------------------
QString ErrorResponse::getErrorMessage() const
{
	if (mData.size() > 2)
	{
		return QString::fromLatin1(mData.mid(2));
	}

	return QString();
}

//---------------------------------------------------------------------------
PrintLineResponse::PrintLineResponse(const BaseResponse & aResponse) : BaseResponse(aResponse)
{
}

//---------------------------------------------------------------------------
bool PrintLineResponse::isLast() const
{
	return mData.size() > 0 && mData[0] == '1';
}

//---------------------------------------------------------------------------
QString PrintLineResponse::getText() const
{
	return (mData.size() > 1) ? QTextCodec::codecForName("Windows-1251")->toUnicode(mData.mid(1)) : "";
}

//---------------------------------------------------------------------------
bool GetStateResponse::isLast() const
{
	return mData.size() > 0 && mData[0] == '1';
}

//---------------------------------------------------------------------------
int GetStateResponse::state() const
{
	if (mData.size() > 2)
	{
		return QString::fromLatin1(mData.mid(1, 2)).toInt(nullptr, 16);
	}

	return 0xff;
}

//---------------------------------------------------------------------------
QString GetStateResponse::getName() const
{
	return QString::fromLatin1(mData.mid(3));
}

//---------------------------------------------------------------------------
bool BreakResponse::isComplete() const
{
	return mData.size() && mData[0] == '0';
}

//---------------------------------------------------------------------------
AuthResponse::AuthResponse(const BaseResponse & aResponse) : BaseResponse(aResponse)
{
	int index = 0;

	auto readTo1B = [&](QString & aString, QTextCodec * aCodec) {
		for (; mData.size() > index && mData.at(index) != 0x1b; index++)
		{
			char c = mData.at(index);
			aString.append(aCodec ? aCodec->toUnicode(&c, 1).at(0) : c);
		}
		index++;
	};

	if (mData.size())
	{
		mOperation = static_cast<Ucs::Operation::Enum>(mData.at(index)); index++;
		mTransactionSumm = QString::fromLatin1(mData.mid(index, 12)).toUInt(); index += 12;
		mCurrency = QString::fromLatin1(mData.mid(index, 3)).toUInt(); index += 3;
		mStamp = QDateTime::fromString(QString::fromLatin1(mData.mid(index, 14)), "yyyyMMddhhmmss"); index += 14;
		mMerchant = QString::fromLatin1(mData.mid(index, 15)); index += 15;
		mRRN = QString::fromLatin1(mData.mid(index, 12)); index += 12;
		mResponse = QString::fromLatin1(mData.mid(index, 2)); index += 2;
			
		readTo1B(mConfirmation, nullptr);
		readTo1B(mCardNumber, nullptr);
		readTo1B(mCardLabel, nullptr);
		readTo1B(mMessage, QTextCodec::codecForName("windows-1251"));
	}
}

//---------------------------------------------------------------------------
bool AuthResponse::isOK() const
{
	return mResponse == "00";
}

//---------------------------------------------------------------------------
QString AuthResponse::toString() const
{
	return mCardLabel + "|" + mMessage;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
LoginResponse::LoginResponse(const BaseResponse & aResponse)
	: BaseResponse(aResponse)
{
	mStatusCode = mData.size() == 2 ? QString::fromLatin1(mData.mid(1, 1)) : "";
}

//---------------------------------------------------------------------------
QString LoginResponse::getStatusCode() const
{
	return mStatusCode;
}

//---------------------------------------------------------------------------
QString LoginResponse::getTerminalID() const
{
	return mTerminalID;
}

//---------------------------------------------------------------------------
bool LoginResponse::needEncashment() const
{
	return mStatusCode == "1";
}

//---------------------------------------------------------------------------
ConsoleResponse::ConsoleResponse(const BaseResponse & aResponse)
	: BaseResponse(aResponse)
{	
	auto readTo1B = [](const QByteArray & aBuffer) -> QString
	{
		QString result; 
		QTextCodec * aCodec = QTextCodec::codecForName("windows-1251");
		for (int i = 0; i < aBuffer.size() && aBuffer[i]; i++)
		{
			char c = aBuffer.at(i);
			result.append(aCodec->toUnicode(&c, 1).at(0));
		}
		
		return result;
	};

	mMessage = readTo1B(mData);
}

//---------------------------------------------------------------------------
QString ConsoleResponse::getMessage() const
{
	return mMessage;
}

//---------------------------------------------------------------------------
MessageResponse::MessageResponse(const BaseResponse & aResponse) :
	BaseResponse(aResponse)
{
	QByteArray response = mData.right(6);

	if (response.size() == 6)
	{
		mCurrentCountTxns = response.left(3);
		mTimeUpload = response.mid(3, 2);
		mStatusCode = response.right(1);
	}
}

//---------------------------------------------------------------------------
bool MessageResponse::needEncashment() const
{
	if (mStatusCode == "1")
	{
		return true;
	}

	if (!mTimeUpload.isEmpty() && mTimeUpload.toUInt() < ::EncashmentThreshold)
	{
		return true;
	}

	if (!mCurrentCountTxns.isEmpty() && mCurrentCountTxns.toUInt() < ::EncashmentThreshold)
	{
		return true;
	}

	return false;
}

} // namespace Uniteller

//---------------------------------------------------------------------------
