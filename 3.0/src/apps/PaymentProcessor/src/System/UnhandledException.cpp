/* @file Обработчик исключений, которые не ловятся связкой try ... catch. */

#include <Common/QtHeadersBegin.h>
#include <QtCore/QtGlobal>
#include <Common/QtHeadersEnd.h>

#include <Common/ILog.h>

#include <DebugUtils/DebugUtils.h>

//---------------------------------------------------------------------------
#ifdef Q_OS_WIN

#include <windows.h>

//---------------------------------------------------------------------------
// Обработчик для Windows-реализации
LONG WINAPI MyUnhandledExceptionFilter(_EXCEPTION_POINTERS * aExceptionInfo)
{
	QStringList stack;
	DumpCallstack(stack, aExceptionInfo->ContextRecord);

	ILog::getInstance("Exceptions")->write(LogLevel::Fatal,
		QString("Unhandled exception caught. Callstack:\n%1").arg(stack.join("\n")));

	abort();

	return EXCEPTION_CONTINUE_EXECUTION;
}

//---------------------------------------------------------------------------
void PurecallHandler(void)
{
	QStringList stack;
	DumpCallstack(stack, nullptr);

	ILog::getInstance("Exceptions")->write(LogLevel::Fatal, 
		QString("Pure virtual call. Callstack:\n%1").arg(stack.join("\n")));

	throw std::exception("Pure virtual call");
}

#else
	#error Unhandled exception handler is not implemented for this platform!
#endif // Q_OS_WIN

//---------------------------------------------------------------------------
void CatchUnhandledExceptions()
{
	SetUnhandledExceptionsHandler(MyUnhandledExceptionFilter);

#ifdef Q_OS_WIN
	_set_purecall_handler(PurecallHandler);
#endif // Q_OS_WIN
}

//---------------------------------------------------------------------------