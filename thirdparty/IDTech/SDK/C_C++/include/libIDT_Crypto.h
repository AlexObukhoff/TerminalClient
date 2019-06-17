#ifndef LIBCRYPTO_H_
#define LIBCRYPTO_H_

#include "IDTDef.h"

/*
 * Sets cipherSettings structure to default values
 * Default values when initialized:
 * - initial vector with all 0x00
 * - CBC mode
 * - PKCS#7 padding
 */
void crypto_initCipherSettings(OUT IDTCipherSettings* cipherSettings);

/*
 * Encrypt && Decrypt using AES
 *
 * @param key - input key, must be 16, 24, 32 bytes
 * @param keyLen - length of key
 * @param input - input data
 * @param inputLen - length of input data
 * @param output - output data
 * @param outputLen - lengh of output data
 * @param cipherSettings - optional cipherSettings, Set to NULL for default (iv is all 0x00, CBC mode, PKCS#7 padding)
 * @param encrypt - 1 for encrypt, 0 for decrypt
 *
 * @return 0 for success, 1 for fail
 */
int crypto_AES(IN BYTE* key, IN int keyLen, IN BYTE* input, IN int inputLen, OUT BYTE* output, OUT int* outputLen, IN IDTCipherSettings* cipherSettings, IN int encrypt);

/*
 * Encrypt using AES
 *
 * @param key - input key, must be 16, 24, 32 bytes
 * @param keyLen - length of key
 * @param input - input data to be encrypted
 * @param inputLen - length of input data
 * @param output - output encrypted data
 * @param outputLen - lengh of output encrypted data
 * @param cipherSettings - optional cipherSettings, Set to NULL for default (iv is all 0x00, CBC mode, PKCS#7 padding)
 *
 * @return 0 for success, 1 for fail
 */
int crypto_encryptAES(IN BYTE* key, IN int keyLen, IN BYTE* input, IN int inputLen, OUT BYTE* output, OUT int* outputLen, IN IDTCipherSettings* cipherSettings);

/*
 * Decrypt using AES
 *
 * @param key - input key, must be 16, 24, 32 bytes
 * @param keyLen - length of key
 * @param input - input data to be decrypted
 * @param inputLen - length of input data
 * @param output - output decrypted data
 * @param outputLen - lengh of output decrypted data
 * @param cipherSettings - optional cipherSettings, Set to NULL for default (iv is all 0x00, CBC mode, PKCS#7 padding)
 *
 * @return 0 for success, 1 for fail
 */
int crypto_decryptAES(IN BYTE* key, IN int keyLen, IN BYTE* input, IN int inputLen, OUT BYTE* output, OUT int* outputLen, IN IDTCipherSettings* cipherSettings);

/*
 * Encrypt & Decrypt using Triple DES
 *
 * @param key - input key, must be 16 or 24 bytes
 * @param keyLen - length of key
 * @param input - input data
 * @param inputLen - length of input data
 * @param output - output data
 * @param outputLen - lengh of output data
 * @param cipherSettings - optional cipherSettings, Set to NULL for default (iv is all 0x00, CBC mode, PKCS#7 padding)
 * @param encrypt - 1 for encrypt, 0 for decrypt
 *
 * @return 0 for success, 1 for fail
 */
int crypto_TDES(IN BYTE* key, IN int keyLen, IN BYTE* input, IN int inputLen, OUT BYTE* output, OUT int* outputLen, IN IDTCipherSettings* cipherSettings, IN int encrypt);

/*
 * Encrypt using Triple DES
 *
 * @param key - input key, must be 16 or 24 bytes
 * @param keyLen - length of key
 * @param input - input data to be encrypted
 * @param inputLen - length of input data
 * @param output - output encrypted data
 * @param outputLen - lengh of output encrypted data
 * @param cipherSettings - optional cipherSettings, Set to NULL for default (iv is all 0x00, CBC mode, PKCS#7 padding)
 *
 * @return 0 for success, 1 for fail
 */
int crypto_encryptTDES(IN BYTE* key, IN int keyLen, IN BYTE* input, IN int inputLen, OUT BYTE* output, OUT int* outputLen, IN IDTCipherSettings* cipherSettings);

