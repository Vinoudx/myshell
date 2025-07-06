#ifndef __CD__
#define __CD__

#include <stdlib.h>
#include <errno.h>

#include <terminal_logger.h>

void cd_(char* pre_result, int* status, const char** args, size_t num_args);

#endif