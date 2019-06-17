//=============================================================================
// File: Utils.h - Utility defintions
//
//    Copyright (c) 2017 Jeff Reeder
//    All Rights Reserved
//
// This header file contains macros to simplify ANSI <-> TCHAR <-> Unicode
// conversion (like T2A(), and others in ATL).  It also has some helpful macros
// to make exception messages much more meaningful as to the origin of the
// exception.
//
// It also has a few other useful utility macros.
//=============================================================================

#pragma once

namespace PhishMe
{
   using namespace std;

   // Convert a std::wstring to std::string
   string WideToAnsi( const wstring sWide );

   // Convert a std::string to std::wstring
   wstring AnsiToWide( const string sAnsi );


   #ifdef _UNICODE
      #define T2Ansi(t)   ( WideToAnsi( (t) ) )          // W->A
      #define T2Wide(t)   ( wstring(t)        )          // W->W   

      #define Ansi2T(a)   ( AnsiToWide( (a) ) )          // A->W
      #define Wide2T(w)   ( wstring(w)        )          // W->W

      #define tstring wstring
   #else
      #define T2Ansi(t)   ( string(t)         )          // A->A
      #define T2Wide(t)   ( AnsiToWide( (t) ) )          // A->W   

      #define Ansi2T(a)   ( string(a)         )          // A->A
      #define Wide2T(w)   ( WideToAnsi( (w) ) )          // A->W

      #define tstring string
   #endif

   #define A2Ansi(a)      ( string(a)         )          // A->A
   #define A2Wide(a)      ( AnsiToWide( (a) ) )          // A->W   
                                                         
   #define W2Ansi(w)      ( WideToAnsi( (w) ) )          // W->A
   #define W2Wide(w)      ( wstring(w)        )          // W->W   


   // Is a string valid (i.e., non-null, and non-empty)
   inline bool IsStringNonEmpty( LPCSTR szString ) 
   {
      return szString  != nullptr  &&
             *szString != '\0';
   }

   // Is a string valid (i.e., non-null, and non-empty)
   inline bool IsStringNonEmpty( LPCWSTR wszString )
   {
      return  wszString != nullptr  &&
             *wszString != L'\0';
   }


   //==========================================================================
   //==[ HANDY MACROS ]========================================================
   //==========================================================================

   // Format an exception message with the function signature as an ANSI string
   #define EX_FN_MSG_A( ex, msg )   ( ( string(__FUNCTION__) + ": " + (msg) + "(" + (ex).what() + ")" ).c_str() )

   // Format an exception message with the function signature as an ANSI string (returns wide string)
   #define EX_FN_MSG_W( ex, msg )   ( A2Wide( EX_FN_MSG_A( (ex), (msg) ) ).c_str() )

   #define EX_FN_MSG( ex, msg )   EX_FN_MSG_W( (ex), (msg) )

   // A simple macro to return a string with the function signature, 
   // a ": " and a text message.  This is quite useful for detailed 
   // exception messages identifying the source of the exception.
   #define FN_MSG(msg)   ( ( string(__FUNCTION__) + ": " + (msg) ).c_str() )


   // Handy macro to output a debug message to the Debug
   // Logger in debug mode, but not in release mode.
   #ifdef _DEBUG
      #define TRACE(msg)   OutputDebugString( (msg)  )
   #else
      #define TRACE(msg)   ( (void) 0 )
   #endif

#define NUM_ELEMENTS(ary)   ( sizeof( (ary) )  /  sizeof( ( (ary)[0] ) ) )
}


