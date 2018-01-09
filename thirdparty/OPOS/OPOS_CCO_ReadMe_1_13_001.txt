                         OPOS Common Control Objects
                                   1.13.001
                            Updated March 1, 2010

Contents of this file:
 * Features
 * Update history
 * Legal
 * Contact information

NOTE: The installation package does not install any system DLLs.
      It only puts files into the directories that you specify.

====================================================================
    Features
====================================================================

 * All 36 control objects of OPOS Release 1.13 are supported.
    Also includes an object that declares all of the OPOS constants.
    To use these constants...
     - Visual Basic: Select the menu item Project / References, and
        check "OPOS 1.13 Constants".
     - Visual C++: Add the line
            #import "path\Opos_Constants.dll" no_namespace
        to a source file, replacing path with the installed location
        of the file (or putting this location in the Include section
        under the Directories tab of the Tools / Options menu).
 * ATL-based, using dual interfaces so that the app can access them
    via IDispatch or COM interfaces (of the form IOPOSCashDrawer,
    etc).
 * Built using Microsoft Visual Studio 2008.
 * Backward compatible with all releases of service objects. This
    means that they check for older SOs, and return the proper
    errors to the app if it accesses unsupported properties or
    methods.
 * Event firing logic supports well-behaved service objects that
    fire events from the thread that created the control, plus other
    service objects that fire them from other threads.
 * Self-contained, requiring only standard OS DLLs. Specifically,
    they do not require MFC or ATL DLLs.
 * Only the Unicode versions are built and posted.
 * Source code for all control objects is available.
 * For future additions, it is easy to add new control objects or
    update old ones. A custom generator was developed that reads a
    data file for each control to be built. To add properties or
    methods, the procedure is update the data files, regenerate, and
    build the resulting projects.

====================================================================
    Update History
====================================================================

1.13.001 - March 1, 2010

 * Correction for backward compatibility.
    Was not registering all previous releases' interfaces.
    Caused marshaling problems when using across process boundaries,
    plus issues for some development environments.

1.13.000 - December 31, 2009

 * Add OPOS Release 1.13 API support.

1.12.000 - August 30, 2008

 * Build and post Unicode versions for Windows 2000 and later.
    Windows 95/98/ME are no longer supported.
    Windows NT 4 is no longer supported.
 * Convert from VC++ 6 to VS 2008.
    Use the standard ATL header files, except for atlcom.h, which was
      modified to support the legacy "Claim" and "Release" methods
      via IDispatch.
    Some preprocessor magic is used to work around issues with
      HardTotal's Create method and LineDisplay's CreateWindow method.
 * COM Interface IDs for versions prior to those used in OPOS 1.6 are no
    longer supported. Applications built against pre-1.6 CCOs that cached
    the old IIDs (instead of using the default IID) will either need to
    continue to use a previous CCO, or be rebuilt against these new ones.
 * Some significant changes were made to the code:
    - To conform to COM rules, previous release interfaces now include
        only their properties and methods, instead of all properties and
        methods. Previously, each interface change effectively extended
        all previous interfaces.
    - Variant variable handling has been reworked for safety and error
        detection: NULL pointers and memory failures are detected and
        cause an appropriate failure HRESULT to be returned, instead of
        assuming good pointers and successful allocation.
    - CCO initialization checks for critical failures and returns a
        failure HRESULT, instead of assuming success.
 * The library names have been stabilized to Opos***_CCO, where *** is
    the device name. Previously, the release number was included, which
    would cause development environments that cache the name to fail when
    loading new CCOs.
 * The names of the current interface continue to be IOPOS***, where ***
    is the device name. The names of previous interfaces now have the form
    IOPOS***_1_*, where *** is the device name and * is the release number.

1.11.001 - December 4, 2007

 * Correct handling of CO methods that are not supported by the SO's
    version. Previously returned OPOS_SUCCESS status, instead of
    OPOS_E_NOSERVICE.

1.11.000 - January 30, 2007

 * Add OPOS Release 1.11 API support.

1.10.002 - December 4, 2007

 * Correct handling of CO methods that are not supported by the SO's
    version. Previously returned OPOS_SUCCESS status, instead of
    OPOS_E_NOSERVICE.
 * Update MSR's WriteTracks method to conform to the updated 1.11 parameters.
    (Since this is a new method in 1.10, the committee decided to correct the
    parameter retroactively.)

1.10.001 - September 22, 2006

 * Update Biometrics to match upcoming 1.11 specification.

