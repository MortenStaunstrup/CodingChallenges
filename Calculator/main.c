#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO create a stack datastructure for the operators https://en.wikipedia.org/wiki/Shunting_yard_algorithm#The_algorithm_in_detail
// TODO create a string for the 'reverse polish notation'
// TODO create a stack datastructure for the finished RPN

#define MAX_TOKENS 500
#define MAX_DIGITS 1000

typedef enum expression {
    ADDITION,
    SUBTRACTION,
    MULTIPLICATION,
    DIVISION,
    NUMBER,
    LEFT_PARENTHESIS,
    RIGHT_PARENTHESIS
} expression;

typedef enum result_type {
    FAILED,
    SUCCESS,
} result_type;

typedef struct token {
    expression expressionType;
    char* valueInString;
} token;

typedef struct result {
    result_type type;
    char* error;
    token tokens[MAX_TOKENS];
    int tokenCount;
} result;

typedef struct stack {
    token tokens[MAX_TOKENS];
    int count;
} stack;

token pop(stack* stack) {

}

void push(stack* stack) {

}

token peek(stack* stack) {

}

char* parse_numeric(char** p) {
    if (p == NULL || *p == NULL) return NULL;

    long long val = 0;
    int found_digit = 0;

    // Parse digits
    while (isdigit(**p)) {
        val = 10 * val + (**p - '0');
        (*p)++;
        found_digit = 1;
    }

    if (!found_digit) {
        // No digits found: return "0"
        char* buffer = malloc(2);
        if (buffer) {
            buffer[0] = '0';
            buffer[1] = '\0';
        }
        return buffer;
    }

    char temp[32];
    int len = snprintf(temp, sizeof(temp), "%lld", val);

    char* buffer = malloc(len + 1); // +1 for null terminator
    if (buffer)
        snprintf(buffer, len + 1, "%lld", val);

    (*p)--;

    return buffer;
}

result tokenize_expression(char* expression) {
    result res;
    res.tokenCount = 0;
    res.type = FAILED;
    char* p = expression;
    while (*p != '\0') {
        token tok;
        tok.valueInString = NULL;

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
            char* result = parse_numeric(&p);
            tok.expressionType = NUMBER;
            tok.valueInString = result;
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
    result res = tokenize_expression(expression);
    if (res.type == FAILED) {
        printf("%s\n", res.error);
        exit(1);
    }

    for (int i = 0; i < res.tokenCount; i++) {
        switch (res.tokens[i].expressionType) {
            case ADDITION:
                printf("ADDITION\n");
                break;
            case SUBTRACTION:
                printf("SUBTRACTION\n");
                break;
            case MULTIPLICATION:
                printf("MULTIPLICATION\n");
                break;
            case DIVISION:
                printf("DIVISION\n");
                break;
            case LEFT_PARENTHESIS:
                printf("LEFT_PARENTHESIS\n");
                break;
            case RIGHT_PARENTHESIS:
                printf("RIGHT_PARENTHESIS\n");
                break;
            case NUMBER:
                printf("NUMBER: %s\n", res.tokens[i].valueInString);
                break;
        }
    }

}
