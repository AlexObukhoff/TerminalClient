#ifndef LIBIDT_DEVMGR_H_
#define LIBIDT_DEVMGR_H_

#include "IDTDef.h"

typedef enum DEVMGR_RESPONSE {
    DEVMGR_SUCCESS,
    DEVMGR_INVALID_RESOURCE,
    DEVMGR_INVALID_IDENTIFIER,
    DEVMGR_INVALID_CALLER_ID,
    DEVMGR_IDENTIFIER_ALREADY_CLAIMED_BY_CALLER,
    DEVMGR_IDENTIFIER_NOT_FOUND,
    DEVMGR_CALLER_ID_NOT_FOUND,
    DEVMGR_COULD_NOT_CONNECT_TO_DATABASE,
    DEVMGR_SQLITE_COMMAND_FAILED,
    DEVMGR_RESOURCE_LOCKED,
    DEVMGR_RESOURCE_NOT_CLAIMED_BY_CALLER,
    DEVMGR_INCORRECT_PASSWORD,
    DEVMGR_RESOURCE_LOCKED_BY_DIFFERENT_CALLER,
    DEVMGR_RESOURCE_NOT_LOCKED,
    DEVMGR_INVALID_CLAIMS,

    DEVMGR_FAILED // Generic error - should try to return another response code if possible
} DEVMGR_RESPONSE;

/**
 * Claims a resource. Resource can be a file, USB device, RS-232 device, or virtual.
 *
 * A file and virtual can only have one claim / lock.
 * USB or RS-232 can have multiple claims, but only one lock.
 * If a file or virtual, claiming resource also automatically locks it
 *
 * @param resource - The type of resource to claim.
 *  | Resource Types   |
 *  | ---------------- |
 *  | RESOURCE_FILE    |
 *  | RESOURCE_USB     |
 *  | RESOURCE_RS232   |
 *  | RESOURCE_VIRTUAL |
 * @param identifier - The resource's identifier
 *  | Resource Type    | Identifier                                    |
 *  | ---------------- | --------------------------------------------- |
 *  | RESOURCE_FILE    | Path                                          |
 *  | RESOURCE_USB     | 4 ASCII characters representing HEX PID value |
 *  |                  | Ex: PID = 0x0312 = "0312"                     |
 *  | RESOURCE_RS232   | ASCII characters representing COM PORT number |
 *  |                  | Ex: COM8 = "8"                                |
 *  | RESOURCE_VIRTUAL | ASCII label "VIRTUAL"                         |
 * @param callerID - Identifier for reporting who is claiming this resource
 * @parampassword - Optional password requirement for LOCK / UNLOCK / RELEASE
 *
 * @return  DEVMGR_SUCCESS
 *          ERROR, DEVMGR_INVALID_RESOURCE
 *          ERROR, DEVMGR_INVALID_IDENTIFIER
 *          ERROR, DEVMGR_INVALID_CALLER_ID
 *          ERROR, DEVMGR_IDENTIFIER_ALREADY_CLAIMED_BY_CALLER
 *          ERROR, DEVMGR_COULD_NOT_CONNECT_TO_DATABASE
 *          ERROR, DEVMGR_SQLITE_COMMAND_FAILED
 *          ERROR, DEVMGR_RESOURCE_LOCKED
 */
DEVMGR_RESPONSE devMgr_claim(IN const RESOURCE_TYPE resource, IN const char *identifier, IN const char *calledID, IN const char *password);

/**
 * Releases a claim to a resource. If resource is locked, will automatically unlock then release.
 *
 * @param resource - The type of resource to release
 *  | Resource Type    |
 *  | ---------------- |
 *  | RESOURCE_FILE    |
 *  | RESOURCE_USB     |
 *  | RESOURCE_RS232   |
 *  | RESOURCE_VIRTUAL
 * @param identifier - The resource's identifier
 *  | Resource Type    | Identifier                                    |
 *  | ---------------- | --------------------------------------------- |
 *  | RESOURCE_FILE    | Path                                          |
 *  | RESOURCE_USB     | 4 ASCII characters representing HEX PID value |
 *  |                  | Ex: PID = 0x0312 = "0312"                     |
 *  | RESOURCE_RS232   | ASCII characters representing COM PORT number |
 *  |                  | Ex: COM8 = "8"                                |
 *  | RESOURCE_VIRTUAL | ASCII label "VIRTUAL"                         |
 * @param callerID - The callerID who established the claim
 * @param password - Password established byothe callerID during claim, if any
 *
 * @return  DEVMGR_SUCCESS
 *          ERROR, DEVMGR_INVALID_RESOURCE
 *          ERROR, DEVMGR_INVALID_IDENTIFIER
 *          ERROR, DEVMGR_INVALID_CALLER_ID
 *          ERROR, DEVMGR_COULD_NOT_CONNECT_TO_DATABASE
 *          ERROR, DEVMGR_IDENTIFIER_NOT_FOUND
 *          ERROR, DEVMGR_CALLER_ID_NOT_FOUND
 *          ERROR, DEVMGR_SQLITE_COMMAND_FAILED
 *          ERROR, DEVMGR_RESOURCE_NOT_CLAIMED_BY_CALLER
 *          ERROR, DEVMGR_RESOURCE_LOCKED_BY_DIFFERENT_CALLER
 */
