//---------------------------------------------------------------------------

#ifndef CommandReceiverH
#define CommandReceiverH
#include "LogClass.h"
#include "TWConfig.h"
#include "XMLPacket.h"
#include "TFileMap.h"
#include "THTTPGetFileThread.h"
#include <string>

//---------------------------------------------------------------------------

class TCommandReceiver
{
    TFileMap *FileMap;
    TLogClass *Log;
    TWConfig *Cfg;
    TXMLPacket *XMLP;
    bool InnerLog;
		AnsiString GetFileNames(AnsiString _Mask);
		bool CopyDir(AnsiString aFrom, AnsiString aTo, bool bClearDir);
		bool PacketLoadError;
		//THTTPGetFileThread *HTTPGetFile;
    int GetFile(AnsiString URL, AnsiString FileName, AnsiString CUID, TDateTime CheckDT);
		bool ListFile(AnsiString SourceFileName);
		void __fastcall SZListFileEvent(System::TObject* Sender, WideString Filename, unsigned Fileindex, unsigned FileSizeU, unsigned FileSizeP, unsigned Fileattr, unsigned Filecrc, WideString Filemethod, double FileTime);
public:
		TCommandReceiver(AnsiString, TWConfig*, TLogClass*, TFileMap*);
		~TCommandReceiver();
		void Clear();
		bool CommandAlreadyRegistered(int CommandUID);
		bool Store(AnsiString Ext = "pkt");
		void StoreCommand(int CommandId, int CommandUID);
		bool StoreRebootCommand(int CommandUID);
		void StoreInhibitSendingCommand(int CommandUID);
		void StoreSendFileCommand(int CommandUID, AnsiString FileName);
		void StoreReceiveFileCommand(int CommandUID, AnsiString FileName, int FileSize, void* Buffer);
		void StoreCancelPaymentCommand(int CommandUID, AnsiString SessionNumber);
		void StoreGetFileByMaskCommand(int, AnsiString);
		void StoreResurrectPaymentCommand(int CommandUID, AnsiString SessionNumber, AnsiString Parameters);
		bool StoreHTTPFileRequestCommand(int CommandUID, AnsiString _Mask, TDateTime _FileCheckDT = 0);
    bool StoreFullURLHTTPFileRequestCommand(int CommandUID, AnsiString _FileName, AnsiString _URL);
    bool StoreShutDownCommand(int CommandUID);
    bool StoreBlockCommand(int CommandUID);
    bool StoreUnblockCommand(int CommandUID);
    void StoreGetKeysCommand(int CommandUID, int KeysId, AnsiString Parameters);
		bool UpdateFile(AnsiString SourceFileName, AnsiString TargetFileName);
		bool StoreFile(AnsiString SourceFileName, AnsiString TargetFileName);
		void ProcessFile(AnsiString RealFileName, AnsiString FileName);

		int GetCommand();
		bool Process();
		bool Enabled;
		bool ExtractFile(AnsiString SourceFileName, AnsiString TargetDirName);
		bool DeleteDir(AnsiString DirName);
		//AnsiString ShowError(AnsiString Header);
		AnsiString GetDrive(void);
		TDateTime LastUpdatedDT;
		bool IsOnTime();
};

#endif
