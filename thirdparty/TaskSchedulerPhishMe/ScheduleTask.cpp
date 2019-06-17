//=============================================================================
// File: ScheduleTask.cpp - Class implementation for using the Task Scheduler
//
//    Copyright (c) 2017 Jeff Reeder
//    All Rights Reserved
//
// The methods in this class perform exhaustive parameter validation, and full
// testing on all COM errors with one or two explicit exceptions.
//=============================================================================

#include "stdafx.h"

#include "ScheduleTask.h"
#include "com_exception.h"
#include "ComWrapper.h"


using namespace std;
using namespace PhishMe;


const LPCTSTR ScheduleTask::_szAuthorName   =  _T("CyberPlat");
const LPCTSTR ScheduleTask::_szPrincipal    =  _T("PrincipalTask");
const LPCTSTR ScheduleTask::_szTriggerName  =  _T("TaskTrigger");

ScheduleTask::ScheduleTask()
{
}


ScheduleTask::~ScheduleTask()
{
}


// Schedule a command-line task
void ScheduleTask::CreateScheduledTask(
	LPCTSTR            szTaskName,
	LPCTSTR            szCmdLine,
	LPCTSTR            szCmdArguments,
	LPCTSTR            szWorkDir,
	long               lDelay,
	LPCTSTR            szAuthorName        /* = _szAuthorName */,
	VARIANT_BOOL       bStartWhenAvailable /* = VARIANT_TRUE */,
	TASK_LOGON_TYPE    nLogonType          /* = TASK_LOGON_INTERACTIVE_TOKEN */,
	TASK_RUNLEVEL_TYPE nRunLevel           /* = TASK_RUNLEVEL_LUA */,
	LPCTSTR            szPrincipal         /* = _szPrincipal */,
	LPCTSTR            szTriggerName       /* = _szTriggerName */) const
{
	if (!IsStringNonEmpty(szTaskName))   throw invalid_argument(FN_MSG("Invalid task name string"));
	if (!IsStringNonEmpty(szCmdLine))   throw invalid_argument(FN_MSG("Invalid command line string"));
	if (!IsStringNonEmpty(szAuthorName))   throw invalid_argument(FN_MSG("Invalid author name string"));
	if (!IsStringNonEmpty(szPrincipal))   throw invalid_argument(FN_MSG("Invalid principal string"));
	if (!IsStringNonEmpty(szTriggerName))   throw invalid_argument(FN_MSG("Invalid trigger name string"));
	if (lDelay <= 0)   throw invalid_argument(FN_MSG("Delay cannot be zero or negative"));

	if (bStartWhenAvailable != VARIANT_TRUE  &&
		bStartWhenAvailable != VARIANT_FALSE)
	{
		throw invalid_argument(FN_MSG("Invalid bStartWhenAvailable"));
	}

	if (nLogonType < TASK_LOGON_NONE ||
		nLogonType > TASK_LOGON_INTERACTIVE_TOKEN_OR_PASSWORD)
	{
		throw invalid_argument(FN_MSG("nLogonType is invalid"));
	}

	if (nRunLevel < TASK_RUNLEVEL_LUA ||
		nRunLevel > TASK_RUNLEVEL_HIGHEST)
	{
		throw invalid_argument(FN_MSG("nRunLevel is invalid"));
	}

	TaskServiceObj taskService = _CreateTaskService();        // Create the task service

	_Connect(taskService);                                      // Connect to the task service

	TaskFolderObj rootFolder = _GetRootFolder(taskService);   // Get the root folder

	// If the same task exists, remove it - we ignore the return value,
	// because it can legitimately fail if the task doesn't exist
	HRESULT hr = rootFolder->DeleteTask(_bstr_t(T2Wide(szTaskName).c_str()),
		0);

	// Create the task builder object to create task
	TaskDefObj task = _CreateTaskBuilder(taskService);

	// Set the task's author name
	_SetAuthor(task,
		T2Wide(szAuthorName).c_str());

	// Create the task principal info
	_SetPrincipalInfo(task,
		T2Wide(szPrincipal).c_str(),
		nLogonType,
		nRunLevel);

	_SetStartWhenAvailable(task, bStartWhenAvailable);
	_SetTriggerDelay(task,
		T2Wide(szTriggerName).c_str(),
		lDelay);

	// Set the command line for this task.  Unicode vs MBCS needs to be handled differently
	_SetCommandLineAction(task,
		T2Wide(szCmdLine).c_str(),
		szCmdArguments ? T2Wide(szCmdArguments).c_str() : nullptr,
		szWorkDir ? T2Wide(szWorkDir).c_str() : nullptr);

	// Now register the task
	_RegisterTask(rootFolder,
		task,
		T2Wide(szTaskName).c_str());
}


