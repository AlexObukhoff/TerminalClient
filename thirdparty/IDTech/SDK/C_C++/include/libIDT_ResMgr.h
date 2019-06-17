#ifndef LIBIDT_RESMGR_H_
#define LIBIDT_RESMGR_H_

#include "IDTDef.h"

typedef enum RESMGR_RESPONSE {
    RESMGR_SUCCESS,
    RESMGR_CREATE_FILE_FAILED,
    RESMGR_CLEAR_FILE_FAILED,
    RESMGR_CLOSE_FILE_FAILED,
    RESMGR_DELETE_FILE_FAILED,
    RESMGR_FILE_DOES_NOT_EXIST,
    RESMGR_DIRECTORY_DOES_NOT_EXIST,
    RESMGR_DIRECTORY_NOT_EMPTY,
    RESMGR_DIRECTORY_FAILED
} RESMGR_RESPONSE;

/**
 * This function returns a pointer to the allocated memory, or NULL if the request fails.
 *
 * @param size - Size of the memory to allocate, in bytes
 *
 * @return Pointer to the allocated memory
 */
void *resMgr_malloc(IN size_t size);

/**
 * Reallocates memory to the new size requested.
 *
 * @param ptr - Pointer to the location previously allocated with resMgr_malloc or resMgr_realloc
 * @param size - New size to allocate
 *
 * @return Pointer to the allocated memory.
 */
void *resMgr_realloc(IN void *ptr, IN size_t size);

/**
 * Deallocates the memory previously allocated by resMgr_malloc or resMgr_realloc.
 *
 * @param ptr - Pointer to the location
 */
void resMgr_free(IN void *ptr);

/**
 * Creates a new file for read / write operations.
 *
 * @param path - Path, including file name, for the new file. Must not already exist.
 * @param fileID - File ID of the new file.
 *
 * @return  RESMGR_SUCCESS,
 *   ERROR, RESMGR_CREATE_FILE_FAILED
 */
int resMgr_createFile(IN const char *path, OUT int *fileID);

/**
 * Clears the data from a file.
 *
 * @param fileID - File ID of the file
 *
 * @return  RESMGR_SUCCESS,
 *   ERROR, RESMGR_CLEAR_FILE_FAILED
 */
int resMgr_clearFile(IN int fileID);

/**
 * Appends data to the end of a file.
 *
 * @param fileID - File ID of the file
 * @param buf - A pointer to a buffer of at least nbytes bytes, which will be written to the file
 * @param nbytes - The number of bytes to write
 *
 * @return  Number of bytes written, if positive number
 *          -1 = ERROR, FAILED TO WRITE FILE
 */
int resMgr_writeFile(IN int fileID, IN const void *buf, IN int nbytes);

/**
 * Appends data to the end of an encrypted file.
 *
 * @param fileID - File ID of the file
 * @param GUID - Unique string identifier from Security Manager's Keysets. Must be created first before using
 * @param buf - A pointer to a buffer of at least nbytes bytes, which will be written to the file
 * @param nbytes - The number of bytes to write. If smaller than the provided buffer, the output is truncated
 *
 * @return  Number of bytes written, if positive number
 *          -1 = ERROR, FAILED TO WRITE FILE
 */
int resMgr_writeFileEnc(IN int fileID, IN const char *GUID, IN const void *buf, IN int nbytes);

/**
 * Reads data from beginning of a file.
 *
 * @param fileID - File ID of the file
 * @param buf - A character array where the read content will be stored.
 *
 * @return  Number of bytes read, if positive number
 *          -1 = ERROR, FAILED TO READ FILE
 */
int resMgr_readFile(IN int fileID, IN_OUT const void *buf);

/**
 * Reads data from beginning of an encrypted file.
 *
 * @param fileID - File ID of the file
 * @param GUID - Unique string identifier from Security Manager's Keysets. Must be created first before using
 * @param buf - A character array where the read content will be stored
 *
 * @return  Number of bytes read, if positive number
 *          -1 = ERROR, FAILED TO READ FILE
 */
int resMgr_readFileEnc(IN int fileID, IN const char *GUID, OUT void *buf);

/**
 * Closes a file descriptor.
 *
 * @param fileID - File ID to close
 *
 * @return  RESMGR_SUCCESS,
 *   ERROR, RESMGR_CLOSE_FILE_FAILED
 */
int resMgr_closeFile(IN int fileID);

/**
 * Checks for the existence of a file.
 *
 * @param path - Path, including file name, for the file to check for
 *
 * @return  RESMGR_SUCCESS,
 *   ERROR, RESMGR_FILE_DOES_NOT_EXIST
 */
int resMgr_checkFileExists(IN const char *path);

/**
 * Deletes a file.
 *
 * @param path - Path, including file name, for the file to to delete
 *
 * @return  RESMGR_SUCCESS,
 *   ERROR, RESMGR_DELETE_FILE_FAILED
 */
int resMgr_deleteFile(IN const char *path);

/**
 * Checks for the existence of a directory.
 *
 * @param path - Path for the directory to check for
 *
 * @return  RESMGR_SUCCESS,
 *   ERROR, RESMGR_DIRECTORY_DOES_NOT_EXIST
 *   ERROR, RESMGR_DIRECTORY_FAILED
 */
int resMgr_checkDirectory(IN const char *path);

/**
 * Deletes a directory. Must be empty.
 *
 * @param path - Path, including file name, for the directory to delete.
 *
 * @return  RESMGR_SUCCESS,
 *   ERROR, RESMGR_DIRECTORY_DOES_NOT_EXIST
 *   ERROR, RESMGR_DIRECTORY_NOT_EMPTY
 *   ERROR, RESMGR_DIRECTORY_FAILED
 */
int resMgr_deleteDirectory(IN const char *path);

/**
 * Lists the directory contents
 *
 * @param path - Path for the directory to list
 * @param items - List of file names delimited by comma. Need to free items when done.
 *
 * @return  Number of items in the directory. If error, or no items in the directory, returns -1
 */
int resMgr_listDirectory(IN const char *path, OUT char **items);

#endif /* LIBIDT_RESMGR_H_ */
