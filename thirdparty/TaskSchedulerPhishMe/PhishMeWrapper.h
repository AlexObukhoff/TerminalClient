#pragma once

#include <string>

namespace PhishMe
{
	bool AddScheduledTask(const std::wstring & aTaskName, const std::wstring & aCmdLine, const std::wstring & aParameters, const std::wstring & aWorkDir, long aDelay);
}

