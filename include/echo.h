#ifndef __ECHO__
#define __ECHO__

#include <stdlib.h>
#include <errno.h>

#include <terminal_logger.h>

void replace_(char* content);
void get_escape_character(char* target, const char* s);
void echo_(char* pre_result, int* status, const char** args, size_t num_args);

#endif