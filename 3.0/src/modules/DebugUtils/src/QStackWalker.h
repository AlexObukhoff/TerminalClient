#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

#include "StackWalker.h"

//---------------------------------------------------------------------------
class QStackWalker : protected StackWalker
{
public:
	QStackWalker() {}
	virtual ~QStackWalker() {}

	inline QStringList getCallstack(void * aContext)
	{
		mCallstack.clear();

		StackWalker::ShowCallstack(GetCurrentThread(), static_cast<CONTEXT *>(aContext));

		return mCallstack;
	}

protected:
	virtual void OnCallstackEntry(CallstackEntryType eType, CallstackEntry &entry)
	{
		CHAR buffer[STACKWALK_MAX_NAMELEN];

		if ((eType != lastEntry) && (entry.offset != 0))
		{
			if (entry.name[0] == 0)
			{
				strcpy_s(entry.name, "(function-name not available)");
			}

			if (entry.undName[0] != 0)
			{
				strcpy_s(entry.name, entry.undName);
			}

			if (entry.undFullName[0] != 0)
			{
				strcpy_s(entry.name, entry.undFullName);
			}

			if (entry.lineFileName[0] == 0)
			{
				strcpy_s(entry.lineFileName, "(filename not available)");
				if (entry.moduleName[0] == 0)
				{
					strcpy_s(entry.moduleName, "module-name not available");
				}
				
				_snprintf_s(buffer, STACKWALK_MAX_NAMELEN, "%p (%s:%p): %s: %s", (LPVOID)entry.offset, entry.moduleName, 
					(LPVOID)(entry.offset - entry.baseOfImage), entry.lineFileName, entry.name);
			}
			else
			{
				_snprintf_s(buffer, STACKWALK_MAX_NAMELEN, "%s (%d): %s", entry.lineFileName, entry.lineNumber, entry.name);
			}
			
			mCallstack.push_back(buffer);
		}
	}

private:
	QStringList mCallstack;
};

//---------------------------------------------------------------------------
