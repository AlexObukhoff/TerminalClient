#ifndef LIBIDT_SECURITY_H_
#define LIBIDT_SECURITY_H_

#include "IDTDef.h"

/*
 * User and Group Permissions
 * This is a reference for the bitmap positions
 * of each function permissions.
 * Permissions go from left to right of the Byte Array.
 *
 * eg. security_removeUser
 *   + security_changePWOther
 *   + security_addUserPermissions
 *   + security_addUserToGroup
 *   = 0x2A, 0x10
 *
 * // First Byte
 * security_createUser                      // 0x80
 * security_createGroup                     // 0x40
 * security_removeUser                      // 0x20
 * security_removeGroup                     // 0x10
 * security_changePWOther                   // 0x08
 * security_setUserPermissions              // 0x04
 * security_addUserPermissions              // 0x02
 * security_removeUserPermissions           // 0x01
 *
 * // Second Byte
 * security_setGroupPermissions             // 0x00, 0x80
 * security_addGroupPermissions             // 0x00, 0x40
 * security_removeGroupPermissions          // 0x00, 0x20
 * security_addUserToGroup                  // 0x00, 0x10
 * security_removeUserFromGroup             // 0x00, 0x08
 * security_createKeyset                    // 0x00, 0x04
 * security_removeKeyset                    // 0x00, 0x02
 *
 * // Third Byte
 * logMgr_enableLogging                     // 0x00, 0x00, 0x80
 * logMgr_disableLogging                    // 0x00, 0x00, 0x40
 * logMgr_retrieveLogEntry                  // 0x00, 0x00, 0x20
 * logMgr_retrieveLogEntryCount             // 0x00, 0x00, 0x10
 * logMgr_retrieveLogEntryCountForLogLevel  // 0x00, 0x00, 0x08
 * logMgr_clearLog                          // 0x00, 0x00, 0x04
 * logMgr_clearLogForLogLevel               // 0x00, 0x00, 0x02
 * logMgr_postLogEntry                      // 0x00, 0x00, 0x01
 *
 * // Fourth Byte
 * logMgr_postLog                           // 0x00, 0x00, 0x00, 0x80
 */

enum SECURITY_RESULT {
    SECURITY_SUCCESS,
    SECURITY_CANNOT_OPEN_DATABASE,
    SECURITY_CANNOT_CLOSE_DATABASE,
    SECURITY_SQLITE_COMMAND_FAILED,
    SECURITY_INVALID_USER_NAME,
    SECURITY_INVALID_GROUP_NAME,
    SECURITY_INCORRECT_NAME_OR_PASSWORD,
    SECURITY_NO_USER_LOGGED_IN,
    SECURITY_USER_IS_LOGGED_IN,
    SECURITY_USER_DOES_NOT_EXIST,
    SECURITY_GROUP_DOES_NOT_EXIST,
    SECURITY_INCORRECT_PASSWORD,
    SECURITY_USER_ALREADY_EXISTS_IN_GROUP,
    SECURITY_USER_DOES_NOT_EXIST_IN_GROUP,
    SECURITY_USER_UNAUTHORIZED_FOR_THIS_FUNCTION,
    SECURITY_FAILED_TO_GENERATE_RSA_KEYS,
    SECURITY_IDENTIFIER_ALREADY_EXISTS,
    SECURITY_IDENTIFIER_DOES_NOT_EXIST,
    SECURITY_INVALID_IDENTIFIER,
    SECURITY_NOT_ENOUGH_BUFFER,
    SECURITY_CRYPTO_FAILED,
    SECURITY_IDG_COMMAND_FAILED,
    SECURITY_INVALID_SLOT,
    SECURITY_CANNOT_OPEN_CERTIFICATE_FOLDER,
    SECURITY_CANNOT_FIND_CERTIFICATE,
    SECURITY_FAILED_TO_DELETE_CERTIFICATE,
    SECURITY_FAILED_TO_READ_CERTIFICATE,
    SECURITY_FAILED_TO_CREATE_CERTIFICATE,
    SECURITY_FAILED_TO_WRITE_CERTIFICATE,
    SECURITY_CERTIFICATE_ALREADY_EXISTS,
    SECURITY_MALLOC_ALLOCATION_FAILED,
    SECURITY_COMMAND_NOT_AVALIABLE
};

