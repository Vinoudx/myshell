#include <cat.h>

#define MAX_INPUT_FILES 64
#define MAX_LINE_LEN 1024

// 处理不可打印字符类似 cat -A 的显示
static void show_nonprint_chars(char* dest, const char* src) {
    // 处理字符，不打印控制字符，换行不变，Tab转为 ^I，其他控制字符用 ^X 形式表示
    while (*src) {
        unsigned char c = (unsigned char)*src;
        if (c == '\t') {
            *dest++ = '^'; *dest++ = 'I';
        } else if (c == '\n') {
            *dest++ = '\n';
        } else if (c < 32) {
            *dest++ = '^';
            *dest++ = c + 64;  // Ctrl-X 表示
        } else if (c == 127) {
            *dest++ = '^';
            *dest++ = '?';
        } else {
            *dest++ = c;
        }
        src++;
    }
    *dest = '\0';
}

void cat_(char* pre_result, int* status, const char** args, size_t num_args) {
    *status = 1;
    pre_result[0] = '\0';

    // 参数标记
    int opt_n = 0;  // -n 所有行编号
    int opt_b = 0;  // -b 非空行编号
    int opt_A = 0;  // -A 显示不可打印字符
    int opt_E = 0;  // -E 显示行尾 $

    int redirect_in = 0, redirect_out = 0;
    const char* output_file = NULL;

    // 输入文件列表
    const char* input_files[MAX_INPUT_FILES];
    size_t input_count = 0;

    // 解析阶段变量
    size_t i = 0;
    int parsing_input_redirect_files = 0;  // 处于 < 后面接收输入文件状态

    while (i < num_args) {
        const char* arg = args[i];
        if (strcmp(arg, "<") == 0) {
            redirect_in = 1;
            parsing_input_redirect_files = 1;
            i++;
            continue;
        }
        if (strcmp(arg, ">") == 0) {
            if (i + 1 >= num_args) {
                snprintf(pre_result, 1024, "cat: syntax error near unexpected token `newline' after '>'\n");
                *status = 0;
                return;
            }
            redirect_out = 1;
            output_file = args[i + 1];
            i += 2;
            break;  // 输出文件是最后的，跳出解析循环
        }
        if (arg[0] == '-' && !parsing_input_redirect_files) {
            // 处理参数 -n, -b, -A, -E，支持连写如 -nA
            size_t j = 1;
            while (arg[j]) {
                switch (arg[j]) {
                    case 'n': opt_n = 1; break;
                    case 'b': opt_b = 1; break;
                    case 'A': opt_A = 1; break;
                    case 'E': opt_E = 1; break;
                    default:
                        snprintf(pre_result, 1024, "cat: invalid option -- '%c'\n", arg[j]);
                        *status = 0;
                        return;
                }
                j++;
            }
            i++;
            continue;
        }
        // 普通文件名处理
        if (parsing_input_redirect_files) {
            if (input_count >= MAX_INPUT_FILES) {
                snprintf(pre_result, 1024, "cat: too many input files\n");
                *status = 0;
                return;
            }
            input_files[input_count++] = arg;
            i++;
            continue;
        }
        // 普通输入文件
        if (!parsing_input_redirect_files) {
            if (input_count >= MAX_INPUT_FILES) {
                snprintf(pre_result, 1024, "cat: too many input files\n");
                *status = 0;
                return;
            }
            input_files[input_count++] = arg;
            i++;
            continue;
        }
    }

    // 如果 < 之后没文件报错
    if (redirect_in && input_count == 0) {
        snprintf(pre_result, 1024, "cat: missing input file after '<'\n");
        *status = 0;
        return;
    }
    // 如果没有任何输入文件，报错
    if (input_count == 0) {
        snprintf(pre_result, 1024, "cat: missing file operand\n");
        *status = 0;
        return;
    }
    // -n 和 -b 互斥，后者优先
    if (opt_b) opt_n = 0;

    FILE* out_fp = NULL;
    if (redirect_out) {
        out_fp = fopen(output_file, "w");
        if (!out_fp) {
            snprintf(pre_result, 1024, "cat: cannot open '%s' for writing\n", output_file);
            *status = 0;
            return;
        }
    }

    size_t line_num = 1;
    char linebuf[MAX_LINE_LEN];
    char tmpbuf[MAX_LINE_LEN * 3];  // 用于 -A 处理后行

    for (size_t fidx = 0; fidx < input_count; ++fidx) {
        FILE* in_fp = fopen(input_files[fidx], "r");
        if (!in_fp) {
            snprintf(pre_result, 1024, "cat: %s: No such file or cannot open\n", input_files[fidx]);
            *status = 0;
            if (out_fp) fclose(out_fp);
            return;
        }

        while (fgets(linebuf, sizeof(linebuf), in_fp)) {
            int print_line_num = 0;
            if (opt_n) {
                print_line_num = 1;
            } else if (opt_b && linebuf[0] != '\n') {
                print_line_num = 1;
            }

            if (opt_A) {
                show_nonprint_chars(tmpbuf, linebuf);
            } else {
                strncpy(tmpbuf, linebuf, sizeof(tmpbuf) - 1);
                tmpbuf[sizeof(tmpbuf) - 1] = '\0';
            }
            // 添加 $ 结尾
            if (opt_E) {
                size_t len = strlen(tmpbuf);
                if (len > 0 && tmpbuf[len-1] == '\n') {
                    tmpbuf[len-1] = '$';
                    tmpbuf[len] = '\n';
                    tmpbuf[len+1] = '\0';
                } else {
                    strcat(tmpbuf, "$");
                }
            }

            char out_line[MAX_LINE_LEN * 4];
            if (print_line_num) {
                snprintf(out_line, sizeof(out_line), "%6zu\t%s", line_num++, tmpbuf);
            } else {
                snprintf(out_line, sizeof(out_line), "%s", tmpbuf);
                if (opt_n || opt_b) {
                    // 若不打印行号，且已启用行号参数，则也自增保证正确行号
                    line_num++;
                }
            }

            if (redirect_out) {
                fputs(out_line, out_fp);
            } else {
                if (strlen(pre_result) + strlen(out_line) >= 8192) {
                    snprintf(pre_result, 1024, "cat: output buffer overflow\n");
                    *status = 0;
                    fclose(in_fp);
                    if (out_fp) fclose(out_fp);
                    return;
                }
                strcat(pre_result, out_line);
            }
        }
        fclose(in_fp);
    }
    if (redirect_out) fclose(out_fp);
}