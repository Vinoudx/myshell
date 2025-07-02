#ifndef __AUTOCOMPLETE__
#define __AUTOCOMPLETE__

#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/stat.h> //用stat来判断是不是文件夹
#include <grp.h>

#include <commandrule.h>
#include <terminal_logger.h>

#define MAX_AUTO_COMPLETE_AMOUNT 128
#define MAX_AUTO_COMPLETE_LENGTH 128

struct AutoCompleteResult{
    size_t amount;
    char results[MAX_AUTO_COMPLETE_AMOUNT][MAX_AUTO_COMPLETE_LENGTH];
    size_t replace_pos;
};

struct AutoCompleteResult file_autocomplete(const char* buffer, const char* working_path, const char* last_word, int isFolder, int isFile, int isExecAble);
struct AutoCompleteResult env_autocomplete(const char* buffer, const char* last_word);
struct AutoCompleteResult command_autocomplete(const char* buffer, const char* last_word);
struct AutoCompleteResult option_autocomplete(const char* buffer, const char* last_word, const char* command_name);
#endif