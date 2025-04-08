#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKEN_LEN 100

enum TokenType {
    TOKEN_TYPE = 1, TOKEN_IF, TOKEN_MAIN, TOKEN_ELSE, TOKEN_WHILE,
    TOKEN_EQUAL = 11, TOKEN_GREATER, TOKEN_LESS, TOKEN_ASSIGN,
    TOKEN_LITERAL, TOKEN_PLUS, TOKEN_MINUS,
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE, TOKEN_SEMICOLON,
    TOKEN_GREATER_EQUAL, TOKEN_LESS_EQUAL, TOKEN_IDENTIFIER
};

int is_alpha(int c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int is_digit(int c) {
    return c >= '0' && c <= '9';
}

int is_alnum_or_underscore(int c) {
    return is_alpha(c) || is_digit(c) || c == '_';
}

int detect_keyword(const char *word) {
    if (strcmp(word, "int") == 0 || strcmp(word, "return") == 0)
        return TOKEN_TYPE;
    if (strcmp(word, "if") == 0)
        return TOKEN_IF;
    if (strcmp(word, "main") == 0)
        return TOKEN_MAIN;
    if (strcmp(word, "else") == 0)
        return TOKEN_ELSE;
    if (strcmp(word, "while") == 0)
        return TOKEN_WHILE;
    return 0;
}

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

void perform_tokenize(FILE *src) {
    int ch;
    char buffer[MAX_TOKEN_LEN];

    while ((ch = fgetc(src)) != EOF) {
        if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
            continue;

        if (is_alpha(ch)) {
            int i = 0;
            buffer[i++] = ch;
            while ((ch = fgetc(src)) != EOF && is_alnum_or_underscore(ch)) {
                buffer[i++] = ch;
            }
            buffer[i] = '\0';
            if (ch != EOF) ungetc(ch, src);

            int token = detect_keyword(buffer);
            switch (token) {
                case TOKEN_TYPE: printf("%s: TYPE_TOKEN\n", buffer); break;
                case TOKEN_IF: printf("%s: IF_TOKEN\n", buffer); break;
                case TOKEN_MAIN: printf("%s: MAIN_TOKEN\n", buffer); break;
                case TOKEN_ELSE: printf("%s: ELSE_TOKEN\n", buffer); break;
                case TOKEN_WHILE: printf("%s: WHILE_TOKEN\n", buffer); break;
                default: printf("%s: ID_TOKEN\n", buffer); break;
            }
        }
        else if (is_digit(ch)) {
            int i = 0;
            buffer[i++] = ch;
            while ((ch = fgetc(src)) != EOF && is_digit(ch)) {
                buffer[i++] = ch;
            }
            buffer[i] = '\0';
            if (ch != EOF) ungetc(ch, src);
            printf("%s: LITERAL_TOKEN\n", buffer);
        }
        else {
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

            int next = fgetc(src);
            if (ch == '=' && next == '=') {
                printf("==: EQUAL_TOKEN\n");
            } else if (ch == '=') {
                if (next != EOF) ungetc(next, src);
                printf("=: ASSIGN_TOKEN\n");
            }
            else if (ch == '>' && next == '=') {
                printf(">=: GREATER_EQUAL_TOKEN\n");
            } else if (ch == '>') {
                if (next != EOF) ungetc(next, src);
                printf(">: GREATER_TOKEN\n");
            }
            else if (ch == '<' && next == '=') {
                printf("<=: LESS_EQUAL_TOKEN\n");
            } else if (ch == '<') {
                if (next != EOF) ungetc(next, src);
                printf("<: LESS_TOKEN\n");
            }
            else {
                if (next != EOF) ungetc(next, src);
                switch (ch) {
                    case '+': printf("+: PLUS_TOKEN\n"); break;
                    case '-': printf("-: MINUS_TOKEN\n"); break;
                    case '(': printf("(: LEFTPAREN_TOKEN\n"); break;
                    case ')': printf("): RIGHTPAREN_TOKEN\n"); break;
                    case '{': printf("{: LEFTBRACE_TOKEN\n"); break;
                    case '}': printf("}: RIGHTBRACE_TOKEN\n"); break;
                    case ';': case ':': printf("%c: SEMICOLON_TOKEN\n", ch); break;
                    default: break;
                }
            }
        }
    }
}

FILE *create_virtual_file(const char *src) {
    FILE *fp = tmpfile();
    if (!fp) return NULL;
    fputs(src, fp);
    rewind(fp);
    return fp;
}

int main() {
    const char *code_sample =
        "int main{\n"
        "int cd2025=5;\n"
        "int cd2025_ = 5；\n"
        "if （cd2025 == 5）｛\n"
        "cd2025_ = 0：\n"
        "｝\n"
        "else {\n"
        "cd2025_ = 1+2+(3+4)+5;\n"
        "}\n"
        "while (cd2025_+cd2025) {\n"
        "cd2025 = cd2025-1;\n"
        "｝\n"
        "return 0;\n"
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
