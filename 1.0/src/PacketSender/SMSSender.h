//---------------------------------------------------------------------------
#ifndef TSMSSenderH
#define TSMSSenderH
#include "LogClass.h"
#include "TWConfig.h"
#include "XMLPacket.h"
#include "TFileMap.h"
//#include <IdTCPClient.hpp>
#include "PacketSender.h"
#include "Modem.h"
#include "ModemSiemensMC35i.h"
//---------------------------------------------------------------------------

class TSMSSender : public TPacketSender
{
  //CModem *Modem;
	bool Connect(AnsiString URL, AnsiString MessageText);
public:
	TSMSSender(AnsiString, TWConfig*, TLogClass*, TFileMap*);
	virtual ~TSMSSender();
	virtual void StorePaymentInitTemp(TDateTime EventDateTime, int OperatorId, AnsiString InitialSessionNum, float Comission, const TNotesVector& _Notes, AnsiString Fields);
	virtual void StorePaymentInitComplete();
	virtual void StoreError(TDateTime EventDateTime, AnsiString Sender, int Type, AnsiString Description, int SubType, AnsiString SubDescription);
	void StoreMessage(AnsiString Message);
	void SendMessage(AnsiString Message);
	virtual void StorePaymentStatusChange(TDateTime EventDateTime, AnsiString InitialSessionNum, int _Status, int _ErrorCode);
	virtual void StorePaymentComplete(TDateTime _EventDateTime, AnsiString _InitialSessionNum, AnsiString _LastSessionNum, int _ErrorCode, int _LastErrorCode, TDateTime _FirstTryDT, int _OperatorId, double _Sum, double _Comission, AnsiString _Fields);
	virtual void StoreSendCommandProcessedTemp(int _CommandUID);
	virtual void StoreCommandProcessed(int _CommandUID);
	virtual void StoreFileSend(AnsiString FileName);
	virtual void StoreFilesSend(AnsiString, AnsiString);

	virtual bool SendHeartBeat(short, short, int, int, int, int, bool bFirstTry = false);
	virtual AnsiString TestConnection(AnsiString URL="");
	virtual bool Process(bool bProcessMessages = false);
};
#endif
