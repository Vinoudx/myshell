#include <syntaxanalyser.h>

void tokenize_(const char* buffer, struct Token tokens[], size_t* token_pos){
    char ch;
    int word_pos = 0;
    char word[1024] = {'\0'};
    int buffer_pos = 0;
    struct Token token;
    INFO(buffer);
    while(buffer[buffer_pos] != '\0'){
        if(buffer[buffer_pos] == ' '){
            buffer_pos++;
        }else if(buffer[buffer_pos] == '-'){
            while(buffer[buffer_pos] != ' ' && buffer[buffer_pos] != '\0'){
                word[word_pos++] = buffer[buffer_pos++];
            }
            INFO(word);
            token.token_type = OPTION;
            memset(token.content, '\0', sizeof(token.content));
            strcpy(token.content, word);
            tokens[(*token_pos)++] = token;
            word_pos = 0;
            memset(word, '\0', sizeof(word));
        }else if(isalpha(buffer[buffer_pos])){
            while(buffer[buffer_pos] != ' ' && buffer[buffer_pos] != '\0'){
                word[word_pos++] = buffer[buffer_pos++];
            }
            INFO(word);
            token.token_type = IDENTIFIER;
            memset(token.content, '\0', sizeof(token.content));
            strcpy(token.content, word);
            tokens[(*token_pos)++] = token;
            word_pos = 0;
            memset(word, '\0', sizeof(word));
        }else if (buffer[buffer_pos] == '.' || buffer[buffer_pos] == '/' || buffer[buffer_pos] == '~') {
            // 捕获一个路径 token
            while (buffer[buffer_pos] != ' ' && buffer[buffer_pos] != '\0') {
                word[word_pos++] = buffer[buffer_pos++];
            }
            word[word_pos] = '\0';
            INFO(word);
            token.token_type = IDENTIFIER;
            memset(token.content, '\0', sizeof(token.content));
            strncpy(token.content, word, MAX_WORD_LENGTH);
            tokens[(*token_pos)++] = token;

            word_pos = 0;
            memset(word, '\0', sizeof(word));

        }else if(buffer[buffer_pos] == '$'){
            while(buffer[buffer_pos] != ' ' && buffer[buffer_pos] != '\0'){
                word[word_pos++] = buffer[buffer_pos++];
            }
            INFO(word);
            token.token_type = ENV;
            memset(token.content, '\0', sizeof(token.content));
            strcpy(token.content, word);
            tokens[(*token_pos)++] = token;
            word_pos = 0;
            memset(word, '\0', sizeof(word));
        }else if(buffer[buffer_pos] == '|'){
            INFO("|");
            token.token_type = PIPE_CHAR;
            memset(token.content, '\0', sizeof(token.content));
            strcpy(token.content, "|");
            tokens[(*token_pos)++] = token;
            buffer_pos++;
        }else if(buffer[buffer_pos] == '<'){
            INFO("<");
            token.token_type = RIN;
            memset(token.content, '\0', sizeof(token.content));
            strcpy(token.content, "<");
            tokens[(*token_pos)++] = token;
            buffer_pos++;
        }else if(buffer[buffer_pos] == '>'){
            INFO(">");
            token.token_type = ROUT;
            memset(token.content, '\0', sizeof(token.content));
            strcpy(token.content, ">");
            tokens[(*token_pos)++] = token;
            buffer_pos++;
        }else if(buffer[buffer_pos] == '&'){
            INFO("&");
            token.token_type = ASYN;
            memset(token.content, '\0', sizeof(token.content));
            strcpy(token.content, "&");
            tokens[(*token_pos)++] = token;
            buffer_pos++;
        }else if(buffer[buffer_pos] == '\"'){
            // 捕获一个字符串
            buffer_pos++; //不要第一个引号
            while (buffer[buffer_pos] != ' ' && buffer[buffer_pos] != '\0' && buffer[buffer_pos] != '\"') {
                word[word_pos++] = buffer[buffer_pos++];
            }
            buffer_pos++; // 不要第二个引号
            word[word_pos] = '\0';
            INFO(word);
            token.token_type = IDENTIFIER;
            memset(token.content, '\0', sizeof(token.content));
            strncpy(token.content, word, MAX_WORD_LENGTH);
            tokens[(*token_pos)++] = token;
            word_pos = 0;
            memset(word, '\0', sizeof(word));
        }
    }
    token.token_type = END;
    strcpy(token.content, "end");
    tokens[(*token_pos)++] = token;
}

struct SyntaxAnalyseResult syntax_analyser(struct Token tokens[], size_t token_pos){
    struct SyntaxAnalyseResult result;
    result.isValid = 1;
    result.num_command = 0; // 这个是从一开始的，属于后期发现前期设计失误不好改了
    result.num_connect_char = 0;
    memset(result.command_length, 0, sizeof(result.command_length));
    strcpy(result.error_info, "No Error");
    // memset(result.command_list, '\0', sizeof(result.command_list));

