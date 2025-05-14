#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define true 1
#define false 0
#define ERROR_STATE 666

// ---------------------- Token定義 ----------------------
typedef enum {
    TYPE_TOKEN,
    MAIN_TOKEN,
    LEFTPAREN_TOKEN,
    RIGHTPAREN_TOKEN,
    LEFTBRACE_TOKEN,
    ID_TOKEN,
    ASSIGN_TOKEN,
    LITERAL_TOKEN,
    SEMICOLON_TOKEN,
    IF_TOKEN,
    ELSE_TOKEN,
    WHILE_TOKEN,
    EQUAL_TOKEN,
    GREATEREQUAL_TOKEN,
    LESSEQUAL_TOKEN,
    GREATER_TOKEN,
    LESS_TOKEN,
    PLUS_TOKEN,
    MINUS_TOKEN,
    RIGHTBRACE_TOKEN
} Token_Type;

// ---------------------- Scanner詞法單元結構 ----------------------
typedef struct token {
    char token[81];              // 儲存Token字串
    Token_Type token_type;       // Token種類
    struct token* next;          // 指向下一個Token
} token_data;

// ---------------------- HW1 Scanner功能 ----------------------
// 判斷字元類型
int is_whitespace(char c) {
    return (c == ' ' || c == '\t' || c == '\n');
}
int is_symbol(char c) {
    return (c == '{' || c == '}' || c == '(' || c == ')' || c == ';');
}
int is_alpha(char c) {
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_');
}
int is_digit(char c) {
    return (c >= '0' && c <= '9');
}
int is_operator(char c) {
    return (c == '+' || c == '-' || c == '=' || c == '<' || c == '>');
}

// 新增一顆Token到linked list
void append_token(token_data** list, char* token, Token_Type type) {
    token_data* new_token = malloc(sizeof(token_data));
    strcpy(new_token->token, token);
    new_token->token_type = type;
    new_token->next = NULL;
    if (*list == NULL) *list = new_token;
    else {
        token_data* cur = *list;
        while (cur->next) cur = cur->next;
        cur->next = new_token;
    }
}

// 釋放整個token linked list
void release_token(token_data* head) {
    token_data* cur = head;
    while (cur) {
        token_data* nx = cur->next;
        free(cur);
        cur = nx;
    }
}

// 核心Scanner
void perform_tokenize(FILE* src, token_data** list) {
    int ch;
    while ((ch = fgetc(src)) != EOF) {
        if (is_whitespace(ch)) continue;

        char buf[81] = {0};
        int idx = 0;

        // 1. 處理關鍵字或變數
        if (is_alpha(ch)) {
            buf[idx++] = ch;
            while (is_alpha(ch = fgetc(src)) || is_digit(ch))
                buf[idx++] = ch;
            ungetc(ch, src);
            buf[idx] = '\0';
            if (!strcmp(buf, "int")) append_token(list, buf, TYPE_TOKEN);
            else if (!strcmp(buf, "main")) append_token(list, buf, MAIN_TOKEN);
            else if (!strcmp(buf, "if")) append_token(list, buf, IF_TOKEN);
            else if (!strcmp(buf, "else")) append_token(list, buf, ELSE_TOKEN);
            else if (!strcmp(buf, "while")) append_token(list, buf, WHILE_TOKEN);
            else append_token(list, buf, ID_TOKEN);
        }
        // 2. 處理數字常數
        else if (is_digit(ch)) {
            buf[idx++] = ch;
            while (is_digit(ch = fgetc(src)))
                buf[idx++] = ch;
            ungetc(ch, src);
            buf[idx] = '\0';
            append_token(list, buf, LITERAL_TOKEN);
        }
        // 3. 處理符號
        else if (is_symbol(ch)) {
            buf[0] = ch; buf[1] = '\0';
            append_token(list, buf,
                (ch == '(') ? LEFTPAREN_TOKEN :
                (ch == ')') ? RIGHTPAREN_TOKEN :
                (ch == '{') ? LEFTBRACE_TOKEN :
                (ch == '}') ? RIGHTBRACE_TOKEN :
                SEMICOLON_TOKEN);
        }
        // 4. 處理運算子
        else if (is_operator(ch)) {
            int next = fgetc(src);
            if (ch == '=' && next == '=') append_token(list, "==", EQUAL_TOKEN);
            else if (ch == '=')
                { append_token(list, "=", ASSIGN_TOKEN); ungetc(next, src); }
            else if (ch == '>' && next == '=')
                append_token(list, ">=", GREATEREQUAL_TOKEN);
            else if (ch == '<' && next == '=')
                append_token(list, "<=", LESSEQUAL_TOKEN);
            else {
                if (next != EOF) ungetc(next, src);
                buf[0] = ch; buf[1] = '\0';
                append_token(list, buf,
                    (ch == '+') ? PLUS_TOKEN :
                    (ch == '-') ? MINUS_TOKEN :
                    (ch == '>') ? GREATER_TOKEN :
                    (ch == '<') ? LESS_TOKEN :
                    ASSIGN_TOKEN);
            }
        }
    }
}

