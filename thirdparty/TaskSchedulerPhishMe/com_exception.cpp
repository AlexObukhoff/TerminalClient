//=============================================================================
// File: com_exception.cpp - COM-specific exception class implementation
//
//    Copyright (c) 2017 Jeff Reeder
//    All Rights Reserved
//=============================================================================

#include "stdafx.h"

#include "com_exception.h"

using namespace std;
using namespace PhishMe;


// Create a COM exception object with just a message (no HRESULT)
com_exception::com_exception( const char* szMessage )
{
   if ( ! IsStringNonEmpty(szMessage) )   throw invalid_argument( FN_MSG( "Invalid szMessage parameter" ) );

   _message = string(szMessage);
}


// Create a COM exception object with just a message with an HRESULT
com_exception::com_exception( const char* szMessage,
                              HRESULT     hResult /* = InvalidHresult */ )
{
   if ( ! IsStringNonEmpty(szMessage) )   throw invalid_argument( FN_MSG( "Invalid szMessage parameter" ) );
   if (  hResult < 0L                 )   throw invalid_argument( FN_MSG( "hResult cannot be negative"  ) );

   if ( hResult < 0 )   _message  =  string(szMessage);
   else                 _message  =  string(szMessage)  +  ".  Result: "  +  to_string((unsigned long long)hResult);
}


// Get the exception message
const char* com_exception::what() const
{
   return _message.c_str();
}
