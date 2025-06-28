#include <terminal_logger.h>

char* get_enum_item(enum logLevel level){
    switch (level){
    case INFO:
        return "INFO";
    case WARNING:
        return "WARNING";
    case ERROR:
        return "ERROR";
    default:
        return "";
    }
}

// INFO time file line information
void formatter(char* target, const char* log, enum logLevel level, const char* file, size_t line_no){
    char* target_log = log;
    if (log == NULL){
        target_log = " ";
    }
    if(strlen(target_log) > MAX_INPUT_LOG_LENTH){
        target_log[90] = '\0';
    }
    char* level_string = get_enum_item(level);
    time_t current_time = time(NULL);
    char result[MAX_LOG_LENTH];
    memset(result, '\0', sizeof(result));
    sprintf(result, "\n%s, %ld in %s line %ld: %s\n",level_string, current_time, file, line_no, target_log);
    memcpy(target, result, strlen(result));
}

void logger(const char* log, enum logLevel level,const char* file, size_t line_no){
    struct termios terminal;
    tcgetattr(STDIN_FILENO, &terminal);
    char log_target[MAX_LOG_LENTH];
    memset(log_target, '\0', sizeof(log_target));
    formatter(log_target, log, level,file, line_no);
    if ((terminal.c_lflag & (ICANON | ECHO)) == 0){
        write(STDOUT_FILENO, log_target, strlen(log_target));
    }else{
        printf("%s", log_target);
    }
}
