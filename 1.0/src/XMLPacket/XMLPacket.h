//---------------------------------------------------------------------------

#ifndef XMLPacketH
#define XMLPacketH

#include "TWConfig.h"
#include "TNote.h"

const int cnNoType      = 0;
const int cnPaymentInit = 1;
const int cnIncassation = 2;
const int cnHeartBeat   = 3;
const int cnError       = 4;
const int cnPaymentStatusChange = 5;
const int cnPaymentComplete = 6;
const int cnCommandProcessed = 7;
const int cnFileSend = 8;
const int cnFileSendNew = 9;
const int cnTestConnection = 666;
const int cnHandshake = 1024;

const int cnCmdNone           	= 0;
const int cnCmdReboot         	= 1;
const int cnCmdReceiveFileOld 	= 2;
const int cnCmdSendConfigOld  	= 3;
const int cnCmdCancelPayment  	= 4;
const int cnCmdSendFile       	= 5;
const int cnCmdInhibitPktSend 	= 6;
const int cnCmdReceiveFile    	= 7;
const int cnCmdResurrectPayment = 8;
const int cnHTTPFileRequest			= 9;
const int cnFullURLHTTPFileRequest = 10;
const int cnCmdShutDown         = 13;
const int cnCmdBlock            = 14;
const int cnCmdUnblock          = 15;
const int cnCmdGetKeys          = 18;

enum _PacketType {cnPayment, cnPaymentTemp, cnStat, cnStatTemp};

typedef struct Params
{
  AnsiString Name;
  AnsiString Value;
} TPParams;
typedef TPParams* PParams;
//---------------------------------------------------------------------------

class TXMLPacket
{
private:
    bool m_xmlParseError;
protected:
    TWConfig* cfg;
    TLogClass* Log;
    bool InnerLog;
    std::string OutboundDir;
    std::string TempOutboundDir;
    //bool InnerNotes;
    void StoreNotes(const TNotesVector& _Notes, xmlGuard <_di_IXMLNode> &xmlParentNode);
    void GetNotes(TNotesVector& _Notes, xmlGuard <_di_IXMLNodeList> &xmlNodeList);
    TFileStream *PktFile;
    virtual bool GetDataFromXML(xmlGuard <_di_IXMLDocument> &XMLDoc);
public:
    void AddNote(int, double);
    void AddNotes(const TNotesVector& _Notes);
    std::vector<TNote> vNotes;
    TList *Params;
    //TStringList *Parameters;
    std::string PacketFileName;
    TXMLPacket(TWConfig*, TLogClass*);
    virtual ~TXMLPacket();
    virtual void Clear();
    void AddParam(AnsiString,AnsiString);
    void AddParamI(AnsiString,int);
    void AddParam(AnsiString,float);
    void SetParamValue(AnsiString,AnsiString);
    void SetParamValue(AnsiString,float);
    void DeleteParam(AnsiString);
    AnsiString GetParamValue(AnsiString);
    AnsiString GetParamName(int);
    AnsiString GetParamValue(int);
    bool IsExist(AnsiString NodeName,xmlGuard <_di_IXMLNodeList> &xmlNodeList);
    AnsiString GetString(AnsiString, xmlGuard <_di_IXMLNodeList> &xmlNodeList);
    TDateTime GetDateTime(AnsiString, xmlGuard <_di_IXMLNodeList> &xmlNodeList);
    void SetValue(AnsiString, AnsiString, xmlGuard <_di_IXMLNode> &xmlNodeList);
    void SetValue(AnsiString, int, xmlGuard <_di_IXMLNode> &xmlNodeList);
    void SetValue(AnsiString, float, xmlGuard <_di_IXMLNode> &xmlNodeList);
    virtual bool StoreToFile(AnsiString,bool);
    virtual bool SaveToFile();
    bool SaveFile(AnsiString _FileName, AnsiString SourceData, bool bNoAutoCreate = false);
    virtual bool SaveToTempFile();
    AnsiString GetFileName(void);
    bool LoadFromFile(AnsiString);
    //virtual bool GetDataFromXML(_di_IXMLDocument XMLDoc);
    virtual AnsiString GetNewFileName(AnsiString AFileExt, AnsiString AFilePrefix="out");
    void RenameTempFile(AnsiString Ext = "pkt");
    bool RenamePaketFile(const char* FileName);
    bool RenamePaketFileFullPath(const char* FileName);
    void DeleteTempFile();
    AnsiString TruncateFileName(AnsiString);
    bool CreateFile(AnsiString FileName);
    bool OpenFile(AnsiString FileName, AnsiString &FileData);
    bool CloseFile();
    bool getXmlParseError();
};

