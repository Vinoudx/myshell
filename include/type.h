#ifndef __TYPE__
#define __TYPE__

#include <stdlib.h>
#include <errno.h>

#include <terminal_logger.h>
#include <commandrule.h>
#include <defines.h>

void type_(char* pre_result, int* status, const char** args, size_t num_args);

#endif