/* @file Обертка над подсистемой Windows BITS. Приватный класс. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDir>
#include <Common/QtHeadersEnd.h>

// Project
#include "WindowsBITS_i.h"
#include "WindowsBITS_p.h"


namespace CBITS
{
//---------------------------------------------------------------------------
QSet<Qt::HANDLE> CopyManager_p::mThreadInitialized;


//---------------------------------------------------------------------------
CopyManager_p::CopyManager_p(ILog * aLog) : ILogable(aLog)
{
	HRESULT hr;

	if (!mThreadInitialized.contains(QThread::currentThreadId()))
	{
		hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
		if (FAILED(hr) && hr != RPC_E_CHANGED_MODE)
		{
			toLog(LogLevel::Error, QString("BITS: Error call CoInitializeEx(), result=0x%1").arg(hr, 0, 16));
			return;
		}

		//The impersonation level must be at least RPC_C_IMP_LEVEL_IMPERSONATE.
		hr = CoInitializeSecurity(NULL,
			-1,
			NULL,
			NULL,
			RPC_C_AUTHN_LEVEL_CONNECT,
			RPC_C_IMP_LEVEL_IMPERSONATE,
			NULL,
			EOAC_DYNAMIC_CLOAKING,
			0);
		if (FAILED(hr) && hr != RPC_E_TOO_LATE)
		{
			toLog(LogLevel::Error, QString("BITS: Error call CoInitializeSecurity(), result=0x%1").arg(hr, 0, 16));
			return;
		}

		mThreadInitialized.insert(QThread::currentThreadId());
	}

	// Connect to BITS.
	hr = CoCreateInstance(__uuidof(BackgroundCopyManager), NULL,
		CLSCTX_LOCAL_SERVER,
		__uuidof(IBackgroundCopyManager),
		(void**)&mQueueMgr);
	if (FAILED(hr))
	{
		toLog(LogLevel::Error, QString("BITS: Failed to connect with BITS, result=0x%1").arg(hr, 0, 16));
	}
}

//---------------------------------------------------------------------------
CopyManager_p::~CopyManager_p()
{
}

//---------------------------------------------------------------------------
bool CopyManager_p::isReady() const
{
	return (mQueueMgr != nullptr);
}

//---------------------------------------------------------------------------
bool fillJobInfo(CComPtr<IBackgroundCopyJob> & aJob, SJob & aJobInfo)
{
	GUID id;
	if (FAILED(aJob->GetId(&id)))
	{
		return false;
	}

	aJobInfo.mGuidID = QUuid(id);

	LPWSTR pwstrText = nullptr;
	if (FAILED(aJob->GetDisplayName(&pwstrText)))
	{
		return false;
	}
	
	aJobInfo.mName = QString::fromWCharArray(pwstrText);
	::CoTaskMemFree(pwstrText);

	if (FAILED(aJob->GetDescription(&pwstrText)))
	{
		return false;
	}

	aJobInfo.mDesc = QString::fromWCharArray(pwstrText);
	::CoTaskMemFree(pwstrText);

	aJob->GetState((BG_JOB_STATE *)&aJobInfo.mState);
	aJob->GetMinimumRetryDelay((ULONG *)&aJobInfo.mMinRetryDelay);
	aJob->GetNoProgressTimeout((ULONG *)&aJobInfo.mNoProgressTimeout);

	BG_JOB_PROGRESS progress;
	if (FAILED(aJob->GetProgress(&progress)))
	{
		return false;
	}

	aJobInfo.mProgress.bytesTotal = progress.BytesTotal;
	aJobInfo.mProgress.bytesTransferred = progress.BytesTransferred;
	aJobInfo.mProgress.filesTotal = progress.FilesTotal;
	aJobInfo.mProgress.filesTransferred = progress.FilesTransferred;

	return true;
}

//---------------------------------------------------------------------------
QMap<QString, SJob> CopyManager_p::getJobs()
{
	QMap<QString, SJob> result;

	if (mQueueMgr == nullptr)
	{
		return result;
	}

	CComPtr<IEnumBackgroundCopyJobs> enumJobs;
	HRESULT hr = mQueueMgr->EnumJobs(0, &enumJobs);
	
	if (SUCCEEDED(hr))
	{
		ULONG ulCount = 0;
		hr = enumJobs->GetCount(&ulCount);
		hr = enumJobs->Reset();
		// signed/unsigned syndrome
		int iCount = ulCount;
		for (int i = 0; i < iCount; i++)
		{
			CComPtr<IBackgroundCopyJob> job;
			hr = enumJobs->Next(1, &job, NULL);
			if (SUCCEEDED(hr))
			{
				SJob jobItem;

				if (fillJobInfo(job, jobItem))
				{
					toLog(LogLevel::Debug, QString("BITS: JOB: %1").arg(jobItem.toString()));

					result[jobItem.mName] = jobItem;
				}
				else
				{
					toLog(LogLevel::Error, QString("BITS: Failed fill job info."));
				}
			}
			else
			{
				toLog(LogLevel::Error, QString("BITS: Failed get next job, result=0x%1").arg(hr, 0, 16));
			}
		}
	}
	else
	{
		toLog(LogLevel::Error, QString("BITS: Failed to enum jobs BITS, result=0x%1").arg(hr, 0, 16));
	}

	return result;
}

//---------------------------------------------------------------------------
bool CopyManager_p::cancel()
{
	return mCurrentJob && SUCCEEDED(mCurrentJob->Cancel());
}

//---------------------------------------------------------------------------
bool CopyManager_p::complete()
{
	HRESULT hr = E_FAIL;

	if (mCurrentJob)
	{
		hr = mCurrentJob->Complete();

		if (FAILED(hr))
		{
			toLog(LogLevel::Error, QString("BITS: Failed to complete job, result=0x%1").arg(hr, 0, 16));
		}
	}

	return SUCCEEDED(hr);
}

//---------------------------------------------------------------------------
BG_JOB_PRIORITY priorityConvert(int aPriority)
{
	switch (aPriority)
	{
	case CBITS::FOREGROUND:  return BG_JOB_PRIORITY_FOREGROUND;
	case CBITS::HIGH:  return BG_JOB_PRIORITY_HIGH;
	case CBITS::NORMAL:  return BG_JOB_PRIORITY_NORMAL;
	case CBITS::LOW:  return BG_JOB_PRIORITY_LOW;
	default:
		return BG_JOB_PRIORITY_HIGH;
	}
}

//---------------------------------------------------------------------------
bool CopyManager_p::createJob(const QString & aName, SJob & aJob, int aPriority)
{
	if (!mQueueMgr)
	{
		return false;
	}

	if (mCurrentJob)
	{
		mCurrentJob.Release();
	}

	GUID guidJob;
	HRESULT hr = mQueueMgr->CreateJob(aName.toStdWString().c_str(), BG_JOB_TYPE_DOWNLOAD, &guidJob, &mCurrentJob);
	if (FAILED(hr) || mCurrentJob == nullptr)
	{
		toLog(LogLevel::Error, QString("BITS: Failed to create job, result=0x%1").arg(hr, 0, 16));

		return false;
	}

	mCurrentJob->SetPriority(priorityConvert(aPriority));

	CComPtr<IBackgroundCopyJobHttpOptions> httpOption;
	if (SUCCEEDED(mCurrentJob->QueryInterface(&httpOption)) && httpOption)
	{
		ULONG flags = 0;
		httpOption->GetSecurityFlags(&flags);
		httpOption->SetSecurityFlags(flags |
			BG_SSL_IGNORE_CERT_CN_INVALID |
			BG_SSL_IGNORE_CERT_DATE_INVALID |
			BG_SSL_IGNORE_UNKNOWN_CA |
			BG_SSL_IGNORE_CERT_WRONG_USAGE);
	}

	fillJobInfo(mCurrentJob, aJob);

	return (mCurrentJob != nullptr);
}

//---------------------------------------------------------------------------
bool CopyManager_p::setJobNotify(const QString & aApplicationPath, const QString & aParamaters)
{
	CComPtr<IBackgroundCopyJob2> job2;
	if (mCurrentJob && SUCCEEDED(mCurrentJob->QueryInterface(&job2)) && job2)
	{
		std::wstring appPath = aApplicationPath.toStdWString();

#ifdef _DEBUG
		appPath = L"C:\\Devil\\Projects\\term_dev\\TerminalClient\\MyWorkDir\\Updater.exe";
#endif

		std::wstring parameters = appPath + L" " + aParamaters.toStdWString();
		auto hr1 = job2->SetNotifyCmdLine(appPath.c_str(), parameters.c_str());
		auto hr2 = job2->SetNotifyFlags(BG_NOTIFY_JOB_TRANSFERRED | BG_NOTIFY_JOB_ERROR);

		LPWSTR namePtr = nullptr;
		mCurrentJob->GetDisplayName(&namePtr);
		QString name = QString::fromWCharArray(namePtr);
		CoTaskMemFree(namePtr);

		if (SUCCEEDED(hr1) && SUCCEEDED(hr2))
		{
			toLog(LogLevel::Normal, QString("Set job '%1' notify: '%2' '%3'").arg(name).arg(aApplicationPath).arg(aParamaters));
			
			return true;
		}
		else
		{
			toLog(LogLevel::Error, QString("Failed set job '%1' notify: '%2' '%3'").arg(name).arg(aApplicationPath).arg(aParamaters));
		}
	}

	return false;
}

//---------------------------------------------------------------------------
CBITS::AddTaskResult::Enum CopyManager_p::addTask(const QUrl & aUrl, const QString & aFileName)
{
	if (mCurrentJob)
	{
		QString url = aUrl.toString();
		QString path = QDir::toNativeSeparators(aFileName);

		auto hr = mCurrentJob->AddFile(url.toStdWString().c_str(), path.toStdWString().c_str());
		if (SUCCEEDED(hr))
		{
			toLog(LogLevel::Debug, QString("BITS: Add task to job OK: %1.").arg(url));

			return AddTaskResult::OK;
		}

		if (hr == BG_E_TOO_MANY_FILES_IN_JOB)
		{
			toLog(LogLevel::Error, QString("BITS: Add task to job failed. Job is FULL. Url: %1").arg(url));

			return AddTaskResult::JobIsFull;
		}

		toLog(LogLevel::Error, QString("BITS: Add task to job failed: %1. HRESULT=0x%2. Url: %3").arg(getJobError()).arg(hr, 8, 16).arg(url));
	}

	return AddTaskResult::Error;
}

//---------------------------------------------------------------------------
bool CopyManager_p::openJob(const SJob & aJob)
{
	if (!mQueueMgr)
	{
		return false;
	}

	if (mCurrentJob)
	{
		mCurrentJob.Release();
	}

	HRESULT hr = mQueueMgr->GetJobW(aJob.mGuidID, &mCurrentJob);
	if (SUCCEEDED(hr) && mCurrentJob)
	{
		return true;
	}

	toLog(LogLevel::Error, QString("BITS: Failed open the job: %1. HRESULT=0x%2.").arg(aJob.toString()).arg(hr, 8, 16));
	return false;
}

//---------------------------------------------------------------------------
bool CopyManager_p::resume()
{
	if (mCurrentJob)
	{
		HRESULT hr = mCurrentJob->Resume();
		if (SUCCEEDED(hr))
		{
			return true;
		}

		toLog(LogLevel::Error, QString("BITS: Resume job failed: %1. HRESULT=0x%2.").arg(getJobError()).arg(hr, 8, 16));
	}
	
	return false;
}

//---------------------------------------------------------------------------
QString CopyManager_p::getJobError()
{
	CComPtr<IBackgroundCopyError> error;
	if (SUCCEEDED(mCurrentJob->GetError(&error)) && error)
	{
		LPWSTR errorMsg = nullptr;
		error->GetErrorDescription(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), &errorMsg);
		if (errorMsg)
		{
			QString msg = QString::fromWCharArray(errorMsg);
			CoTaskMemFree(errorMsg);
			
			return msg;
		}
	}

	return QString();
}

//---------------------------------------------------------------------------
} // namespace CBITS