DEVMGR_RESPONSE devMgr_release(IN const RESOURCE_TYPE resource, IN const char *identifier, IN const char *callerID, IN const char *password);

/**
 * Locks a claim to a resource.
 *
 * @param resource - The type of resource to lock
 *  | Resource Type    |
 *  | ---------------- |
 *  | RESOURCE_FILE    |
 *  | RESOURCE_USB     |
 *  | RESOURCE_RS232   |
 *  | RESOURCE_VIRTUAL |
 * @param identifier - The resource's identifier
 *  | Resource Type    | Identifier                                    |
 *  | ---------------- | --------------------------------------------- |
 *  | RESOURCE_FILE    | Path                                          |
 *  | RESOURCE_USB     | 4 ASCII characters representing HEX PID value |
 *  |                  | Ex: PID = 0x0312 = "0312"                     |
 *  | RESOURCE_RS232   | ASCII characters representing COM PORT number |
 *  |                  | Ex: COM8 = "8"                                |
 *  | RESOURCE_VIRTUAL | ASCII label "VIRTUAL"                         |
 * @param callerID - The callerID who established the claim
 * @param password - Password established by the callerID during claim, if any
 *
 * @return  DEVMGR_SUCCESS
 *          ERROR, DEVMGR_INVALID_RESOURCE
 *          ERROR, DEVMGR_INVALID_IDENTIFIER
 *          ERROR, DEVMGR_INVALID_CALLER_ID
 *          ERROR, DEVMGR_COULD_NOT_CONNECT_TO_DATABASE
 *          ERROR, DEVMGR_SQLITE_COMMAND_FAILED
 *          ERROR, DEVMGR_CALLER_ID_NOT_FOUND
 *          ERROR, DEVMGR_IDENTIFIER_NOT_FOUND
 *          ERROR, DEVMGR_RESOURCE_NOT_CLAIMED_BY_CALLER
 *          ERROR, DEVMGR_RESOURCE_LOCKED
 */
DEVMGR_RESPONSE devMgr_lock(IN const RESOURCE_TYPE resource, IN const char *identifier, IN const char *callerID, IN const char *password);

/**
 * Unlocks a locked resource. If resource is file or virtual, unlocking will also release.
 *
 * @param resource - The type of resource to unlock
 *  | Resource Type    |
 *  | ---------------- |
 *  | RESOURCE_FILE    |
 *  | RESOURCE_USB     |
 *  | RESOURCE_RS232   |
 *  | RESOURCE_VIRTUAL |
 * @param identifier - The resource's identifier
 *  | Resource Type    | Identifier                                    |
 *  | ---------------- | --------------------------------------------- |
 *  | RESOURCE_FILE    | Path                                          |
 *  | RESOURCE_USB     | 4 ASCII characters representing HEX PID value |
 *  |                  | Ex: PID = 0x0312 = "0312"                     |
 *  | RESOURCE_RS232   | ASCII characters representing COM PORT number |
 *  |                  | Ex: COM8 = "8"                                |
 *  | RESOURCE_VIRTUAL | ASCII label "VIRTUAL"                         |
 * @param callerID - The callerID who established the claim
 * @param password - Password established by the callerID during claim, if any
 *
 * @return  DEVMGR_SUCCESS
 *          ERROR, DEVMGR_INVALID_RESOURCE
 *          ERROR, DEVMGR_INVALID_IDENTIFIER
 *          ERROR, DEVMGR_INVALID_CALLER_ID
 *          ERROR, DEVMGR_COULD_NOT_CONNECT_TO_DATABASE
 *          ERROR, DEVMGR_SQLITE_COMMAND_FAILED
 *          ERROR, DEVMGR_CALLER_ID_NOT_FOUND
 *          ERROR, DEVMGR_IDENTIFIER_NOT_FOUND
 *          ERROR, DEVMGR_RESOURCE_NOT_CLAIMED_BY_CALLER
 *          ERROR, DEVMGR_INCORRECT_PASSWORD
 *          ERROR, DEVMGR_RESOURCE_NOT_LOCKED
 */
