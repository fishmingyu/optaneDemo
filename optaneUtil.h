#ifndef OPTANEUTIL_H
#define OPTANEUTIL_H 

#include <memkind.h>

enum MemoryType
{
    DRAM,
    OPTANE
};

void memkind_fatal(int err)
{
	char error_message[MEMKIND_ERROR_MESSAGE_SIZE];
	memkind_error_message(err, error_message,
		MEMKIND_ERROR_MESSAGE_SIZE);
	fprintf(stderr, "%s\n", error_message);
	exit(1);
}
#endif //OPTANEUTIL_H