//---------------------------------------------------------------------------

class TPaymentPacket : public TXMLPacket
{
private:
	virtual bool GetDataFromXML(xmlGuard <_di_IXMLDocument> &XMLDoc);
public:
	int OperatorId;
	int Status;
	int LastErrorCode;
	AnsiString LastSession;
	AnsiString InitialSessionNum;
	TDateTime PaymentCreateDT;
	TDateTime FirstTryDT;
	TDateTime NextTryDT;
	int NumOfTries;
	bool Cancelled;
	int ResultExternal;
	AnsiString TransId;
	AnsiString ProcessorType;
  AnsiString SavedData;
  AnsiString SavedCardString;
  int LastPaymentErrorCode;

	TPaymentPacket(TWConfig*, TLogClass*);
        virtual ~TPaymentPacket();
	virtual void Clear();
//    virtual bool StoreToFile();
	virtual bool StoreToFile(AnsiString,bool);
	virtual bool SaveToFile();
	virtual bool SaveToTempFile();
	//virtual bool LoadFromFile(AnsiString);
	virtual AnsiString GetNewFileName(AnsiString AFileExt, AnsiString AFilePrefix="out");
	void ClearForResurrect();
        //TPaymentPacket(const TPaymentPacket& rhs);
        //TPaymentPacket& operator=(const TPaymentPacket& rhs);
};


//---------------------------------------------------------------------------

class TStatPacket : public TXMLPacket
{
protected:
    virtual bool GetDataFromXML(xmlGuard <_di_IXMLDocument> &XMLDoc);
public:
    int PacketType;
    int Status;
    int TerminalID;
    TDateTime EventDateTime;
    TDateTime InitDT;
    int OperatorID;
    AnsiString SessionNum;
    AnsiString InitialSessionNum;
    float Comission;
//    int NoteCounts[8];
    int ErrorCode;
    AnsiString ErrSender;
    int ErrType;
    AnsiString ErrDescription;
    int ErrSubType;
    AnsiString ErrSubDescription;
    AnsiString SendFileName;
    int BillCount;
    int BillSum;
    AnsiString ForcedURL;
    int ChequeCounter;

    TStatPacket(TWConfig*, TLogClass*);
    virtual ~TStatPacket();

    virtual void Clear();
    virtual bool StoreToFile(AnsiString,bool);
    virtual bool SaveToFile();
    virtual bool SaveToTempFile();
    //virtual bool LoadFromFile(AnsiString);
};

//---------------------------------------------------------------------------

class TEMailPacket : public TXMLPacket
{
public:
    TEMailPacket(TWConfig*, TLogClass*);
    virtual ~TEMailPacket();
};

//---------------------------------------------------------------------------

class TSMSPacket : public TStatPacket
{
public:
    TSMSPacket(TWConfig*, TLogClass*);
    virtual ~TSMSPacket();
};

struct _spec_packet_node
{
    std::string name;
    std::string value;
    std::string lastFileName;

    _spec_packet_node(std::string a_name, std::string a_value, std::string a_lastFileName = ""):
        name(a_name), value(a_value), lastFileName(a_lastFileName) {}
    _spec_packet_node(): name(""), value(""), lastFileName("") {}
};

//---------------------------------------------------------------------------

typedef std::vector<_spec_packet_node> TPacketNodeVector;
struct _filesProperties
{
    friend std::string getDeleteStatFilesString(_filesProperties* CFilesProperties, int a_count = 0, int a_deleted = 0);
    private:
        TWConfig* m_pCfg;
        TLogClass* m_pLog;

        int packet_type;
        int count;
        int undeleted;
        TPacketNodeVector m_nodes;
        TStringVector m_deletedFiles;

    public:
        _filesProperties(int a_packet_type, std::string a_filesTypeName, TWConfig*, TLogClass* = NULL);

        const std::string filesTypeName;
        void getLastFileName(AnsiString a_work_directory, int nodeIndex);
        int  getCount() {return count;}
        int  getUndeleted() {return undeleted;}

        void prepareToDeletingFile(AnsiString& FileName, TStatPacket* packet, char* unconditionalExtention = "");

        int  deletingFiles();
        int  includeNode(std::string, std::string);

        bool isValidType(int);
        bool isValidValue(std::string);
        bool isLastFile(std::string&);

};

//---------------------------------------------------------------------------

#endif
