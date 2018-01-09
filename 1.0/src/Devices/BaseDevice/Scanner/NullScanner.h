//---------------------------------------------------------------------------

#ifndef NullScannerH
#define NullScannerH

#include "Scanner.h"
//---------------------------------------------------------------------------
class CNullScanner : public CScanner
{
 protected:
  void Start();
  void Stop();
 public:
  CNullScanner(TLogClass* logClass = NULL);
  ~CNullScanner();

  void StartDevice();
};
#endif
