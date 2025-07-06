#include <alias.h>

void alias_replace(char* buffer) {
    FILE* fp = fopen(ALIAS_PATH, "r");
    if (fp == NULL) {
        return;
    }

    char line[1024];
    while (fgets(line, sizeof(line), fp) != NULL) {
        // 去掉换行符
        line[strcspn(line, "\r\n")] = '\0';

        char* p = strchr(line, '=');
        if (!p) continue;

        *p = '\0';  // 将 '=' 替换为 '\0'，分成两部分
        const char* alias_name = line;
        const char* alias_value = p + 1;

        size_t alias_len = strlen(alias_name);

        // 判断 buffer 是否以 alias_name 开头，且后面是空格或结尾
        if (strncmp(buffer, alias_name, alias_len) == 0 &&
            (buffer[alias_len] == ' ' || buffer[alias_len] == '\0')) {

            // 构建新的 buffer： alias_value + 原 alias 后面的部分
            char temp[1024];
            snprintf(temp, sizeof(temp), "%s%s", alias_value, buffer + alias_len);
            strncpy(buffer, temp, 1024);
            buffer[1023] = '\0';
            break;  // 只替换一次
        }
    }

    fclose(fp);
}

void alias_(char* pre_result, int* status, const char** args, size_t num_args){
    if(num_args == 0){
        FILE* fp = fopen(ALIAS_PATH, "r");
        if(fp == NULL){
            strcpy(pre_result, "\n");
            fclose(fp);
            return;
        }
        char line[1024]; 
        while (fgets(line, sizeof(line), fp) != NULL) {
            strcat(pre_result, "alias ");
            strcat(pre_result, line);
        }
        fclose(fp);
        return;
    }else if(num_args > 1){
        strcpy(pre_result, "alias: too many arguments\n");
        *status = 0;
        return;
    }else{
        char* p = strchr(args[0], '=');
        if(p == NULL){
            strcpy(pre_result, "alias: invalid argument ");
            strcpy(pre_result, args[0]);
            strcpy(pre_result, "\n");
            *status = 0;
            return;
        }else if(strlen(p) == 1){
            strcpy(pre_result, "alias: invalid argument ");
            strcpy(pre_result, args[0]);
            strcpy(pre_result, "\n");
            *status = 0;
            return;
        }
        FILE* fp = fopen(ALIAS_PATH, "a+");
        char line[1024]; 
        while (fgets(line, sizeof(line), fp) != NULL) {
            if(strncmp(line, args[0], strlen(args[0])) == 0 && (line[strlen(args[0])] == '\n' || line[strlen(args[0])] == '\0')){
                return;
            }
        }
        fputs(args[0], fp);
        fputc('\n', fp);
        fclose(fp);
    }
}