/**
 * Retrieves a listing of all the device key certificates
 *
 * @param numCerts – The number of certificates retrieved
 * @param certs – Pointer to certificate list. Each certificate will return name/serial number as
 *  strings separated by Field Separators (0x1C). Must free variable when done.
 *  Example, if 3 certs are returned:
 *  {NAME1}<FS>{SN1}<FS>{NAME2}<FS>{{SN2}<FS>{{NAME3}<FS>{{SN3}\0
 *
 * @return SECURITY_COMMAND_NOT_AVALIABLE
 */
int security_retrieveKeyCertList(OUT int *numCerts, OUT char **certs);

/**
 * Retrieves info of a device key certificate
 *
 * @param serial – The serial number as a null terminated string
 * @param cert – Pointer to certificate Info. Must free variable when done.
 *
 * @return SECURITY_COMMAND_NOT_AVALIABLE
 */
int security_retrieveKeyCertInfo(IN char *serial, OUT char **cert);

/**
 * Retrieves a device key certificate
 *
 * @param serial – The serial number as a null terminated string
 * @param buffer – Certificate
 * @param bufferLen – Length of data in buffer
 *
 * @return SECURITY_COMMAND_NOT_AVALIABLE
 */
int security_retrieveKeyCert(IN char *serial, OUT BYTE *buffer, OUT int *bufferLen);

/**
 * Checks and returns the state of the DUKPT key associated with each slot
 *
 * @param buffer – DUKPT Key Slot info, 4 bytes used for each index/slot.
 *      byte 0 = Key Index Number
      byte 1-2 = Key slot number
        byte 3 = Key state
            00 = Unused
            01 = Valid
            02 = End of life
            FF = Not Available
 * @param bufferLen – Length of data in buffer. Example, if 20 bytes, then 5 key are reported
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_IDG_COMMAND_FAILED
 */
int security_checkDUKPTKeys(OUT BYTE *buffer, OUT int *bufferLen);

/**
 * This command checks whether a valid DUKPT key is stored at the specified index and
 * slot and if a valid key is found then some basic information related to the type of
 * key is returned. The actual Key data is never returned.
 *
 * @param index The key index
 * @param slot The key slot
 * @param buffer – DUKPT Key Info, 5 bytes if valid key, otherwise only first byte returned
        byte 0 = Key state
            00 = Unused
            01 = Valid
            02 = End of life
            FF = Not Available
        byte 1-2 = Key Usage
            ‘K0’ = Key Encryption or Wrapping
            ‘P0’ = PIN Encryption
            ‘D0’ = Data Encryption
            ‘M0’ = MAC Verification
            ‘B1’ = IPEK
        byte 3 = Algorithm
           ‘T’ = TDES
           ‘D’ = DES
           ‘A’ = AES
        byte 4 = Mode of Use
           ‘N’ = No special restrictions
           ‘E’ = Encryption Only
           ‘D’ = Decryption Only
           ‘B’ = Both Encryption and Decryption
 * @param bufferLen – Length of data in buffer. 1 or 5 bytes
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_IDG_COMMAND_FAILED
 */
int security_getDUKPTKeyInfo(IN int index, IN int slot, OUT BYTE *buffer, OUT int *bufferLen);

/**
 * Sets the slot to use for PIN encryption
 *
 * @param slot The key slot
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_INVALID_SLOT
 *  ERROR, SECURITY_IDG_COMMAND_FAILED
 */
int security_setPINKey(IN int slot);

/**
 * Gets the slot used for PIN encryption
 *
 * @param slot The key slot
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_INVALID_SLOT
 *  ERROR, SECURITY_IDG_COMMAND_FAILED
 */
int security_getPINKey(OUT int *slot);

/**
 * Sets the slot to use for Data encryption
 *
 * @param slot The key slot
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_INVALID_SLOT
 *  ERROR, SECURITY_IDG_COMMAND_FAILED
 */
int security_setDataKey(IN int slot);

/**
 * Gets the slot used for Data encryption
 *
 * @param slot The key slot
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_INVALID_SLOT
 *  ERROR, SECURITY_IDG_COMMAND_FAILED
 */
