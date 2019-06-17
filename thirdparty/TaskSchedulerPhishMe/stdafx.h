//=============================================================================
// File: stdafx.h - Precompiled header file for the application
//
//    Copyright (c) 2017 Jeff Reeder
//    All Rights Reserved
//=============================================================================

#pragma once

#define _WIN32_DCOM                       // For enabling DCOM support

// Windows headers
#include <Windows.h>
#include <tchar.h>
#include <comdef.h>
#include <wincred.h>
#include <taskschd.h>                     // Task scheduler definitions

// C runTime header files
#include <cstdio>
#include <cstdlib>

// C++ class headers
#include <iostream>
#include <sstream>
#include <string>
#include <exception>

// MBCS <-> Unicode conversion - extracted from ATL
#include "Utils.h"                        // Generic utilities


#include <InitGuid.h>
