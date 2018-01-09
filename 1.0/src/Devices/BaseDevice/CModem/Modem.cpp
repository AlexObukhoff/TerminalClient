//---------------------------------------------------------------------------
#include <string>
#pragma hdrstop
#include "Modem.h"
#include "globals.h"
#include "boost/format.hpp"
//---------------------------------------------------------------------------
#pragma package(smart_init)

using namespace std;

CModem::CModem(int ComPort,TLogClass* _Log, AnsiString Prefix) : TDeviceClass(ComPort,_Log, Prefix)
{
    LoggingErrors = true;
    Error         = 0;
    ModemEnable   = true;
    EndSignal     = 0x0D;
}

bool CModem::ReopenOn19200()
{
    bool result = false;
    if (Port)
    {
        Port->COMParameters->BaudRate = 19200;
        Port->ReopenPort();
        result = Port->PortInit;
    }
    else
        result = false;

    if(!result)
        if (Log)
            Log->Write("Can't open port with speed 19200");
    return result;
}

bool CModem::ReopenOn115200()
{
    bool result = false;
    if (Port)
    {
        Port->COMParameters->BaudRate = 115200;
        Port->ReopenPort();
        result = Port->PortInit;
    }
    else
        result = false;

    if(!result)
        if (Log)
            Log->Write("Can't open port with speed 115200");
    return result;
}


//---------------------------------------------------------------------------

void CModem::SendCommand()
{
    StopThread();
    DeviceThread = new TDeviceThread(true, false);
    Start();
    delete DeviceThread;
    DeviceThread = NULL;
}

//---------------------------------------------------------------------------

void CModem::SendPacket(BYTE* command, int bytes_count,int datalen, BYTE* data, bool datafirst)
{
    if (!Port->PortInit)    return;
    //try {
    //здесь в буфер пишем команду для выполнения
        ClearCommand();
        ClearAnswer();

        if (!datafirst) {
            for(int i=0; i<bytes_count; i++) Command[i] = command[i];

            if (datalen != 0) {
                if (data!=NULL) memcpy(&Command[bytes_count],data,datalen);
            }
        } else {
            if (datalen != 0) {
                if (data!=NULL)	memcpy(&Command[0],data,datalen);
            }
            for(int i=datalen; i<datalen+bytes_count; i++) Command[i] = command[i-datalen];
        }

        //CommandParameters->SetParameters(Command,bytes_count+datalen,command[0],0);
        CommandSize = bytes_count+datalen;
    //} __finally
    //{
   //	}
}

//---------------------------------------------------------------------------