int security_getDataKey(OUT int *slot);

/**
 * This will have the Security Manager generate a public/private key pair for PKI and a
 * KSN/BDK for DUKPT functionality and associate it with a provided identifier.
 * Required permission bit position: 0x00, 0x04
 *
 * @param identifier – Unique identifier, such as application GUID
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 *  ERROR, SECURITY_NO_USER_LOGGED_IN
 *  ERROR, SECURITY_USER_UNAUTHORIZED_FOR_THIS_FUNCTION
 *  ERROR, SECURITY_INVALID_IDENTIFIER
 *  ERROR, SECURITY_IDENTIFIER_ALREADY_EXISTS
 *  ERROR, SECURITY_FAILED_TO_GENERATE_RSA_KEYS
 */
int security_createKeyset(OUT char *identifier);

/**
 * This will remove the public/private keys and BDK/KSN for a provided identifier.
 * Required permission bit position: 0x00, 0x02
 *
 * @param identifier – Unique identifier, such as application GUID
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 *  ERROR, SECURITY_NO_USER_LOGGED_IN
 *  ERROR, SECURITY_USER_UNAUTHORIZED_FOR_THIS_FUNCTION
 *  ERROR, SECURITY_INVALID_IDENTIFIER
 *  ERROR, SECURITY_IDENTIFIER_DOES_NOT_EXIST
 */
int security_removeKeyset(IN char *identifier);

/**
 * Retrieves a device key certificate
 *
 * @param identifier – Keyset identifier
 * @param buffer – Public Key
 * @param bufferLen – Length of data in buffer
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 *  ERROR, SECURITY_INVALID_IDENTIFIER
 *  ERROR, SECURITY_NOT_ENOUGH_BUFFER
 *  ERROR, SECURITY_IDENTIFIER_DOES_NOT_EXIST
 */
int security_retrieveKeysetPKI(IN char *identifier, OUT BYTE *buffer, OUT int *bufferLen);

/**
 * Decrypts data using a Keyset Private Key
 *
 * @param identifier – Keyset identifier
 * @param data – encrypted data
 * @param dataLen – encrypted data length
 * @param buffer – decrypted data
 * @param bufferLen – Length of data in buffer
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 *  ERROR, SECURITY_INVALID_IDENTIFIER
 *  ERROR, SECURITY_CRYPTO_FAILED
 *  ERROR, SECURITY_IDENTIFIER_DOES_NOT_EXIST
 */
int security_decryptKeysetPKI(IN char *identifier, IN BYTE *data, IN int dataLen, OUT BYTE *buffer, OUT int *bufferLen);

/**
 * Signs data using a Keyset Private Key
 *
 * @param identifier – Keyset identifier
 * @param data – data to sign
 * @param dataLen – data to sign length
 * @param buffer – signature
 * @param bufferLen – Length of signature data
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 *  ERROR, SECURITY_INVALID_IDENTIFIER
 *  ERROR, SECURITY_CRYPTO_FAILED
 *  ERROR, SECURITY_IDENTIFIER_DOES_NOT_EXIST
 */
int security_signKeysetPKI(IN char *identifier, IN BYTE *data, IN int dataLen, OUT BYTE *buffer, OUT int *bufferLen);

/**
 * Encrypt buffer data using a Keyset Private Key
 *
 * @param identifier – Keyset identifier
 * @param data – data to encrypt
 * @param dataLen –data to encrypt length
 * @param buffer – encrypted data
 * @param bufferLen – Length of encrypted data
 * @param cipherSettings - encryption settings, null for default
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 *  ERROR, SECURITY_INVALID_IDENTIFIER
 *  ERROR, SECURITY_CRYPTO_FAILED
 *  ERROR, SECURITY_IDENTIFIER_DOES_NOT_EXIST
 */
int security_encryptKeysetBuffer(IN char *identifier, IN BYTE *data, IN int dataLen, OUT BYTE *buffer, OUT int *bufferLen, IN IDTCipherSettings* cipherSettings);

