#ifndef __SYNTAXANALYSER__
#define __SYNTAXANALYSER__

#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <pwd.h>
#include <aio.h>

#include <commandrule.h>
#include <terminal_logger.h>

#define MAX_WORD_LENGTH 128
#define MAX_COMMAND_LENGTH 256
#define MAX_COMMAND_AMOUNT 10

enum CommandType{
    SINGLE,
    PIPE,
    I_REDIRECTION,
    O_REDIRECTION,
    HYBRID
};

enum TokenType{
    FOLDER,         // 需要为一个路径
    FILE_,    // ./开头，文件或到文件的中间路径
    EXEC_FILE,
    ENV,            // $开头
    // 以上四个都为parameter
    ESCAPE,         //不需要自动补全
    COMMAND,        // 没有./, 直接一个命令
    OPTION,         // 一个选项，以-开头
    IDENTIFIER,     // 语法分析前的待定项
    PIPE_CHAR,
    RIN,
    ROUT,
    ASYN,
    END
};


struct Token{
    enum TokenType token_type;
    char content[MAX_WORD_LENGTH];
};


struct SyntaxAnalyseResult{
    int isValid;
    char error_info[128];
    struct Token last_word;

    enum CommandType type;
    size_t num_command;
    char command_list[MAX_COMMAND_AMOUNT][MAX_COMMAND_LENGTH];
    size_t num_connect_char;    
    char connection_char[MAX_COMMAND_AMOUNT];

    int isAsyn;
};

void tokenize_(const char* buffer, struct Token tokens[], size_t* token_pos);
struct SyntaxAnalyseResult syntax_analyser(struct Token tokens[], size_t token_pos);



#endif