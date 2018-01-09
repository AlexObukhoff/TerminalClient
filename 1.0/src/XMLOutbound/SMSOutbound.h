//---------------------------------------------------------------------------

#ifndef SMSOutboundH
#define SMSOutboundH

#include "XMLOutbound.h"
#include "SMSSender.h"

//enum _OutboundType {cnPaymentOutbound, cnStatOutbound};

//---------------------------------------------------------------------------

class TSMSOutbound : public TOutbound
{
	virtual bool ProcessFile(AnsiString);
	TSMSSender *SMSSender;
public:
	TSMSOutbound(TWConfig*, TLogClass*, TFileMap*);
	virtual ~TSMSOutbound();
};

#endif
