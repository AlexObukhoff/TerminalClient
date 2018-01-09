#ifndef CRYPTLIBEXPORT_H_
#define CRYPTLIBEXPORT_H_

extern "C"
{
    __declspec(dllimport) int WINAPI CryptLib_Init(std::string& log);
    __declspec(dllimport) int WINAPI CryptLib_Close(std::string& log);
    __declspec(dllimport) int WINAPI CryptLib_AddKeys(const std::string& secretKey, const std::string& publicKey, const std::string& passPhrase, unsigned long serial, std::string& log);
    __declspec(dllimport) std::string WINAPI CryptLib_Sign(int keyNum, const std::string& src, std::string& log);
    __declspec(dllimport) std::string WINAPI CryptLib_SignD(int keyNum, const std::string& src, std::string& signature, std::string& log);
    __declspec(dllimport) std::string WINAPI CryptLib_Verify(int keyNum, const std::string& src, std::string& log);
    __declspec(dllimport) std::string WINAPI CryptLib_VerifyD(int keyNum, const std::string& src, std::string signature, std::string& log);
    __declspec(dllimport) std::string WINAPI CryptLib_Encrypt(int keyNum, const std::string& src, std::string& log);
    __declspec(dllimport) std::string WINAPI CryptLib_Decrypt(int keyNum, const std::string& src, std::string& log);
}

#endif /* CRYPTLIBEXPORT_H_ */
