/* @file Файл, предназначенный для подавления warning'ов от библиотеки Qt. После
         включения этого файла необходимо добавить требующиеся файлы Qt и затем
         QtHeadersEnd.h
 
         Пример:
           #include <Common/QtHeadersBegin.h>
           #include <QtCore/QList>
           #include <Common/QtHeadersEnd.h>
 */

#ifdef _MSC_VER 

#ifdef CYBER_SUPPRESS_QT_WARNINGS
#error QtHeadersBegin.h included without QtHeadersEnd.h!!!
#endif

#define CYBER_SUPPRESS_QT_WARNINGS

#pragma warning(push)

#pragma warning(disable : 4127) // warning C4127: conditional expression is constant
#pragma warning(disable : 4231) // warning C4231: nonstandard extension used : 'extern' before template explicit instantiation
#pragma warning(disable : 4244) // warning C4244: conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable : 4251) // warning C4251: 'QVariant::d' : struct 'QVariant::Private' needs to have dll-interface to be used by clients of class  'QVariant'
#pragma warning(disable : 4275) // warning C4275: non dll-interface class 'QRunnable' used as base for dll-interface class 'QtConcurrent::ThreadEngineBase'
#pragma warning(disable : 4290) // warning C4290: C++ exception specification ignored except to indicate a function is not __declspec(nothrow)
#pragma warning(disable : 4481) // warning C4481: nonstandard extension used: override specifier 'override'
#pragma warning(disable : 4512) // warning C4512: 'QtConcurrent::BlockSizeManager' : assignment operator could not be generated
#pragma warning(disable : 4718) // warning C4718: 'QMapNode<unsigned short,wchar_t *>::destroySubTree' : recursive call has no side effects, deleting
#pragma warning(disable : 4800) // warning C4800: 'QTextBoundaryFinderPrivate *const ' : forcing value to bool 'true' or 'false' (performance warning)
#pragma warning(disable : 4005) // warning C4005: 'QT_LARGEFILE_SUPPORT' : macro redefinition

// возникает в файлах автоматически-сгенерированных из .ui
#pragma warning(disable : 4125) // warning C4125: decimal digit terminates octal escape sequence

#endif