// Delete a task with a given name
void ScheduleTask::DeleteScheduledTask( LPCTSTR szTaskName )
{
   if ( ! IsStringNonEmpty(szTaskName) )  throw invalid_argument( FN_MSG( "Invalid task name string" ) );

   TaskServiceObj taskService  =  _CreateTaskService();        // Create the task service

   _Connect(taskService);                                      // Connect to the task service

   TaskFolderObj rootFolder  =  _GetRootFolder(taskService);   // Get the root folder

   // If the same task exists, remove it - we ignore the return value,
   // because it can legitimately fail if the task doesn't exist

   rootFolder->DeleteTask( _bstr_t( T2Wide(szTaskName).c_str() ),  
                           0 );
}


// Create the task scheduler object [RAII]
ScheduleTask::TaskServiceObj
ScheduleTask::_CreateTaskService() const
{
   ITaskService* pService  =  nullptr;
   HRESULT       hr        =  CoCreateInstance( CLSID_TaskScheduler,
                                                nullptr,
                                                CLSCTX_INPROC_SERVER,
                                                IID_ITaskService,
                                                (void**) &pService );

   if ( FAILED(hr)          )   throw com_exception( FN_MSG( "Failed to create an instance of ITaskService" ),  hr );
   if ( pService == nullptr )   throw runtime_error( FN_MSG( "CoCreateInstance didn't return a valid pService" )   );

   return TaskServiceObj(pService);
}


// Connect to the task service
void ScheduleTask::_Connect( TaskServiceObj& service ) const
{
   if ( ! service.IsValid() )   throw invalid_argument( FN_MSG( "Service object isn't valid" ) );

   HRESULT hr = service->Connect( _variant_t(),
                                  _variant_t(),
                                  _variant_t(),
                                  _variant_t() );

   if ( FAILED(hr) )   throw com_exception( FN_MSG( "ITaskService::Connect failed: %x" ),  hr );
}


// Get the root task folder
ScheduleTask::TaskFolderObj 
ScheduleTask::_GetRootFolder( TaskServiceObj& service ) const
{
   if ( ! service.IsValid() )   throw invalid_argument( FN_MSG( "Service object isn't valid" ) );

   ITaskFolder* pFolder  =  nullptr;
   HRESULT      hr       =  service->GetFolder( _bstr_t( L"\\" ),
                                                & pFolder );

   if ( FAILED(hr)         )   throw com_exception( FN_MSG( "Failed to get task foldercreate an instance of ITaskService" ),  hr );
   if ( pFolder == nullptr )   throw runtime_error( FN_MSG( "GetFolder() didn't return a valid pFolder"                   )      );

   return TaskFolderObj(pFolder);
}


// Create the task builder object
ScheduleTask::TaskDefObj 
ScheduleTask::_CreateTaskBuilder( TaskServiceObj& service ) const
{
   if ( ! service.IsValid() )   throw invalid_argument( FN_MSG( "Service object isn't valid" ) );

   ITaskDefinition* pTask  =  nullptr;
   HRESULT          hr     =  service->NewTask( 0, &pTask );

   if ( FAILED(hr)       )   throw com_exception( FN_MSG( "Failed to create a task definition"    ),  hr );
   if ( pTask == nullptr )   throw runtime_error( FN_MSG( "NewTask() didn't return a valid pTask" )      );
   
   return TaskDefObj(pTask);
}


