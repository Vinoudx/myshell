#include <commandrule.h>

const struct CommandRule alias_command = {"alias", "shell build in", 1, 0, {{"main", 1, ARG_STRING, 0}}};

const struct CommandRule cat_command = {"cat", "cat", 1, 0, {
    {"main", 1, ARG_FILE, 1}
}};

const struct CommandRule cd_command = {"cd", "shell build in", 1, 0, {
    {"main", 1, ARG_DIR, 1}
}};

const struct CommandRule echo_command = {"echo", "shell build in", 1, 0, {
    {"main", 1, ARG_STRING, 1},
    {"-n", 0, ARG_NONE, 0},
    {"-E", 0, ARG_NONE, 0},
    {"-e", 0, ARG_NONE, 0}
}};

const struct CommandRule grep_command = {"grep", "grep", 1, 0, {
    {"main", 1, ARG_STRING, 1}
}};

const struct CommandRule history_command = {"history", "shell build in", 1, 0, {
    {"main", 1, ARG_STRING, 0}
}};

const struct CommandRule ls_command = {"ls", "ls", 0, 2, {
    {"-l", 0, ARG_NONE, 0},
    {"-a", 0, ARG_NONE, 0}
}};

const struct CommandRule type_command = {"type", "shell build in", 1, 0, {
    {"main", 1, ARG_COMMAND, 1}
}};

int num_commands = 8;
struct CommandRule rules[] = {alias_command, cat_command, cd_command, echo_command, grep_command, history_command, ls_command, type_command};

struct CommandRule find_rule(const char* name){
    
    for(int i = 0; i < num_commands; i++){
        if(strcmp(name, rules[i].command_name) == 0){
            return rules[i];
        }
    }
    struct CommandRule err = {"error", 0, 0, {"err", 0, ARG_NONE, 0}};
    return err;
}

struct OptionRule find_option(const char* command_name, const char* op_name){
    struct CommandRule rule = find_rule(command_name);
    int ctrl = 0;
    if(strcmp(op_name, "main") == 0){
        ctrl = rule.num_main_params;
    }else{
        ctrl = rule.num_options;
    }
    for (int i=0; i<ctrl; i++){
        if(strncmp(op_name, rule.options[i].option, strlen(op_name)) == 0){
            return rule.options[i];
        }
    }
    struct OptionRule err = {"err", 0, ARG_NONE, 0};
    return err;
}
