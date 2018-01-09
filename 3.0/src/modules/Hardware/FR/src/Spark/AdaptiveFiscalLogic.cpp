/* @file Логика адаптивной печати фискального чека. */

// STL
#include <list>
#include <algorithm>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegExp>
#include <QtCore/QByteArray>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/HardwareConstants.h>

// Modules
#include "PaymentProcessor/PrintConstants.h"

// Project
#include "AdaptiveFiscalLogic.h"

namespace ETextPosition = CSparkFR::TextProperties::EPosition;

//--------------------------------------------------------------------------------
AdaptiveFiscalLogic::AdaptiveFiscalLogic(const QVariantMap & aConfiguration) : mConfiguration(aConfiguration)
{
}

//--------------------------------------------------------------------------------
void AdaptiveFiscalLogic::removeDelimeter(QStringList & aBuffer, ETextPosition::Enum aPosition)
{
	QRegExp regExp("^[ \t_-]*$");

	if (((aPosition == ETextPosition::Bound) || (aPosition == ETextPosition::Up  )) && (regExp.indexIn(aBuffer.first()) != -1)) aBuffer.removeFirst();
	if (((aPosition == ETextPosition::Bound) || (aPosition == ETextPosition::Down)) && (regExp.indexIn(aBuffer.last())  != -1)) aBuffer.removeLast();

	if (aPosition == ETextPosition::AllOver)
	{
		for (int i = 0; i < aBuffer.size(); ++i)
		{
			if (regExp.indexIn(aBuffer[i]) != -1)
			{
				aBuffer.removeAt(i--);
			}
		}
	}
}

//--------------------------------------------------------------------------------
QStringList AdaptiveFiscalLogic::getSeparatedBuffer(const QString & aLine)
{
	QStringList result;
	int size = qCeil(qreal(aLine.size()) / CSparkFR::TextProperties::LineSize);

	for (int j = 0; j < size; ++j)
	{
		result << aLine.mid(j * CSparkFR::TextProperties::LineSize, CSparkFR::TextProperties::LineSize);
	}

	return result;
}

//--------------------------------------------------------------------------------
bool AdaptiveFiscalLogic::adjustBuffer(QStringList & aBuffer, ETextPosition::Enum aPosition)
{
	if (aPosition != ETextPosition::No)
	{
		removeDelimeter(aBuffer, aPosition);
	}

	for (int i = 0; i < aBuffer.size(); ++i)
	{
		QStringList newBuffer = getSeparatedBuffer(aBuffer[i]);
		int size = newBuffer.size();

		if (size > 1)
		{
			for (int j = 0; j < newBuffer.size(); ++j)
			{
				aBuffer.insert(i + j, newBuffer[j]);
			}

			aBuffer.removeAt(i + size);
			i += size - 1;
		}
	}

	return (aBuffer.size() <= CSparkFR::TextProperties::MaxListSize) && (aBuffer.join(CSparkFR::TextProperties::Separator).size() <= CSparkFR::TextProperties::MaxSize);
}

