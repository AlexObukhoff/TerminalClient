#ifndef LIBIDT_ISO8583_H_
#define LIBIDT_ISO8583_H_

#include "IDTDef.h"

#define MAX_TAG_LENGTH 64
#define MAX_TAG_DATA_LENGTH 1024

static const char *openingDataElementTag = "<DataElement>";
static const char *closingDataElementTag = "</DataElement>";
static const char *fieldUsageTagName = "FieldUsage";
static const char *dataTagName = "Data";
static const char *openingDataTag = "<Data>";
static const char *closingDataTag = "</Data>";
static const char *dataFieldTagName = "DataField";
static const char *openingDataFieldTag = "<DataField>";
static const char *closingDataFieldTag = "</DataField>";
static const char *ISO8583_1987TagName = "ISO8583_1987_Message";
static const char *ISO8583_1993TagName = "ISO8583_1993_Message";
static const char *ISO8583_2003TagName = "ISO8583_2003_Message";

typedef struct {
    DL_ISO8583_FIELD_DEF *fieldDef;
    DL_ISO8583_MSG_FIELD msgField;
} DataElement;

typedef struct {
    DL_UINT8 openingTag[MAX_TAG_LENGTH];
    DL_UINT8 closingTag[MAX_TAG_LENGTH];

    DataElement dataElements[kDL_ISO8583_MAX_FIELD_IDX + 1];
    DL_UINT8 dataElementsLength;
} ISO8583XMLStructure;

/**
 * Get the ISO8583 1987 version handler.
 *
 * @param ISOHandler A handler with knowledge of the ISO8583 1987 version fields
 *
 * @return RETURN_CODE: Values can be parsed with device_getErrorString()
 */
int iso8583_get1987Handler(OUT DL_ISO8583_HANDLER *ISOHandler);

/**
 * Get the ISO8583 1993 version handler.
 *
 * @param ISOHandler A handler with knowledge of the ISO8583 1993 version fields
 *
 * @return RETURN_CODE: Values can be parsed with device_getErrorString()
 */
int iso8583_get1993Handler(OUT DL_ISO8583_HANDLER *ISOHandler);

/**
 * Get the ISO8583 2003 version handler.
 *
 * @param ISOHandler A handler with knowledge of the ISO8583 2003 version fields
 *
 * @return RETURN_CODE: Values can be parsed with device_getErrorString()
 */
int iso8583_get2003Handler(OUT DL_ISO8583_HANDLER *ISOHandler);

/**
 * Get the specified field's information using the data field.
 *
 * @param dataField - The data field number
 * @param ISOHandler - The ISO8583 handler
 * @param field - The requested field
 *
 * @return RETURN_CODE: Values can be parsed with device_getErrorString()
 */
int iso8583_getField(IN DL_UINT16 dataField, IN DL_ISO8583_HANDLER *ISOHandler, OUT DL_ISO8583_FIELD_DEF *field);

/**
 * Initialize the ISO8583 message structure.
 *
 * @param ISOMessage - The initialized ISO8583 message structure
 *
 * @return RETURN_CODE: Values can be parsed with device_getErrorString()
 */
int iso8583_initializeMessage(OUT DL_ISO8583_MSG *ISOMessage);

/**
 * Get the specified message field using the data field.
 *
 * @param dataField - The data field number
 * @param ISOMessage - The ISO8583 message structure
 * @param messageField - The requested message field
 *
 * @return 0 if the if the setting was applied; otherwise, return -1 on failure
 */
int iso8583_getMessageField(IN DL_UINT16 dataField, IN DL_ISO8583_MSG *ISOMessage, OUT DL_ISO8583_MSG_FIELD *messageField);

/**
 * Set the specified message field.
 *
 * @param dataField - The data field number
 * @param data - The data to apply
 * @param ISOMessage - The ISO8583 message structure
 *
 * @return RETURN_CODE: Values can be parsed with device_getErrorString()
 */
int iso8583_setMessageField(IN DL_UINT16 dataField, IN const DL_UINT8 *data, IN_OUT DL_ISO8583_MSG *ISOMessage);

/**
 * Remove the specified message field.
 *
 * @param dataField - The data field number
 * @param ISOMessage - The ISO8583 message structure
 *
 * @return RETURN_CODE: Values can be parsed with device_getErrorString()
 */
int iso8583_removeMessageField(IN DL_UINT16 dataField, IN_OUT DL_ISO8583_MSG *ISOMessage);

/**
 * Pack the message fields into an array.
 *
 * @param ISOHandler - The ISO8583 handler
 * @param ISOMessage - The ISO8583 message structure
 * @param packedData - The packaged data
 * @param packedDataLength - The packaged data's length
 *
 * @return RETURN_CODE: Values can be parsed with device_getErrorString()
 */
int iso8583_packMessage(IN const DL_ISO8583_HANDLER *ISOHandler, IN const DL_ISO8583_MSG *ISOMessage, OUT DL_UINT8 *packedData, OUT DL_UINT16 *packedDataLength);

/**
 * Unpack the message field array into the ISO8583 message structure.
 *
 * @param ISOHandler - The ISO8583 handler
 * @param packedData - The packaged data
 * @param packedDataLength - The packaged data's length
 * @param ISOMessage - The ISO8583 message structure
 *
 * @return RETURN_CODE: Values can be parsed with device_getErrorString()
 */
int iso8583_unpackMessage(IN const DL_ISO8583_HANDLER *ISOHandler, IN const DL_UINT8 *packedData, IN DL_UINT16 packedDataLength, IN_OUT DL_ISO8583_MSG *ISOMessage);

/**
 * Deallocate the ISO8583 message structure's memory.
 *
 * @param ISOMessage - The ISO8583 message structure
 *
 * @return RETURN_CODE: Values can be parsed with device_getErrorString()
 */
int iso8583_freeMessage(IN DL_ISO8583_MSG *ISOMessage);

/**
 * Serialize the message fields into an XML format.
 *
 * @param ISOHandler - The ISO8583 handler
 * @param ISOMessage - The ISO8583 message structure
 * @param serializedMessage - The XML-formatted message
 * @param serializedMessageLength - The XML message's length
 *
 * @return RETURN_CODE: Values can be parsed with device_getErrorString()
 */
int iso8583_serializeToXML(IN DL_ISO8583_HANDLER *ISOHandler, IN DL_ISO8583_MSG *ISOMessage, OUT BYTE *serializedMessage, OUT int *serializedMessageLength);

/**
 * Deserialize the XML-formatted ISO8583 message.
 *
 * @param serializedMessage - The XML-formatted message
 * @param serializedMessageLength - The length of the XML-formatted message
 * @param ISOHandler - A null ISO8583 handler
 * @param ISOMessage - The ISO8583 message structure
 *
 * @return RETURN_CODE: Values can be parsed with device_getErrorString()
 */
int iso8583_deserializeFromXML(IN BYTE *serializedMessage, IN int serializedMessageLength, OUT DL_ISO8583_HANDLER *ISOHandler, OUT DL_ISO8583_MSG *ISOMessage);

/**
 * Display the messages in a formatted manner on the screen for verifying results.
 *
 * @param ISOHandler - The ISO8583 handler
 * @param ISOMessage - The ISO8583 message structure
 *
 * @return RETURN_CODE: Values can be parsed with device_getErrorString()
 */
int iso8583_displayMessage(IN DL_ISO8583_HANDLER *ISOHandler, IN DL_ISO8583_MSG *ISOMessage);

#endif /* LIBIDT_ISO8583_H_ */
