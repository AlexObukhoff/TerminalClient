//---------------------------------------------------------------------------
#ifndef TCSPacketSenderH
#define TCSPacketSenderH
//#include <IdSMTP.hpp>
//#include <SevenZipVCL.hpp>
#include "LogClass.h"
#include "TWConfig.h"
#include "XMLPacket.h"
#include "TFileMap.h"
#include "PacketSender.h"
#include "TConnectThread.h"

//---------------------------------------------------------------------------

class TCSPacketSender : public TPacketSender
{
  bool Connected;
	short WCState;
	TDateTime LastPaymentReceived;
	TDateTime LastPaymentProcessed;
	int BillsCount;
	int BillsSum;
	int SIMBalance;
	int GSMSignalQuality;
	int ValidatorState;
	int PrinterState;
	int SW0;
	int SW1;
	int SW2;
	int GSMOperatorID;

	short tmpWCState;
	TDateTime tmpLastPaymentReceived;
	TDateTime tmpLastPaymentProcessed;
	int tmpBillsCount;
	int tmpBillsSum;
	int tmpSIMBalance;
	int tmpGSMSignalQuality;
	int tmpValidatorState;
	int tmpPrinterState;
	int tmpSW0;
	int tmpSW1;
	int tmpSW2;
	int tmpGSMOperatorID;

	bool SendIncassationMessage();
	AnsiString Connect(AnsiString URL, AnsiString MessageText);
	void PrepareAnswer(AnsiString&, TStringList*);
  AnsiString PrepareString(AnsiString);
	AnsiString GetAnswerValue(TStringList*, AnsiString);
	bool HasAnswerValue(TStringList *slSrc, AnsiString AName);
	int GetAnswerIntegerValue(TStringList*, AnsiString);
	bool SendPaymentCompleteMessage();
	bool SendFileViaEMailMessage();
	void ReceiveCommand(int Command, int CID, AnsiString  CmdParam);
  AnsiString GetErrorDescr(int);
public:
	TCSPacketSender(AnsiString, TWConfig*, TLogClass*, TFileMap*);
	virtual ~TCSPacketSender();
	virtual void StorePaymentInitTemp(TDateTime EventDateTime, int OperatorId, AnsiString InitialSessionNum, float Comission, const TNotesVector& _Notes, AnsiString Fields) {};
	virtual void StorePaymentInitComplete() {};
	virtual void StoreError(TDateTime EventDateTime, AnsiString Sender, int Type, AnsiString Description, int SubType, AnsiString SubDescription) {};
	virtual void StorePaymentStatusChange(TDateTime EventDateTime, AnsiString InitialSessionNum, int _Status, int _ErrorCode) {};
	virtual void StorePaymentComplete(TDateTime _EventDateTime, AnsiString _InitialSessionNum, AnsiString _LastSessionNum, int _ErrorCode, int _LastErrorCode, TDateTime _FirstTryDT, int _OperatorId, double _Sum, double _Comission, AnsiString _Fields);
	//virtual void StoreSendCommandProcessedTemp(int _CommandUID);
	//virtual void StoreCommandProcessed(int _CommandUID);
	virtual void StoreFileSend(AnsiString FileName) {};
//	virtual void StoreFilesSend(AnsiString, AnsiString);

	virtual bool SendHeartBeat(short, short, int, int, int, int, bool bFirstTry = false, int _ChequeCounter = -1);
	virtual AnsiString TestConnection(AnsiString URL="");
	virtual bool Process(bool bProcessMessages = false);
};
#endif