/*
 * Decrypt using Triple DES
 *
 * @param key - input key, must be 16 or 24 bytes
 * @param keyLen - length of key
 * @param input - input data to be decrypted
 * @param inputLen - length of input data
 * @param output - output decrypted data
 * @param outputLen - lengh of output decrypted data
 * @param cipherSettings - optional cipherSettings, Set to NULL for default (iv is all 0x00, CBC mode, PKCS#7 padding)
 *
 * @return 0 for success, 1 for fail
 */
int crypto_decryptTDES(IN BYTE* key, IN int keyLen, IN BYTE* input, IN int inputLen, OUT BYTE* output, OUT int* outputLen, IN IDTCipherSettings* cipherSettings);

/*
 * Reads from Certificate / Key files and output content as a BYTE Array
 *
 * @param filename - file name of the certificate / keys
 * @param key - output key
 * @param keyLen - length of output key
 *  NOTE: keyLen MUST be initialized as the BYTE array size first when passed in.
 *        It is checked first to prevent memory leaks.
 *
 * @return 0 for success, 1 for fail
 */
int crypto_fileToByteArray(IN char* filename, OUT BYTE* key, IN_OUT int* keyLen);

/*
 * Encrypt using RSA Public Key / Certificate
 *
 * @param key - RSA Public Key / Certificate
 * @param keyLen - length of RSA Public Key / Certificate
 * @param password - optional password if using PKCS12 Certificate, NULL if no password
 * @param input - input data
 * @param inputLen - length of input data
 * @param output - output data
 * @param outputLen - lengh of output data
 *
 * Compatible Keys and Certificates:
 * RSA public/private Keys (File types: .key, .pem)
 * PEM/DER encoded Certificate (File types: .pem, .der, .cer, .crt)
 * PEM/DER PKCS7 Certificate (File types: .p7b, .p7c)
 * PKCS12 Certificate (File types: .p12, .pfx)
 *
 * @return 0 for success, 1 for fail
 */
int crypto_encryptRSA(IN BYTE* key, IN int keyLen, IN char* password, IN BYTE* input, IN int inputLen, OUT BYTE* output, OUT int* outputLen);

/*
 * Decrypt using RSA Private Key / Certificate
 *
 * @param key - RSA Private Key / Certificate
 * @param keyLen - length of RSA Private Key / Certificate
 * @param password - optional password if using PKCS12 Certificate
 * @param input - input data
 * @param inputLen - length of input data
 * @param output - output data
 * @param outputLen - lengh of output data
 *
 * Compatible Keys and Certificates:
 * RSA private Keys (File types: .key, .pem)
 * PKCS12 Certificate (File types: .p12, .pfx)
 *
 * @return 0 for success, 1 for fail
 */
int crypto_decryptRSA(IN BYTE* key, IN int keyLen, IN char* password, IN BYTE* input, IN int inputLen, OUT BYTE* output, OUT int* outputLen);

/*
 * Sign using RSA Private Key / Certificate
 *
 * @param key - RSA Private Key / Certificate
 * @param keyLen - length of RSA Private Key / Certificate
 * @param password - optional password if using PKCS12 Certificate
 * @param message - message data
 * @param messageLen - length of message data
 * @param sig - signature data
 * @param sigLen - lengh of signature data
 *
 * Compatible Keys and Certificates:
 * RSA private Keys (File types: .key, .pem)
 * PKCS12 Certificate (File types: .p12, .pfx)
 *
 * @return 0 for success, 1 for fail
 */
int crypto_signRSA(IN BYTE* key, IN int keyLen, IN char* password, IN BYTE* message, IN int messageLen, OUT BYTE* sig, OUT int* sigLen);

/*
 * Validate using RSA Public Key / Certificate
 *
 * @param key - RSA Public Key / Certificate
 * @param keyLen - length of RSA Public Key / Certificate
 * @param password - optional password if using PKCS12 Certificate
 * @param message - message data
 * @param messageLen - length of message data
 * @param sig - signature data
 * @param sigLen - lengh of signature data
 * @param validated - 1 if message is valid, 0 if not valid
 *
 * Compatible Keys and Certificates:
 * RSA public/private Keys (File types: .key, .pem)
 * PEM/DER encoded Certificate (File types: .pem, .der, .cer, .crt)
 * PEM/DER PKCS7 Certificate (File types: .p7b, .p7c)
 * PKCS12 Certificate (File types: .p12, .pfx)
 *
 * @return 0 for success, 1 for fail
 */
