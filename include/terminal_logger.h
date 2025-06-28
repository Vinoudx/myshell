#ifndef __TERMINAL_LOGGER__
#define __TERMINAL_LOGGER__

#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <time.h>

#define MAX_LOG_LENTH 100
#define MAX_INPUT_LOG_LENTH 90

enum logLevel{
    INFO,
    WARNING,
    ERROR,
};

#define INFO(log) logger(log, INFO, __FILE__, __LINE__)
#define WARNING(log) logger(log, WARNING, __FILE__, __LINE__)
#define ERROR(log) logger(log, ERROR, __FILE__, __LINE__)

char* get_enum_item(enum logLevel);
void formatter(char* target, const char* log, enum logLevel level, const char* file, size_t line_no);
void logger(const char* log, enum logLevel level,const char* file, size_t line_no);

#endif