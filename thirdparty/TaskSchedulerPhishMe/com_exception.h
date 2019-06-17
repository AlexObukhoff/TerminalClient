//=============================================================================
// File: com_exception.h - COM-specific exception class definition
//
//    Copyright (c) 2017 Jeff Reeder
//    All Rights Reserved
//
// This is a specialized exception class that allows you to associate an 
// HRESULT with the exception, and if provided will generate a what() message
// that includes the HRESULT along with the message in one tidy operation.  
// Plus, using this exception class lets us perform specialized catch 
// operations on COM errors.
//=============================================================================

#pragma once

namespace PhishMe
{
   // The com_exception class is a simple wrapper around the base exception
   // class, but allows you optional provide an HRESULT.  If present, then
   // the what() method creates formatted COM-style error with the HRESULT.
   class com_exception  :  public std::exception
   {
   public:

      // Create a COM exception object with just a message (no HRESULT)
      com_exception( const char* szMessage );

      // Create a COM exception object with just a message with an HRESULT
      com_exception( const char* szMessage,
                     HRESULT     hResult );

      // Get the exception message
      virtual const char* what() const;


   private:

      std::string _message;                     // The formatted what() message

      com_exception() {}                        // Require use of custom constructor
   };
}

