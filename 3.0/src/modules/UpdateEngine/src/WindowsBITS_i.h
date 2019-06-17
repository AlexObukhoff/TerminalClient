#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QUuid>
#include <QtCore/QDebug>
#include <Common/QtHeadersEnd.h>

namespace CBITS
{

//---------------------------------------------------------------------------
enum EJobStates
{
	EJobStateQueued = 0,
	EJobStateConnecting = 1,
	EJobStateTransferring = 2,
	EJobStateSuspended = 3,
	EJobStateError = 4,
	EJobStateTransientError = 5,
	EJobStateTransferred = 6,
	EJobStateAcknowledged = 7,
	EJobStateCancelled = 8,
	EJobStateUnknown = 15,
};

//---------------------------------------------------------------------------
enum EFileStates
{
	EFileStateTransferring = 9,
	EFileStateTransferred = 6,
	EFileStateUnknown = 15,
};

enum EJobPriority
{
	FOREGROUND = 0,
	HIGH,
	NORMAL,
	LOW
};

//-------------------------------------------------------------------------------------------------
struct SJobProgress
{
	quint64 bytesTotal;
	quint64 bytesTransferred;
	quint32 filesTotal;
	quint32 filesTransferred;

	SJobProgress() : bytesTotal(0), bytesTransferred(0), filesTotal(0), filesTransferred(0) {}

	QString toString() const
	{
		return QString("%1/%2 bytes in %3/%4 files").arg(bytesTransferred).arg(bytesTotal).arg(filesTransferred).arg(filesTotal);
	}
};

//-------------------------------------------------------------------------------------------------
class SJob
{
public:
	SJob()
	{
		mState = EJobStateUnknown;
		mMinRetryDelay = 0;
		mNoProgressTimeout = 0;
	}

	bool isComplete() const
	{
		return mState == EJobStateTransferred || mState == EJobStateAcknowledged;
	}

	bool isFatal() const
	{
		return mState == EJobStateError || mState == EJobStateCancelled;
	}

	bool inProgress() const
	{
		return !isComplete() && !isFatal();
	}

	QString toString() const
	{
		return QString("UUID: %1. Name: '%2'. Dscr: %3. State: %4. Progress: %5. MinRetryDelay: %6. NoProgressTimeout: %7.")
			.arg(mGuidID.toString()).arg(mName).arg(mDesc)
			.arg(mState).arg(mProgress.toString()).arg(mMinRetryDelay).arg(mNoProgressTimeout);
	}

public:
	QUuid        mGuidID;
	QString		 mName;
	QString      mDesc;
	quint32      mState;
	SJobProgress mProgress;
	quint32      mMinRetryDelay;
	quint32      mNoProgressTimeout;
};


//---------------------------------------------------------------------------
} // namespace CBITS
