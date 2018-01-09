/* @file Типы-функторы. */

#pragma once

// STL
#include <functional>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMetaType>
#include <Common/QtHeadersEnd.h>

//--------------------------------------------------------------------------------
typedef std::function<void()> TVoidMethod;
typedef std::function<bool()> TBoolMethod;
typedef std::function<int()> TIntMethod;
typedef std::function<double()> TDoubleMethod;
typedef std::function<QString()> TStringMethod;

Q_DECLARE_METATYPE(TVoidMethod);
Q_DECLARE_METATYPE(TBoolMethod);
Q_DECLARE_METATYPE(TIntMethod);
Q_DECLARE_METATYPE(TDoubleMethod);
Q_DECLARE_METATYPE(TStringMethod);

/// Регистрация.
const int VoidFunctionType   = qRegisterMetaType<TVoidMethod>("TVoidMethod");
const int BoolFunctionType   = qRegisterMetaType<TBoolMethod>("TBoolMethod");
const int IntFunctionType    = qRegisterMetaType<TIntMethod>("TIntMethod");
const int DoubleFunctionType = qRegisterMetaType<TDoubleMethod>("TDoubleMethod");
const int StringFunctionType = qRegisterMetaType<TStringMethod>("TStringMethod");

//--------------------------------------------------------------------------------
