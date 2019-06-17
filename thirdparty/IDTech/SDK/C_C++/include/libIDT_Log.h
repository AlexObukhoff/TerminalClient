#ifndef LIBIDT_LOG_H_
#define LIBIDT_LOG_H_

#include "IDTDef.h"
#include <stdarg.h>

typedef enum LOG_RESPONSE {
    LOG_SUCCESS,
    LOG_INVALID_CALLBACK,
    LOG_INVALID_LOG_TYPE,
    LOG_INVALID_FILE_NAME,
    LOG_INVALID_FUNCTION_NAME,
    LOG_INVALID_LINE_NUMBER,
    LOG_INVALID_FORMAT,
    LOG_INVALID_DATA,
    LOG_INVALID_DATA_LENGTH,
    LOG_INVALID_DIRECTION,

    LOG_FAILED // Generic error - should try to return another response code if possible
} LOG_RESPONSE;

/**
 * Define the log callback function to receive log messages.
 */
typedef void (*pLog_callback)(BYTE, char*);

/**
 * Register logging callback.
 *
 * @param callback - The callback to receive all logging messages
 *
 * @return  LOG_SUCCESS
 *          ERROR, LOG_INVALID_CALLBACK
 */
LOG_RESPONSE log_registerLogCallback(IN pLog_callback callback);

/**
 * Relays the log message to the logging manager for processing.
 *
 * @param logLevel - A byte whose bits indicate which type of information is being logged
 *  | Bit Representation | Type of Information           |
 *  | ------------------ | ----------------------------- |
 *  | - - - - - - - 1    | Function entry and exit       |
 *  | - - - - - - 1 -    | Information                   |
 *  | - - - - - 1 - -    | Error                         |
 *  | - - - - 1 - - -    | Warning                       |
 *  | - - - 1 - - - -    | Debug                         |
 *  | - - 1 - - - - -    | Reserved for future use (RFU) |
 *  | - 1 - - - - - -    | RFU                           |
 *  | 1 - - - - - - -    | RFU                           |
 * @param fileName - The current file name
 * @param functionName - The current function name
 * @param lineNumber - The current line number
 * @param format - The desired format of the log message
 * @param ... - The parameters for the log message
 *
 * @return  LOG_SUCCESS
 *          ERROR, LOG_INVALID_LOG_TYPE
 *          ERROR, LOG_INVALID_FILE_NAME
 *          ERROR, LOG_INVALID_FUNCTION_NAME
 *          ERROR, LOG_INVALID_LINE_NUMBER
 *          ERROR, LOG_INVALID_FORMAT
 */
LOG_RESPONSE log_entryInvocation(IN BYTE logType, IN const char *fileName, IN const char *functionName, IN const int lineNumber, IN const char *format, IN ...) __attribute((no_instrument_function));

/**
 * Relays the log message to the logging manager for processing.
 *
 * @param logLevel - A byte whose bits indicate which type of information is being logged
 *  | Bit Representation | Type of Information           |
 *  | ------------------ | ----------------------------- |
 *  | - - - - - - - 1    | Function entry and exit       |
 *  | - - - - - - 1 -    | Information                   |
 *  | - - - - - 1 - -    | Error                         |
 *  | - - - - 1 - - -    | Warning                       |
 *  | - - - 1 - - - -    | Debug                         |
 *  | - - 1 - - - - -    | Reserved for future use (RFU) |
 *  | - 1 - - - - - -    | RFU                           |
 *  | 1 - - - - - - -    | RFU                           |
 * @param fileName - The current file name
 * @param functionName - The current function name
 * @param lineNumber - The current line number
 * @param format - The desired format of the log message
 * @param ... - The parameters for the log message
 *
 * @return  LOG_SUCCESS
 *          ERROR, LOG_INVALID_LOG_TYPE
 *          ERROR, LOG_INVALID_FILE_NAME
 *          ERROR, LOG_INVALID_FUNCTION_NAME
 *          ERROR, LOG_INVALID_LINE_NUMBER
 *          ERROR, LOG_INVALID_FORMAT
 */
LOG_RESPONSE log_entryInvocationVAList(IN BYTE logType, IN const char *fileName, IN const char *functionName, IN const int lineNumber, IN const char *format, IN va_list args) __attribute((no_instrument_function));

