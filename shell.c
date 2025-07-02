#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <pwd.h>
#include <aio.h>

#include <terminal_logger.h>
#include <autocomplete.h>
#include <syntaxanalyser.h>
#include <all_commands.h>

#define std_print(buffer) write(STDOUT_FILENO, buffer, strlen(buffer));
#define MAX_INPUT_LENGTH 256
#define MAX_NUM_ARGS 8
#define MAX_ARG_LENGTH 64
#define CONFIG_PATH "/home/vinoudx/work/myshell/configs/"


struct termios termios_previous;

void change_terminal_mode() {
    tcgetattr(STDIN_FILENO, &termios_previous);
    struct termios termios_modified = termios_previous;
    termios_modified.c_lflag &= ~(ICANON | ECHO); // 禁用标准输入模式和回显
    tcsetattr(STDIN_FILENO, TCSANOW, &termios_modified);
    fflush(stdout);
}

void restore_terminal_mode(){
    tcsetattr(STDIN_FILENO, TCSANOW, &termios_previous);
    fflush(stdout);
}

void refresh_terminal(const char* buffer,const char* user_name,const char* working_path, int need_head, int from_head){
    if(from_head == 1)
        std_print("\r\033[2K");

    char isRoot;
    if(getuid() == 0){
        isRoot = '#';
    }else{
        isRoot = '$';
    }

    char output[1024];
    memset(output, '\0', sizeof(output));
    if (need_head != 0){
        sprintf(output, "%s:~%s%c%s", user_name, working_path, isRoot, buffer);
    }else{
        sprintf(output, "%s", buffer);
    }
    std_print(output);
}

void auto_complete_task(char* buffer, const char* working_path, char* to_pick, int* pos){

    struct Token tokens[MAX_COMMAND_LENGTH] = {{END, "end"}};
    size_t token_pos = 0;
    tokenize_(buffer, tokens, &token_pos);
    struct SyntaxAnalyseResult res = syntax_analyser(tokens, token_pos);
    struct AutoCompleteResult cres;
    switch (res.last_word.token_type)
    {
    case FOLDER: // OK
        cres = file_autocomplete(buffer, working_path, res.last_word.content, 1, 0, 0);
        break;
    case FILE_:
        cres = file_autocomplete(buffer, working_path, res.last_word.content, 1, 1, 0);
        break;     
    case EXEC_FILE: // OK
        cres = file_autocomplete(buffer, working_path, res.last_word.content, 1, 1, 1);
        break;   
    case ENV: // OK
        cres = env_autocomplete(buffer, res.last_word.content);
        cres.replace_pos++; //不要替换$
        break;
    case COMMAND: // OK
        cres = command_autocomplete(buffer, res.last_word.content);
        if(cres.amount == 0){
            cres = file_autocomplete(buffer, working_path, res.last_word.content, 1, 1, 1);
        }
        break;
    case OPTION: // OK
        INFO(res.command_list[res.num_command][0].content);
        cres = option_autocomplete(buffer, res.last_word.content, res.command_list[res.num_command][0].content);
        break;
    default:
        cres.amount = 1;
        cres.replace_pos = 0;
        memset(cres.results, '\0', sizeof(cres.results));
        break;
    }

    if (cres.amount == 1){
        size_t keep_len = cres.replace_pos;
        size_t back_len = strlen(buffer) - keep_len;
        memset(buffer + keep_len, '\0', back_len);
        (*pos) -= (back_len);
        strcat(buffer, cres.results[0]);
        (*pos) += strlen(cres.results[0]);

    }else if(cres.amount > 1){
        for (int i = 0; i < cres.amount; i++){
            strcat(to_pick, cres.results[i]);
            strcat(to_pick, "\t");
        }
        strcat(to_pick, "\n");
    }
}

