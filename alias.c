#include <alias.h>

void alias_replace(char* buffer){
    // FILE* fp = fopen("../configs/alias", "r");
    FILE* fp = fopen(ALIAS_PATH, "r");
    if (fp == NULL) {
        perror("fopen");
        return;
    }

    char line[1024];
    char tmpbuf[8192]; 
    size_t bufsize = 1024;
    while (fgets(line, sizeof(line), fp) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        char* equal_sign = strchr(line, '=');
        if (!equal_sign) continue;

        *equal_sign = '\0';
        char* alias_name = line;
        char* alias_value = equal_sign + 1;

        size_t name_len = strlen(alias_name);
        size_t value_len = strlen(alias_value);

        char* src = buffer;
        char* dest = tmpbuf;
        size_t remaining = bufsize - 1;

        while (*src && remaining > 0) {
            if (strncmp(src, alias_name, name_len) == 0) {
                // 替换 alias_name 为 alias_value
                if (value_len > remaining) break;
                memcpy(dest, alias_value, value_len);
                dest += value_len;
                src += name_len;
                remaining -= value_len;
            } else {
                *dest++ = *src++;
                remaining--;
            }
        }
        *dest = '\0';

        strncpy(buffer, tmpbuf, bufsize);
        buffer[bufsize - 1] = '\0';
    }

    fclose(fp);
}

void alias_(char* pre_result, int* status, const char** args, size_t num_args){
    if(num_args == 0){
        FILE* fp = fopen("ALIAS_PATH", "r");
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
        FILE* fp = fopen("ALIAS_PATH", "a+");
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