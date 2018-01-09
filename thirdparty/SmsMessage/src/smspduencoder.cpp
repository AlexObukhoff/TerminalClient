/* @file Декодер сообщения в формат PDU. */

#include "smspduencoder.h"

//------------------------------------------------------------------------------
quint8 SmsPduEncoder::mEncodeMask[] = { 1, 3, 7, 15, 31, 63, 127 };

//------------------------------------------------------------------------------
QByteArray SmsPduEncoder::encode(const QByteArray & aUnpackedBytes, quint8 aDefaultByte /*= ' '*/)
{
	QByteArray shiftedBytes;

	int shiftOffset = 0;
	int shiftIndex = 0;

	shiftedBytes.resize(aUnpackedBytes.size() - (aUnpackedBytes.size()/ 8));

	// Shift the unpacked bytes to the right according to the offset (position of the quint8)
	foreach (quint8 b, aUnpackedBytes)
	{
		quint8 tmpByte = b;

		// Handle invalid characters (bytes out of range)
		if (tmpByte > 127)
		{
			tmpByte = aDefaultByte;
		}

		// Perform the quint8 shifting
		if (shiftOffset == 7)
		{
			shiftOffset = 0;
		}
		else
		{
			shiftedBytes[shiftIndex] = (quint8)(tmpByte >> shiftOffset);
			shiftOffset++;
			shiftIndex++;
		}
	}

	int moveOffset = 1;
	int moveIndex = 1;
	int packIndex = 0;
	QByteArray packedBytes;
	packedBytes.resize(shiftedBytes.size());

	// Move the bits to the appropriate quint8 (pack the bits)
	foreach (quint8 b, aUnpackedBytes)
	{
		if (moveOffset == 8)
		{
			moveOffset = 1;
		}
		else
		{
			if (moveIndex != aUnpackedBytes.size())
			{
				// Extract the bits to be moved
				int extractedBitsByte = (aUnpackedBytes[moveIndex] & mEncodeMask[moveOffset - 1]);
				// Shift the extracted bits to the proper offset
				extractedBitsByte = (extractedBitsByte << (8 - moveOffset));
				// Move the bits to the appropriate quint8 (pack the bits)
				int movedBitsByte = (extractedBitsByte | shiftedBytes[packIndex]);

				packedBytes[packIndex] = (quint8)movedBitsByte;

				moveOffset++;
				packIndex++;
			}
			else
			{
				packedBytes[packIndex] = shiftedBytes[packIndex];
			}
		}

		moveIndex++;
	}

	return packedBytes;
}
