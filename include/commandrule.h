#ifndef __COMMAND_RULE__
#define __COMMAND_RULE__

#include <stdlib.h>
#include <string.h>

enum ArgType{
    ARG_NONE,
    ARG_STRING,
    ARG_FILE,
    ARG_DIR,
    ARG_COMMAND
};

struct OptionRule{
    char option[4];     // 如 "-o", 主参数则为“main”
    int has_arg;            // 是否有参数
    enum ArgType arg_type;  // 参数类型
    int required;           // 参数最大数量
};

struct CommandRule{
    char command_name[16];
    char type[16];
    int num_main_params; // 至少有几个main参数
    int num_options;
    struct OptionRule options[4];
};

extern int num_commands;
extern struct CommandRule rules[];

struct CommandRule find_rule(const char* name);
struct OptionRule find_option(const char* command_name, const char* op_name);
#endif