1.10.000 - March 15, 2006

 * Add OPOS Release 1.10 API support.
 * Add initialization of [out] BSTR* parameters before calling the service
    object. This was added for safety: A COM object is supposed to assume that
    the value at the incoming pointer is invalid, and just write over any
    value. This addition ensures that the string pointer is NULL just in case
    the service incorrectly treats a non-NULL string pointer as valid.

1.9.003 - August 20, 2008

 * Correct debug CCOs. The 1.9.002 CCOs inadvertantly required a tracing COM
    object that is not distributed.

1.9.002 - December 4, 2007

 * Correct handling of CO methods that are not supported by the SO's
    version. Previously returned OPOS_SUCCESS status, instead of
    OPOS_E_NOSERVICE.
 * Add Debug tracing of event delivery.

1.9.000 - April 29, 2005

 * Add OPOS Release 1.9 API support.

1.8.001 - October 26, 2004

 * Correct memory leak when getting string property.
 * Update several header files with constants that were missed in previous
    releases.
 * Add new device statistics header file, OposStat.h.
 * Minor enhancement to Debug tracing of strings.

1.8.000 - March 22, 2004

 * Add OPOS Release 1.8 API support.
 * Add significantly more tracing when Debug.

1.7.002 - December 2, 2003

 * Rebuild after applying updates to some ATL files per Microsoft
    Knowledge Base Articles: ATLBASE.H (190686, 305746), ATLCOM.H
    (266713), ATLCONV.H (266713).
 * Changed the legal license language.

1.7.001 - May 29, 2003

 * On Debug builds, add tracing of Open and Close to
    C:\OposCCO_@DeviceName@.log
 * Update OposPtr.h with constants added in OPOS Release 1.7.

1.7.000 - August 17, 2002

 * Add OPOS Release 1.7 API support.

1.6.001 - March 7, 2002

 * Mark CCOs as safe for initialization and scripting.
 * Correct FiscalPrinter method name from PrintRecItemVoidFuel to
    PrintRecItemFuelVoid.

1.6.000 - July 19, 2001

 * Add OPOS Release 1.6 API support.

1.5.103 - July 9, 2001

 * Updated EventClose() to only discard user messages.
    Was causing some CE apps to hang due to bogus WinCE messages.

1.5.102 - June 23, 2001

 * Enhance (correct) to properly support multiple SOs per device
    type.
 * Includes PINPad built with updated OposPpad.hi (which corrected
    Track4Data property index).

1.5.101 - January 2, 2001

 * OPOS Release 1.5 API support.
 * Added OPOS Constants DLL (Opos_Constants.dll).

1.5.100 Alpha - September 18, 2000

 * Initial pre-Beta version with OPOS Release 1.5 API support.

1.5.3 Beta  -  December 7, 1999
1.5.2 Beta  -  August 23, 1999
1.5.1 Beta  -  July 28, 1999
1.5.0 Beta  -  June 18, 1999

1.4.994 Beta  -  March 25, 1999
1.4.993 Beta  -  March 22, 1999
1.4.992 Beta  -  March 20, 1999
1.4.991 Limited Beta  -  February 7, 1999

====================================================================
    Legal
====================================================================

The following lines appear in many of the source code files from which the
binary Control Objects are built, in the version resource's "License" item,
and in the binary Control Object files (following the marker "~~License~~"):

Copyright (c) 1999-2009; RCS; A Division of NCR; Dayton, Ohio, USA.
Developed by Curtiss Monroe.

This software is provided "AS IS", without warranty of any kind, express or
implied. In no event shall NCR (including its subsidiaries, employees, and
contributors) be held liable for any direct, indirect, incidental, special,
or consequential damages arising out of the use of or inability to use this
software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it freely,
subject to the following restrictions:
    1. Redistributions of source code, whether original or altered, must
       retain this license.
    2. Altered versions of this software -- including, but not limited to,
       ports to new operating systems or environments, bug fix versions, and
       rebuilt versions -- must be plainly marked as such and must not be
       misrepresented as being the original source or binary. Such altered
       versions also must not be misrepresented as RCS or NCR software
       releases -- including, but not limited to, labeling of the altered
       versions with the names "OPOS Common Control Objects" or "OPOS CCOs"
       (or any variation thereof, including, but not limited to, different
       capitalizations).

====================================================================

Curtiss Monroe

RCS; A Division of NCR
Work e-mail:     curtiss.monroe@ncr.com
Personal e-mail: crmonroe@monroecs.com

Check the web site
    http://monroecs.com/oposccos.htm
for the latest Common Control Object information.

