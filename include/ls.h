#ifndef __LS__
#define __LS__

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <unistd.h>

#include <terminal_logger.h>

void ls_(char* pre_result, int* status, const char** args, size_t num_args);

#endif