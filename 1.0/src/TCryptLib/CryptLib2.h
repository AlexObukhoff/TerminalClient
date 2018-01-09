//---------------------------------------------------------------------------

#ifndef CryptLib2H
#define CryptLib2H

#include <string>
#include <boost/format.hpp>
#include "LogClass.h"
#include "CryptLib.h"

//---------------------------------------------------------------------------

class crypt
{
private:
    static TLogClass* m_pLog ;
    static bool m_isInit;
public:
    static void init(TLogClass* pLog)
    {
        std::string log = "";
        if(CryptLib_Init(log) == 0)
            m_isInit = true;

        if(pLog)
        {
            m_pLog = pLog;
            m_pLog->Write(log.c_str());
        }
        else
        {
            m_isInit = false;
        }
    }
    
    static void close()
    {
        if(m_isInit)
        {
            std::string log = "";
            CryptLib_Close(log);
            m_pLog->Write(log.c_str());
            m_isInit = false;
        }
    }

    static void addKeys(const std::string& secretKeyFile, const std::string& publicKeyFile, const std::string& passPhrase, unsigned long serial)
    {
        if(m_isInit)
        {
            std::string log = "";
            CryptLib_AddKeys(secretKeyFile, publicKeyFile, passPhrase, serial, log);
            m_pLog->Write((boost::format("TCryptLib.AddKeys: %1% %2%") % secretKeyFile % publicKeyFile).str().c_str());
            m_pLog->Write(log.c_str());
        }
    }

    static std::string sign(int keyNum, const std::string& src)
    {
        if(!m_isInit)
            return "";
            
        std::string log;
        std::string result = CryptLib_Sign(keyNum, src, log);
        m_pLog->Write(log.c_str());

        return result;
    }

    static std::string signD(int keyNum, const std::string& src, std::string& signature)
    {
        if(!m_isInit)
            return "";
            
        std::string log;
        std::string result = CryptLib_SignD(keyNum, src, signature, log);
        m_pLog->Write(log.c_str());

        return result;
    }

    static std::string verify(int keyNum, const std::string& src)
    {
        if(!m_isInit)
            return "";

        std::string log;
        std::string result = CryptLib_Verify(keyNum, src, log);
        m_pLog->Write(log.c_str());

        return result;
    }

    static std::string verifyD(int keyNum, const std::string& src, std::string& signature)
    {
        if(!m_isInit)
            return "";
            
        std::string log;
        std::string result = CryptLib_VerifyD(keyNum, src, signature, log);
        m_pLog->Write(log.c_str());

        return result;
    }

    static std::string decrypt(int keyNum, const std::string& src)
    {
        if(!m_isInit)
            return "";

        std::string log;
        std::string result = CryptLib_Decrypt(keyNum, src, log);
        m_pLog->Write(log.c_str());

        return result;
    }

    static std::string encrypt(int keyNum, const std::string& src)
    {
        if(!m_isInit)
            return "";

        std::string log;
        std::string result = CryptLib_Encrypt(keyNum, src, log);
        m_pLog->Write(log.c_str());

        return result;
    }
};

TLogClass* crypt::m_pLog = 0;
bool crypt::m_isInit = false;

#endif