/**
 * Decrypts buffer data using a Keyset Private Key
 *
 * @param identifier – Keyset identifier
 * @param data – encrypted data
 * @param dataLen –data to decrypt length
 * @param buffer – decrypted data
 * @param bufferLen – Length of decrypted data
 * @param cipherSettings - decryption settings, null for default
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 *  ERROR, SECURITY_INVALID_IDENTIFIER
 *  ERROR, SECURITY_CRYPTO_FAILED
 *  ERROR, SECURITY_IDENTIFIER_DOES_NOT_EXIST
 */
int security_decryptKeysetBuffer(IN char *identifier, IN BYTE *data, IN int dataLen, OUT BYTE *buffer, OUT int *bufferLen, IN IDTCipherSettings* cipherSettings);

/**
 * Encrypt data using a Keyset Private Key
 *
 * @param identifier – Keyset identifier
 * @param data – data to encrypt
 * @param dataLen –data to encrypt length
 * @param variant – 0 = DATA, 1 = PIN, 2 = MAC
 * @param KSN – Key Serial Number 10 bytes
 * @param increment – 0 = increment KSN, 1 = don’t increment KSN
 * @param buffer – encrypted data
 * @param bufferLen – Length of encrypted data
 * @param cipherSettings - encryption settings, null for default
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 *  ERROR, SECURITY_INVALID_IDENTIFIER
 *  ERROR, SECURITY_CRYPTO_FAILED
 *  ERROR, SECURITY_NOT_ENOUGH_BUFFER
 *  ERROR, SECURITY_IDENTIFIER_DOES_NOT_EXIST
 */
int security_encryptKeysetDUKPT(IN char *identifier, IN BYTE *data, IN int dataLen, IN int variant, OUT BYTE *KSN, IN int increment, OUT BYTE *buffer, OUT int *bufferLen, IN IDTCipherSettings* cipherSettings);

/**
 * Decrypts data using a Keyset Private Key
 *
 * @param identifier – Keyset identifier
 * @param data – encrypted data
 * @param dataLen –data to decrypt length
 * @param variant – 0 = DATA, 1 = PIN, 2 = MAC
 * @param KSN – Key Serial Number 10 bytes
 * @param buffer – decrypted data
 * @param bufferLen – Length of decrypted data
 * @param cipherSettings - decryption settings, null for default
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 *  ERROR, SECURITY_INVALID_IDENTIFIER
 *  ERROR, SECURITY_CRYPTO_FAILED
 *  ERROR, SECURITY_NOT_ENOUGH_BUFFER
 *  ERROR, SECURITY_IDENTIFIER_DOES_NOT_EXIST
 */
int security_decryptKeysetDUKPT(IN char *identifier, IN BYTE *data, IN int dataLen, IN int variant, IN BYTE *KSN, OUT BYTE *buffer, OUT int *bufferLen, IN IDTCipherSettings* cipherSettings);

/**
 * Attemps to log in user
 *
 * @param name – user name
 * @param password – user password
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 *  ERROR, SECURITY_INVALID_USER_NAME
 *  ERROR, SECURITY_INCORRECT_NAME_OR_PASSWORD
 */
int security_login(IN char *name, IN char *password);

/**
 * Attemps to logout current user
 *
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_NO_USER_LOGGED_IN
 */
int security_logout();

/**
 * Returns the name of the current user logged in
 *
 * @param name – name of current user
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 *  ERROR, SECURITY_NO_USER_LOGGED_IN
 */
int security_getLoggedUserName(OUT char *name);

/**
 * Returns the permissions of the current user logged in
 *
 * @param permissions – permissions of the current user
 * @param permissionsLen – Length of permissions
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 *  ERROR, SECURITY_NO_USER_LOGGED_IN
 */
int security_getLoggedUserPerm(OUT BYTE *permissions, OUT int *permissionsLen);

/**
 * Validates a users permission against provided permissions
 *
 * @param permission – permission to validate against the current user
 * @param permissionLen – Length of permissions
 * @param result – 0 = User has permission, 1 = User does not have permission
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 *  ERROR, SECURITY_NO_USER_LOGGED_IN
 */
int security_validateLoggedUserPerm(IN BYTE *permission, IN int permissionLen, OUT int *result);