int crypto_validateRSA(IN BYTE* key, IN int keyLen, IN char* password, IN BYTE* message, IN int messageLen, IN BYTE* sig, IN int sigLen, OUT int* validated);

/*
 * Hash a BYTE array
 *
 * @param plain - BYTE array to be hashed
 * @param plen - length of BYTE array
 * @param hashType - 0 for SHA1, 1 for SHA256
 * @param hashed - output hashed data
 * @param hlen - length of output hashed data
 *
 * @return 0 for success, 1 for fail
 */
int crypto_hashSHA(IN BYTE* plain, IN int plen, IN int hashType, OUT BYTE* hashed, OUT unsigned int* hlen);

/*
 * get HMAC from MAC Key and message
 *
 * @param macKey - 16 bytes input MAC key to hash the message
 * @param hashType - 0 for SHA1, 1 for SHA256
 * @param input - input message data
 * @param inputLen - length of input message data
 * @param mac - output HMAC data
 * @param macLen - length of output HMAC data
 *
 * @return 0 for success, 1 for fail
 */
int crypto_getHMAC(IN BYTE* macKey, IN int hashType, IN BYTE* input, IN int inputLen, OUT BYTE* mac, OUT int* macLen);

/*
 * Get IPEK from BDK and KSN
 *
 * @param bdk - input bdk, must be 16 or 24 bytes
 * @param bdkLen - length of bdk
 * @param ksn - input ksn, must be 10 bytes
 * @param ipek - 16 bytes output ipek
 *
 * @return 0 for success, 1 for fail
 */
int crypto_getIPEK(IN BYTE* bdk, IN int bdkLen, IN BYTE* ksn, OUT BYTE* ipek);

/*
 * Get derived key from IPEK and KSN
 *
 * @param ipek - input ipek, must be 16 bytes
 * @param ksn - input ksn, must be 10 bytes
 * @param ipek - 16 bytes output derived key
 */
void crypto_getDerivedKey(IN BYTE* ipek, IN BYTE* ksn, OUT BYTE* derivedKey);

/*
 * Get data key variant from derived key
 *
 * @param derivedKey - input derived key, must be 16 bytes
 * @param dataKey - 16 bytes output data key
 */
void crypto_getDataKey(IN BYTE* derivedKey, OUT BYTE* dataKey);

/*
 * Get PIN key variant from derived key
 *
 * @param derivedKey - input derived key, must be 16 bytes
 * @param pinKey - 16 bytes output PIN key
 */
void crypto_getPINKey(IN BYTE* derivedKey, OUT BYTE* pinKey);

/*
 * Get MAC key variant from derived key
 *
 * @param derivedKey - input derived key, must be 16 bytes
 * @param macKey - 16 bytes output MAC key
 */
void crypto_getMACKey(IN BYTE* derivedKey, OUT BYTE* macKey);

/*
 * Encrypt PIN number using Triple DES
 *
 * @param key - input key, must be 16 or 24 bytes
 * @param keyLen - length of key
 * @param pinBlockType - type 0 (pads with 0xf) or type 3 (pads with random value from 0x0 to 0xf)
 * @param pin - input PIN number to encrypt, must be between 1 - 14 numbers
 * @param pan - input PAN number, must be at least 13 numbers
 * @param pinBlock - output encrypted PIN block
 * @param pinBlockLen - length of output encrypted PIN block
 *
 * @return 0 for success, 1 for fail
 */
int crypto_encryptPinTDES(IN BYTE* key, IN int keyLen, IN int pinBlockType, IN char* pin, IN char* pan, OUT BYTE* pinBlock, OUT int* pinBlockLen);

