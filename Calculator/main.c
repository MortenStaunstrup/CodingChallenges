#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

// TODO lav array af tokens når et resultat er blevet udregnet, så fjern nummer, operation og nummer.
// TODO bliv ved indtil der kun er én token tilbage.

#define MAX_TOKENS 500

typedef enum expression {
    ADDITION,
    SUBTRACTION,
    MULTIPLICATION,
    DIVISION,
    NUMBER,
    LEFT_PARENTHESIS,
    RIGHT_PARENTHESIS,
} expression;

typedef enum result_type {
    FAILED,
    SUCCESS,
} result_type;

typedef struct token {
    expression expressionType;
    long long value;
} token;

typedef struct result {
    result_type type;
    char* error;
    token tokens[MAX_TOKENS];
    int tokenCount;
} result;

long long parse_numeric(char* p) {
    long long val = 0;
    while (isdigit(*p)) {
        val = 10 * val + (*p++ - '0');
    }
    p--;
    return val;
}

result tokenize_expression(char* expression) {
    result res;
    res.tokenCount = 0;
    res.type = FAILED;
    char* p = expression;
    while (*p != '\0') {
        token tok;
        tok.value = 0;

        if (*p == '+') {
            tok.expressionType = ADDITION;
        } else if (*p == '-') {
            tok.expressionType = SUBTRACTION;
        } else if (*p == '*') {
            tok.expressionType = MULTIPLICATION;
        } else if (*p == '/') {
            tok.expressionType = DIVISION;
        } else if (*p == '(') {
            tok.expressionType = LEFT_PARENTHESIS;
        } else if (*p == ')') {
            tok.expressionType = RIGHT_PARENTHESIS;
        } else if (isdigit(*p)) {
            long long result = parse_numeric(p);
            tok.expressionType = NUMBER;
            tok.value = result;
        } else {
            char* errMsh = "Unexpected token\0";
            res.error = errMsh;
            return res;
        }
        res.tokens[res.tokenCount++] = tok;
        p++;
    }
    res.type = SUCCESS;
    res.error = NULL;
    return res;
}


int main(int argc, char* argv[]) {
    if (argc > 2) {
        printf("Calc expects only 1 expression (the math expression)\n");
        exit(0);
    }

    char* expression = argv[1];

}
