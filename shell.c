#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <pwd.h>
#include <aio.h>
#include <sys/wait.h>

#include <terminal_logger.h>
#include <autocomplete.h>
#include <syntaxanalyser.h>
#include <all_commands.h>

#define std_print(buffer) write(STDOUT_FILENO, buffer, strlen(buffer));
#define MAX_INPUT_LENGTH 2048
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

    char output[2048];
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
    if(res.isValid == 0){
        return;
    }
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
    
    if(strlen(buffer) != 0)
        save_history(buffer);

    char temp_result[2048] = {'\0'};
    struct Token tokens[MAX_COMMAND_LENGTH] = {{END, "end"}};
    size_t token_pos = 0;
    tokenize_(buffer, tokens, &token_pos);
    struct SyntaxAnalyseResult res = syntax_analyser(tokens, token_pos);
    if(res.isValid == 0){
        memset(buffer, '\0', MAX_INPUT_LENGTH);
        strcpy(buffer, res.error_info);
        strcat(buffer, "\n");
        return;
    }


    int current_connection_char = 0;
    memset(buffer, '\0', MAX_INPUT_LENGTH);
    for(int i=1;i<=res.num_command;i++){
        char command[MAX_INPUT_LENGTH] = {'\0'};
        for(int j=0;j<res.command_length[i]; j++){
            strcat(command, res.command_list[i][j].content);
            strcat(command, " ");
        }
        alias_replace(command);
        strcat(buffer, command);
        strcat(buffer, " ");
        char ch[2] = {res.connection_char[current_connection_char++], '\0'};
        strcat(buffer, ch);
        strcat(buffer, " ");
    }
    if(res.isAsyn == 1){
        strcat(buffer, "&");
    }

    INFO(buffer);
    token_pos = 0;
    tokenize_(buffer, tokens, &token_pos);
    res = syntax_analyser(tokens, token_pos);
    if(res.isValid == 0){
        memset(buffer, '\0', MAX_INPUT_LENGTH);
        strcpy(buffer, res.error_info);
        strcat(buffer, "\n");
        return;
    }

    for (int i=1;i<=res.num_command;i++){
        char* command_name = res.command_list[i][0].content;
        if(strcmp(command_name, "cd") == 0){
            strcpy(buffer, "cd: async not supported\n");
            return;
        }
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe error");
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return;
    }

    if(pid == 0){
        close(pipefd[0]);

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
                INFO("ls");
                ls_(temp_result, &status, argv, res.command_length[i] - 1);

            }else if(strcmp(command_name, "cd") == 0){
                INFO("cd");
                cd_(temp_result, &status, argv, res.command_length[i] - 1);
                getcwd(working_path, 1024);

            }else if(strcmp(command_name, "cat") == 0){
                INFO("cat");
                cat_(temp_result, &status, argv, res.command_length[i] - 1);

            }else if(strcmp(command_name, "grep") == 0){
                INFO("grep");
                grep_(temp_result, &status, argv, res.command_length[i] - 1);
                
            }else if(strcmp(command_name, "echo") == 0){
                INFO("echo");
                echo_(temp_result, &status, argv, res.command_length[i] - 1);

            }else if(strcmp(command_name, "type") == 0){
                INFO("type");
                type_(temp_result, &status, argv, res.command_length[i] - 1);
                
            }else if(strcmp(command_name, "history") == 0){
                INFO("history");
                history_(temp_result, &status, argv, res.command_length[i] - 1);
                
            }else if(strcmp(command_name, "alias") == 0){
                INFO("alias");
                alias_(temp_result, &status, argv, res.command_length[i] - 1);

            }else{
                strcpy(temp_result, "shell: unknown command\n");
                break;
            }

        }
        write(pipefd[1], temp_result, strlen(temp_result));
        close(pipefd[1]);
        exit(0);
    }else {
        close(pipefd[1]);

        if (res.isAsyn == 1) {
            sprintf(buffer, "child pid: %d\n", pid);
        } else {
            waitpid(pid, NULL, 0);

            ssize_t count = read(pipefd[0], buffer, MAX_INPUT_LENGTH - 1);
            if (count >= 0) {
                buffer[count] = '\0';
            } else {
                strcpy(buffer, "read error");
            }
        }
        close(pipefd[0]);
    }

}


int main() {
    change_terminal_mode();

    char buffer[MAX_INPUT_LENGTH];
    memset(buffer, '\0', sizeof(buffer));
    int pos = 0;
    char ch;

    char* working_path = NULL;
    char* current_user = getlogin();
    struct passwd* pwd = getpwnam(current_user);
    working_path = pwd->pw_dir;
    chdir(working_path);

    int current_history = 0;

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
        } else if (ch == 0x1b) {
            char seq[2];
            read(STDIN_FILENO, &seq[0], 1);
            read(STDIN_FILENO, &seq[1], 1);

            if (seq[0] == '[') {
                if (seq[1] == 'A') {
                    memset(buffer, '\0', MAX_INPUT_LENGTH);
                    read_last_nth_line(buffer, ++current_history);
                    pos = strlen(buffer);
                    refresh_terminal(buffer, current_user, working_path, 1, 1);
                } else if (seq[1] == 'B') {
                    memset(buffer, '\0', MAX_INPUT_LENGTH);
                    current_history = --current_history >= 0 ? current_history : 0;
                    if(current_history == 0){
                        pos = 0;
                    }else{
                        read_last_nth_line(buffer, current_history);
                        pos = strlen(buffer);
                    }
                    refresh_terminal(buffer, current_user, working_path, 1, 1);
                }
            }
        }
    }
    std_print("\n");
    restore_terminal_mode();
    return 0;
}