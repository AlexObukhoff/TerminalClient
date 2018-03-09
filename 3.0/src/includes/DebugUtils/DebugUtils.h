/* @file Различные средства отладки. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

#ifdef Q_OS_WIN
#define NOMINMAX
#include <windows.h>
typedef LPTOP_LEVEL_EXCEPTION_FILTER TExceptionHandler;
#else
	#error The type TExceptionHandler is not defined for the current platform.
#endif

//---------------------------------------------------------------------------
/// Заполняет список aStack именами процедур из активного call-stack'a.
/// Контекст, для которого определяется стек вызовов передаётся в параметре aContext.
void DumpCallstack(QStringList & aStack, void * aContext = 0);

/// Устанавливает перехватчик необработанных исключений.
void SetUnhandledExceptionsHandler(TExceptionHandler aHandler);

//---------------------------------------------------------------------------
