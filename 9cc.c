#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TK_RESERVED,  // 記号
    TK_NUM,       // 整数
    TK_EOF,       // 入力終わり
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;  // トークンの型
    Token *next;     // 次の入力トークン
    int val;         // トークンがTK_NUMの場合、その数値
    char *str;       // トークン文字列
};

Token *token;

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// トークンが記号かつ引数で与えたものなら、トークンを一つ進めてtrue
// それ以外ならfalseを返す
bool consume(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op) {
        return false;
    }
    token = token->next;
    return true;
}

// トークンが記号かつ引数で与えたものなら、トークンを一つ進める
// それ以外の場合はエラーをraiseする
void expect(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op) {
        error("not '%c'", op);
    }
    token = token->next;
}

// トークンが数値なら、トークンを一つ進めてその数値を返す。
// それ以外の場合はエラーをraiseする
int expect_number() {
    if (token->kind != TK_NUM) {
        error("not a number");
    }
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() { return token->kind == TK_EOF; }

// 新しいトークンを作成し、第二引数のcurの次に繋げる
Token *new_token(TokenKind kind, Token *cur, char *str) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

// 入力文字列pをトークナイズし、head
Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (*p == '+' || *p == '-') {
            cur = new_token(TK_RESERVED, cur, p);
            p++;
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error("can not tokenize");
    }

    new_token(TK_EOF, cur, p);

    // headは空トークン
    // head.nextから先頭トークンなので先頭トークンのアドレスを返す
    return head.next;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "the number of args is wrong\n");
        return 1;
    }

    token = tokenize(argv[1]);

    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    // 最初の数字部分のmov命令を出力
    printf("    mov rax, %d\n", expect_number());

    while (!at_eof()) {
        if (consume('+')) {
            printf("    add rax, %d\n", expect_number());
            continue;
        }

        expect('-');
        printf("    sub rax, %d\n", expect_number());
    }

    printf("    ret\n");
    return 0;
}