/**
 * Create a new user
 * Required permission bit position: 0x80
 *
 * @param name – user name
 * @param password – user password
 * @param permission – user permissions
 * @param permissionLen – Length of permissions
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 *  ERROR, SECURITY_NO_USER_LOGGED_IN
 *  ERROR, SECURITY_USER_UNAUTHORIZED_FOR_THIS_FUNCTION
 *  ERROR, SECURITY_INVALID_USER_NAME
 */
int security_createUser(IN char *name, IN char *password, IN BYTE *permission, IN int permissionLen);

/**
 * Create a new group
 * Required permission bit position: 0x40
 *
 * @param name – group name
 * @param permission – group permissions
 * @param permissionLen – Length of permissions
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 *  ERROR, SECURITY_NO_USER_LOGGED_IN
 *  ERROR, SECURITY_USER_UNAUTHORIZED_FOR_THIS_FUNCTION
 *  ERROR, SECURITY_INVALID_GROUP_NAME
 */
int security_createGroup(IN char *name, IN BYTE *permission, IN int permissionLen);

/**
 * Remove user. User “admin” cannot be removed.
 * Required permission bit position: 0x20
 *
 * @param name – user name
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 *  ERROR, SECURITY_NO_USER_LOGGED_IN
 *  ERROR, SECURITY_USER_UNAUTHORIZED_FOR_THIS_FUNCTION
 *  ERROR, SECURITY_INVALID_USER_NAME
 *  ERROR, SECURITY_USER_IS_LOGGED_IN
 *  ERROR, SECURITY_USER_DOES_NOT_EXIST
 */
int security_removeUser(IN char *name);

/**
 * Remove group. Group “Administrators” cannot be removed.
 * Required permission bit position: 0x10
 *
 * @param name – group name
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 *  ERROR, SECURITY_NO_USER_LOGGED_IN
 *  ERROR, SECURITY_USER_UNAUTHORIZED_FOR_THIS_FUNCTION
 *  ERROR, SECURITY_INVALID_GROUP_NAME
 *  ERROR, SECURITY_GROUP_DOES_NOT_EXIST
 */
int security_removeGroup(IN char *name);

/**
 * List All Users
 *
 * @param users – User names, separated by Field Separators (0x1C): {name1}<FS>{name2}<FS>…{name_n}\0
*
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 */
int security_listAllUsers(OUT char *users);

/**
 * List All Groups
 *
 * @param groups – Groups, separated by Field Separators (0x1C): {group1}<FS>{group2}<FS>…{name_n}\0
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 */
int security_listAllGroups(OUT char *groups);

/**
 * List All Users in a Group
 *
 * @param group – Group name
 * @param users – User names, separated by Field Separators (0x1C): {name1}<FS>{name2}<FS>…{name_n}\0
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 */
int security_listAllGroupUsers(IN char *group, OUT char *users);

/**
 * List All Groups for a User
 *
 * @param user – User name
 * @param groups – Groups, separated by Field Separators (0x1C): {group1}<FS>{group2}<FS>…{name_n}\0
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 */
int security_listAllUserGroups(IN char *user, OUT char *groups);

/**
 * Change the password for another user not currently logged in.
 * Required permission bit position: 0x08
 *
 * @param name – user name
 * @param old_password – old password
 * @param new_password – new password
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 *  ERROR, SECURITY_NO_USER_LOGGED_IN
 *  ERROR, SECURITY_USER_UNAUTHORIZED_FOR_THIS_FUNCTION
 *  ERROR, SECURITY_INVALID_USER_NAME
 *  ERROR, SECURITY_INCORRECT_NAME_OR_PASSWORD
 */
int security_changePWOther(IN char *name, IN char *old_password, IN char *new_password);

/**
 * Changes the password for the user currently logged in.
 *
 * @param old_password – old password
 * @param new_password – new password
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 *  ERROR, SECURITY_NO_USER_LOGGED_IN
 *  ERROR, SECURITY_INCORRECT_PASSWORD
 */
int security_changePW(IN char *old_password, IN char *new_password);

/**
 * Sets/replaces the permissions for a user.
 * Required permission bit position: 0x04
 *
 * @param name – user name
 * @param permissions – user permissions
 * @param permissionsLen – Length of permissions
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 *  ERROR, SECURITY_NO_USER_LOGGED_IN
 *  ERROR, SECURITY_USER_UNAUTHORIZED_FOR_THIS_FUNCTION
 *  ERROR, SECURITY_INVALID_USER_NAME
 */
