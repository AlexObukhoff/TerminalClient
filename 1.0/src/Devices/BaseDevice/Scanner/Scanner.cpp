//---------------------------------------------------------------------------


#pragma hdrstop

#include "Scanner.h"
#include <boost/format.hpp>

//---------------------------------------------------------------------------

CScanner::CScanner(unsigned int comPortNum, std::string prefix, TLogClass* logClass)
: TDeviceClass(comPortNum, logClass, prefix.c_str())
{
 IsWaitingData = false;
}

void CScanner::StartWaitData()
{
 StartDevice();
}

void CScanner::StopWaitData()
{
 IsWaitingData = false;
 Stop();
}

int CScanner::Initialize()
{
 return 0;
}

// implementation of CQuantumScanner
CQuantumScanner::CQuantumScanner(unsigned int comPortNum, TLogClass* logClass)
: CScanner(comPortNum, "QuantumScanner3480", logClass)
{
}

CQuantumScanner::~CQuantumScanner()
{
 Stop();
}

void CQuantumScanner::StartDevice()
{
  StartPooling();
  IsWaitingData = true;
}

void CQuantumScanner::Start()
{
  DeviceState->State = Wait;
  DeviceState->Scanner = true;
  DeviceThread = new CScannerThread();
  DeviceThread->Log = Log;
  DeviceThread->DeviceState = DeviceState;
  DeviceThread->CommandParameters = CommandParameters;
  DeviceThread->SendType = SendType;
  DeviceThread->Port = Port;
  DeviceThread->Command = Command;
  DeviceThread->CommandSize = CommandSize;
  DeviceThread->Answer = Answer;
  DeviceThread->AnswerSize = AnswerSize;
  DeviceThread->data = data;
  DeviceThread->len_data = &len_data;

  DeviceThread->CommandCriticalSection = CommandCriticalSection;

  DeviceThread->DataLengthIndex = DataLengthIndex;
  DeviceThread->BeginByte = BeginByte;
  DeviceThread->LoggingErrors = LoggingErrors;
  DeviceThread->LastError = LastError;
  DeviceThread->EndByte = EndByte;
  DeviceThread->CRCLength = CRCLength;
  DeviceThread->DataLength = DataLength;

  DeviceThread->DeviceStarted = DeviceStarted;
  DeviceThread->DeviceStopped = DeviceStopped;
  DeviceThread->DevicePaused = DevicePaused;
  DeviceThread->DeviceStateChanged = DeviceStateChanged;
  DeviceThread->CommandStarted = CommandStarted;
  DeviceThread->CommandPaused = CommandPaused;
  DeviceThread->CommandFinished = CommandFinished;

  DeviceThread->ChangeEvent = ChangeEvent;
}

// статические методы разбора данных сканера
/*
 CScannerDataParser::monthNames[0] = "нет месяца";
 CScannerDataParser::monthNames[1] = "Январь";
 CScannerDataParser::monthNames[2] = "Февраль";
 CScannerDataParser::monthNames[3] = "Март";
 CScannerDataParser::monthNames[4] = "Апрель";
 CScannerDataParser::monthNames[5] = "Май";
 CScannerDataParser::monthNames[6] = "Июнь";
 CScannerDataParser::monthNames[7] = "Июль";
 CScannerDataParser::monthNames[8] = "Август";
 CScannerDataParser::monthNames[9] = "Сентябрь";
 CScannerDataParser::monthNames[10] = "Октябрь";
 CScannerDataParser::monthNames[11] = "Ноябрь";
 CScannerDataParser::monthNames[12] = "Декабрь";
//CScannerDataParser::CScannerDataParser()
//{
//}
*/
std::string CScannerDataParser::moscow_jkh(std::string data)
{
 std::string result = "";

 /*std::string monthNames[] =
     {
      "нет месяца",
      "Январь",
      "Февраль",
      "Март",
      "Апрель",
      "Май",
      "Июнь",
      "Июль",
      "Август",
      "Сентябрь",
      "Октябрь",
      "Ноябрь",
      "Декабрь"
     };
    */
 if(data.length() == 28)
 {
  std::string abCode, year, month;

//  unsigned int month;
  char* end_ptr;

  abCode = data.substr(0, 10);
  //month = monthNames[strtol(data.substr(10, 2).c_str(), &end_ptr, 10)];
  month = data.substr(10, 2);
  year = data.substr(12, 2);


  result = (boost::format("&field100=%1%&field101=%2%&field102=%3%") % abCode % /*CScannerDataParser::monthNames[month]*/month % year).str();
 }

 return result;
}

std::string CScannerDataParser::Parse(std::string serviceGuid, std::string data, BYTE type)
{
 std::string result = "";

 if(serviceGuid == Moscow_JKH && type == 0x6A)
  result = CScannerDataParser::moscow_jkh(data);

 return result;
}

#pragma package(smart_init)