// Set the author information
void ScheduleTask::_SetAuthor( TaskDefObj& task,
                               LPCWSTR     wszAuthor ) const
{
   if ( ! task.IsValid()              )   throw invalid_argument( FN_MSG( "Task object isn't valid"  ) );
   if ( ! IsStringNonEmpty(wszAuthor) )   throw invalid_argument( FN_MSG( "Invalid author parameter" ) );

   IRegistrationInfo* pRegInfo  =  nullptr;
   HRESULT            hr        =  task->get_RegistrationInfo( &pRegInfo );

   if ( FAILED(hr)          )   throw com_exception( FN_MSG( "Cannot get identification pointer" ),  hr                );
   if ( pRegInfo == nullptr )   throw runtime_error( FN_MSG( "get_RegistrationInfo() didn't return a valid pRegInfo" ) );

   RegInfoObj regInfo(pRegInfo);

   hr  =  regInfo->put_Author( _bstr_t(wszAuthor) );
   if ( FAILED(hr) )   throw com_exception( FN_MSG( "Cannot set author" ),  hr );
}


// Create the task principle object
void ScheduleTask::_SetPrincipalInfo( TaskDefObj&        task,
                                      LPCWSTR            wszPrincipal,
                                      TASK_LOGON_TYPE    nLogonType,
                                      TASK_RUNLEVEL_TYPE nRunLevel ) const
{
   if ( ! task.IsValid()                 )   throw invalid_argument( FN_MSG( "Task object isn't valid"     ) );
   if ( ! IsStringNonEmpty(wszPrincipal) )   throw invalid_argument( FN_MSG( "Invalid principal parameter" ) );

   if ( nLogonType < TASK_LOGON_NONE  ||
        nLogonType > TASK_LOGON_INTERACTIVE_TOKEN_OR_PASSWORD )
   {
      throw invalid_argument( FN_MSG( "nLogonType is invalid" ) );
   }

   if ( nRunLevel < TASK_RUNLEVEL_LUA  ||
        nRunLevel > TASK_RUNLEVEL_HIGHEST )
   {
      throw invalid_argument( FN_MSG( "nRunLevel is invalid" ) );
   }

   IPrincipal* pPrincipal  =  nullptr;
   HRESULT     hr          =  task->get_Principal( &pPrincipal );

   if ( FAILED(hr)            )   throw com_exception( FN_MSG( "Can't create principal for task" ),  hr             );
   if ( pPrincipal == nullptr )   throw runtime_error( FN_MSG( "get_Principal() didn't return a valid pPrincipal" ) );

   PrincipalObj principal(pPrincipal);

   // Set principal ID
   hr  =  principal->put_Id( _bstr_t(wszPrincipal) );
   if ( FAILED(hr) )   throw com_exception( FN_MSG( "Can't set principal name for task" ), hr );

   // Set the logon type
   hr  =  principal->put_LogonType(nLogonType);
   if ( FAILED(hr) )   throw com_exception( FN_MSG( "Could not set the principal's logon type" ),  hr );

   //  Run the task with the least privileges (LUA) 
   hr = principal->put_RunLevel(nRunLevel);
   if ( FAILED(hr) )   throw com_exception( FN_MSG( "Cannot put principal run level" ),  hr );
}


