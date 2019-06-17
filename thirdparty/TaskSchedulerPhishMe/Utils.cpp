//=============================================================================
// File: Utils.cpp - Helper functions for ANSI <-> Unicode conversion
//
//    Copyright (c) 2017 Jeff Reeder
//    All Rights Reserved
//
// These functions are used extensively by macros defined in Utils.h
//=============================================================================

#include "stdafx.h"

using namespace std;
using namespace PhishMe;


// Convert a std::wstring to std::string
string PhishMe::WideToAnsi( const wstring sWide )
{
   size_t nLen   =  sWide.length();
   char*  szBuf  =  new char[ nLen + 1 ];

   szBuf[nLen] = '\0';

   WideCharToMultiByte( CP_ACP, 
                        0, 
                        sWide.c_str(), 
                        -1, 
                        szBuf, 
                        static_cast<int>(nLen), 
                        NULL, 
                        NULL );

    string sAnsi(szBuf);

    delete[] szBuf;

    return sAnsi;
}


// Convert a std::string to std::wstring
wstring PhishMe::AnsiToWide( const string sAnsi )
{
   size_t   nLen   =  sAnsi.length();
   wchar_t* szBuf  =  new wchar_t[ nLen + 1 ];

   szBuf[nLen] = L'\0';

   MultiByteToWideChar( CP_ACP, 
                        0, 
                        sAnsi.c_str(), 
                        -1, 
                        szBuf, 
                        static_cast<int>(nLen) );

    wstring sWide(szBuf);

    delete[] szBuf;

    return sWide;
}
