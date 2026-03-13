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
    double numericValue;
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
    token popped = stack->tokens[stack->count-1];
    stack->count--;
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

double parse_numeric(char **p) {

    int found_digit = 0;
    double val = 0.0;

    // Integer part
    while (isdigit(**p)) {
        val = 10.0 * val + (**p - '0');
        (*p)++;
        found_digit = 1;
    }

    // Fractional part
    if (**p == '.') {
        (*p)++;
        double frac = 0.0, base = 0.1;
        while (isdigit(**p)) {
            frac += (**p - '0') * base;
            base /= 10.0;
            (*p)++;
            found_digit = 1;
        }
        val += frac;
    }

    if (!found_digit) {
        return 0.0;
    }

    (*p)--;

    return val;
}

result tokenize_expression(char* expression) {
    result res;
    res.tokenCount = 0;
    res.type = FAILED;
    char* p = expression;
    int applyUnary = 0;
    while (*p != '\0') {
        token tok;
        tok.numericValue = 0;
        tok.precedence = -1;
        tok.rightAssociative = 0;

        if (*p == '+') {
            // If is unary operator
            if (res.tokenCount == 0 || res.tokens[res.tokenCount-1].expressionType == ADDITION
                || res.tokens[res.tokenCount-1].expressionType == SUBTRACTION
                || res.tokens[res.tokenCount-1].expressionType == MULTIPLICATION
                || res.tokens[res.tokenCount-1].expressionType == DIVISION
                || res.tokens[res.tokenCount-1].expressionType == LEFT_PARENTHESIS
                || res.tokens[res.tokenCount-1].expressionType == RIGHT_PARENTHESIS) {
                applyUnary = 2;
            } else {
                tok.expressionType = ADDITION;
                tok.precedence = 2;
            }
        } else if (*p == '-') {
            // If is unary operator
            if (res.tokenCount == 0 || res.tokens[res.tokenCount-1].expressionType == ADDITION
                || res.tokens[res.tokenCount-1].expressionType == SUBTRACTION
                || res.tokens[res.tokenCount-1].expressionType == MULTIPLICATION
                || res.tokens[res.tokenCount-1].expressionType == DIVISION
                || res.tokens[res.tokenCount-1].expressionType == LEFT_PARENTHESIS
                || res.tokens[res.tokenCount-1].expressionType == RIGHT_PARENTHESIS) {
                    applyUnary = 1;
                } else {
                    tok.expressionType = SUBTRACTION;
                    tok.precedence = 2;
                }
        } else if (*p == '*') {
            tok.expressionType = MULTIPLICATION;
            tok.precedence = 3;
        } else if (*p == '/') {
            tok.expressionType = DIVISION;
            tok.precedence = 3;
        } else if (*p == '(') {
            // Handle implicit multiplication
            if (res.tokenCount > 0 && res.tokens[res.tokenCount-1].expressionType == NUMBER) {
                token mult;
                mult.numericValue = 0;
                mult.precedence = 3;
                mult.rightAssociative = 0;
                mult.expressionType = MULTIPLICATION;
                res.tokens[res.tokenCount++] = mult;
            }
            tok.expressionType = LEFT_PARENTHESIS;
            tok.precedence = 1;
        } else if (*p == ')') {
            tok.expressionType = RIGHT_PARENTHESIS;
            tok.precedence = 1;
        } else if (isdigit(*p)) {
            double result = parse_numeric(&p);
            if (applyUnary == 1) {
                result = -result;
                applyUnary = 0;
            }
            tok.expressionType = NUMBER;
            tok.numericValue = result;
            tok.precedence = 0;
        } else {
            char* errMsh = "Unexpected token\0";
            res.error = errMsh;
            return res;
        }
        // If applyUnary, skip adding the '-', the number is already converted to a negative number
        if (applyUnary > 0) {
            p++;
            continue;
        }
        res.tokens[res.tokenCount++] = tok;
        p++;
    }
    res.type = SUCCESS;
    res.error = NULL;
    return res;
}