bool CModem::SendATCommand(BYTE* ATCommand, int AtCommand)
{
    if (!Port->PortInit) 	return false;
    try {
        string sATCommand((const char*)ATCommand);

        SendType = NotRecieveAnswer;

        if(AtCommand == CALL_COMMAND) {
            sATCommand.insert(0, "ATD");
            //sATCommand.push_back('\"');
            //sATCommand.push_back(';');
        }
        if(AtCommand == AT_COMMAND) sATCommand.insert(0, "AT");

        Log->Write((boost::format("Sending command '%1%'...") % sATCommand).str().c_str());

        for(int i=0; i<(int)sATCommand.size(); i++) {
            SendPacket((unsigned char*)&sATCommand[i], 1, 0, NULL, false);
            SendCommand();
        }

      //if(sATCommand.find("+CMGS") == string::npos) // ... если SMS, EndSignal не нужен
      //  {
            SendPacket(&EndSignal, 1, 0, NULL, false);
            SendCommand();
            //Answer = GetAnswerBuffer(50,"");
        //}
        return true;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return false;
}

//---------------------------------------------------------------------------

string  CModem::GetAnswerBuffer(long timer_loop_limit, const char* WaitWord, bool Scan)
{
    UNREFERENCED_PARAMETER(WaitWord);
    UNREFERENCED_PARAMETER(Scan);
    string Buffer("");
    long timer_loop = 0;
    //int temp_pos = 0, x = 0; //  LoggingErrors = true; //sss_pos = 0,

    while(timer_loop < timer_loop_limit) {
        timer_loop++;
        AnswerSize = DCBufferSize;
        AnswerSize = ReadPort(Answer);
        Buffer+=(const char*)Answer;
        if (AnswerSize>0)
            timer_loop=0;

/*		if(Buffer.find("ERROR") != string::npos) break;
		else if(Buffer.find("\"") != string::npos)
		{
			if(!x)
			{
					temp_pos = Buffer.find("\"");
					x++;
			}
			else
			{
					if(Buffer.find("\"", temp_pos+1) != string::npos)   break;
			}
		}
		else if( (WaitWord != NULL) && (Buffer.find(WaitWord, 0) != string::npos) ) break;*/

		Sleep(100);
	}
	return Buffer;
}

//---------------------------------------------------------------------------

AnsiString CModem::GetModemType()
{
    try
    {
        AnsiString ATemp;
        SendATCommand((BYTE *)"I");
        int iPos = -1;
        std::string Temp;
        Temp = GetAnswerBuffer(20, "OK");
        //Temp.assign((const char*)Answer);
        if (Temp.length()>0) {
            do {
                iPos = Temp.find('\r');
                if(iPos != string::npos)
                    Temp.erase(iPos, 1);
            } while(iPos != string::npos);

            do {
                iPos = Temp.find('\n');
                if(iPos != string::npos)
                    Temp.erase(iPos, 1);
                } while(iPos != string::npos);

                ATemp = AnsiString(Temp.c_str());
            }
        return ATemp;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return "";
}

//---------------------------------------------------------------------------

void CModem::Start()
{
	if (Port == NULL) return;  								// Port - объект класса TComPort
  if (DeviceThread == NULL)	return;							// DeviceThread - объект класса TDeviceThread
  //включаем устройство, переход в режим ожидания
  if (DeviceState != NULL)    DeviceState->State = Wait;  	// DeviceState - объект класса TDeviceState

  DeviceThread->Log = Log;
  DeviceThread->DeviceState = DeviceState;
  DeviceThread->CommandParameters = CommandParameters;
	DeviceThread->SendType = SendType;
  DeviceThread->Port = Port;
  DeviceThread->Command = Command;
  DeviceThread->CommandSize = CommandSize;
  DeviceThread->Answer = Answer;
  DeviceThread->AnswerSize = AnswerSize;
  DeviceThread->data = data;
  DeviceThread->len_data = &len_data;
  DeviceThread->LifeTime = ThreadLifeTime;

  DeviceThread->CommandCriticalSection = CommandCriticalSection;

  DeviceThread->DataLengthIndex = DataLengthIndex;
  DeviceThread->BeginByte = BeginByte;
  DeviceThread->LoggingErrors = LoggingErrors;
  DeviceThread->LastError = LastError;
  DeviceThread->EndByte = EndByte;
  DeviceThread->CRCLength = CRCLength;
  DeviceThread->DataLength = DataLength;

  DeviceThread->DeviceStateChanged = DeviceStateChanged;
  DeviceThread->CommandFinished = CommandFinished;

  DeviceThread->ChangeEvent = ChangeEvent;

  //запускаем отправку команды устройству
  Resume();
  //ждём завершения отправки
  WaitSendingFinish();
}

int CModem::InitModem()
{
		string Buf("");

		SendATCommand((BYTE *)"Z");
		Buf = GetAnswerBuffer(10, "OK");
		if ( Buf.find("OK") != string::npos )
		{
				SendATCommand((BYTE *)"E0");  // AT ->
				Buf = GetAnswerBuffer(10, "OK");
        if ( Buf.find("OK") != string::npos )
        {
            SendATCommand((BYTE *)"+CPIN?");
            Buf = GetAnswerBuffer(10, "READY");
            if ( Buf.find("READY") != string::npos )
            {
//                char Type[30];
								Log->Write((boost::format("Modem type: %1%") % GetModemType().c_str()).str().c_str());
								//if(GetModemType(Type))  Log->Write("Тип используемого модема: " + (AnsiString)Type);
								//Log->Write(">>> Модем инициализирован.");
                return 0;
            }
            else
            {
                Log->Write(">>> Error: no SIM card!");
                return 3;
            }
        }
        else
        {
            Log->Write(">>> Error: Modem is not ready!");
            return 2;
        }
    }
    else
    {
        Log->Write(">>> Error: Can't init modem!");
        return 1;
    }
}
bool CModem::SendSMS(const char* SmsNumber, const char* SmsText)
{
    string Buf("");
    if (Log)
        Log->Write((boost::format(">>> SendSMS Number: %1%; Text: %2%") % SmsNumber % SmsText).str().c_str());

    SendATCommand((BYTE *)"+CSMS=0");
		Buf = GetAnswerBuffer(20, "OK");
    if ( Buf.find("OK") != string::npos )
    {
        SendATCommand((BYTE *)"+CMGF=1");
				Buf = GetAnswerBuffer(20, "OK");
        if ( Buf.find("OK") != string::npos )
        {
            SendATCommand((BYTE *)"+CNMI=0,0,0,0,1");
						Buf = GetAnswerBuffer(20, "OK");
            if ( Buf.find("OK") != string::npos )
            {
                SendATCommand((BYTE *)"+CMGS=?");
								Buf = GetAnswerBuffer(20, "OK");
                if ( Buf.find("OK") != string::npos )
                {
                    string sCommandString("+CMGS=\"");
                    sCommandString.append(SmsNumber);
                    sCommandString.push_back('"');

                    SendATCommand((unsigned char*)sCommandString.c_str());
										Buf = GetAnswerBuffer(10, ">");
                    if ( Buf.find(">") != string::npos )
                    {
                        sCommandString.assign(SmsText);

                        sCommandString.push_back((char)0x1A);
                        SendATCommand((unsigned char*)sCommandString.c_str(), DATA_COMMAND);
												Buf = GetAnswerBuffer(100, "OK");
												if ( Buf.find("OK") != string::npos )
												return true;
												else
												return false;
                    }
                }
                else Log->Write((boost::format(">>> Error: AT+CMGS - Answer: %1%") % Buf).str().c_str());
            }
            else Log->Write((boost::format(">>> Error: AT+CNMI - Answer: %1%") % Buf).str().c_str());
        }
        else Log->Write((boost::format(">>> Error: AT+CMGF - Answer: %1%") % Buf).str().c_str());
    }
    else Log->Write((boost::format(">>> Error: AT+CSMS - Answer: %1%") % Buf).str().c_str());
    return false;
}

DWORD CModem::ReadPort(BYTE* Buffer)
{
    COMSTAT cs;
    DWORD LastError;
    DWORD BytesCount;

    ClearBuffer(Buffer);
    ClearCommError(Port->Port, &LastError, &cs);

    // читаем из порта
    if ( !ReadFile(Port->Port, Buffer, cs.cbInQue, &BytesCount, NULL)) {
        if (Log != NULL)
            Log->Write("Can't read answer!");
         return -1;
    }
    if ((Log != NULL)&&(BytesCount>0)) {
        Log->WriteInLine((boost::format("Read from handle port: %1% Data: %2%") % Port->Port % Buffer).str().c_str());
    }
    return BytesCount;
}

void CModem::GetOperatorItems(AnsiString Answer, int index, int& OperatorID, AnsiString& OperatorName)
{
    UNREFERENCED_PARAMETER(OperatorName);
    try
    {
        int Length =  Answer.Length();
        AnsiString Text = Answer.SubString(index,Length-index+1);
        //take operator id
        int DelimiterIndex = Text.Pos(",");
        AnsiString OperatorID_str = Text.SubString(index,DelimiterIndex-index);
        OperatorID = StrToInt(OperatorID_str);
        //take operator name
        Text.Delete(DelimiterIndex+2,1);
        int DelimiterIndex_1 = Text.Pos("\"");
        AnsiString OperatorName = Text.SubString(DelimiterIndex+2,DelimiterIndex+2-DelimiterIndex_1);
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

AnsiString CModem::GetOperatorName(int ID)
{
    try
    {
        AnsiString result = "";
        AnsiString OperatorName = "";
        int OperatorID = ID;

        if (Log)
            Log->Write(">>> GetOperatorName...");
        AnsiString command = "+WOPN=0,"+AnsiString(ID);
        if(!SendATCommand((BYTE *)command.c_str(),AT_COMMAND))
        {
            if (Log)
                Log->Write("Wrong COM port!");
            return result;
        }

        string str("");
        str = GetAnswerBuffer(20, "OK");
        AnsiString Answer = str.c_str();
        if (Log)
            Log->Write((boost::format("Recieved answer: %1%") % Answer.c_str()).str().c_str());
        int index = Answer.Pos(":");
        GetOperatorItems(Answer,index+2,OperatorID,OperatorName);
        if (Log)
            Log->Write((boost::format("Found operator with ID = %1%; Name = %2%") % OperatorID % OperatorName.c_str()).str().c_str());
        return result = OperatorName;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return "";
}

AnsiString CModem::SendCommand(AnsiString Command)
{
    try {
        AnsiString CmdToSend = Command;
        if ((CmdToSend.LowerCase()).Pos("at")==1) {
            CmdToSend = CmdToSend.SubString(2,CmdToSend.Length());
        }
        //if (Log)
        //    Log->Write("Sending command '"+CmdToSend+"'...");
        if(!SendATCommand((BYTE *)CmdToSend.c_str(),AT_COMMAND)) {
            if (Log)
                Log->Write("Wrong COM port!");
            return "ERROR";
        }

        string str("");
        str = GetAnswerBuffer(50, "OK");
        AnsiString Answer = str.c_str();
        if (Log)
            Log->Write((boost::format("Received answer: %1%") % Answer.c_str()).str().c_str());
        return Answer;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
    return "";
}

AnsiString CModem::GetOperatorName()
{
    try
    {
        AnsiString OperatorName = "";

        if (Log)
            Log->Write(">>> GetOperatorName...");
        AnsiString command = "+COPS?";
        if(!SendATCommand((BYTE *)command.c_str(),AT_COMMAND))
        {
            if (Log)
                Log->Write("Wrong COM port!");
            return "";
        }

        string str("");
        str = GetAnswerBuffer(20, "OK");
        AnsiString Answer = str.c_str();
        if (Log)
            Log->Write((boost::format("Recieved answer: %1%") % Answer.c_str()).str().c_str());
        int index = Answer.Pos("\"");
        Answer.Delete(index,1);
        int index_2 = Answer.Pos("\"");
        OperatorName = Answer.SubString(index,index_2-index);
        if (Log)
            Log->Write((boost::format("Found operator with name %1%") % OperatorName.c_str()).str().c_str());
        return OperatorName;
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }

    return "";
}

bool CModem::ResetModem()
{
    return false;
}

bool CModem::ChangeSIM_No(int SIM_No)
{
    UNREFERENCED_PARAMETER(SIM_No);
    return false;
}

bool CModem::ChangeSIM()
{
    return false;
}

