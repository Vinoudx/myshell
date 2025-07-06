#ifndef __ALIAS__
#define __ALIAS__

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include <terminal_logger.h>
#include <defines.h>

void alias_replace(char* buffer);
void alias_(char* pre_result, int* status, const char** args, size_t num_args);

#endif