#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKEN_LEN 100

// 定義各種詞法單元（Token）種類
enum TokenType {
    TOKEN_TYPE = 1, TOKEN_IF, TOKEN_MAIN, TOKEN_ELSE, TOKEN_WHILE,
    TOKEN_EQUAL = 11, TOKEN_GREATER, TOKEN_LESS, TOKEN_ASSIGN,
    TOKEN_LITERAL, TOKEN_PLUS, TOKEN_MINUS,
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE, TOKEN_SEMICOLON,
    TOKEN_GREATER_EQUAL, TOKEN_LESS_EQUAL, TOKEN_IDENTIFIER
};

// 定義 DFA 的三種狀態
enum State {
    STATE_START,
    STATE_IDENTIFIER,
    STATE_NUMBER
};

// 檢查是否為字母
int is_alpha(int c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

// 檢查是否為數字
int is_digit(int c) {
    return c >= '0' && c <= '9';
}

// 檢查是否為字母、數字或底線
int is_alnum_or_underscore(int c) {
    return is_alpha(c) || is_digit(c) || c == '_';
}

// 辨識關鍵字（保留字）
int detect_keyword(const char *word) {
    if (strcmp(word, "int") == 0 || strcmp(word, "return") == 0) return TOKEN_TYPE;
    if (strcmp(word, "if") == 0) return TOKEN_IF;
    if (strcmp(word, "main") == 0) return TOKEN_MAIN;
    if (strcmp(word, "else") == 0) return TOKEN_ELSE;
    if (strcmp(word, "while") == 0) return TOKEN_WHILE;
    return TOKEN_IDENTIFIER;
}

// 輸出 token 與對應種類
void print_token(const char *str, int token_type) {
    switch (token_type) {
        case TOKEN_TYPE: printf("%s: TYPE_TOKEN\n", str); break;
        case TOKEN_IF: printf("%s: IF_TOKEN\n", str); break;
        case TOKEN_MAIN: printf("%s: MAIN_TOKEN\n", str); break;
        case TOKEN_ELSE: printf("%s: ELSE_TOKEN\n", str); break;
        case TOKEN_WHILE: printf("%s: WHILE_TOKEN\n", str); break;
        case TOKEN_IDENTIFIER: printf("%s: ID_TOKEN\n", str); break;
        case TOKEN_LITERAL: printf("%s: LITERAL_TOKEN\n", str); break;
    }
}

// 將全形符號轉換為對應半形符號
int convert_fullwidth_operator(unsigned char a, unsigned char b, unsigned char c) {
    if (a == 0xEF && b == 0xBC) {
        switch (c) {
            case 0x88: return '(';
            case 0x89: return ')';
            case 0xBB: return ';';
            case 0x9A: return ':';
            case 0x9C: return '<';
            case 0x9E: return '>';
        }
    }
    if (a == 0xEF && b == 0xBD) {
        if (c == 0x9B) return '{';
        if (c == 0x9D) return '}';
    }
    return 0;
}

// DFA 詞法分析主程式
void perform_tokenize(FILE *src) {
    int ch;
    char buffer[MAX_TOKEN_LEN];
    int buf_idx = 0;
    enum State state = STATE_START;

    while ((ch = fgetc(src)) != EOF) {
        // 嘗試轉換全形符號
        if ((unsigned char)ch >= 0xE0) {
            unsigned char a = ch, b = fgetc(src), c = fgetc(src);
            int converted = convert_fullwidth_operator(a, b, c);
            if (converted) {
                ch = converted;
            } else {
                ungetc(c, src);
                ungetc(b, src);
            }
        }

        switch (state) {
            // 初始狀態：判斷起始字元類型
            case STATE_START:
                if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
                    // 跳過空白字元
                }
                else if (is_alpha(ch)) {
                    buffer[0] = ch;
                    buf_idx = 1;
                    state = STATE_IDENTIFIER;  // 轉為識別字狀態
                }
                else if (is_digit(ch)) {
                    buffer[0] = ch;
                    buf_idx = 1;
                    state = STATE_NUMBER;  // 轉為數字狀態
                }
                else {
                    // 處理符號與運算子
                    int next = fgetc(src);
                    if (ch == '=' && next == '=') {
                        printf("==: EQUAL_TOKEN\n");
                    } else if (ch == '=') {
                        if (next != EOF) ungetc(next, src);
                        printf("=: ASSIGN_TOKEN\n");
                    } else if (ch == '>' && next == '=') {
                        printf(">=: GREATER_EQUAL_TOKEN\n");
                    } else if (ch == '>') {
                        if (next != EOF) ungetc(next, src);
                        printf(">: GREATER_TOKEN\n");
                    } else if (ch == '<' && next == '=') {
                        printf("<=: LESS_EQUAL_TOKEN\n");
                    } else if (ch == '<') {
                        if (next != EOF) ungetc(next, src);
                        printf("<: LESS_TOKEN\n");
                    } else {
                        if (next != EOF) ungetc(next, src);
                        switch (ch) {
                            case '+': printf("+: PLUS_TOKEN\n"); break;
                            case '-': printf("-: MINUS_TOKEN\n"); break;
                            case '(': printf("(: LEFTPAREN_TOKEN\n"); break;
                            case ')': printf("): RIGHTPAREN_TOKEN\n"); break;
                            case '{': printf("{: LEFTBRACE_TOKEN\n"); break;
                            case '}': printf("}: RIGHTBRACE_TOKEN\n"); break;
                            case ';': case ':': printf("%c: SEMICOLON_TOKEN\n", ch); break;
                        }
                    }
                }
                break;

            // 處理識別字狀態（ID 或保留字）
            case STATE_IDENTIFIER:
                if (is_alnum_or_underscore(ch)) {
                    buffer[buf_idx++] = ch;
                } else {
                    buffer[buf_idx] = '\0';
                    ungetc(ch, src);
                    int token = detect_keyword(buffer);
                    print_token(buffer, token);
                    state = STATE_START;
                }
                break;

            // 處理整數常數（Literal）
            case STATE_NUMBER:
                if (is_digit(ch)) {
                    buffer[buf_idx++] = ch;
                } else {
                    buffer[buf_idx] = '\0';
                    ungetc(ch, src);
                    print_token(buffer, TOKEN_LITERAL);
                    state = STATE_START;
                }
                break;
        }
    }

    // 檢查是否在結尾仍在識別字或數字狀態
    if (state == STATE_IDENTIFIER || state == STATE_NUMBER) {
        buffer[buf_idx] = '\0';
        if (state == STATE_IDENTIFIER)
            print_token(buffer, detect_keyword(buffer));
        else
            print_token(buffer, TOKEN_LITERAL);
    }
}

// 建立虛擬檔案以模擬輸入來源
FILE *create_virtual_file(const char *src) {
    FILE *fp = tmpfile();
    if (!fp) return NULL;
    fputs(src, fp);
    rewind(fp);
    return fp;
}

int main() {
    // 測試用 C 程式碼，含有全形符號
    const char *code_sample =
        "int main(){\n"
        "int cd2025=5;\n"
        "int cd2025_ = 5；\n"
        "if （cd2025 == 5）｛\n"
        "cd2025_ = 0;\n"
        "｝\n"
        "else {\n"
        "cd2025_ = 1+2+(3+4)+5;\n"
        "}\n"
        "while (cd2025_+cd2025) {\n"
        "cd2025 = cd2025-1;\n"
        "｝\n"
        "}";

    FILE *virtual_file = create_virtual_file(code_sample);
    if (!virtual_file) {
        printf("Failed to create file\n");
        return 1;
    }

    perform_tokenize(virtual_file);
    fclose(virtual_file);
    return 0;
}