/*
 * Encrypt PIN number using AES
 *
 * @param key - input key, must be 16, 24, or 32 bytes
 * @param keyLen - length of key
 * @param pin - input PIN number to encrypt, must be between 1 - 14 numbers
 * @param pan - input PAN number, must be at least 13 numbers
 * @param pinBlock - output encrypted PIN block
 * @param pinBlockLen - length of output encrypted PIN block
 *
 * @return 0 for success, 1 for fail
 */
int crypto_encryptPinAES(IN BYTE* key, IN int keyLen, IN char* pin, IN char* pan, OUT BYTE* pinBlock, OUT int* pinBlockLen);

/*
 * Decrypt PIN number using Triple DES
 *
 * @param key - input key, must be 16 or 24 bytes
 * @param keyLen - length of key
 * @param pinBlock - input encrypted PIN block
 * @param pinBlockLen - length of input encrypted PIN block
 * @param pan - input PAN number, must be at least 13 numbers
 * @param pin - output PIN number
 *
 * @return 0 for success, 1 for fail
 */
int crypto_decryptPinTDES(IN BYTE* key, IN int keyLen, IN BYTE* pinBlock, IN int pinBlockLen, IN char* pan, OUT char* pin);

/*
 * Decrypt PIN number using AES
 *
 * @param key - input key, must be 16, 24, or 32 bytes
 * @param keyLen - length of key
 * @param pinBlock - input encrypted PIN block
 * @param pinBlockLen - length of input encrypted PIN block
 * @param pan - input PAN number, must be at least 13 numbers
 * @param pin - output PIN number
 *
 * @return 0 for success, 1 for fail
 */
int crypto_decryptPinAES(IN BYTE* key, IN int keyLen, IN BYTE* pinBlock, IN int pinBlockLen, IN char* pan, OUT char* pin);

/*
 * Process DUKPT related functions
 *
 * @param IDTCryptoData - struct to organize all inputs and outputs
 * @param cipherSettings - optional cipherSettings, Set to NULL for default (iv is all 0x00, CBC mode, PKCS#7 padding)
 *
 * @return 0 for success, 1 for fail
 *
 * Variables in IDTCryptoData:
 * ---------------------------------------------
 * key (required) - contains either BDK (TDES: 16 or 24 bytes, AES: 16, 24, or 32 bytes) or IPEK (16 bytes)
 * keyLen (required) - either BDK or IPEK Length
 * ksn (required) - contains ksn (10 bytes)
 * isIPEK (required) - 1 if key varible is IPEK, 0 for BDK
 * isDecryption (required) - 1 for decryption, 0 for encryption
 * isTDES (required) - 1 for TDES, 0 for AES
 * isPin (required) - 1 for processing PIN numbers, 0 for other data
 * keyVariant (required when not processing PIN numbers) - 0 for Data, 1 for PIN, and 2 for MAC
 * pinBlockType (required when using TDES) - type 0 (pads key with 0xf) or type 3 (pads with random value from 0x0 to 0xf)
 * dataToProcess (required expect PIN encryption) - store input data to process
 * dataToProcessLen (required expect PIN encryption) - length of input data to process
 * dataResults (required expect PIN decryption) - store output processed result
 * dataResultsLen (required expect PIN decryption) - length of output processed result
 * pin (required when processing PIN) - stores PIN number for both PIN number encryption and decryption (must be between 1 - 14 numbers)
 * pan (required when processing PIN) - for processing PIN numbers, contains PAN number (must be at least 13 numbers)
 *
 * Usages:
 * ---------------------------------------------
 * BDK/IPEK + KSN + encrypted/decrypted data -> encrypted/decrypted data
 * Options: TDES(pinBlockType 0 or 3)/AES, Data/PIN/MAC
 *
 * BDK/IPEK + KSN + PIN + PAN -> PIN Block
 * BDK/IPEK + KSN + PIN Block + PAN -> PIN
 * Options: TDES(pinBlockType 0 or 3)/AES
 */
int crypto_processDUKPT(IN_OUT IDTCryptoData* data, IN IDTCipherSettings* settings);

#endif /* LIBCRYPTO_H_ */

/*! \file libIDT_Crypto.h
 \brief Crypto Handling Library.

 Crypto Library API methods.
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
