#ifndef LIBIDT_LOGMGR_H_
#define LIBIDT_LOGMGR_H_

#include "IDTDef.h"

typedef enum LOGMGR_RESPONSE {
    LOGMGR_SUCCESS,
    LOGMGR_USER_NOT_VALIDATED_FOR_THIS_FUNCTION,
    LOGMGR_USER_NOT_VALIDATED_FOR_THIS_LOG_LEVEL,
    LOGMGR_LOG_DESTINATION_NOT_CONFIGURED,
    LOGMGR_UNABLE_TO_INITIALIZE_OR_OPEN_LOG_FILE,
    LOGMGR_NO_LOG_LEVELS_DEFINED,
    LOGMGR_LOGGING_NOT_ENABLED,
    LOGMGR_ACTION_ONLY_POSSIBLE_WITH_SQLITE_DESTINATION,
    LOGMGR_COULD_NOT_CONNECT_TO_DATABASE,
    LOGMGR_SQLITE_COMMAND_FAILED,
    LOGMGR_INVALID_ENTRY_NUMBER,
    LOGMGR_INVALID_MESSAGE,
    LOGMGR_ENTRY_NUMBER_DOES_NOT_EXIST,
    LOGMGR_INVALID_NUMBER_OF_ENTRIES,
    LOGMGR_INVALID_ENDPOINT,
    LOGMGR_FAILED_TO_SEND_LOG_ENTRY,
    LOGMGR_INVALID_MULTIPOST_VALUE,

    LOGMGR_DLL_FAILED, // Error in the IDT_Device function
    LOGMGR_FAILED // Generic error - should try to return another respose code if possible
} LOGMGR_RESPONSE;

#define MAX_PAYLOAD_LENGTH 2048

static void logCallback(BYTE type, char *message) __attribute((no_instrument_function));

/**
 * Enable the logging of debug information. If already enabled, updates the logging level if different than current logging level.
 *
 * @param logLevel - A byte whose bits indicate which type of information that should be logged
 *  | Bit Representation | Type of Information           |
 *  | ------------------ | ----------------------------- |
 *  | - - - - - - - 1    | Raw communication data        |
 *  | - - - - - - 1 -    | Function entry and exit       |
 *  | - - - - - 1 - -    | Information                   |
 *  | - - - - 1 - - -    | Reserved for future use (RFU) |
 *  | - - - 1 - - - -    | RFU                           |
 *  | - - 1 - - - - -    | RFU                           |
 *  | - 1 - - - - - -    | RFU                           |
 *  | 1 - - - - - - -    | RFU                           |
 *
 * @return  LOGMGR_SUCCESS,
 *          ERROR, LOGMGR_NO_LOG_LEVELS_DEFINED
 *          ERROR, LOGMGR_USER_NOT_VALIDATED_FOR_THIS_FUNCTION
 *          ERROR, LOGMGR_COULD_NOT_CONNECT_TO_DATABASE
 */
LOGMGR_RESPONSE logMgr_enableLogging(IN BYTE logLevel);

/**
 * Disable the logging of some or all the log levels. Sending a value of 0xFF disables all logging.
 *
 * @param logLevel - A byte whose bits indicate which type of information that should not be logged
 *  | Bit Representation | Type of Information           |
 *  | ------------------ | ----------------------------- |
 *  | - - - - - - - 1    | Raw communication data        |
 *  | - - - - - - 1 -    | Function entry and exit       |
 *  | - - - - - 1 - -    | Information                   |
 *  | - - - - 1 - - -    | Reserved for future use (RFU) |
 *  | - - - 1 - - - -    | RFU                           |
 *  | - - 1 - - - - -    | RFU                           |
 *  | - 1 - - - - - -    | RFU                           |
 *  | 1 - - - - - - -    | RFU                           |
 *
 * @return  LOGMGR_SUCCESS
 *          ERROR, LOGMGR_NO_LOG_LEVELS_DEFINED
 *          ERROR, LOGMGR_LOGGING_NOT_ENABLED
 *          ERROR, LOGMGR_USER_NOT_VALIDATED_FOR_THIS_FUNCTION
 */
