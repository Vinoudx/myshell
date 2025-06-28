#ifndef __AUTOCOMPLETE__
#define __AUTOCOMPLETE__

#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>

#include <terminal_logger.h>

#define MAX_AUTO_COMPLETE_AMOUNT 100

struct AutoCompleteResult{
    size_t amount;
    char* results[MAX_AUTO_COMPLETE_AMOUNT];
};

struct AutoCompleteResult autocomplete(const char *prefix, const char* working_path);

#endif