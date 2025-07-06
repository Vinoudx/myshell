#ifndef __GREP__
#define __GREP__

#include <stdlib.h>
#include <errno.h>

#include <terminal_logger.h>

const char* strcasestr_custom(const char* haystack, const char* needle);
int grep_lines_in_string(const char* pattern, const char* input, char* buffer, int i_flag);
int grep_lines(const char* pattern, const char* path, char* buffer, int i_flag);
void grep_(char* pre_result, int* status, const char** args, size_t num_args);

#endif