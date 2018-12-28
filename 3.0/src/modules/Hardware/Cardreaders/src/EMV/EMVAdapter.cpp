/* @file Класс-обёртка над EMV протоколом. */

#include "EMVAdapter.h"
#include "EMVConstants.h"

#include "TLV.h"

namespace EMV
{
Application AidList[] = 
{
	Application("A0000000031010", "VISA"),
	Application("A0000000041010", "MC"),
	Application("A0000004320001", "PRO100"),
	Application("A0000000032010", "VISA Electron"),
	Application("A0000000046000", "Cirrus"),
	Application("A00000002501", "AMEX")
};
}

// Во многом логика взята отсюда: http://blog.saush.com/2006/09/08/getting-information-from-an-emv-chip-card/

//------------------------------------------------------------------------------
EMVAdapter::EMVAdapter() : mReader(nullptr)
{
}

//------------------------------------------------------------------------------
EMVAdapter::EMVAdapter(SDK::Driver::IMifareReader * aReader) : mReader(aReader)
{
}

//------------------------------------------------------------------------------
bool EMVAdapter::selectApplication(const EMV::Application & aApp, bool aFirst)
{
	return selectApplication(QByteArray::fromHex(aApp.aid.toLatin1()), aFirst);
}

//------------------------------------------------------------------------------
bool EMVAdapter::selectApplication(const QByteArray & aAppID, bool aFirst)
{
	QByteArray request = EMV::Command::SelectPSE;

	if (!aFirst)
	{
		request[3] = 0x02;
	}

	request.append(char(aAppID.size()));
	request.append(aAppID);
	request.append(char(0));

	QByteArray response;

	return mReader->communicate(request, response) &&
		(response.left(2) == QByteArray::fromRawData("\x90\x00", 2) || response.at(0) == EMV::Tags::FCI);
}

//------------------------------------------------------------------------------
bool EMVAdapter::getTrack2(QByteArray & aData)
{
	QByteArray answer;

	if (!mReader->reset(answer))
	{
		return false;
	}

	// Пытаемся выбрать платежное средство какие знаем
	for (int i = 0; i < sizeof(EMV::AidList) / sizeof(EMV::AidList[0]); ++i)
	{
		if (selectApplication(EMV::AidList[i]))
		{
			mApp = EMV::AidList[i];

			answer.clear();

			if (getAFL(answer))
			{
				mApp.sfi = answer[0] >> 3;
				mApp.recordIndex = answer[1];
			}

			answer.clear();

			if (readRecord(mApp.recordIndex, answer))
			{
				EMV::TLV::SItem item = EMV::TLV::TLVs(answer).getTag(EMV::Tags::Track2);

				if (!item.isEmpty())
				{
					aData = item.body.toHex().replace('D', '=').replace('d', '=');

					return true;
				}
			}
		
			// если нашли проложение но не смогли получить номер карты - значит дальше перебирать смысла нет
			return false;
		}
	}

	return false;
}

//------------------------------------------------------------------------------
bool EMVAdapter::readRecord(quint8 aRecIndex, QByteArray & aResponse)
{
	QByteArray request = EMV::Command::ReadRecord;
	request[2] = aRecIndex;
	request[3] = (mApp.sfi << 3) | 0x04;

	if (mReader->communicate(request, aResponse))
	{
		if (aResponse.at(0) == EMV::Tags::WrongLen)
		{
			request[4] = aResponse[1];
			aResponse.clear();
			
			return mReader->communicate(request, aResponse) && aResponse.at(0) == EMV::Tags::EMVTemplate;
		}

		return aResponse.at(0) == EMV::Tags::EMVTemplate;
	}

	return false;
}

//------------------------------------------------------------------------------
bool EMVAdapter::getAFL(QByteArray & aResponse)
{
	QByteArray response;

	if (mReader->communicate(EMV::Command::GetProcessingOptions, response) && !response.isEmpty())
	{
		// EMV Book 3: 6.5.8.4 Data Field Returned in the Response Message
		if (response.at(0) == char(EMV::Tags::ResponseFormat1))
		{
			EMV::TLV::SItem item = EMV::TLV::TLVs(response).getTag(EMV::Tags::ResponseFormat1);
			aResponse = item.body.mid(2);
			return !aResponse.isEmpty();
		}

		EMV::TLV::SItem item = EMV::TLV::TLVs(response).getTag(EMV::Tags::AFL);

		if (!item.isEmpty())
		{
			aResponse = item.body;
			return !aResponse.isEmpty();
		}
	}

	return false;
}

//------------------------------------------------------------------------------
