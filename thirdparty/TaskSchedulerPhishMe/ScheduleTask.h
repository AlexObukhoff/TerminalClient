//=============================================================================
// File: ScheduleTask.h - Class definition for using the Task Scheduler
//
//    Copyright (c) 2017 Jeff Reeder
//    All Rights Reserved
//
// Right now, this class is used to register an EXE to be run after a specific
// number of seconds have transpired.
//
// NOTE: To be able to unit test the class' private support method, the 
//       "private:" clause is #ifdef'd out by the SCHEDULER_TESTING macro which
//       is only defined from within the UnitTesting project's build sandbox.  
//       That way those private methods are exposed to the unit testing code.
//=============================================================================

#pragma once

#include "ComWrapper.h"

namespace PhishMe
{
   // Schedule a task to be run in NN seconds.  The task is an executable file
   class ScheduleTask
   {
   public:

      ScheduleTask();
      ~ScheduleTask();

      // Schedule a command-line task
      void CreateScheduledTask( LPCTSTR            szTaskName,
                                LPCTSTR            szCmdLine,
                                LPCTSTR            szCmdArguments,
                                LPCTSTR            szWorkgDir,
                                long               lDelay,
                                LPCTSTR            szAuthorName        = _szAuthorName,
                                VARIANT_BOOL       bStartWhenAvailable = VARIANT_TRUE,
                                TASK_LOGON_TYPE    nLogonType          = TASK_LOGON_INTERACTIVE_TOKEN,
                                TASK_RUNLEVEL_TYPE nRunLevel           = TASK_RUNLEVEL_LUA,
                                LPCTSTR            szPrincipal         = _szPrincipal,
                                LPCTSTR            szTriggerName       = _szTriggerName ) const;

      // Delete a task with a given name
      void DeleteScheduledTask( LPCTSTR szTaskName );


#ifndef SCHEDULER_TESTING              // Make things public so we can test private methods
   private:
#endif

      static const LPCTSTR _szAuthorName;
      static const LPCTSTR _szPrincipal;
      static const LPCTSTR _szTriggerName;

      // Internal ComWrapper<> object types to use with the task scheduler COM object
      typedef ComWrapper<ITaskService>         TaskServiceObj;
      typedef ComWrapper<ITaskFolder>          TaskFolderObj;
      typedef ComWrapper<ITaskDefinition>      TaskDefObj;
      typedef ComWrapper<IRegistrationInfo>    RegInfoObj;
      typedef ComWrapper<IPrincipal>           PrincipalObj;
      typedef ComWrapper<ITaskSettings>        SettingsObj;
      typedef ComWrapper<ITriggerCollection>   TriggerCollObj;
      typedef ComWrapper<ITrigger>             TriggerObj;
      typedef ComWrapper<IRegistrationTrigger> RegTriggerObj;
      typedef ComWrapper<IActionCollection>    ActionCollObj;
      typedef ComWrapper<IAction>              ActionObj;
      typedef ComWrapper<IExecAction>          ExecActionObj;
      typedef ComWrapper<IRegisteredTask>      RegisterTaskObj;

      // Create the task scheduler object [RAII]
      TaskServiceObj _CreateTaskService() const;

      // Get the root task folder
      TaskFolderObj  _GetRootFolder( TaskServiceObj& service ) const;

      // Connect to the task service
      void _Connect( TaskServiceObj& service ) const;

      // Create the task builder object
      TaskDefObj _CreateTaskBuilder( TaskServiceObj& service ) const;

      // Set the author information
      void _SetAuthor( TaskDefObj& task,
                       LPCWSTR     wszAuthor ) const;

      // Create the task principle object
      void _SetPrincipalInfo( TaskDefObj&        task,
                              LPCWSTR            wszPrincipal,
                              TASK_LOGON_TYPE    nLogonType,
                              TASK_RUNLEVEL_TYPE nRunLevel ) const;

      // Define the settings for the task
      void _SetStartWhenAvailable( TaskDefObj&  task,
                                   VARIANT_BOOL bWhenAvailable ) const;

      // Set the task delay
      void _SetTriggerDelay( TaskDefObj& task,
                             LPCWSTR     wszTriggerName,
                             long        lDelay ) const;

      // Set the task's command line action
      void _SetCommandLineAction( TaskDefObj& task,
		  LPCWSTR     wszCmdLine,
		  LPCWSTR     wszArguments = nullptr,
		  LPCWSTR     wszWorkDir = nullptr) const;

      // Register the task with the task scheduler
      void _RegisterTask( TaskFolderObj& folder,
                          TaskDefObj&    task,
                          LPCWSTR        wszTaskName ) const;
   };
}