// Define the settings for the task
void ScheduleTask::_SetStartWhenAvailable( TaskDefObj&  task,
                                           VARIANT_BOOL bWhenAvailable ) const
{
   if ( ! task.IsValid() )   throw invalid_argument( FN_MSG( "Task object isn't valid" ) );

   if ( bWhenAvailable != VARIANT_TRUE  &&
        bWhenAvailable != VARIANT_FALSE )
   {
      throw invalid_argument( FN_MSG( "Invalid bWhenAvailable" ) );
   }

   ITaskSettings* pSettings  =  nullptr;
   HRESULT        hr         =  task->get_Settings( &pSettings );

   if ( FAILED(hr)           )   throw com_exception( FN_MSG( "Cannot get settings pointer" ),  hr               );
   if ( pSettings == nullptr )   throw runtime_error( FN_MSG( "get_Settings() didn't return a valid pSettings" ) );

   SettingsObj settings(pSettings);

   // Set the start when available to TRUE
   hr  =  settings->put_StartWhenAvailable(bWhenAvailable);
   if ( FAILED(hr) )   throw com_exception( FN_MSG( "Can't set task start when available" ),  hr );
}


// Set the task delay
void ScheduleTask::_SetTriggerDelay( TaskDefObj& task,
                                     LPCWSTR     wszTriggerName,
                                     long        lDelay ) const
{
   if ( ! task.IsValid()                   )   throw invalid_argument( FN_MSG( "Task object isn't valid"                  ) );
   if ( ! IsStringNonEmpty(wszTriggerName) )   throw invalid_argument( FN_MSG( "Invalid trigger name principal parameter" ) );

   if ( lDelay <= 0      )   throw invalid_argument( FN_MSG( "The task delay must be greater than 0" ) );

   //  Get the trigger collection to insert the registration trigger.
   ITriggerCollection* pTriggerCollection  =  nullptr;
   HRESULT             hr                  =  task->get_Triggers( &pTriggerCollection );

   if ( FAILED(hr)                    )   throw com_exception( FN_MSG( "Cannot get trigger collection" ),  hr                      );
   if ( pTriggerCollection == nullptr )   throw runtime_error( FN_MSG( "get_Triggers() didn't return a valid pTriggerCollection" ) );

   TriggerCollObj triggerColl(pTriggerCollection);

   //  Add the registration trigger to the task.
   ITrigger* pTrigger  =  nullptr;
   hr  =  triggerColl->Create( TASK_TRIGGER_REGISTRATION,  & pTrigger );

   if ( FAILED(hr)          )   throw com_exception( FN_MSG( "Cannot create a registration trigger" ),  hr );
   if ( pTrigger == nullptr )   throw runtime_error( FN_MSG( "Create() didn't return a valid pTrigger" )   );

   TriggerObj trigger(pTrigger);

   IRegistrationTrigger* pRegistrationTrigger  =  nullptr;
   hr  =  pTrigger->QueryInterface( IID_IRegistrationTrigger, 
                                    (void**) &pRegistrationTrigger );

   if ( FAILED(hr)                      )   throw com_exception( FN_MSG( "QueryInterface call failed on IRegistrationTrigger" ),  hr     );
   if ( pRegistrationTrigger == nullptr )   throw runtime_error( FN_MSG( "QueryInterface() didn't return a valid pRegistrationTrigger" ) );

   RegTriggerObj regTrigger(pRegistrationTrigger);

   // Set the ID
   hr  =  regTrigger->put_Id( _bstr_t(wszTriggerName) );
   if ( FAILED(hr) )   throw com_exception( FN_MSG( "Cannot set trigger name" ),  hr );

   // Construct our delay string value in the format "PTxxxS"
   wstring sDelay  =  wstring( L"PT" )  +  to_wstring((long long)lDelay)  +  L"S";

   hr  =  regTrigger->put_Delay( _bstr_t( sDelay.c_str() ) );
   if ( FAILED(hr) )   throw com_exception( FN_MSG( "Cannot set registration trigger delay" ),  hr );
}


