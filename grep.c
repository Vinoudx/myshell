#include <grep.h>

#define MAX_LINE_LEN 1024

const char* strcasestr_custom(const char* haystack, const char* needle) {
    if (!*needle) return haystack;

    for (; *haystack; haystack++) {
        const char* h = haystack;
        const char* n = needle;

        while (*h && *n && tolower((unsigned char)*h) == tolower((unsigned char)*n)) {
            h++;
            n++;
        }

        if (!*n) return haystack;  // 完全匹配
    }

    return NULL;
}

int grep_lines_in_string(const char* pattern, const char* input, char* buffer, int flag) {
    size_t bufsize = 1024;
    if (!pattern || !input || !buffer || bufsize == 0) return -1;

    size_t used = 0;
    int count = 0;

    const char* line_start = input;
    while (*line_start) {
        const char* line_end = strchr(line_start, '\n');
        size_t line_len = line_end ? (size_t)(line_end - line_start) : strlen(line_start);

        // 提取当前行
        char line[MAX_LINE_LEN];
        if (line_len >= sizeof(line)) line_len = sizeof(line) - 1;
        strncpy(line, line_start, line_len);
        line[line_len] = '\0';

        // 判断是否匹配（根据 flag）
        int matched = 0;
        if (flag == 1) {
            if (strcasestr_custom(line, pattern)) matched = 1;
        } else {
            if (strstr(line, pattern)) matched = 1;
        }

        if (matched) {
            size_t len = strlen(line);
            if (used + len + 2 >= bufsize) break; // 空间不足
            strcpy(buffer + used, line);
            used += len;
            buffer[used++] = '\n';
            buffer[used] = '\0';
            count++;
        }

        // 移动到下一行
        line_start = line_end ? line_end + 1 : NULL;
        if (!line_start) break;
    }

    return count;
}

int grep_lines(const char* pattern, const char* path, char* buffer, int flag) {
    size_t bufsize = 1024;
    if (!pattern || !path || !buffer || bufsize == 0) return -1;

    FILE* fp = fopen(path, "r");
    if (!fp) {
        strcpy(buffer, "grep: no such file\n");
        return -1;
    }

    char line[MAX_LINE_LEN];
    size_t used = 0;
    int count = 0;

    while (fgets(line, sizeof(line), fp)) {
        int matched = 0;

        if (flag == 1) {
            if (strcasestr_custom(line, pattern)) matched = 1;
        } else {
            if (strstr(line, pattern)) matched = 1;
        }

        if (matched) {
            size_t len = strlen(line);

            // 去除换行符
            if (len > 0 && line[len - 1] == '\n') {
                line[len - 1] = '\0';
                len--;
            }

            if (used + len + 2 >= bufsize) {
                break; 
            }

            strcpy(buffer + used, line);
            used += len;
            buffer[used++] = '\n';
            buffer[used] = '\0';
            count++;
        }
    }

    fclose(fp);
    return count;
}

void grep_(char* pre_result, int* status, const char** args, size_t num_args){
    int pip = 0;

    if(strlen(pre_result) != 0){
        pip = 1;
    }

    char patten[1024] = {'\0'};
    char in_file[1024] = {'\0'};
    char out_file[1024] = {'\0'};
    int i_flag = 0;

    for(int i=0;i<num_args;i++){
        if(i == 0){
            strcpy(patten, args[i]);
        }
        if(strcmp(args[i], "<") == 0){
            i++;
            if(i >= num_args || pip){
                strcpy(pre_result, "grep: invalid io_redirection\n");
                *status = 0;
                return;
            }
            strcpy(in_file, args[i]);
        }else if(strcmp(args[i], ">") == 0){
            i++;
            if(i >= num_args){
                strcpy(pre_result, "grep: invalid io_redirection\n");
                *status = 0;
                return;
            }
            strcpy(out_file, args[i]);
        }else if(strcmp(args[i], "-i") == 0){
            i_flag = 1;
        }else if(i == 1){
            strcpy(in_file, args[i]);
        }
    }
    if(strlen(in_file) == 0 && pip == 0){
        strcpy(pre_result, "grep: missing input file\n");
        *status = 0;
        return;
    }

    INFO(pre_result);
    INFO(out_file);

    char temp[1024] = {'\0'};
    if(pip == 1){
        grep_lines_in_string(patten, pre_result, temp, i_flag);
    }else{
        grep_lines(patten, in_file, temp, i_flag);
    }
    if(strlen(out_file) == 0){
        strcpy(pre_result, temp);
    }else{
        FILE* fp = fopen(out_file, "w");
        if(fp == NULL){
            strcpy(pre_result, "grep: no such file\n");
            return -1;
        }
        fputs(temp, fp);
        fclose(fp);
        memset(pre_result, '\0', 1024);
    }

}