//--------------------------------------------------------------------------------
TMoneyPosition AdaptiveFiscalLogic::getMoneyPosition()
{
	std::list<QString> reciptTemplate = mConfiguration[CHardwareSDK::Printer::ReceiptTemplate].toStringList().toStdList();
	
	TMoneyPosition result(-1, -1);
	QString currencyTemplate = "%" + QString(CPrintConstants::Currency) + "%";

	auto firstIt = std::find_if(reciptTemplate.begin(), reciptTemplate.end(), [&] (const QString & aLine) -> bool
		{ return aLine.contains(currencyTemplate); });

	if (firstIt != reciptTemplate.end())
	{
		auto lastIt = std::find_if(reciptTemplate.rbegin(), reciptTemplate.rend(), [&] (const QString & aLine) -> bool
			{ return aLine.contains(currencyTemplate); });

		if (lastIt != reciptTemplate.rend())
		{
			QStringList qreciptTemplate = QStringList::fromStdList(reciptTemplate);
			result = TMoneyPosition(qreciptTemplate.indexOf(*firstIt), qreciptTemplate.lastIndexOf(*lastIt));
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
bool AdaptiveFiscalLogic::adjustReceipt(const QStringList & aReceipt)
{
	TMoneyPosition moneyPosition = getMoneyPosition();

	if (adjustReceipt(aReceipt, moneyPosition, ETextPosition::No))
	{
		return true;
	}

	if ((moneyPosition != TMoneyPosition(-1, -1)) && adjustReceipt(aReceipt, TMoneyPosition(-1, -1), ETextPosition::No))
	{
		return true;
	}

	return adjustReceipt(aReceipt, TMoneyPosition(-1, -1), ETextPosition::Up) ||
	       adjustReceipt(aReceipt, TMoneyPosition(-1, -1), ETextPosition::Bound) ||
	       adjustReceipt(aReceipt, TMoneyPosition(-1, -1), ETextPosition::AllOver);
}

//--------------------------------------------------------------------------------
bool AdaptiveFiscalLogic::adjustReceipt(const QStringList & aReceipt, const TMoneyPosition & aMoneyPosition, ETextPosition::Enum aPosition)
{
	QStringList receipt(aReceipt);
	removeDelimeter(receipt, aPosition);

	QStringList result;
	result << "";

	bool smartAdaptive = (aMoneyPosition.first > -1) && (aMoneyPosition.first < receipt.size()) &&
		(aMoneyPosition.second >= aMoneyPosition.first) && (aMoneyPosition.second < receipt.size());

	QStringList payDataBuffer;
	QStringList endDataBuffer;

	if (smartAdaptive)
	{
		payDataBuffer = receipt.mid(aMoneyPosition.first, aMoneyPosition.second - aMoneyPosition.first + 1);
		endDataBuffer = receipt.mid(aMoneyPosition.second + 1);

		auto isDataOK = [&] (QStringList & aBuffer, ETextPosition::Enum aPosition) -> bool {
			return adjustBuffer(aBuffer, aPosition) ||
			       adjustBuffer(aBuffer, ETextPosition::Bound) ||
			       adjustBuffer(aBuffer, ETextPosition::AllOver); };

		while (isDataOK(payDataBuffer, ETextPosition::Down) && !isDataOK(endDataBuffer, ETextPosition::Up))
		{
			payDataBuffer << endDataBuffer.first();
			endDataBuffer.removeFirst();
		}

		smartAdaptive = isDataOK(payDataBuffer, ETextPosition::Down) && isDataOK(endDataBuffer, ETextPosition::Up);
	}

	for (int i = 0; i < receipt.size(); ++i)
	{
		if (smartAdaptive && (aMoneyPosition.first == i))
		{
			if (result.size() > (CSparkFR::TextProperties::Numbers.size() - 2))
			{
				return false;
			}

			while (result.size() != (CSparkFR::TextProperties::Numbers.size() - 2))
			{
				result << "";
			}

			result << payDataBuffer.join(CSparkFR::TextProperties::Separator);
			result << endDataBuffer.join(CSparkFR::TextProperties::Separator);

			break;
		}

		// предполагается, что строка < MaxLine
		QString newLine = getSeparatedBuffer(receipt[i]).join(CSparkFR::TextProperties::Separator);
		int lines = result.last().count(CSparkFR::TextProperties::Separator) + 1;

		if ((lines < CSparkFR::TextProperties::MaxListSize) && ((result.last() + newLine).size() < CSparkFR::TextProperties::MaxSize))
		{
			if (!result.last().isEmpty())
			{
				newLine.prepend(CSparkFR::TextProperties::Separator);
			}

			result.last() += newLine;
		}
		else if (result.size() < CSparkFR::TextProperties::Numbers.size())
		{
			result << newLine;
		}
		else
		{
			return false;
		}
	}

	mTextProperties = result;

	return true;
}

//--------------------------------------------------------------------------------
QStringList & AdaptiveFiscalLogic::getTextProperties()
{
	return mTextProperties;
}

//--------------------------------------------------------------------------------