    char single_command[MAX_COMMAND_LENGTH];
    int first_word = 1;
    int now = 0;
    int pre = 0;
    size_t current_length = 0;
    char current_command[MAX_COMMAND_LENGTH];
    while(now < token_pos){
        if(tokens[now].token_type == IDENTIFIER){ // 当前是标识符
            if(first_word == 1){ // 第一个标识符，是一个命令
                first_word = 0;
                tokens[now].token_type = COMMAND;
                memset(current_command, '\0', sizeof(current_command));
                strcpy(current_command, tokens[now].content);
                result.num_command++;
                pre = now; now++;
            }else{                 
                // 不是第一个标识符，看前一个词，如果是命令，则是main参数
                if(tokens[pre].token_type == COMMAND){
                    struct CommandRule rule = find_rule(current_command);
                    if(strcmp(rule.command_name, "error") == 0){
                        result.isValid = 0;
                        snprintf(result.error_info, sizeof(result.error_info), "you have syntax error near %.30s", tokens[now].content);
                        return result;
                    }
                    struct OptionRule op = find_option(tokens[pre].content, "main");
                    for (int i=0; i<rule.num_main_params; i++){
                        if(tokens[now].token_type == IDENTIFIER){
                            switch (op.arg_type)
                            {
                            case ARG_STRING:
                                tokens[now].token_type = ESCAPE;
                                break;
                            case ARG_FILE:
                                tokens[now].token_type = FILE_;
                                break;
                            case ARG_DIR:
                                tokens[now].token_type = FOLDER;
                                break;
                            case COMMAND:
                                tokens[now].token_type = COMMAND;
                                break;
                            }
                            pre = now; now++;
                        }else{
                            result.isValid = 0;
                            snprintf(result.error_info, sizeof(result.error_info), "you have syntax error near %.30s", tokens[now].content);
                            return result;
                        }
                    }
                }else if(tokens[pre].token_type == OPTION){
                    // 如果前一个是选项，则是选项参数
                    if (strlen(current_command) == 0){
                        result.isValid = 0;
                        snprintf(result.error_info, sizeof(result.error_info), "you have syntax error near %.30s", tokens[now].content);
                        return result;
                    }
                    struct OptionRule op = find_option(current_command, tokens[pre].content);
                        switch (op.arg_type)
                        {
                        case ARG_STRING:
                            tokens[now].token_type = ESCAPE;
                            break;
                        case ARG_FILE:
                            tokens[now].token_type = FILE_;
                            break;
                        case ARG_DIR:
                            tokens[now].token_type = FOLDER;
                            break;
                        case COMMAND:
                            tokens[now].token_type = COMMAND;
                            break;
                        }
                        pre = now; now++;
                }else if(tokens[pre].token_type == RIN || tokens[pre].token_type == ROUT){
                    //如果前一个是重定向，则是文件
                    tokens[now].token_type = FILE_;
                    pre = now; now++;
                }else if(tokens[pre].token_type == EXEC_FILE){
                    // 如果前一个是可执行文件，则不用管
                    tokens[now].token_type = ESCAPE;
                    pre = now; now++;
                }else if(tokens[pre].token_type == ENV && strcmp(current_command, "echo") == 0){
                    // 如果前一个是环境变量，给echo开后门，则不用管
                    tokens[now].token_type = ESCAPE;
                    pre = now; now++;
                }else{
                    // 如果是其他的那么出错    
                    result.isValid = 0;
                    snprintf(result.error_info, sizeof(result.error_info), "you have syntax error near %.30s", tokens[now].content);
                    return result;
                }
            }
        }else if(tokens[now].token_type == PIPE_CHAR){
            if(tokens[now+1].token_type != IDENTIFIER){
                result.isValid = 0;
                snprintf(result.error_info, sizeof(result.error_info), "you have syntax error near %.30s", tokens[now].content);      
                return result;         
            }
            first_word = 1;
            result.connection_char[result.num_connect_char++] = tokens[now].content[0];
            // strcpy(result.command_list[result.num_command], single_command);
            // memset(single_command, '\0', sizeof(single_command));
            result.command_length[result.num_command] = current_length;
            current_length = 0;
            
            result.num_connect_char++;
            pre = now; now++;
            continue;
        }else if(tokens[now].token_type == END){
            // strcpy(result.command_list[result.num_command], single_command);
            result.command_length[result.num_command] = current_length;
            result.last_word = tokens[pre];
            pre = now; now++;
            break;
        }else if(tokens[now].token_type == ASYN){
            result.isAsyn = 1;
            // strcpy(result.command_list[result.num_command], single_command);
            result.command_length[result.num_command] = current_length;
            result.last_word = tokens[pre];
            pre = now; now++;
            break;
        }else{
            pre = now; now++;
        }
        // strcat(single_command, tokens[pre].content);
        // strcat(single_command, " ");
        result.command_list[result.num_command][current_length++] = tokens[pre];
    }
    return result;
}
