#include <history.h>

#define MAX_LINE_LEN 1024
#define MAX_RESULT_LEN 8192

void read_last_n_lines(char* pre_result, int n) {
    FILE* fp = fopen(HISTORY_PATH, "r");

    fseek(fp, 0, SEEK_END);
    long pos = ftell(fp);
    int line_count = 0;

    while (pos > 0 && line_count <= n) {
        pos--;
        fseek(fp, pos, SEEK_SET);
        char ch = fgetc(fp);
        if (ch == '\n') {
            line_count++;
        }
    }

    if (pos != 0) {
        fseek(fp, pos + 1, SEEK_SET);
    } else {
        rewind(fp);
    }

    pre_result[0] = '\0';
    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), fp)) {
        strcat(pre_result, line);
    }

    fclose(fp);
}

int count_lines() {
    FILE* fp = fopen(HISTORY_PATH, "r");
    if (!fp) return 0;

    int count = 0;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), fp)) {
        count++;
    }

    fclose(fp);
    return count;
}

int save_history(const char* line_content) {
    int line_number = count_lines() + 1;

    FILE* fp = fopen(HISTORY_PATH, "a");

    fprintf(fp, "%d %s\n", line_number, line_content);
    fclose(fp);
    return 0;
}

int read_last_nth_line(char* buffer, int nn) {
    int n = nn;
    if (n <= 0 || buffer == NULL) return 0;

    FILE* fp = fopen(HISTORY_PATH, "r");
    if (!fp) {
        perror("fopen failed");
        return 0;
    }

    int total_lines = count_lines();
    char line[MAX_LINE_LEN];

    n = ((n-1) % total_lines) + 1;

    rewind(fp);
    int target_line = total_lines - n + 1;
    int current_line = 0;

    while (fgets(line, sizeof(line), fp)) {
        current_line++;
        if (current_line == target_line) {
            char* p = strchr(line, ' ');
            p++;
            strncpy(buffer, p, strlen(p) - 1);
            buffer[MAX_LINE_LEN - 1] = '\0';
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}


void history_(char* pre_result, int* status, const char** args, size_t num_args){
    int num_lines = 0;
    if(num_args == 0){
        num_lines = count_lines();
    }else if(num_args == 1){
        if(strcmp(args[0], "-c") == 0){
            FILE* fp = fopen(HISTORY_PATH, "w");
            fclose(fp);
        }else{
            num_lines = atoi(args[0]);
        }    
    }else{
        strcpy(pre_result, "history: too many arguments\n");
        *status = 0;
        return;
    }
    read_last_n_lines(pre_result, num_lines);
}