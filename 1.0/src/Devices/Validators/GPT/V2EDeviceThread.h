//---------------------------------------------------------------------------

#ifndef V2EDeviceThreadH
#define V2EDeviceThreadH
//---------------------------------------------------------------------------
#include <map.h>
#include "DeviceThread.h"
#include "TNote.h"
#include "boost/format.hpp"
namespace v2e
{
    typedef enum
    {
        ENABLING        = 0x000100,
        DISABLING       = 0x800000,
        ACCEPTING       = 0x000200,
        ESCROW          = 0x000020,     //0x000020
        BAD_ESCROW      = 0x002000,
        STACKING        = 0x000800,
        STACKED         = 0x000080,
        REJECTING       = 0x001000,
        RETURNING       = 0x000400,

        HARDWARE_ERROR  = 0x400000,
        POWER_ON        = 0x004000,
        POWER_ON_BILL   = 0x100000,
        STACKER_OPEN    = 0x040000,
        STACKER_FULL    = 0x080000,
        JAM_ON_ACCEPTOR = 0x010000,
        JAM_ON_STACKER  = 0x020000,
        CHEATED         = 0x008000,

        //protocol v2e: This flag may also be set at the first poll after power on and it should be ignored at that time.
        COMMUNICATION_TIMEOUT = 0x200000,
    } states;

    const DWORD REJECT_MASK = 0x00001F;
    const DWORD IDLE_MASK   = ENABLING | DISABLING;
    const DWORD STATE_MASK  = ~(IDLE_MASK | REJECT_MASK | COMMUNICATION_TIMEOUT);
    const DWORD ERROR_MASK  = STATE_MASK & ~(HARDWARE_ERROR | POWER_ON | POWER_ON_BILL);

    namespace reject_reason
    {
        typedef enum
        {
            VALIDATION_FAIL         = 0x000007,
            NOMINAL_DISABLED        = 0x000008,
            DUPLICATE_CORRELATION   = 0x00000B,
            MAGNETIC_TEST           = 0x00000C,
            LENGTH_TEST             = 0x00000D,
            STRING                  = 0x00000E,
            EJECT_FAIL              = 0x00000F,
            ALL_NOMINALS_DISABLED   = 0x000010,
            STACKED_FAIL            = 0x000011,
            CONTROLLER              = 0x000012,
        } reject_reasons;

        //*********************************************************************************************************************
        typedef std::pair<reject_reasons, std::string> RejectReasonElement;
        typedef std::map <reject_reasons, std::string> RejectReasonElements;
        class CReasons
        {
            private:
                static RejectReasonElements m_reasons;
                static void setValues()
                {
                  if(m_reasons.empty())
                  {
                      RejectReasonElement RejectReasons[] = {
                          make_pair<reject_reasons, std::string>(VALIDATION_FAIL,       "Bill fails validation test"),
                          make_pair<reject_reasons, std::string>(NOMINAL_DISABLED,      "Bill denomination disabled"),
                          make_pair<reject_reasons, std::string>(DUPLICATE_CORRELATION, "Duplicate correlation"),
                          make_pair<reject_reasons, std::string>(MAGNETIC_TEST,         "Bill fails magnetic tests"),
                          make_pair<reject_reasons, std::string>(LENGTH_TEST,           "Bill is unable to pass through chamber – second bill follows, or bill fails the length test"),
                          make_pair<reject_reasons, std::string>(STRING,                "Bill was strung"),
                          make_pair<reject_reasons, std::string>(EJECT_FAIL,            "Bill cannot be ejected from unit"),
                          make_pair<reject_reasons, std::string>(ALL_NOMINALS_DISABLED, "All bills inhibited through RS232"),
                          make_pair<reject_reasons, std::string>(STACKED_FAIL,          "Bill cannot be stacked"),
                          make_pair<reject_reasons, std::string>(CONTROLLER,            "Bill rejected by controller")
                      };
                      m_reasons = RejectReasonElements(RejectReasons, RejectReasons + sizeof(RejectReasons) / sizeof(RejectReasons[0]));
                  }
                }
                static RejectReasonElements::iterator getRejectReason(DWORD _state)
                {
                    return m_reasons.find(static_cast<reject_reasons>(_state & REJECT_MASK));
                }
            public:
                static std::string getDescription(DWORD _state)
                {
                    setValues();
                    RejectReasonElements::iterator it_reject_reason = getRejectReason(_state);
                    if (it_reject_reason != m_reasons.end())
                    {
                        std::string asd = it_reject_reason->second;
                        return it_reject_reason->second;
                    }
                    else
                        return "Unknown reject reason or no rejecting";
                }
        };
        RejectReasonElements CReasons::m_reasons;
        //*********************************************************************************************************************
    }

    //*********************************************************************************************************************
    typedef std::pair<states, std::string> StateElement;
    typedef std::map <states, std::string> StateElements;
    class CStates
    {

