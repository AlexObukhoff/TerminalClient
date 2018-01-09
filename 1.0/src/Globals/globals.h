#include <string>
#include <LogClass.h>
#include <boost/current_function.hpp>
#include <boost/throw_exception.hpp>

extern std::string FileVersion;
extern void ReadVersion();
bool killProgramm(char* programmName, TLogClass* Log = NULL);
HANDLE findProgramm(char* programmName);
void enableDebugPrivilege(bool fEnable = true);

extern void ExceptionFilter(const char* File, const char* Function, int Line, TLogClass* Log);
std::string GetCurrentDateTime(const char *Format);
