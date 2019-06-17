#ifndef LIBIDT_TLV_H_
#define LIBIDT_TLV_H_

#include "IDTDef.h"

/**
 * Split a IDTech-TLV stream into 3 BER-TLV streams - unencrypted, encrypted, and masked.
 *
 * @param data - input data array containing the IDTech-TLV stream
 * @param startPos - starting index position of the IDTech-TLV stream
 * @param tlvLen - maximum length of the IDTech-TLV stream
 *
 * @param unencTLV - output unencrypted BER-TLV stream
 * @param unencLen - input and output length of the unencrypted BER-TLV stream
 * @param encTLV - output encrypted BER-TLV stream
 * @param encLen - input and output length of the encrypted BER-TLV stream
 * @param maskTLV - output masked BER-TLV stream
 * @param maskLen - input and output length of the masked BER-TLV stream
 * @return 0 if successful, 1 if unsuccessful
 */
int tlv_processIDTTLV(IN BYTE* data, IN int startPos, IN int tlvLen, OUT BYTE* unencTLV, OUT int* unencLen, OUT BYTE* encTLV, OUT int* encLen, OUT BYTE* maskTLV, OUT int* maskLen);

/**
 * Sort a BER-TLV stream based on the tags from low to high.
 *
 * @param data - BER-TLV stream
 * @param tlvLen - length of the BER-TLV stream
 * @param recursive - 1= Sorted nested TLV streams within compound tags, 0= only sort top level
 *
 * @return 0 if sort was successful, 1 if unsuccessful
 */
int tlv_sortTLV(IN_OUT BYTE* data, IN int tlvLen, IN int recursive);

/**
 * Get the value of a tag in the provided BER-TLV stream. Top level only, no retrieval within compound tags.
 * The TLV values in a compound tag should be submitted as a separate inquiry to Get Tag Value.
 *
 * @param data - BER-TLV stream
 * @param tlvLen - length of the BER-TLV stream
 * @param tag - the input tag for the value
 * @param tagLen � the length of the input tag
 *
 * @param value - output value
 * @param valueLen - length of output value
 * @param recursive � 1= Search nested TLV streams within compound tags, 0= only search top level
 *
 * @return 0 if successful, 1 if unsuccessful
 *
 */
int tlv_getTagValue(IN BYTE* data, IN int tlvLen, IN BYTE* tag, IN int tagLen, OUT BYTE* value, OUT int* valueLen);

/**
 * Append a BER-TLV stream to another BER-TLV stream.
 *
 * @param data - main BER-TLV stream that will include the other stream
 * @param tlvLen - length of the main BER-TLV stream
 * @param tlvAdd - other BER-TLV stream that will be added to the main stream
 * @param tlvAddLen - length of the other BER-TLV stream
 * @param overWrite - if 1, tags in other stream will overwrite,
 *   if 0, tags in other stream will be ignored
 *
 * @return 0 if successful, 1 if unsuccessful
 */
int tlv_addTLV(IN_OUT BYTE* data, IN_OUT int* tlvLen, IN BYTE* tlvAdd, IN int tlvAddLen, IN int overWrite);

/**
 * Remove a BER-TLV from a BER-TLV stream.
 *
 * @param data - BER-TLV stream
 * @param tlvLen - length of the BER-TLV stream
 * @param tag - the tag of the BER-TLV to remove
 * @param tagLen � the length of the input tag
 * @param recursive � 1= Search nested TLV streams within compound tags, 0= only search top level
 *
 * @return 0 if tag removed, else 1
 */
int tlv_removeTLV(IN_OUT BYTE* data, IN_OUT int* tlvLen, IN BYTE* tag, IN int tagLen, IN int recursive);

/**
 * Count the number of BER-TLV in a BER-TLV stream.
 *
 * @param data - BER-TLV stream
 * @param tlvLen - length of the BER-TLV stream
 * @param numTags � Number of tags in the valid BER-TLV stream.
 *
 * @return 0 if successful, 1 if unsuccessful
 */
int tlv_countTLV(IN BYTE* data, IN int tlvLen, OUT int* numTags);

/**
 *  Validates the data as a complete TLV stream (BER-TLV or IDTech Enhanced TLV).
 *
 * @param data - TLV stream
 * @param tlvLen - length of the BER-TLV stream
 * @param recursive � 1= Validate nested TLV streams within compound tags, 0= only validate top level
 *
 * @return 0 if valid TLV stream, else 1
 */
int tlv_validateTLV(IN BYTE* data, IN int tlvLen, IN int recursive);

#endif /* LIBIDT_TLV_H_ */

/*! \file libIDT_TLV.h
 \brief TLV Handling Library.

 TLV Library API methods.
 */

/*! \def IN
  INPUT parameter.
 */

/*! \def OUT
  OUTPUT parameter.
 */

/*! \def IN_OUT
  INPUT / OUTPUT PARAMETER.
 */

/*! \def _DATA_BUF_LEN
 DATA BUFFER LENGTH
 */
