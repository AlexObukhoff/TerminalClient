
#include "stdafx.h"
#include "PhishMeWrapper.h"
#include "ScheduleTask.h"

#pragma comment(lib, "taskschd.lib")

namespace PhishMe
{
	// Add the task to Task Scheduler
	bool AddScheduledTask(
		const std::wstring & aTaskName, 
		const std::wstring & aCmdLine, 
		const std::wstring & aParameters, 
		const std::wstring & aWorkDir, 
		long aDelay)
	{
		ScheduleTask scheduler;
		scheduler.CreateScheduledTask(aTaskName.c_str(), aCmdLine.c_str(), aParameters.c_str(), aWorkDir.c_str(), aDelay);
		return true;
	}
}
