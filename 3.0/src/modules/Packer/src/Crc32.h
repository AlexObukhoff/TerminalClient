#ifndef CRC32_H
#define CRC32_H

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QByteArray>
#include <Common/QtHeadersEnd.h>

class Crc32
{
private:
    quint32 mCrcTable[256];
    QMap<int, quint32> instances;

public:
	Crc32()
	{
		quint32 crc;

		// initialize CRC table
		for (int i = 0; i < 256; i++)
		{
			crc = i;
			for (int j = 0; j < 8; j++)
				crc = crc & 1 ? (crc >> 1) ^ 0xEDB88320UL : crc >> 1;

			mCrcTable[i] = crc;
		}
	}

	quint32 fromFile(const QString & aFileName)
	{
		quint32 crc;
		QFile file;

		char buffer[16000];
		int len, i;

		crc = 0xFFFFFFFFUL;

		file.setFileName(aFileName);
		if (file.open(QIODevice::ReadOnly))
		{
			while (!file.atEnd())
			{
				len = file.read(buffer, 16000);
				for (i = 0; i < len; i++)
					crc = mCrcTable[(crc ^ buffer[i]) & 0xFF] ^ (crc >> 8);
			}

			file.close();
		}

		return crc ^ 0xFFFFFFFFUL;
	}
	
	quint32 fromByteArray(const QByteArray & aBuffer)
	{
		quint32 crc;

		crc = 0xFFFFFFFFUL;

		for (int i = 0; i < aBuffer.size(); i++)
			crc = mCrcTable[(crc ^ aBuffer[i]) & 0xFF] ^ (crc >> 8);

		return crc ^ 0xFFFFFFFFUL;
	}

	void initInstance(int i)
	{
		instances[i] = 0xFFFFFFFFUL;
	}

	void pushData(int i, char * aData, int aLen)
	{
		quint32 crc = instances[i];
		if (crc)
		{
			for (int j = 0; j < aLen; j++)
				crc = mCrcTable[(crc ^ aData[j]) & 0xFF] ^ (crc >> 8);

			instances[i] = crc;
		}
	}

	quint32 releaseInstance(int i)
	{
		quint32 crc32 = instances[i];
		if (crc32) {
			instances.remove(i);
			return crc32 ^ 0xFFFFFFFFUL;
		}
		else {
			return 0;
		}
	}
};

#endif // CRC32_H
