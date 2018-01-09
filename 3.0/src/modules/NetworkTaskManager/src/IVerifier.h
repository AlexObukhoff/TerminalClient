/* @file Интерфейс верификатора данных. */

#pragma once

class QByteArray;
class NetworkTask;

//------------------------------------------------------------------------
class IVerifier
{
public:
	virtual ~IVerifier() {}

	virtual bool verify(NetworkTask * aTask, const QByteArray & aData) = 0;
};

//------------------------------------------------------------------------
