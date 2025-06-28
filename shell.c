#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <pwd.h>

#include <terminal_logger.h>
#include <autocomplete.h>

#define std_print(buffer) write(STDOUT_FILENO, buffer, strlen(buffer));
#define MAX_INPUT 256


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

void refresh_terminal(const char* buffer,const char* user_name,const char* working_path, int need_head){
    std_print("\r\033[2K");

    char output[1024];
    memset(output, '\0', sizeof(output));
    if (need_head != 0){
        sprintf(output, "%s:~%s$%s", user_name, working_path, buffer);
    }else{
        sprintf(output, "%s", buffer);
    }
    std_print(output);
}

void auto_complete_task(char* buffer, const char* working_path, char* to_pick, int* pos){
    struct AutoCompleteResult res = autocomplete(buffer, working_path);

    // char temp[1024];
    // sprintf(temp, "%d", res.amount);
    // INFO(temp);

    if (res.amount == 1){
        char* back = strrchr(buffer, '/');
        if (back != NULL){
            size_t back_len = strlen(back);
            size_t keep_len = strlen(buffer) - strlen(back);
            memset(buffer + keep_len + 1, '\0', strlen(back));
            (*pos) -= (back_len - 1);
        }else{
            back = strrchr(buffer, ' ');
            if (back != NULL){
                size_t back_len = strlen(back);
                size_t keep_len = strlen(buffer) - strlen(back);
                memset(buffer + keep_len + 1, '\0', strlen(back));
                (*pos) -= (back_len - 1);
            }else{
                memset(buffer, '\0', MAX_INPUT);
                (*pos) = 0;
            }
        }
        
        strcat(buffer, res.results[0]);
        (*pos) += strlen(res.results[0]);

    }else if(res.amount > 1){
        for (int i = 0; i < res.amount; i++){
            strcat(to_pick, res.results[i]);
            strcat(to_pick, "\t");
        }
        strcat(to_pick, "\n");
    }
}

int main() {
    change_terminal_mode();


    // logger("13awoufha", INFO, __FILE__, __LINE__);
    char buffer[MAX_INPUT];
    memset(buffer, '\0', sizeof(buffer));
    int pos = 0;
    char ch;

    char* working_path = NULL;
    char* current_user = getlogin();
    struct passwd* pwd = getpwnam(current_user);
    working_path = pwd->pw_dir;
    chdir(working_path);

    refresh_terminal(buffer, current_user, working_path, 1);
    while (read(STDIN_FILENO, &ch, 1) == 1) {
        if (ch == '\n') {
            printf("\n最终输入:%s\n", buffer);
            refresh_terminal(buffer, current_user, working_path, 1);
        } else if (ch == '\t') {
            char to_pick[1024];
            memset(to_pick, '\0', sizeof(to_pick));
            auto_complete_task(buffer, working_path, to_pick, &pos);
            refresh_terminal(buffer, current_user, working_path, 1); 
            if (strlen(to_pick) != 0){
                std_print("\n");
                refresh_terminal(to_pick, "", "", 0);
                refresh_terminal(buffer, current_user, working_path, 1); 
            }

        } else if (ch == '\b' || ch == 127){
            if (pos > 0){
                buffer[--pos] = '\0';
            }
            refresh_terminal(buffer, current_user, working_path, 1);
        } else if (isprint(ch)) {
            buffer[pos++] = ch;
            refresh_terminal(buffer, current_user, working_path, 1);
        }
    }

    restore_terminal_mode();
    return 0;
}