DEVMGR_RESPONSE devMgr_unlock(IN const RESOURCE_TYPE resource, IN const char *identifier, IN const char *callerID, IN const char *password);

/**
 * List the claims to a resource.
 *
 * @param resource - The type of resource to get claims for
 *  | Resource Type    |
 *  | ---------------- |
 *  | RESOURCE_FILE    |
 *  | RESOURCE_USB     |
 *  | RESOURCE_RS232   |
 *  | RESOURCE_VIRTUAL |
 * @param identifier - The resource's identifier
 *  | Resource Type    | Identifier                                    |
 *  | ---------------- | --------------------------------------------- |
 *  | RESOURCE_FILE    | Path                                          |
 *  | RESOURCE_USB     | 4 ASCII characters representing HEX PID value |
 *  |                  | Ex: PID = 0x0312 = "0312"                     |
 *  | RESOURCE_RS232   | ASCII characters representing COM PORT number |
 *  |                  | Ex: COM8 = "8"                                |
 *  | RESOURCE_VIRTUAL | ASCII label "VIRTUAL"                         |
 * @param claims - Comma-separated list of all claims to the resource
 *
 * @return  DEVMGR_SUCCESS,
 *          ERROR, DEVMGR_INVALID_RESOURCE
 *          ERROR, DEVMGR_INVALID_IDENTIFIER
 *          ERROR, DEVMGR_COULD_NOT_CONNECT_TO_DATABASE
 *          ERROR, DEVMGR_SQLITE_COMMAND_FAILED
 *          ERROR, DEVMGR_IDENTIFIER_NOT_FOUND
 *          ERROR, DEVMGR_INVALID_CLAIMS
 */
DEVMGR_RESPONSE devMgr_getClaims(IN const RESOURCE_TYPE resource, IN const char *identifier, OUT char *claims);

/**
 * Reports if the resource is currently locked, and if so, retrieves the callerID claim.
 *
 * @param resource - The type of resource to check the status of
 *  | Resource Type    |
 *  | ---------------- |
 *  | RESOURCE_FILE    |
 *  | RESOURCE_USB     |
 *  | RESOURCE_RS232   |
 *  | RESOURCE_VIRTUAL |
 * @param identifier - The resource's identifier
 *  | Resource Type    | Identifier                                    |
 *  | ---------------- | --------------------------------------------- |
 *  | RESOURCE_FILE    | Path                                          |
 *  | RESOURCE_USB     | 4 ASCII characters representing HEX PID value |
 *  |                  | Ex: PID = 0x0312 = "0312"                     |
 *  | RESOURCE_RS232   | ASCII characters representing COM PORT number |
 *  |                  | Ex: COM8 = "8"                                |
 *  | RESOURCE_VIRTUAL | ASCII label "VIRTUAL"                         |
 * @param callerID - The callerID who has a lock on the claim
 *
 * @return  DEVMGR_RESOURCE_NOT_LOCKED
 *          DEVMGR_RESOURCE_LOCKED
 *          ERROR, DEVMGR_INVALID_RESOURCE
 *          ERROR, DEVMGR_INVALID_IDENTIFIER
 *          ERROR, DEVMGR_INVALID_CALLER_ID
 *          ERROR, DEVMGR_COULD_NOT_CONNECT_TO_DATABASE
 *          ERROR, DEVMGR_SQLITE_COMMAND_FAILED
 *          ERROR, DEVMGR_IDENTIFIER_NOT_FOUND
 */
DEVMGR_RESPONSE devMgr_getLockStatus(IN const RESOURCE_TYPE resource, IN const char *identifier, OUT char *callerID);

/**
 * Removes the specified caller and their corresponding claims.
 *
 * @param callerID - The caller ID to remove
 *
 * @return  DEVMGR_SUCCESS
 *          ERROR, DEVMGR_INVALID_CALLER_ID
 *          ERROR, DEVMGR_COULD_NOT_CONNECT_TO_DATABASE
 *          ERROR, DEVMGR_CALLER_ID_NOT_FOUND
 *          ERROR, DEVMGR_SQLITE_COMMAND_FAILED
 */
DEVMGR_RESPONSE devMgr_removeCallerAndReleaseClaims(IN const char *callerID);

#endif /* LIBIDT_DEVMGR_H_ */