int security_setUserPermissions(IN char *name, IN BYTE *permissions, IN int permissionsLen);

/**
 * Gets the permissions for a user.
 *
 * @param name – user name
 * @param permissions – user permissions
 * @param permissionsLen – Length of permissions
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 *  ERROR, SECURITY_INVALID_USER_NAME
 */
int security_getUserPermissions(IN char *name, OUT BYTE *permissions, OUT int *permissionsLen);

/**
 * Adds the permission to the users current permissions.
 * Required permission bit position: 0x02
 *
 * @param name – user name
 * @param permissions – permission to add
 * @param permissionsLen – Length of permissions
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 *  ERROR, SECURITY_NO_USER_LOGGED_IN
 *  ERROR, SECURITY_USER_UNAUTHORIZED_FOR_THIS_FUNCTION
 *  ERROR, SECURITY_INVALID_USER_NAME
 */
int security_addUserPermissions(IN char *name, IN BYTE *permissions, IN int permissionsLen);

/**
 * Removes the permission from the users current permissions.
 * Required permission bit position: 0x01
 *
 * @param name – user name
 * @param permissions – permission to remove
 * @param permissionsLen – Length of permissions
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 *  ERROR, SECURITY_NO_USER_LOGGED_IN
 *  ERROR, SECURITY_USER_UNAUTHORIZED_FOR_THIS_FUNCTION
 *  ERROR, SECURITY_INVALID_USER_NAME
 */
int security_removeUserPermissions(IN char *name, IN BYTE *permissions, IN int permissionsLen);

/**
 * Sets/replaces the permissions for a group.
 * Required permission bit position: 0x00, 0x80
 *
 * @param name – group name
 * @param permissions – group permissions
 * @param permissionsLen – Length of permissions
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 *  ERROR, SECURITY_NO_USER_LOGGED_IN
 *  ERROR, SECURITY_USER_UNAUTHORIZED_FOR_THIS_FUNCTION
 *  ERROR, SECURITY_INVALID_GROUP_NAME
 */
int security_setGroupPermissions(IN char *name, IN BYTE *permissions, IN int permissionsLen);

/**
 * Gets the permissions for a group.
 *
 * @param name – group name
 * @param permissions – group permissions
 * @param permissionsLen – Length of permissions
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 *  ERROR, SECURITY_INVALID_GROUP_NAME
 */
int security_getGroupPermissions(IN char *name, OUT BYTE *permissions, OUT int *permissionsLen);

/**
 * Adds the permission to the groups current permissions.
 * Required permission bit position: 0x00, 0x40
 *
 * @param name – group name
 * @param permissions – permission to add
 * @param permissionsLen – Length of permissions
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 *  ERROR, SECURITY_NO_USER_LOGGED_IN
 *  ERROR, SECURITY_USER_UNAUTHORIZED_FOR_THIS_FUNCTION
 *  ERROR, SECURITY_INVALID_GROUP_NAME
 */
int security_addGroupPermissions(IN char *name, IN BYTE *permissions, IN int permissionsLen);

/**
 * Removes the permission from the groups current permissions.
 * Required permission bit position: 0x00, 0x20
 *
 * @param name – group name
 * @param permissions – permission to remove
 * @param permissionsLen – Length of permissions
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 *  ERROR, SECURITY_NO_USER_LOGGED_IN
 *  ERROR, SECURITY_USER_UNAUTHORIZED_FOR_THIS_FUNCTION
 *  ERROR, SECURITY_INVALID_GROUP_NAME
 */
int security_removeGroupPermissions(IN char *name, IN BYTE *permissions, IN int permissionsLen);

/**
 * Adds the user to the group.
 * Required permission bit position: 0x00, 0x10
 *
 * @param user – user name
 * @param group – group name
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 *  ERROR, SECURITY_NO_USER_LOGGED_IN
 *  ERROR, SECURITY_USER_UNAUTHORIZED_FOR_THIS_FUNCTION
 *  ERROR, SECURITY_INVALID_USER_NAME
 *  ERROR, SECURITY_INVALID_GROUP_NAME
 *  ERROR, SECURITY_USER_DOES_NOT_EXIST
 *  ERROR, SECURITY_GROUP_DOES_NOT_EXIST
 *  ERROR, SECURITY_USER_ALREADY_EXISTS_IN_GROUP
 */
