#include <autocomplete.h>

// ..
// ./aaa.txt
// aaa.txt
// ../ 


struct AutoCompleteResult autocomplete(const char* buffer, const char* working_path) {
    int ch_res = 0;
    struct AutoCompleteResult results;
    results.amount = 0;
    char* parameter = strrchr(buffer, ' ');
    if (parameter == NULL){
        parameter = buffer;
    }else{
        parameter = parameter+1;
    }

    //直接找/，/前是路径，/后是prefix
    char relative_path[100] = {'\0'};
    char* prefix = strrchr(parameter, '/');
    char* target_path = NULL;

    if (prefix == NULL){
        prefix = parameter;
    }else {
        strncpy(relative_path, parameter, strlen(parameter) - strlen(prefix)); 
        prefix = prefix+1;
    }

    if (strlen(relative_path) != 0){
        target_path = relative_path;
    }else{
        target_path = working_path;
    }
 
    char current_path1[100];
    memset(current_path1, '\0', sizeof(current_path1));
    getcwd(current_path1, sizeof(current_path1));

    ch_res = chdir(target_path);
    if (ch_res != 0){
        WARNING("no such folder");
        return results;
    }

    char current_path[100];
    memset(current_path, '\0', sizeof(current_path));
    getcwd(current_path, sizeof(current_path));
    
    // logger(relative_path, INFO, __FILE__, __LINE__);
    // logger(prefix, INFO, __FILE__, __LINE__);
    // logger(current_path, INFO, __FILE__, __LINE__);

    DIR *dir = opendir(current_path);
    if (!dir) return results;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, prefix, strlen(prefix)) == 0) {
            results.results[results.amount] = entry->d_name;
            if (entry->d_type == DT_DIR){
                strcat(results.results[results.amount], "/");
            }
            results.amount ++;
        }
    }
    closedir(dir);
    ch_res = chdir(working_path);
    if (ch_res != 0){
        logger("cant switch working folder back", ERROR, __FILE__, __LINE__);
    }
    return results;
}