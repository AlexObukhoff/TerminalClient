//---------------------------------------------------------------------------
#include <classes.hpp>
#include <system.hpp>
#include <XMLDoc.hpp>
#include <SysUtils.hpp>
#pragma hdrstop
#include "SMSOutbound.h"
#include "globals.h"
#include "boost/format.hpp"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------

TSMSOutbound::TSMSOutbound(TWConfig *cfg, TLogClass *log, TFileMap *fileMap)
: TOutbound(cfg, log, fileMap, NULL)
{
    try	{
        Name="EMail";
		if(cfg) {
		    Dir = cfg->Dirs.SMSOutbound.c_str();
			DirTemp = cfg->Dirs.SMSOutboundTemp.c_str();
        }
		SMSSender=NULL;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, NULL);
    }
}
//---------------------------------------------------------------------------
TSMSOutbound::~TSMSOutbound()
{
    try	{
	    if(SMSSender) {
		    delete SMSSender;
		    SMSSender = NULL;
		}
	}
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, NULL);
    }
}
//---------------------------------------------------------------------------
bool TSMSOutbound::ProcessFile(AnsiString _fileName)
{
    bool bResult(false);
    try {
        try {
    		if(SMSSender) {
			    delete SMSSender;
				SMSSender=NULL;
            }
			m_pLog->Write((boost::format(" Processing file: %1%") % TruncateFileName(_fileName).c_str()).str().c_str());
			if(!terminating) {
                SMSSender = new TSMSSender(_fileName, m_pCfg, m_pLog, m_pFileMap);
                if(SMSSender) {
                    busy = true;
					bResult = SMSSender->Process();
                    busy = false;
                }
            }
        }
        catch(...)
        {
            ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, NULL);
        }
    }
    __finally {
		if(SMSSender) {
		    delete SMSSender;
            SMSSender = NULL;
        }
        return bResult;
    }
}
//---------------------------------------------------------------------------

