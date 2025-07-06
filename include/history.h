#ifndef __HISTORY__
#define __HISTORY__

#include <stdlib.h>
#include <errno.h>

#include <terminal_logger.h>
#include <defines.h>

int count_lines();
int save_history(const char* line_content);
void read_last_n_lines(char* pre_result, int n);
int read_last_nth_line(char* buffer, int nn);
void history_(char* pre_result, int* status, const char** args, size_t num_args);

#endif