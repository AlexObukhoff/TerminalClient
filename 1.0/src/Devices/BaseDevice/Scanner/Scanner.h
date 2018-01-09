//---------------------------------------------------------------------------

#ifndef ScanerH
#define ScanerH
//---------------------------------------------------------------------------
#include "DeviceClass.h"
#include "LogClass.h"
#include "ScannerThread.h"

// гуиды услуг
const std::string Moscow_JKH = "{2F4B68AB-3875-494d-BB9B-2C194F3A1D9D}"; // ЖКХ москва

class CScanner : public TDeviceClass
{
 public:
  CScanner(unsigned int comPortNum, std::string prefix, TLogClass* logClass = NULL);
  virtual ~CScanner() {};

  virtual void StartWaitData();
  virtual void StopWaitData();
  bool IsWaitingData;
  int Initialize();
};

class CQuantumScanner : public CScanner
{
 protected:
  void Start();
 public:
  CQuantumScanner(unsigned int comPortNum, TLogClass* logClass = NULL);
  ~CQuantumScanner();

  void StartDevice();
};

class CScannerDataParser
{
 private:
  static std::string moscow_jkh(std::string data);
 public:
  static std::string Parse(std::string serviceGuid, std::string data, BYTE type);
};
#endif
