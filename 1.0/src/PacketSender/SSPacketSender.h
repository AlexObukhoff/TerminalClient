//---------------------------------------------------------------------------
#ifndef TSSPacketSenderH
#define TSSPacketSenderH
#include "LogClass.h"
#include "TWConfig.h"
#include "XMLPacket.h"
//#include "TSendThread.h"
#include "TFileMap.h"
#include "PacketSender.h"
//---------------------------------------------------------------------------

class TSSPacketSender : public TPacketSender
{
    bool HeartBeatOK;

public:
    TSSPacketSender(AnsiString, TWConfig*, TLogClass*, TFileMap*);
    virtual ~TSSPacketSender() {};
    virtual bool SendHeartBeat(short, short, int, int, int, int, bool, int);
    virtual AnsiString TestConnection(AnsiString URL="");
    virtual bool Process(bool bProcessMessages = false);
    };
#endif
