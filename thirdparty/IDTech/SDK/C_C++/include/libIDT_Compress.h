#ifndef __LIBIDT_COMPRESS_H___
#define __LIBIDT_COMPRESS_H___

#include "IDTDef.h"

/**
 * Compresses a tar.gz or zip archive.
 *
 * @param source – source path of the file/folder to archive. Null terminated string.
 * @param destination – destination path and filename for the zip archive. Null terminated string. Will automatically create any required missing folders in the path.
 * @param password – password if archive is to be password protected. Null terminated string. A value of 0x00 signifies no password required. Not available for tar.gz archives
 * @param type – 0 = tar.gz, 1 = zip
 *
 * @return 0 if successful, 1 if unsuccessful
 */
int compress_compressArchive(IN char* source, IN char* destination, IN char* password, IN int type);

/**
 * Uncompresses a tar.gz or zip archive.
 *
 * @param source – source path of the zip archive. Null terminated string.
 * @param destination – destination path for the uncompressed files. Null terminated string. Will automatically create any required missing folders in the path.
 * @param password – password if archive is password protected. Null terminated string. A value of 0x00 signifies no password required. Not available for tar.gz archives
 *
 * @return 0 if successful, 1 if unsuccessful
 */
int compress_uncompressArchive(IN char* source, IN char* destination, IN char* password);

#endif /* __LIBIDT_COMPRESS_H___ */