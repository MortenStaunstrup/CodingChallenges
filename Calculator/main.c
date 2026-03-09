#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO create a stack datastructure for the operators https://en.wikipedia.org/wiki/Shunting_yard_algorithm#The_algorithm_in_detail
// TODO create a string for the 'reverse polish notation'
// TODO create a stack datastructure for the finished RPN

// Using 'Shunting yard algorithm'

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
    short precedence;
    int rightAssociative;
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
    if (stack->count == 0) {
        printf("Stack is empty, can't pop\n");
        exit(1);
    }
    token popped = stack->tokens[stack->count--];
    return popped;
}

void push(stack* stack, token token) {
    stack->tokens[stack->count++] = token;
}

token peek(stack* stack) {
    if (stack->count == 0) {
        printf("Empty Stack, can't peek\n");
        exit(1);
    }

    return stack->tokens[stack->count-1];
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
        tok.precedence = -1;
        tok.rightAssociative = 0;

        if (*p == '+') {
            tok.expressionType = ADDITION;
            tok.precedence = 2;
        } else if (*p == '-') {
            tok.expressionType = SUBTRACTION;
            tok.precedence = 2;
        } else if (*p == '*') {
            tok.expressionType = MULTIPLICATION;
            tok.precedence = 3;
        } else if (*p == '/') {
            tok.expressionType = DIVISION;
            tok.precedence = 3;
        } else if (*p == '(') {
            tok.expressionType = LEFT_PARENTHESIS;
            tok.precedence = 1;
        } else if (*p == ')') {
            tok.expressionType = RIGHT_PARENTHESIS;
            tok.precedence = 1;
        } else if (isdigit(*p)) {
            char* result = parse_numeric(&p);
            tok.expressionType = NUMBER;
            tok.valueInString = result;
            tok.precedence = 0;
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

void create_RPN(stack* operatorStack, stack* POLISHstack, token* tokens, int tokenCount) {
    for (int i = 0; i < tokenCount; i++) {
        int topOperatorExists = 0;
        token topOperator;
        if (operatorStack->count != 0) {
            topOperator = peek(operatorStack);
            topOperatorExists = 1;
        }

        switch (tokens[i].expressionType) {
            case ADDITION:
                push(operatorStack, tokens[i]);
                break;
            case SUBTRACTION:
                push(operatorStack, tokens[i]);
                break;
            case MULTIPLICATION:
                if (topOperatorExists && topOperator.precedence >= tokens[i].precedence) {
                    token popedOp = pop(operatorStack);
                    if (popedOp.expressionType != topOperator.expressionType) {
                        printf("Error occurred popping and stuff\n");
                        exit(1);
                    }
                } else {
                    push(POLISHstack, tokens[i]);
                }
                break;
            case DIVISION:
                if (topOperatorExists && topOperator.precedence >= tokens[i].precedence) {
                    token popedOp = pop(operatorStack);
                    if (popedOp.expressionType != topOperator.expressionType) {
                        printf("Error occurred popping and stuff\n");
                        exit(1);
                    }
                } else {
                    push(POLISHstack, tokens[i]);
                }
                break;
            case LEFT_PARENTHESIS:
                push(POLISHstack, tokens[i]);
                break;
            case RIGHT_PARENTHESIS:
                break;
            case NUMBER:
                // Number always goes to output
                push(POLISHstack, tokens[i]);
                break;
        }

    }
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

    stack operatorStack;
    operatorStack.count = 0;
    stack POLISHstack;
    POLISHstack.count = 0;

    create_RPN(&operatorStack, &POLISHstack, res.tokens, res.tokenCount);

}