LOGMGR_RESPONSE logMgr_disableLogging(IN BYTE logLevel);

/**
 * Retrieves log entry.
 *
 * @param logLevel - A byte whose bits indicate which type of log levels should be filtered for
 *  | Bit Representation | Type of Information           |
 *  | ------------------ | ----------------------------- |
 *  | - - - - - - - 1    | Raw communication data        |
 *  | - - - - - - 1 -    | Function entry and exit       |
 *  | - - - - - 1 - -    | Information                   |
 *  | - - - - 1 - - -    | Reserved for future use (RFU) |
 *  | - - - 1 - - - -    | RFU                           |
 *  | - - 1 - - - - -    | RFU                           |
 *  | - 1 - - - - - -    | RFU                           |
 *  | 1 - - - - - - -    | RFU                           |
 * @param entryNum - Log entry position (1 = first entry, n = number of entries / last entry)
 * @param message - A null terminated message string, maximum size 2048 bytes
 *
 * @return  LOGMGR_SUCCESS
 *          ERROR, LOGMGR_NO_LOG_LEVELS_DEFINED
 *          ERROR, LOGMGR_INVALID_ENTRY_NUMBER
 *          ERROR, LOGMGR_INVALID_MESSAGE
 *          ERROR, LOGMGR_USER_NOT_VALIDATED_FOR_THIS_FUNCTION
 *          ERROR, LOGMGR_COULD_NOT_CONNECT_TO_DATABASE
 *          ERROR, LOGMGR_SQLITE_COMMAND_FAILED
 *          ERROR, LOGMGR_ENTRY_NUMBER_DOES_NOT_EXIST
 */
LOGMGR_RESPONSE logMgr_retrieveLogEntry(IN BYTE logLevel, IN int entryNumber, OUT const char *message);

/**
 * Retrieves log entry.
 *
 * @param numEntries - Number of log entries
 *
 * @return  LOGMGR_SUCCESS
 *          ERROR, LOGMGR_INVALID_NUMBER_OF_ENTRIES
 *          ERROR, LOGMGR_USER_NOT_VALIDATED_FOR_THIS_FUNCTION
 *          ERROR, LOGMGR_COULD_NOT_CONNECT_TO_DATABASE
 *          ERROR, LOGMGR_SQLITE_COMMAND_FAILED
 */
LOGMGR_RESPONSE logMgr_retrieveLog(IN BYTE logLevel, IN const int pos, IN_OUT char *buffer, IN_OUT int *bufferLen);

/**
 * Retrieves the log entries count for the specified log level.
 *
 * @param logLevel - The log level to retrieve log entries for
 * @param numEntries - Number of log entries
 *
 * @return  LOGMGR_SUCCESS
 *          ERROR, LOGMGR_NO_LOG_LEVELS_DEFINED
 *          ERROR, LOGMGR_INVALID_NUMBER_OF_ENTRIES
 *          ERROR, LOGMGR_USER_NOT_VALIDATED_FOR_THIS_FUNCTION
 *          ERROR, LOGMGR_COULD_NOT_CONNECT_TO_DATABASE
 *          ERROR, LOGMGR_SQLITE_COMMAND_FAILED
 */
LOGMGR_RESPONSE logMgr_retrieveLogEntryCount(OUT int *numEntries);

/**
 * Retrieves the log entries count for the specified log level.
 *
 * @param logLevel - The log level to retrieve log entries for
 * @param numEntries - Number of log entries
 *
 * @return  LOGMGR_SUCCESS
 *          ERROR, LOGMGR_NO_LOG_LEVELS_DEFINED
 *          ERROR, LOGMGR_INVALID_NUMBER_OF_ENTRIES
 *          ERROR, LOGMGR_USER_NOT_VALIDATED_FOR_THIS_FUNCTION
 *          ERROR, LOGMGR_COULD_NOT_CONNECT_TO_DATABASE
 *          ERROR, LOGMGR_SQLITE_COMMAND_FAILED
 */