/**
 * Notify the log system when low-level communication takes place.
 *
 * @param logLevel - A byte whose bits indicate which type of information is being logged
 *  | Bit Representation | Type of Information           |
 *  | ------------------ | ----------------------------- |
 *  | - - - - - - - 1    | Function entry and exit       |
 *  | - - - - - - 1 -    | Information                   |
 *  | - - - - - 1 - -    | Error                         |
 *  | - - - - 1 - - -    | Warning                       |
 *  | - - - 1 - - - -    | Debug                         |
 *  | - - 1 - - - - -    | Reserved for future use (RFU) |
 *  | - 1 - - - - - -    | RFU                           |
 *  | 1 - - - - - - -    | RFU                           |
 * @param data - The raw data
 * @param dataLength - The raw data's length
 * @param direction - A number indicating the direction of the data (0 = data received by device, 1 = data sent from device)
 * @param fileName - The current file name
 * @param functionName - The current function name
 * @param lineNumber - The current line number
 *
 * @return  LOG_SUCCESS
 *          ERROR, LOG_INVALID_LOG_TYPE
 *          ERROR, LOG_INVALID_DATA
 *          ERROR, LOG_INVALID_DATA_LENGTH
 *          ERROR, LOG_INVALID_DIRECTION
 *          ERROR, LOG_INVALID_FILE_NAME
 *          ERROR, LOG_INVALID_FUNCTION_NAME
 *          ERROR, LOG_INVALID_LINE_NUMBER
 */
LOG_RESPONSE log_rawCommData(IN const BYTE logType, IN const BYTE *data, IN const int dataLength, IN const int direction, IN const char *fileName, IN const char *functionName, IN const int lineNumber);

/**
 * Notify the log system when low-level communication takes place.
 *
 * @param logLevel - A byte whose bits indicate which type of information is being logged
 *  | Bit Representation | Type of Information           |
 *  | ------------------ | ----------------------------- |
 *  | - - - - - - - 1    | Function entry and exit       |
 *  | - - - - - - 1 -    | Information                   |
 *  | - - - - - 1 - -    | Error                         |
 *  | - - - - 1 - - -    | Warning                       |
 *  | - - - 1 - - - -    | Debug                         |
 *  | - - 1 - - - - -    | Reserved for future use (RFU) |
 *  | - 1 - - - - - -    | RFU                           |
 *  | 1 - - - - - - -    | RFU                           |
 * @param data - The raw data
 * @param dataLength - The raw data's length
 * @param direction - A number indicating the direction of the data (0 = data received by device, 1 = data sent from device)
 *
 * @return  LOG_SUCCESS
 *          ERROR, LOG_INVALID_LOG_TYPE
 *          ERROR, LOG_INVALID_DATA
 *          ERROR, LOG_INVALID_DATA_LENGTH
 *          ERROR, LOG_INVALID_DIRECTION
 */
#define log_raw(logType, data, dataLength, direction) \
    log_rawCommData(logType, data, dataLength, direction, __FILE_NAME__, __func__, __LINE__)

/**
 * Notify the log system when a function is entered or exited with a custom message.
 *
 * @param format - The desired format of the log message
 * @param ... - The parameters for the log message
 *
 * @return  LOG_SUCCESS
 *          ERROR, LOG_INVALID_FORMAT
 */
#define log_functionEntryOrExit(format, ...) \
    log_entryInvocation(LOG_LEVEL_FUNCTION_ENTRY_AND_EXIT, __FILE_NAME__, __func__, __LINE__, format, __VA_ARGS__)

/**
 * Notify the log system when a function is entered with a predefined message.
 *
 * Predefined message: "Entered <FUNTION_NAME>()"
 *
 * @return  LOG_SUCCESS
 */
#define log_functionEntry() \
    log_functionEntryOrExit("Entered %s()\n", __func__)

/**
 * Notify the log system when a function is exited with a predfined message.
 *
 * Predefined message: "Exited <FUNCTION_NAME>()";
 *
 * @return  LOG_SUCCESS
 */
#define log_functionExit() \
    log_functionEntryOrExit("Exited %s()\n", __func__)

/**
 * Notify the log system of specific information with a custom message.
 *
 * @param format - The desired format of the log message
 * @param ... - The parameters for the log message
 *
 * @return  LOG_SUCCESS
 *          ERROR, LOG_INVALID_FORMAT
 */
#define log_information(format, ...) \
    log_entryInvocation(LOG_LEVEL_INFORMATION, __FILE_NAME__, __func__, __LINE__, format, __VA_ARGS__)

#endif /* LIBIDT_LOG_H_ */