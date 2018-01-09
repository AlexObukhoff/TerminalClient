/////////////////////////////////////////////////////////////////////
//
// OposEvrw.h
//
//   Electronic Value Reader Writer header file for OPOS Applications.
//
// Modification history
// ------------------------------------------------------------------
// 2008-08-30 OPOS Release 1.12                                  CRM
//
/////////////////////////////////////////////////////////////////////

#if !defined(OPOSEVRW_H)
#define      OPOSEVRW_H


#include "Opos.h"


/////////////////////////////////////////////////////////////////////
// "CapCardSensor" Property Constants
/////////////////////////////////////////////////////////////////////

const LONG EVRW_CCS_ENTRY               = 0x00000001;
const LONG EVRW_CCS_DETECT              = 0x00000002;
const LONG EVRW_CCS_CAPTURE             = 0x00000004;


/////////////////////////////////////////////////////////////////////
// "CapDetectionControl" Property Constants
/////////////////////////////////////////////////////////////////////

const LONG EVRW_CDC_RWCONTROL           = 0x00000001;
const LONG EVRW_CDC_APPLICATIONCONTROL  = 0x00000002;


/////////////////////////////////////////////////////////////////////
// "DetectionStatus" Property Constants
/////////////////////////////////////////////////////////////////////

const LONG EVRW_DS_NOCARD               = 1;
const LONG EVRW_DS_DETECTED             = 2;
const LONG EVRW_DS_ENTERED              = 3;
const LONG EVRW_DS_CAPTURED             = 4;


/////////////////////////////////////////////////////////////////////
// "LogStatus" Property Constants
/////////////////////////////////////////////////////////////////////

const LONG EVRW_LS_OK                   = 1;
const LONG EVRW_LS_NEARFULL             = 2;
const LONG EVRW_LS_FULL                 = 3;


/////////////////////////////////////////////////////////////////////
// "AccessLog" Method: "Type" Parameter Constants
/////////////////////////////////////////////////////////////////////

const LONG EVRW_AL_REPORTING            = 1;
const LONG EVRW_AL_SETTLEMENT           = 2;


/////////////////////////////////////////////////////////////////////
// "BeginDetection" Method: "Type" Parameter Constants
/////////////////////////////////////////////////////////////////////

const LONG EVRW_BD_ANY                  = 1;
const LONG EVRW_BD_SPECIFIC             = 2;


/////////////////////////////////////////////////////////////////////
// "TransactionAccess" Method: "Control" Parameter Constants
/////////////////////////////////////////////////////////////////////

const LONG EVRW_TA_TRANSACTION          = 11;
const LONG EVRW_TA_NORMAL               = 12;


/////////////////////////////////////////////////////////////////////
// "StatusUpdateEvent" Event: "Data" Parameter Constants
/////////////////////////////////////////////////////////////////////

const LONG EVRW_SUE_LS_OK                  = 11;
const LONG EVRW_SUE_LS_NEARFULL            = 12;
const LONG EVRW_SUE_LS_FULL                = 13;
const LONG EVRW_SUE_DS_NOCARD              = 21;
const LONG EVRW_SUE_DS_DETECTED            = 22;
const LONG EVRW_SUE_DS_ENTERED             = 23;
const LONG EVRW_SUE_DS_CAPTURED            = 24;


/////////////////////////////////////////////////////////////////////
// "ResultCodeExtended" Property Constants
/////////////////////////////////////////////////////////////////////

const LONG OPOS_EVRW_NOCARD                = 201;
const LONG OPOS_EVRW_RELEASE               = 202;
const LONG OPOS_EVRW_CENTERERROR           = 203;
const LONG OPOS_EVRW_COMMANDERROR          = 204;
const LONG OPOS_EVRW_RESET                 = 205;
const LONG OPOS_EVRW_COMMUNICATIONERROR    = 206;
const LONG OPOS_EVRW_LOGOVERFLOW           = 207;
const LONG OPOS_EVRW_DAILYLOGOVERFLOW      = 208;
const LONG OPOS_EVRW_DEFICIENT             = 209;
const LONG OPOS_EVRW_OVERDEPOSIT           = 210;


#endif                  // !defined(OPOSEVRW_H)
