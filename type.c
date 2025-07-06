#include <type.h>

void search_alias(const char* name, char* result){
    FILE* fp = fopen(ALIAS_PATH, "r");
    char line[1024]; 
    while (fgets(line, sizeof(line), fp) != NULL) {
        if(strncmp(line, name, strlen(name)) == 0 && (line[strlen(name)] == '=')){
            strncpy(result, line, strlen(line) - 1);
            return;
        }
    }
    fclose(fp);
}

int is_executable(const char* path) {
    return access(path, F_OK | X_OK) == 0;
}

void command_type(const char* name, char* result){
    struct CommandRule rule = find_rule(name);
    if(strcmp(rule.command_name, "error") != 0){
        strcpy(result, rule.type);
    }
}

void type_(char* pre_result, int* status, const char** args, size_t num_args){
    char targets[16][128];
    int num_targets = 0;
    int abs_path = 0;
    int t = 0;
    for(int i=0; i<num_args; i++){
        if(strcmp(args[i], "-t") == 0){
            t = 1;
        }else if(strcmp(args[i], "-p") == 0){
            abs_path = 1;
        }else{
            strcpy(targets[num_targets++], args[i]);
        }
    }

    for(int i=0;i<num_targets;i++){
        char result[1024] = {'\0'};
        char temp[1024] = {'\0'};
        search_alias(targets[i], result);
        if(strlen(result) != 0){
            if(t == 0){
                sprintf(temp, "%s is alias: %s\n", targets[i], result);
                strcat(pre_result, temp);
            }else{
                strcat(pre_result, "alias\n");
            }
            continue;
        }

        command_type(targets[i], result);
        if(strlen(result) != 0){
            if(t == 0){
                if(abs_path == 1){
                    sprintf(temp, "%s %s\n", BIN_PATH, targets[i]);
                    strcat(pre_result, temp);
                }else{
                    sprintf(temp, "%s is %s\n", targets[i], result);
                    strcat(pre_result, temp);
                }
            }else{
                if(strcmp(result, "shell build in") == 0){
                    strcat(pre_result, "bulidin\n");
                }else{
                    strcat(pre_result, "file\n");
                }
            }
            continue;
        }

        if(is_executable(args[i])){
            if(t == 0){
                if(abs_path == 1){
                    sprintf(temp, "%s %s\n", BIN_PATH, targets[i]);
                    strcat(pre_result, temp);
                }else{
                    sprintf(temp, "%s is %s\n", targets[i], targets[i]);
                    strcat(pre_result, temp);
                }
            }else{
                strcat(pre_result, "file\n");
            }
            continue;
        }else{
            sprintf(temp, "type: unknown argument: %s\n", targets[i]);
            strcat(pre_result, temp);
        }
    }

}