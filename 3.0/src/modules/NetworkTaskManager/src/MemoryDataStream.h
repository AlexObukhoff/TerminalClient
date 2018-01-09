/* @file Реализация потока данных в памяти. */

#pragma once

#include "DataStream.h"

class QByteArray;
class QString;

//------------------------------------------------------------------------
class MemoryDataStream : public DataStream
{
public:
	MemoryDataStream();

	virtual bool clear();
};

//------------------------------------------------------------------------