LOGMGR_RESPONSE logMgr_retrieveLogEntryCountForLogLevel(IN LOG_LEVEL logLevel, OUT int *numEntries);

/**
 * Clears the log.
 *
 * @return  LOGMGR_SUCCESS
 *          ERROR, LOGMGR_USER_NOT_VALIDATED_FOR_THIS_FUNCTION
 *          ERROR, LOGMGR_COULD_NOT_CONNECT_TO_DATABASE
 *          ERROR, LOGMGR_SQLITE_COMMAND_FAILED
 */
LOGMGR_RESPONSE logMgr_clearLog();

/**
 * Clears the logs for the specified log level.
 *
 * @param logLevel - The log level to clear logs from
 *
 * @return  LOGMGR_SUCCESS
 *          ERROR, LOGMGR_NO_LOG_LEVELS_DEFINED
 *          ERROR, LOGMGR_USER_NOT_VALIDATED_FOR_THIS_FUNCTION
 *          ERROR, LOGMGR_COULD_NOT_CONNECT_TO_DATABASE
 *          ERROR, LOGMGR_SQLITE_COMMAND_FAILED
 */
LOGMGR_RESPONSE logMgr_clearLogForLogLevel(IN LOG_LEVEL logLevel);

/**
 * Post log entry with content type of text/plain.
 *
 * @param logLevel - A byte whose bits indicate which type of log levels should be filtered for.
 * @param entryNumber - Log entry position (1 = first entry, n = number of entries / last entry)
 * @param url - A null terminated url string to post
 *
 * @return  LOGMGR_SUCCESS
 *          ERROR, LOGMGR_NO_LOG_LEVELS_DEFINED
 *          ERROR, LOGMGR_INVALID_ENTRY_NUMBER
 *          ERROR, LOGMGR_INVALID_ENDPOINT
 *          ERROR, LOGMGR_USER_NOT_VALIDATED_FOR_THIS_FUNCTION
 *          ERROR, LOGMGR_COULD_NOT_CONNECT_TO_DATABASE
 *          ERROR, LOGMGR_SQLITE_COMMAND_FAILED
 *          ERROR, LOGMGR_ENTRY_NUMBER_DOES_NOT_EXIST
 *          ERROR, LOGMGR_FAILED_TO_SEND_LOG_ENTRY
 */
LOGMGR_RESPONSE logMgr_postLogEntry(IN BYTE logLevel, IN const int entryNumber, IN const char *url);

/**
 * Posts log file with content type of text/plain. Maximum body size for a single post is 2k (2048 bytes).
 *
 * @param logLevel - A byte whose bits indicate which type of log levels should be filtered for
 * @param multiPost -   0 = Post additional items as necessary until all log data is transmitted
 *                      1 = Only post a single time the amount of data up to the maximum body size
 *
 * @return  LOGMGR_SUCCESS
 *          ERROR, LOGMGR_NO_LOG_LEVELS_DEFINED
 *          ERROR, LOGMGR_INVALID_MULTIPOST_VALUE
 *          ERROR, LOGMGR_INVALID_ENDPOINT
 *          ERROR, LOGMGR_USER_NOT_VALIDATED_FOR_THIS_FUNCTION
 *          ERROR, LOGMGR_COULD_NOT_CONNECT_TO_DATABASE
 *          ERROR, LOGMGR_SQLITE_COMMAND_FAILED
 *          ERROR, LOGMGR_FAILED_TO_SEND_LOG_ENTRY
 */
LOGMGR_RESPONSE logMgr_postLog(IN BYTE logLevel, IN const int multiPost, IN const char *url);

#endif /* LIBIDT_LOGMGR_H_ */