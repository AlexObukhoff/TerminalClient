#include <Common/QtHeadersBegin.h>
#include <QtCore/QtGlobal>
#include <Common/QtHeadersEnd.h>

#include <DebugUtils/DebugUtils.h>

#ifdef Q_OS_WIN
#define NOMINMAX // HACK for QDateTime Qt 5.0.0
#include <windows.h>
#include "QStackWalker.h"
#endif

//---------------------------------------------------------------------------
void DumpCallstack(QStringList & aStack, void * aContext)
{
#ifdef Q_OS_WIN
	QStackWalker walker;

	aStack = walker.getCallstack(aContext);
#else
	#error The method DumpCallStack is not implemented for the current platform.
#endif // Q_OS_WIN
}

//---------------------------------------------------------------------------
void SetUnhandledExceptionsHandler(TExceptionHandler aHandler)
{
	_set_abort_behavior( 0, _WRITE_ABORT_MSG);
	_set_abort_behavior( 0, _CALL_REPORTFAULT);

#ifdef Q_OS_WIN
	SetUnhandledExceptionFilter(aHandler);
#else
	#error The method SetUnhandledExceptionsHandler is not implemented for the current platform.
#endif // Q_OS_WIN
}

//---------------------------------------------------------------------------