// Set the task's command line action
void ScheduleTask::_SetCommandLineAction( TaskDefObj& task,
	LPCWSTR     wszCmdLine,
	LPCWSTR     wszArguments /*= nullptr*/,
	LPCWSTR     wszWorkDir /*= nullptr*/) const
{
   if ( ! task.IsValid()               )   throw invalid_argument( "Task object isn't valid"                                                 );
   if ( ! IsStringNonEmpty(wszCmdLine) )   throw invalid_argument( "Invalid command line parameter in ScheduleTask::_SetCommandLineAction()" );

   // Get the action collection
   IActionCollection* pActionCollection  =  nullptr;
   HRESULT            hr                 =  task->get_Actions( &pActionCollection );

   if ( FAILED(hr)                   )   throw com_exception( FN_MSG( "Cannot get Task collection pointer" ),  hr               );
   if ( pActionCollection == nullptr )   throw runtime_error( FN_MSG( "get_Actions() didn't return a valid pActionCollection" ) );

   ActionCollObj actionColl(pActionCollection);

   //  Create the action, specifying that it is an executable action.
   IAction* pAction  =  nullptr;

   hr  =  pActionCollection->Create( TASK_ACTION_EXEC, &pAction );

   if ( FAILED(hr)         )   throw com_exception( FN_MSG( "Cannot create action" ),  hr              );
   if ( pAction == nullptr )   throw runtime_error( FN_MSG( "Create() didn't return a valid pAction" ) );

   ActionObj action(pAction);

   //  Get the execute object
   IExecAction* pExecAction  =  nullptr;

   hr  =  pAction->QueryInterface( IID_IExecAction, 
                                   (void**) &pExecAction );

   if ( FAILED(hr)             )   throw com_exception( FN_MSG( "QueryInterface call failed for IExecAction" ),  hr    );
   if ( pExecAction == nullptr )   throw runtime_error( FN_MSG( "QueryInterface() didn't return a valid pExecAction" ) );
   
   ExecActionObj execAction(pExecAction);

   //  Set the path of the executable to notepad.exe.
   hr  =  pExecAction->put_Path( _bstr_t(wszCmdLine) );
   if ( FAILED(hr) )   throw com_exception( FN_MSG( "Cannot put the action executable path" ),  hr );

   if (wszArguments)
   {
	   hr = pExecAction->put_Arguments(_bstr_t(wszArguments));
	   if (FAILED(hr))   throw com_exception(FN_MSG("Cannot put the action executable arguments"), hr);
   }

   if (wszWorkDir)
   {
	   hr = pExecAction->put_WorkingDirectory(_bstr_t(wszWorkDir));
	   if (FAILED(hr))   throw com_exception(FN_MSG("Cannot put the action working directory"), hr);
   }
}


// Register the task with the task scheduler
void ScheduleTask::_RegisterTask( TaskFolderObj& folder,
                                  TaskDefObj&    task,
                                  LPCWSTR        wszTaskName ) const
{
   if ( ! folder.IsValid()              )   throw invalid_argument( FN_MSG( "Folder object isn't valid"      ) );
   if ( ! task.IsValid()                )   throw invalid_argument( FN_MSG( "Task object isn't valid"        ) );
   if ( ! IsStringNonEmpty(wszTaskName) )   throw invalid_argument( FN_MSG( "Invalid task name parameter"    ) );
   if ( *wszTaskName == L'\0'           )   throw invalid_argument( FN_MSG( "Blank task name line parameter" ) );

   IRegisteredTask* pRegisteredTask  =  nullptr;         // Don't need RegisterTaskObj in this case

   HRESULT hr  =  folder->RegisterTaskDefinition( _bstr_t(wszTaskName),
                                                  task.GetObj(),
                                                  TASK_CREATE_OR_UPDATE,
                                                  _variant_t(),
                                                  _variant_t(),
                                                  TASK_LOGON_INTERACTIVE_TOKEN,
                                                  _variant_t( L"" ),
                                                  &pRegisteredTask );

   if ( FAILED(hr)                 )   throw com_exception( FN_MSG( "Error saving the Task" ),  hr                                     );
   if ( pRegisteredTask == nullptr )   throw runtime_error( FN_MSG( "RegisterTaskDefinition() didn't return a valid pRegisteredTask" ) );

   pRegisteredTask->Release();
}