int security_addUserToGroup(IN char *user, IN char *group);

/**
 * Removes the user to the group.
 * Required permission bit position: 0x00, 0x08
 *
 * @param user – user name
 * @param group – group name
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_CANNOT_OPEN_DATABASE
 *  ERROR, SECURITY_SQLITE_COMMAND_FAILED
 *  ERROR, SECURITY_NO_USER_LOGGED_IN
 *  ERROR, SECURITY_USER_UNAUTHORIZED_FOR_THIS_FUNCTION
 *  ERROR, SECURITY_INVALID_USER_NAME
 *  ERROR, SECURITY_INVALID_GROUP_NAME
 *  ERROR, SECURITY_USER_DOES_NOT_EXIST
 *  ERROR, SECURITY_GROUP_DOES_NOT_EXIST
 *  ERROR, SECURITY_USER_DOES_NOT_EXIST_IN_GROUP
 */
int security_removeUserFromGroup(IN char *user, IN char *group);

/**
 * Retrieves a listing of all the device communication certificates
 *
 * @param numCerts – The number of certificates retrieved
 * @param certs – Pointer to certificate list. Each certificate will return name/serial number
 *      with field separators (0x1C), as a null terminated string. Must free variable when done.
 * 	Example, if 3 certs are returned:
 * 		{NAME1}<FS>{SN1}<FS>{NAME2}<FS>{SN2}<FS>{NAME3}<FS>{SN3}\0
 *
 * @return SECURITY_SUCCESS
 *  ERROR, SECURITY_MALLOC_ALLOCATION_FAILED
 *  ERROR, SECURITY_CANNOT_OPEN_CERTIFICATE_FOLDER
 */
int security_retrieveCommCertList(OUT int *numCerts, OUT char **certs);

/**
 * Retrieves info of a device communication certificate
 *
 * @param serial – The serial number as a null terminated string
 * @param cert – Pointer to certificate Info. Must free variable when done.
 *
 * @return SECURITY_SUCCESS
 *   ERROR, SECURITY_CANNOT_OPEN_CERTIFICATE_FOLDER
 *   ERROR, SECURITY_CANNOT_FIND_CERTIFICATE
 */
int security_retrieveCommCertInfo(IN char *serial, OUT char **cert);

/**
 * Retrieves a device communication certificate
 *
 * @param serial – The serial number as a null terminated string
 * @param buffer – Certificate
 * @param bufferLen – Length of data in buffer (max size 4096)
 *
 * @return SECURITY_SUCCESS
 *   ERROR, SECURITY_CANNOT_OPEN_CERTIFICATE_FOLDER
 *   ERROR, SECURITY_CANNOT_FIND_CERTIFICATE
 *   ERROR, SECURITY_FAILED_TO_READ_CERTIFICATE
 */
int security_retrieveCommCert(IN char *serial, OUT BYTE *buffer, OUT int *bufferLen);

/**
 * Adds a device communication certificate
 *
 * @param buffer – Certificate
 * @param bufferLen – Length of data in buffer
 *
 * @return SECURITY_SUCCESS
 *   ERROR, SECURITY_FAILED_TO_READ_CERTIFICATE
 *   ERROR, SECURITY_CERTIFICATE_ALREADY_EXISTS
 *   ERROR, SECURITY_FAILED_TO_CREATE_CERTIFICATE
 *   ERROR, SECURITY_FAILED_TO_WRITE_CERTIFICATE
 */
int security_addCommCert(IN BYTE *buffer, IN int bufferLen);

/**
 * Removes a device communication certificate
 *
 * @param serial – The serial number as a null terminated string
 *
 * @return SECURITY_SUCCESS
 *   ERROR, SECURITY_CANNOT_OPEN_CERTIFICATE_FOLDER
 *   ERROR, SECURITY_CANNOT_FIND_CERTIFICATE
 *   ERROR, SECURITY_FAILED_TO_DELETE_CERTIFICATE
 */
int security_removeCommCert(IN char *serial);

#endif /* LIBIDT_SECURITY_H_ */
