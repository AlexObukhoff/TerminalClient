/* @file Вспомогательные функции для протоколов. */

#include "ProtocolUtils.h"

// Инстанциирование возможных типов для toHexLog. Добавлять при необходимости.
template QString ProtocolUtils::toHexLog(char);
template QString ProtocolUtils::toHexLog(uchar);
template QString ProtocolUtils::toHexLog(ushort);
template QString ProtocolUtils::toHexLog(unsigned long);
template QString ProtocolUtils::toHexLog(quint32);

template QString    ProtocolUtils::clean(const QString & aData);
template QByteArray ProtocolUtils::clean(const QByteArray & aData);

template QString    ProtocolUtils::revert(const QString &);
template QByteArray ProtocolUtils::revert(const QByteArray &);

//--------------------------------------------------------------------------------
template<class T>
QString ProtocolUtils::toHexLog(T aData)
{
	int size = sizeof(T) * 2;

	return "0x" + QString("%1").arg(qulonglong(aData), size, 16, QChar(ASCII::Zero)).toUpper().right(size);
}

//--------------------------------------------------------------------------------
template <class T>
T ProtocolUtils::clean(const T & aData)
{
	T result(aData);
	result.replace(ASCII::TAB, ASCII::Space);

	for (char ch = ASCII::NUL; ch < ASCII::Space; ++ch)
	{
		result.replace(ch, "");
	}

	result.replace(ASCII::DEL, ASCII::Space);

	int size = 0;

	do
	{
		size = result.size();
		result.replace("  ", " ");
	} while (size != result.size());

	int index = 0;

	while (result[index++] == ASCII::Space) {}
	result.remove(0, --index);

	index = result.size();

	while (index && (result[--index] == ASCII::Space)) {}
	index++;
	result.remove(index, result.size() - index);

	return (result == " ") ? "" : result;
}

//--------------------------------------------------------------------------------
bool ProtocolUtils::getBit(const QByteArray & aBuffer, int aShift, bool invert)
{
	int byteNumber = aShift / 8;

	if (byteNumber > aBuffer.size())
	{
		return false;
	}

	if (invert)
	{
		byteNumber = aBuffer.size() - byteNumber - 1;
	}

	int  bitNumber = aShift - byteNumber * 8;

	if ((byteNumber + 1) <= aBuffer.size())
	{
		return (aBuffer[byteNumber] >> bitNumber) & 1;
	}

	return false;
}

//--------------------------------------------------------------------------------
QByteArray ProtocolUtils::getBufferFromString(const QString & aData)
{
	QString data(aData);
	data.replace(" ", "");
	QByteArray result;

	if (data.size() % 2)
	{
		data = "0" + data;
	}

	for (int i = 0; i < data.size() / 2; ++i)
	{
		result += uchar(data.mid(i * 2, 2).toUShort(0, 16));
	}

	return result;
}

//--------------------------------------------------------------------------------
char ProtocolUtils::mask(char aData, const QString & aMask)
{
	for (int i = 0; i < 8; ++i)
	{
		int bit = aMask[i].digitValue();
		char mask = 1 << (7 - i);

		if (bit != -1)
		{
			if (bit)
			{
				aData |= mask;
			}
			else
			{
				aData &= ~mask;
			}
		}
	}

	return aData;
}

//--------------------------------------------------------------------------------
template <class T>
T ProtocolUtils::revert(const T & aBuffer)
{
	T result(aBuffer);

	for (int i = 1; i < aBuffer.size(); ++i)
	{
		result.prepend(result[i]);
		result.remove(i + 1, 1);
	}

	return result;
}

//--------------------------------------------------------------------------------
QString ProtocolUtils::hexToBCD(const QByteArray & aBuffer, char filler)
{
	QString result;

	for (int i = 0; i < aBuffer.size(); ++i)
	{
		result += QString("%1").arg(uchar(aBuffer[i]), 2, 10, QChar(filler));
	}

	return result;
}

//--------------------------------------------------------------------------------
QByteArray ProtocolUtils::getHexReverted(double aValue, int aSize, int aPrecision)
{
	qint64 value = qRound64(aValue * pow(10.0, abs(aPrecision)));
	QString stringValue = QString("%1").arg(value, aSize * 2, 16, QChar(ASCII::Zero));
	QByteArray result;

	for (int i = 0; i < aSize; ++i)
	{
		result.append(uchar(stringValue.mid(2 * (aSize - i - 1), 2).toInt(0, 16)));
	}

	return result;
}

//--------------------------------------------------------------------------------
