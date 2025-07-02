#include <autocomplete.h>


extern char **environ;
extern int num_commands;
extern struct CommandRule rules[];

// ..
// ./aaa.txt
// aaa.txt
// ../ 

// 负责文件的补全
struct AutoCompleteResult file_autocomplete(const char* buffer, const char* working_path, const char* last_word, int isFolder, int isFile, int isExecAble) {
    int ch_res = 0;
    struct AutoCompleteResult results;
    results.amount = 0 ;
    memset(results.results, '\0', sizeof(results.results));
    int all_match = 0;
    char* prefix = strrchr(last_word, '/');
    char relative_path[100];
    memset(relative_path, '\0', sizeof(relative_path));
    if(prefix == NULL)prefix = "\0";

    if(last_word[0] == '.'){
        strncpy(relative_path, last_word, strlen(last_word) - strlen(prefix));
        if(strlen(prefix) == 0 || strlen(prefix) == 1){all_match = 1;}else{prefix += 1;} // 若/后没有内容或者没有/则为全部匹配
        results.replace_pos = strlen(buffer) - strlen(strstr(buffer, prefix));
    }else{
        if(strlen(prefix) == 0){
            prefix = last_word;
            results.replace_pos = strlen(buffer) - strlen(strstr(buffer, prefix));
        }else{
            strncpy(relative_path, last_word, strlen(last_word) - strlen(prefix));
            if(strlen(prefix) == 0 || strlen(prefix) == 1){all_match = 1;}else{prefix += 1;} // 若/后没有内容或者没有/则为全部匹配
            results.replace_pos = strlen(buffer) - strlen(strstr(buffer, prefix));
        }
    }

    INFO(prefix);
    INFO(relative_path);

    char* target_path = NULL;

    // 切换路径
    if (strlen(relative_path) != 0){
        target_path = relative_path;
    }else{
        target_path = working_path;
    }
    
    INFO(target_path);
    ch_res = chdir(target_path);
    if (ch_res != 0){
        WARNING("no such folder");
        return results;
    }
    // 获得当前目录路径
    char current_path[100];
    memset(current_path, '\0', sizeof(current_path));
    getcwd(current_path, sizeof(current_path));

    DIR *dir = opendir(current_path);
    if (!dir) return results;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, prefix, strlen(prefix)) == 0 || all_match == 1) {
            struct stat st;
            stat(entry->d_name, &st);
            if (isFolder == 1){
                if(S_ISDIR(st.st_mode)){
                    strcpy(results.results[results.amount], entry->d_name);
                    strcat(results.results[results.amount], "/");
                    results.amount ++;
                }
            }
            if(isExecAble == 1 && isFile == 1){
                __uid_t uid = getuid();   // 当前用户ID
                __gid_t gid = getgid();   // 当前组ID
                if ((S_ISREG(st.st_mode) &&
    (
        (uid == st.st_uid && (st.st_mode & S_IXUSR)) ||
        (gid == st.st_gid && (st.st_mode & S_IXGRP)) ||
        (st.st_mode & S_IXOTH)
    ))) {
                    strcpy(results.results[results.amount++], entry->d_name);
                }
            }
            if(isFile == 1 && isExecAble == 0){
                if(S_ISREG(st.st_mode)){
                    strcpy(results.results[results.amount++], entry->d_name);
                }
            }
            //else{
            //     strcpy(results.results[results.amount], entry->d_name);
            //     if(S_ISDIR(st.st_mode)) strcat(results.results[results.amount], "/");
            //     results.amount ++;
            // }
        }
    }
    closedir(dir);
    ch_res = chdir(working_path);
    if (ch_res != 0){
        logger("cant switch working folder back", ERROR, __FILE__, __LINE__);
    }
    return results;
}

struct AutoCompleteResult env_autocomplete(const char* buffer, const char* last_word){
    struct AutoCompleteResult results;
    results.amount = 0 ;
    results.replace_pos = strlen(buffer) - strlen(strstr(buffer, last_word));
    memset(results.results, '\0', sizeof(results.results));
    char* prefix = strchr(last_word, '$');
    int all_match = 0;
    if(prefix == NULL)prefix = "\0";
    if(strlen(prefix) == 0 || strlen(prefix) == 1){
        all_match = 1;
        prefix = "";
    }else{
        prefix += 1;
    }
    char **env = environ;
    while (*env) {
        // 环境变量格式是 "NAME=VALUE"
        if ((strncmp(*env, prefix, strlen(prefix)) == 0) || all_match == 1) {
            strncpy(results.results[results.amount], *env, strlen(*env) - strlen(strchr(*env, '=')));
            results.amount++;
        }
        env++;
    }
    return results;
}

struct AutoCompleteResult command_autocomplete(const char* buffer, const char* last_word){
    struct AutoCompleteResult results;
    results.amount = 0 ;
    results.replace_pos = strlen(buffer) - strlen(strstr(buffer, last_word));
    memset(results.results, '\0', sizeof(results.results));
    for (int i=0; i<num_commands; i++){
        if(strncmp(rules[i].command_name, last_word, strlen(last_word)) == 0){
            strcpy(results.results[results.amount++], rules[i].command_name);
        }
    }
    return results;
}

struct AutoCompleteResult option_autocomplete(const char* buffer, const char* last_word, const char* command_name){
    struct AutoCompleteResult results;
    results.amount = 0 ;
    results.replace_pos = strlen(buffer) - strlen(strstr(buffer, last_word));
    memset(results.results, '\0', sizeof(results.results));

    struct CommandRule command = find_rule(command_name);
    for (int i=0; i<command.num_options; i++){
        INFO(command.options[i].option);
        INFO(last_word);
        if(strncmp(command.options[i].option, last_word, strlen(last_word)) == 0){
            strcpy(results.results[results.amount++], command.options[i].option);
        }
    }
    return results;
}