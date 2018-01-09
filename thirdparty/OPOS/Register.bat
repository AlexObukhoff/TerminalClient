@echo off
echo **
echo ** Register or Unregister OPOS Common Control Objects. Formats:
echo ** Register        Register with confirmation for each file.
echo ** Register /s     Register silently.
echo ** Register /u     Unregister with confirmation for each file.
echo ** Register /u /s  Unregister silently.
echo **
echo --} Press Control-Break to exit, or
pause

@echo on

regsvr32 %1 %2 CommonCO\Opos_Constants.dll
regsvr32 %1 %2 CommonCO\OPOSBelt.ocx
regsvr32 %1 %2 CommonCO\OPOSBillAcceptor.ocx
regsvr32 %1 %2 CommonCO\OPOSBillDispenser.ocx
regsvr32 %1 %2 CommonCO\OPOSBiometrics.ocx
regsvr32 %1 %2 CommonCO\OPOSBumpBar.ocx
regsvr32 %1 %2 CommonCO\OPOSCashChanger.ocx
regsvr32 %1 %2 CommonCO\OPOSCashDrawer.ocx
regsvr32 %1 %2 CommonCO\OPOSCAT.ocx
regsvr32 %1 %2 CommonCO\OPOSCheckScanner.ocx
regsvr32 %1 %2 CommonCO\OPOSCoinAcceptor.ocx
regsvr32 %1 %2 CommonCO\OPOSCoinDispenser.ocx
regsvr32 %1 %2 CommonCO\OPOSElectronicJournal.ocx
regsvr32 %1 %2 CommonCO\OPOSElectronicValueRW.ocx
regsvr32 %1 %2 CommonCO\OPOSFiscalPrinter.ocx
regsvr32 %1 %2 CommonCO\OPOSGate.ocx
regsvr32 %1 %2 CommonCO\OPOSImageScanner.ocx
regsvr32 %1 %2 CommonCO\OPOSItemDispenser.ocx
regsvr32 %1 %2 CommonCO\OPOSKeylock.ocx
regsvr32 %1 %2 CommonCO\OPOSLights.ocx
regsvr32 %1 %2 CommonCO\OPOSLineDisplay.ocx
regsvr32 %1 %2 CommonCO\OPOSMICR.ocx
regsvr32 %1 %2 CommonCO\OPOSMotionSensor.ocx
regsvr32 %1 %2 CommonCO\OPOSMSR.ocx
regsvr32 %1 %2 CommonCO\OPOSPINPad.ocx
regsvr32 %1 %2 CommonCO\OPOSPointCardRW.ocx
regsvr32 %1 %2 CommonCO\OPOSPOSKeyboard.ocx
regsvr32 %1 %2 CommonCO\OPOSPOSPower.ocx
regsvr32 %1 %2 CommonCO\OPOSPOSPrinter.ocx
regsvr32 %1 %2 CommonCO\OPOSRemoteOrderDisplay.ocx
regsvr32 %1 %2 CommonCO\OPOSRFIDScanner.ocx
regsvr32 %1 %2 CommonCO\OPOSScale.ocx
regsvr32 %1 %2 CommonCO\OPOSScanner.ocx
regsvr32 %1 %2 CommonCO\OPOSSigCap.ocx
regsvr32 %1 %2 CommonCO\OPOSSmartCardRW.ocx
regsvr32 %1 %2 CommonCO\OPOSToneIndicator.ocx
regsvr32 %1 %2 CommonCO\OPOSTotals.ocx

rem ** End
