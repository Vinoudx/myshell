#include <cd.h>

void cd_(char* pre_result, int* status, const char** args, size_t num_args){
    if(num_args > 1){
        strcpy(pre_result, "cd: too many arguments\n");
        *status = 0;
        return;
    }
    int cd_ctrl = chdir(args[0]);
    INFO(args[0]);
    if(cd_ctrl == 0){
        // strcpy(pre_result, "\n");
    }else{
        strcpy(pre_result, "cd: ");
        strcat(pre_result, strerror(errno));
        strcat(pre_result, "\n");
        *status = 0;
    }
    INFO(pre_result);
}