void execute_task(char* buffer, char* working_path){
    if(strcmp(buffer, "cd ..") == 0){
        int a = 1;
    }
    char temp_result[1024] = {'\0'};
    struct Token tokens[MAX_COMMAND_LENGTH] = {{END, "end"}};
    size_t token_pos = 0;
    tokenize_(buffer, tokens, &token_pos);
    struct SyntaxAnalyseResult res = syntax_analyser(tokens, token_pos);
    if(res.isValid == 0){
        memset(buffer, '\0', 1024);
        strcpy(buffer, res.error_info);
        strcat(buffer, "\n");
        return;
    }
    int status = 1;
    for (int i=1;i<=res.num_command;i++){
        if(status == 0){
            break;
        }

        char* command_name = res.command_list[i][0].content;
        char args[MAX_NUM_ARGS][MAX_ARG_LENGTH];
        memset(args, '\0', sizeof(args));

        for (int j=1;j<res.command_length[i];j++){
            strcpy(args[j-1], res.command_list[i][j].content);
        }
        char* argv[MAX_NUM_ARGS];
        memset(argv, '\0', sizeof(argv));
        for (int k = 0; k < res.command_length[i] - 1; k++) {
            argv[k] = args[k];
        }

        if(strcmp(command_name, "ls") == 0){

        }else if(strcmp(command_name, "cd") == 0){
            
            cd_(temp_result, &status, argv, res.command_length[i] - 1);
            getcwd(working_path, 1024);
            INFO(working_path);

        }else if(strcmp(command_name, "cat") == 0){
            
        }else if(strcmp(command_name, "grep") == 0){
            
        }else if(strcmp(command_name, "echo") == 0){

            echo_(temp_result, &status, argv, res.command_length[i] - 1);

        }else if(strcmp(command_name, "type") == 0){
            
        }else if(strcmp(command_name, "history") == 0){
            
        }else if(strcmp(command_name, "alias") == 0){
            
        }else{
            strcpy(temp_result, "shell: unknown command\n");
            break;
        }

    }

    strncpy(buffer, temp_result, MAX_INPUT_LENGTH);
}


int main() {
    change_terminal_mode();

    // logger("13awoufha", INFO, __FILE__, __LINE__);
    char buffer[MAX_INPUT_LENGTH];
    memset(buffer, '\0', sizeof(buffer));
    int pos = 0;
    char ch;

    char* working_path = NULL;
    char* current_user = getlogin();
    struct passwd* pwd = getpwnam(current_user);
    working_path = pwd->pw_dir;
    chdir(working_path);

    time_t time_begin = time(NULL);
    refresh_terminal(buffer, current_user, working_path, 1, 1);
    while (read(STDIN_FILENO, &ch, 1) == 1) {
        if (ch == '\n') {
            if(strcmp(buffer, "exit") == 0)break;
            alias_replace(buffer);
            INFO(buffer);
            execute_task(buffer, working_path);
            std_print("\n");
            if(strlen(buffer) != 0){ // 命令返回的信息
                refresh_terminal(buffer, "", "", 0, 1);
            }
            memset(buffer, '\0', sizeof(buffer));
            pos = 0;
            refresh_terminal(buffer, current_user, working_path, 1, 0);

        } else if (ch == '\t') {
            char to_pick[1024];
            memset(to_pick, '\0', sizeof(to_pick));
            auto_complete_task(buffer, working_path, to_pick, &pos);
            refresh_terminal(buffer, current_user, working_path, 1, 1);

            if(difftime(time(NULL), time_begin) < 0.05 && strlen(to_pick) > 0){
                std_print("\n");
                refresh_terminal(to_pick, "", "", 0, 1);
                refresh_terminal(buffer, current_user, working_path, 1, 1); 
            }
            time_begin = time(NULL);
        } else if (ch == '\b' || ch == 127){
            if (pos > 0){
                buffer[--pos] = '\0';
            }
            refresh_terminal(buffer, current_user, working_path, 1, 1);
        } else if (isprint(ch)) {
            buffer[pos++] = ch;
            refresh_terminal(buffer, current_user, working_path, 1, 1);
        }
    }
    std_print("\n");
    restore_terminal_mode();
    return 0;
}