// ---------------------- HW2 Parser結構 ----------------------
typedef struct Node {
    char* label;         // 標籤文字
    int nchildren;       // 子節點數
    struct Node** children;  // 子節點陣列
} Node;

// 建立Node
Node* make_node(const char* lbl) {
    Node* n = malloc(sizeof(Node));
    n->label = strdup(lbl);
    n->nchildren = 0;
    n->children = NULL;
    return n;
}
// 加入子節點
void add_child(Node* p, Node* c) {
    p->children = realloc(p->children, sizeof(Node*) * (p->nchildren + 1));
    p->children[p->nchildren++] = c;
}
// 釋放整棵樹
void free_tree(Node* n) {
    for (int i = 0; i < n->nchildren; i++) free_tree(n->children[i]);
    free(n->children);
    free(n->label);
    free(n);
}
// 印出語法樹
void print_tree(Node* node, const char* prefix, int is_last) {
    printf("%s", prefix);
    printf(is_last ? "└── " : "├── ");
    printf("%s\n", node->label);
    char buf[1024];
    for (int i = 0; i < node->nchildren; i++) {
        strcpy(buf, prefix);
        strcat(buf, is_last ? "    " : "│   ");
        print_tree(node->children[i], buf, i == node->nchildren - 1);
    }
}

// ---------------------- HW2 Parser主程式 ----------------------
token_data* cur_token;
Token_Type token;
char token_str[81];

// 從Scanner拿下一個Token
void next_token() {
    if (cur_token == NULL) { token = EOF; return; }
    token = cur_token->token_type;
    strcpy(token_str, cur_token->token);
    cur_token = cur_token->next;
}

// 遞迴下降文法
Node* parse_S();
Node* parse_Sprime();
Node* parse_E();

// S -> E S'
Node* parse_S() {
    Node* n = make_node("S");
    if (token == LITERAL_TOKEN || token == LEFTPAREN_TOKEN) {
        add_child(n, parse_E());
        add_child(n, parse_Sprime());
        return n;
    }
    fprintf(stderr, "Parse error in S\n");
    exit(1);
}
// S' -> + S | ε
Node* parse_Sprime() {
    Node* n = make_node("S'");
    if (token == PLUS_TOKEN) {
        add_child(n, make_node("+"));
        next_token();
        add_child(n, parse_S());
    } else add_child(n, make_node("ε"));
    return n;
}
// E -> (S) | 整數
Node* parse_E() {
    Node* n = make_node("E");
    if (token == LITERAL_TOKEN) {
        add_child(n, make_node(token_str));
        next_token();
    } else if (token == LEFTPAREN_TOKEN) {
        add_child(n, make_node("("));
        next_token();
        add_child(n, parse_S());
        if (token != RIGHTPAREN_TOKEN) { fprintf(stderr, "Missing )\n"); exit(1); }
        add_child(n, make_node(")"));
        next_token();
    } else {
        fprintf(stderr, "Parse error in E\n");
        exit(1);
    }
    return n;
}

// ---------------------- 主程式 ----------------------
FILE* create_virtual_file(const char* src) {
    FILE* fp = tmpfile();
    if (!fp) return NULL;
    fputs(src, fp);
    rewind(fp);
    return fp;
}

int main() {
    // 測試輸入 (B1129042)
    const char* code_sample = "(1+2+(3+4))+5";
    FILE* virtual_file = create_virtual_file(code_sample);
    if (!virtual_file) { printf("Failed to create file\n"); return 1; }

    // 呼叫HW1 Scanner
    token_data* head = NULL;
    perform_tokenize(virtual_file, &head);
    fclose(virtual_file);

    // 設定解析器初始
    cur_token = head;
    next_token();

    // 執行HW2 Parser
    Node* root = parse_S();
    if (token != EOF) { fprintf(stderr, "Extra after expr\n"); free_tree(root); release_token(head); return 1; }
    print_tree(root, "", 1); // 印出語法樹

    // 清理
    free_tree(root);
    release_token(head);
    return 0;
}
