Windows Task Scheduler Sample App in C++ [Nov 12, 2017]

Copyright (c) 2017  Jeff Reeder
All Rights Reserved


*******************************************************************************
*** NOTE: When bringing this solution up for the first itme, make sure      ***
***       that Visual Studio is set to build x86 architecture, and that     ***
***       in Build | Configuration Manager that the selected configuration  ***
***       is configured to "Build".                                         ***
*******************************************************************************

This source demonstrates how to use DCOM from a C++ / Win32 application to
add an executable task to the Windows Task Scheduler that will be run after
a certain number of seconds.

Unlike a typical "sample app", this is not a console application.  It is a
dialog-based app in its own right.  

This code does not use MFC, .Net, ATL or anything special other than the
low-level Win32 SDK and C++.  It takes extensive advantage of C++11 features 
including the following:

   - Lambda Expressions
   - The auto keyword
   - Move semantics
   - Raw literal strings
   - Universal initialization syntax
   - nullptr instead of NULL
   ... And extensive amounts of major C++ object-oriented behavior

There is only global symbol: _tWinMain().

It utilizes several very useful design patterns:

   - Singleton (used for the GUI dialog box)
   - Uses a dialog box message pump that's part of the Dialog class, and
     not a global function
   - RAII (Resource Allocation As Initialization) [Used for COM object
     lifetime management] in an "exception safe" way.  See ComWrapper<>
     for the definition, and see its usage in ScheduleTask.cpp

This solution has been setup with (and tested for) the following build 
configurations:

   Configuration     Build Type     Characer Set      Processor
   ------------------------------------------------------------
   DebugU            Debug          Unicode           x86
   DebugA            Debug          ANSI              x86
   ReleaseU          Release        Unicode           x86
   ReleaseA          Release        ANSI              x86

The code is made up of two projects - the main TaskScheduler application, and
the UnitTesting project which can be run to perform exhaustive testing on the
primary application logic code:

   ComWrapper
   com_exception
   ScheduleTask

Currently, the high-level GUI code in App.cpp and Dialog.cpp does not have any
unit tests applied to it; only the core logic components are unit tested.

NOTE: ScheduleTask unit tests could be expanded to create artificial COM 
      operational failure conditions, but that would have considerably
      complicated the internal logic of the ScheduleTask class methods
      and I wanted to keep things as clear as possible.

===============================================================================
                                     # # #
===============================================================================