        private:
            static reject_reason::CReasons RejectReason;
            static StateElements m_states;
            static void setValues()
            {
              if(m_states.empty())
              {
                  StateElement States[] = {
                      make_pair<states, std::string>(ENABLING,   "ENABLING"),
                      make_pair<states, std::string>(DISABLING,  "DISABLING"),
                      make_pair<states, std::string>(ACCEPTING,  "ACCEPTING"),
                      make_pair<states, std::string>(ESCROW,     "ESCROW"),
                      make_pair<states, std::string>(BAD_ESCROW, "BAD_ESCROW"),
                      make_pair<states, std::string>(STACKING,   "STACKING"),
                      make_pair<states, std::string>(STACKED,    "STACKED"),
                      make_pair<states, std::string>(REJECTING,  "REJECTING"),
                      make_pair<states, std::string>(RETURNING,  "RETURNING"),

                      make_pair<states, std::string>(HARDWARE_ERROR,  "HARDWARE_ERROR"),
                      make_pair<states, std::string>(POWER_ON,        "POWER_ON"),
                      make_pair<states, std::string>(POWER_ON_BILL,   "POWER_ON_BILL"),
                      make_pair<states, std::string>(STACKER_OPEN,    "STACKER_OPEN"),
                      make_pair<states, std::string>(STACKER_FULL,    "STACKER_FULL"),
                      make_pair<states, std::string>(JAM_ON_ACCEPTOR, "JAM_ON_ACCEPTOR"),
                      make_pair<states, std::string>(JAM_ON_STACKER,  "JAM_ON_STACKER"),
                      make_pair<states, std::string>(CHEATED,         "CHEATED"),
                  };
                  m_states = StateElements(States, States + sizeof(States) / sizeof(States[0]));
              }
            }
            static StateElements::iterator getState(DWORD _state, DWORD mask)
            {
                return m_states.find(static_cast<states>(_state & mask));
            }
        public:
            static std::string getDescription(DWORD _state)
            {
                setValues();
                StateElements::iterator itState = getState(_state, STATE_MASK);
                if (itState != m_states.end())
                {
                    std::string reject_reason_str;
                    if ((_state & STATE_MASK) == REJECTING)
                    {
                        reject_reason_str = (boost::format(", reject reason = %1%")
                          % RejectReason.getDescription(_state)).str().c_str();
                    }
                    return (boost::format("%1%%2%") % itState->second % reject_reason_str).str();
                }
                else
                {

                    itState = getState(_state, IDLE_MASK);
                    if (itState != m_states.end())
                        return itState->second;
                    else
                        return "UNKNOWN STATE";
                }
            }
    };
    StateElements CStates::m_states;
    //*********************************************************************************************************************

    typedef enum
    {
        ENABLE          = 0x41,
        SET_MODE        = 0x40,
        SET_ORIENTATION = 0x43,
        SET_SECURITY    = 0x42,
        POLL            = 0xCC,
        RESET           = 0x36,
        STACK           = 0x80,
        RETURN          = 0x81,
        HOLD            = 0x82,
        GET_NOMINAL_TABLE = 0xE5
    } commands;

    //*********************************************************************************************************************

    typedef std::pair<commands, std::string> CommandElement;
    typedef std::map <commands, std::string> CommandElements;
    class CCommands
    {
        private:
            static CommandElements m_commands;
            static void setValues()
            {
              if(m_commands.empty())
              {
                  CommandElement Commands[] = {
                      make_pair<commands, std::string>(ENABLE,            "ENABLE"),
                      make_pair<commands, std::string>(SET_MODE,          "SET_MODE"),
                      make_pair<commands, std::string>(SET_ORIENTATION,   "SET_ORIENTATION"),
                      make_pair<commands, std::string>(SET_SECURITY,      "SET_SECURITY"),
                      make_pair<commands, std::string>(POLL,              "POLL"),
                      make_pair<commands, std::string>(RESET,             "RESET"),
                      make_pair<commands, std::string>(STACK,             "STACK"),
                      make_pair<commands, std::string>(RETURN,            "RETURN"),
                      make_pair<commands, std::string>(HOLD,              "HOLD"),
                      make_pair<commands, std::string>(GET_NOMINAL_TABLE, "GET_NOMINAL_TABLE")
                  };
                  m_commands = CommandElements(Commands, Commands + sizeof(Commands) / sizeof(Commands[0]));
              }
            }
            static CommandElements::iterator getCommand(DWORD _command)
            {
                return m_commands.find(static_cast<commands>(_command));
            }
        public:
            static std::string getDescription(DWORD _command)
            {
                setValues();
                CommandElements::iterator itCommand = getCommand(_command);
                if (itCommand != m_commands.end())
                {
                    return itCommand->second;
                }
                else
                {
                    return "UNKNOWN COMMAND";
                }
            }
    };
    CommandElements CCommands::m_commands;
    
    //*********************************************************************************************************************


    const int PACKET_SIZE = 7;
    const int NOTE_SIZE = 6;

    const int NominalBytesCount = 8;
    const int protocolID_BytesCount = 1;


    namespace MODE
    {
        typedef enum
        {
            POLL = 0,
            INIT = 1
        } modes;
    }
}
class TV2EDeviceThread : public TDeviceThread
{
private:
    TNotesVector BillNotes;
    int OfflineCount;
    bool noPoll;

    bool isWork();
    bool isMiddleProcessing();
    int GetCurrentNominal();
    void InitNominalTable();
protected:
    void SendPacket(BYTE command, BYTE* _data = NULL, BYTE len_packet = 7);
    BYTE getCheckByte(BYTE* DataBuf, int end = 7);

    void Poll();
    void Reset();
    void Return();
    void Hold();
    void Stack();
    void SetMode();
    void SetOrientation();
    void SetSecurity();
    void GetNominalTable();
    void changeAccept(bool);
    
    virtual void PollingLoop();
    virtual void ProcessOutCommand();
    virtual void ParseAnswer(int mode = 0);
    virtual void __fastcall ProcessLoopCommand();
public:
    __fastcall TV2EDeviceThread();
    virtual __fastcall ~TV2EDeviceThread();
    bool IsItYou();
};
#endif
