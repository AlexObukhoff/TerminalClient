#pragma once

#include "Common/QtHeadersBegin.h"
#include <QtCore/QThread>
#include "Common/QtHeadersEnd.h"

//---------------------------------------------------------------------------
class SleepHelper : public QThread
{
public:
	using QThread::sleep;
	using QThread::usleep;
	using QThread::msleep;
};

//---------------------------------------------------------------------------
