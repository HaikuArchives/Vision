#ifndef STORAGE_DEFS_H__
#define STORAGE_DEFS_H__

#include "PlatformDefines.h"

// Limits
const int32 B_FILE_NAME_LENGTH = 1024;
const int32 B_PATH_NAME_LENGTH = 1024;
const int32 B_ATTR_NAME_LENGTH = B_FILE_NAME_LENGTH - 1;
const int32 B_MIME_TYPE_LENGTH = B_ATTR_NAME_LENGTH - 15;


// File opem modes
#define B_READ_ONLY O_RDONLY  	// read only
#define B_WRITE_ONLY O_WRONLY 	// write only
#define B_READ_WRITE O_RDWR   	// read and write

#define	B_FAIL_IF_EXISTS O_EXCL		// exclusive create
#define B_CREATE_FILE O_CREAT		// create the file
#define B_ERASE_FILE O_TRUNC		// erase the file's data
#define B_OPEN_AT_END O_APPEND	// point to the end of the data


enum node_flavor {
	B_FILE_NODE = 0x01,
	B_SYMLINK_NODE = 0x02,
	B_DIRECTORY_NODE = 0x04,
	B_ANY_NODE = 0x07
};

#endif
