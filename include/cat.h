#ifndef __CAT__
#define __CAT__

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <terminal_logger.h>

void cat_(char* pre_result, int* status, const char** args, size_t num_args);

#endif