void create_RPN(stack* operatorStack, stack* POLISHstack, token* tokens, int tokenCount) {
    int i = 0;
    while (i < tokenCount) {
        token top;
        if (operatorStack->count > 0) {
            top = peek(operatorStack);
        }
        switch (tokens[i].expressionType) {
            case NUMBER:
                push(POLISHstack, tokens[i]);
                i++;
                break;
            case ADDITION:
                if (operatorStack->count > 0) {
                    while (top.expressionType != LEFT_PARENTHESIS && (top.precedence > tokens[i].precedence ||
                        (top.precedence == tokens[i].precedence && tokens[i].rightAssociative == 0))
                        ) {
                        push(POLISHstack, pop(operatorStack));
                        if (operatorStack->count == 0) {
                            break;
                        }
                        top = peek(operatorStack);
                    }
                    push(operatorStack, tokens[i]);
                    i++;
                } else {
                    push(operatorStack, tokens[i]);
                    i++;
                }
                break;
            case SUBTRACTION:
                if (operatorStack->count > 0) {
                    while (top.expressionType != LEFT_PARENTHESIS && (top.precedence > tokens[i].precedence ||
                        (top.precedence == tokens[i].precedence && tokens[i].rightAssociative == 0))
                        ) {
                        push(POLISHstack, pop(operatorStack));
                        if (operatorStack->count == 0) {
                            break;
                        }
                        top = peek(operatorStack);
                        }
                    push(operatorStack, tokens[i]);
                    i++;
                } else {
                    push(operatorStack, tokens[i]);
                    i++;
                }
                break;
            case MULTIPLICATION:
                if (operatorStack->count > 0) {
                    while (top.expressionType != LEFT_PARENTHESIS && (top.precedence > tokens[i].precedence ||
                        (top.precedence == tokens[i].precedence && tokens[i].rightAssociative == 0))
                        ) {
                        push(POLISHstack, pop(operatorStack));
                        if (operatorStack->count == 0) {
                            break;
                        }
                        top = peek(operatorStack);
                        }
                    push(operatorStack, tokens[i]);
                    i++;
                } else {
                    push(operatorStack, tokens[i]);
                    i++;
                }
                break;
            case DIVISION:
                if (operatorStack->count > 0) {
                    while (top.expressionType != LEFT_PARENTHESIS && (top.precedence > tokens[i].precedence ||
                        (top.precedence == tokens[i].precedence && tokens[i].rightAssociative == 0))
                        ) {
                        push(POLISHstack, pop(operatorStack));
                        if (operatorStack->count == 0) {
                            break;
                        }
                        top = peek(operatorStack);
                        }
                    push(operatorStack, tokens[i]);
                    i++;
                } else {
                    push(operatorStack, tokens[i]);
                    i++;
                }
                break;
            case LEFT_PARENTHESIS:
                push(operatorStack, tokens[i]);
                i++;
                break;
            case RIGHT_PARENTHESIS:
                while (operatorStack->count > 0 && top.expressionType != LEFT_PARENTHESIS) {
                    push(POLISHstack, pop(operatorStack));
                    if (operatorStack->count == 0) {
                        printf("Could not find matching parenthesis\n");
                        exit(1);
                    }
                    top = peek(operatorStack);
                }
                pop(operatorStack);
                i++;
                break;
        }
    }

    while (operatorStack->count > 0) {
        push(POLISHstack, pop(operatorStack));
    }
}

void print_RPN(stack* POLISHstack) {
    printf("Printing and popping polish stack\n");
    while (POLISHstack->count != 0) {
        token popedOp = pop(POLISHstack);
        switch (popedOp.expressionType) {
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
                printf("NUMBER: %f\n", popedOp.numericValue);
                break;
        }
    }
}

stack* reverse_stack(stack* orgStack) {
    int count = orgStack->count;
    stack* reversedStack = malloc(sizeof(stack));
    reversedStack->count = 0;

    while (reversedStack->count != count) {
        push(reversedStack, pop(orgStack));
    }
    return reversedStack;
}

token applyOperator(token left, token right, token operator) {
    token res;
    res.precedence = 0;
    res.expressionType = NUMBER;
    res.rightAssociative = 0;
    switch (operator.expressionType) {
        case ADDITION:
            res.numericValue = left.numericValue + right.numericValue;
            break;
        case SUBTRACTION:
            res.numericValue = left.numericValue - right.numericValue;
            break;
        case MULTIPLICATION:
            res.numericValue = left.numericValue * right.numericValue;
            break;
        case DIVISION:
            if (right.numericValue == 0) {
                printf("Division by zero\n");
                exit(1);
            }
            res.numericValue = left.numericValue / right.numericValue;
            break;
        default:
            printf("Unknown operator\n");
            exit(1);
    }
    return res;
}

void evaluate_RPN(stack* POLISHstack) {
    stack* evaluationStack = malloc(sizeof(stack));
    evaluationStack->count = 0;
    while (POLISHstack->count != 0) {
        token curr = pop(POLISHstack);
        if (evaluationStack->count < 2 && curr.expressionType != NUMBER) {
            printf("Error handling expression evaluation: expected number but got operator\n");
            exit(1);
        }
        if (curr.expressionType == NUMBER) {
            push(evaluationStack, curr);
        }
        if (curr.expressionType != NUMBER) {
            token right = pop(evaluationStack);
            token left = pop(evaluationStack);
            token evaluatedExpression = applyOperator(left, right, curr);
            push(evaluationStack, evaluatedExpression);
        }
    }

    if (evaluationStack->count != 1) {
        printf("Error handling expression evaluation: expected number remaining but got multiple\n");
        exit(1);
    }
    printf("Result: %f\n", pop(evaluationStack).numericValue);
    free(evaluationStack);
}
//5*(-23)
// -18, *
// *, -, 23, 5


int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Calc expects only 1 expression (the math expression)\n");
        exit(0);
    }

    char* expression = argv[1];
    result res = tokenize_expression(expression);
    if (res.type == FAILED) {
        printf("%s\n", res.error);
        exit(1);
    }

    stack operatorStack;
    operatorStack.count = 0;
    stack POLISHstack;
    POLISHstack.count = 0;

    create_RPN(&operatorStack, &POLISHstack, res.tokens, res.tokenCount);
    stack* reversedStack = reverse_stack(&POLISHstack);

    //print_RPN(reversedStack);
    evaluate_RPN(reversedStack);

    free(reversedStack);

    return 0;

}
