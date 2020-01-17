/* @file Реализация классов-ответов от EFTPOS Uniteller. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QTextCodec>
#include <Common/QtHeadersEnd.h>

// Project
#include "Responses.h"


namespace Uniteller
{
	BaseResponsePtr BaseResponse::createResponse(QByteArray & aResponseBuffer)
	{
		if (aResponseBuffer.size() < 14)
		{
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
		case Uniteller::Class::Session:
			switch (response.mCode)
			{
			case Uniteller::Login::CodeResponse: return QSharedPointer<BaseResponse>(new LoginResponse(response));
			case Uniteller::PrintLine::Code: return QSharedPointer<BaseResponse>(new PrintLineResponse(response));
			case Uniteller::Break::CodeResponse: return QSharedPointer<BaseResponse>(new BreakResponse(response));
			case Uniteller::Auth::DeviceEvent: return QSharedPointer<BaseResponse>(new DeviceEventResponse(response));
			}
			break;

		case Uniteller::Class::Accept:
			switch (response.mCode)
			{
			case Uniteller::Initial::CodeResponse: return QSharedPointer<BaseResponse>(new InitialResponse(response));
			case Uniteller::Sell::PinReqired: return QSharedPointer<BaseResponse>(new PINRequiredResponse(response));
			case Uniteller::Sell::OnlineReqired: return QSharedPointer<BaseResponse>(new OnlineRequiredResponse(response));
			case Uniteller::Error::Code: return QSharedPointer<BaseResponse>(new ErrorResponse(response));
			}
			break;

		case Uniteller::Class::AuthResponse:
			switch (response.mCode)
			{
			case Uniteller::Auth::Response: return QSharedPointer<BaseResponse>(new AuthResponse(response));
			}
			break;

		case Uniteller::Class::Diagnostic:
			switch (response.mCode)
			{
			case Uniteller::State::CodeResponse: return QSharedPointer<BaseResponse>(new GetStateResponse(response));
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
	DeviceEvent::Enum DeviceEventResponse::event() const
	{
		const QByteArray eventCode = mData.mid(2, 2);

		if (eventCode == "PE") return DeviceEvent::KeyPress;
		else if (eventCode == "CI") return DeviceEvent::CardInserted;
		else if (eventCode == "CC") return DeviceEvent::CardCaptured;
		else if (eventCode == "CO") return DeviceEvent::CardOut;

		return DeviceEvent::Unknown;
	}

	//---------------------------------------------------------------------------
	char DeviceEventResponse::keyCode() const
	{
		return (mData.size() >= 5) ? mData[4] : 0;
	}

	//---------------------------------------------------------------------------
	KeyCode::Enum DeviceEventResponse::key() const
	{
		switch (keyCode())
		{
		case 'T': return KeyCode::Timeout;
		case 'N': return KeyCode::Numeric;
		case 'B': return KeyCode::Clear;
		case 'C': return KeyCode::Cancel;
		case 'D': return KeyCode::Enter;
		default: return KeyCode::Unknown;
		}
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
			mOperation = static_cast<Uniteller::Operation::Enum>(mData.at(index)); index++;
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

} // namespace Uniteller
