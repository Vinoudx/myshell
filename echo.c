#include <echo.h>

void get_escape_character(char* target, const char* s){
    if(strcmp(s, "\\\\") == 0){
        *target = '\\';
    }else if(strcmp(s, "\\'") == 0){
        *target = '\'';
    }else
    if(strcmp(s, "\\\"") == 0){
        *target = '\"';
    }else
    if(strcmp(s, "\\n") == 0){
        *target = '\n';
    }else
    if(strcmp(s, "\\r") == 0){
        *target = '\r';
    }else
    if(strcmp(s, "\\t") == 0){
        *target = '\t';
    }else
    if(strcmp(s, "\\v") == 0){
        *target = '\v';
    }else
    if(strcmp(s, "\\b") == 0){
        *target = '\b';
    }else
    if(strcmp(s, "\\f") == 0){
        *target = '\f';
    }else
    if(strcmp(s, "\\a") == 0){
        *target = '\a';
    }else
    if(strcmp(s, "\\?") == 0){
        *target = '\?';
    }else
    if(strcmp(s, "\\0") == 0){
        *target = '\0';
    }else
    {*target = 'a';}

}

void replace_(char* content){
    int n = strlen(content);
    for (int i=0;i<n;i++){
        if(content[i] == '\\'){
            char s[2];
            char target[1];
            strncpy(s, content+i, 2);
            get_escape_character(target, s);
            if(*target != 'a'){
                *(content+i) = target[0];
                memmove((content+i) + 1, (content+i) + 2, strlen((content+i) + 2) + 1);
                n--;
            }
        }
    }
}

void echo_(char* pre_result, int* status, const char** args, size_t num_args){
    char content[1024] = {'\0'};
    int escape_n = 0;
    int activate_char = 0;
    char in_name[1024] = {'\0'};
    char out_name[1024] = {'\0'};
    for (int i=0; i<num_args; i++){
        if(strcmp(args[i], "-n") == 0){
            escape_n = 1;
        }else if(strcmp(args[i], "-e") == 0){
            activate_char = 1;
        }else if(strcmp(args[i], "-E") == 0){
            activate_char = 0;
        }else if(strcmp(args[i], "<") == 0){
            i++;
            if(i >= num_args){
                strcpy(pre_result, "echo: invalid io_redirection\n");
                *status = 0;
                return;
            }
            strcpy(in_name, args[i]);

        }else if(strcmp(args[i], ">") == 0){
            i++;
            if(i >= num_args){
                strcpy(pre_result, "echo: invalid io_redirection\n");
                *status = 0;
                return;
            }
            strcpy(out_name, args[i]);
        }else{
            char* p = strchr(args[i], '$');
            if(p != NULL){
                strcat(content, getenv(++p));
            }else{
                strcat(content, args[i]);
            }
            strcat(content, " ");
        }
    }

    if(strlen(in_name) != 0){
        FILE* fp = fopen(in_name, "r");
        if(fp == NULL){
            strcpy(pre_result, "echo: no such file\n");
            *status = 0;
            return;
        }
        char line[1024]; 
        while (fgets(line, sizeof(line), fp) != NULL) {
            strncat(content, line, strlen(line) - 1);
        }
        fclose(fp);
    }

    if(activate_char == 1){
        replace_(content);
    }

    if(strlen(out_name) != 0){
        FILE* fp = fopen(out_name, "w");
        char line[1024]; 
        fputs(content, fp);
        fclose(fp);
    }else{
        strncpy(pre_result, content, 1024);
        if(escape_n != 1){
            strcat(pre_result, "\n");
        }
    }
}