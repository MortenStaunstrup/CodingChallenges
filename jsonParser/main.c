#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <math.h>

#define MAX_TEXT_SIZE 256

enum json_kind {
    JSON_TOKEN_LBRACKET,
    JSON_TOKEN_RBRACKET,
    JSON_TOKEN_LBRACE,
    JSON_TOKEN_RBRACE,
    JSON_TOKEN_COMMA,
    JSON_TOKEN_COLON,
    JSON_TOKEN_STRING,
    JSON_TOKEN_NUMBER,
    JSON_TOKEN_BOOLEAN_TRUE,
    JSON_TOKEN_BOOLEAN_FALSE,
    JSON_TOKEN_NULL,
    JSON_TOKEN_EOF,
    JSON_TOKEN_ILLEGAL
};

enum result {
    SUCCESS,
    FAILED,
    NONE
};


typedef struct json_token {
    enum json_kind kind;
    union {
        char* strValue;
        double numValue;
    } value;
    int isENotation;
    unsigned long long pos;
    unsigned long long line;
} json_token;

typedef struct parse_result {
    enum result result;
    char* text;
} parse_result;

parse_result parse_unicode(char **iter, unsigned long long *textCurrentPos, char *buffer, int *i);
parse_result parse_escape_sequence(char **iter, unsigned long long *textCurrentPos, char *buffer, int *i);
parse_result parse_object(json_token *tokens, unsigned long long *token);
parse_result parse_array(json_token *tokens, unsigned long long *token);
parse_result parser(json_token *tokens);
parse_result parse_string(char **iter, unsigned long long *textCurrentPos);
parse_result handle_scientific_notation(char **iter, unsigned long long *textCurrentPos, unsigned long long *textLine, double *number);
json_token parse_value(char **iter, unsigned long long *textCurrentPos, unsigned long long *textLine);
void skip_spaces(char **text, unsigned long long *textCurrentPos, unsigned long long *textLine);
char *loadfile(char *text);
json_token *lexor(char *text, unsigned long long textLength);
json_token parse_numeric(char **iter, unsigned long long *textCurrentPos, unsigned long long *textLine);
json_token parse_literal(char **iter, unsigned long long *textCurrentPos, unsigned long long *textLine);

// TODO: fail29 and fail31
// TODO: Must be able to pass pass1.json


int main(int argc, char *argv[]) {
    if (argc == 2) {


        DIR *dir;
        dir = opendir("C:\\Users\\mort4\\Downloads\\test");

        if (!dir) {
            perror("Directory could not be accessed");
            return 1;
        }

        struct dirent *entry;

        while ((entry = readdir(dir)) != NULL) {

            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            char fullpath[1024];
            // Adjust length as needed
            snprintf(fullpath, sizeof(fullpath), "C:\\Users\\mort4\\Downloads\\test\\%s", entry->d_name);
            char *jsonText = loadfile(fullpath);

            if (jsonText == NULL) {
                perror("Could not load file");
            }

            json_token *tokens = lexor(jsonText, strlen(jsonText));

            parse_result result = parser(tokens);

            if (result.result == FAILED) {
                printf("%s FAILED: %s\n", entry -> d_name, result.text);
            } else if (result.result == SUCCESS) {
                printf("%s SUCCESS\n", entry -> d_name);
            }

            size_t token = 0;
            while (tokens[token].kind != JSON_TOKEN_EOF && tokens[token].kind != JSON_TOKEN_ILLEGAL) {
                if (tokens[token].kind == JSON_TOKEN_STRING) {
                    free(tokens[token].value.strValue);
                }
                token++;
            }

            free(tokens);
            free(jsonText);
        }

        closedir(dir);

        /*
        char *jsonText = loadfile(argv[1]);
        if (jsonText == NULL) {
            perror("Error loading file\n");
            return 1;
        }

        json_token *tokens = lexor(jsonText, strlen(jsonText));

        parse_result result = parser(tokens);

        size_t tok = 0;
        while (tokens[tok].kind != JSON_TOKEN_EOF) {
            switch (tokens[tok].kind) {
                case JSON_TOKEN_LBRACKET:
                    printf("[\n");
                    break;
                case JSON_TOKEN_RBRACKET:
                    printf("]\n");
                    break;
                case JSON_TOKEN_LBRACE:
                    printf("{\n");
                    break;
                case JSON_TOKEN_RBRACE:
                    printf("}\n");
                    break;
                case JSON_TOKEN_COMMA:
                    printf(",\n");
                    break;
                case JSON_TOKEN_COLON:
                    printf(":\n");
                    break;
                case JSON_TOKEN_STRING:
                    printf("STRING: \"%s\"\n", tokens[tok].value.strValue);
                    break;
                case JSON_TOKEN_NUMBER:
                    if (tokens[tok].isENotation)
                        printf("NUMBER E NOTATION: %e\n", tokens[tok].value.numValue);
                    else
                        printf("NUMBER: %f\n", tokens[tok].value.numValue);
                    break;
                case JSON_TOKEN_BOOLEAN_TRUE:
                    printf("true\n");
                    break;
                case JSON_TOKEN_BOOLEAN_FALSE:
                    printf("false\n");
                    break;
                case JSON_TOKEN_NULL:
                    printf("null\n");
                    break;
                case JSON_TOKEN_ILLEGAL:
                    printf("ILLEGAL TOKEN at line: %llu\npos %llu\n", tokens[tok].line, tokens[tok].pos);
                    break;
                default:
                    printf("UNKNOWN TOKEN\n");
            }
            tok++; // Move to the next token in the array
        }

        if (result.result == SUCCESS) {
            printf("SUCCESS\n");
        } else if (result.result == FAILED) {
            printf("FAILED: %s\n", result.text);
        }

        size_t token = 0;
        while (tokens[token].kind != JSON_TOKEN_EOF && tokens[token].kind != JSON_TOKEN_ILLEGAL) {
            if (tokens[token].kind == JSON_TOKEN_STRING || tokens[token].kind == JSON_TOKEN_ILLEGAL) {
                free(tokens[token].value.strValue);
            }
            token++;
        }

        free(tokens);
        free(jsonText); */
    }
    return 0;
}

parse_result parser(json_token *tokens) {

    unsigned long long token = 0;
    if (tokens[0].kind == JSON_TOKEN_EOF) {
        parse_result result;
        result.result = FAILED;
        result.text = "No inputs given";
        return result;
    }

    if (tokens[0].kind == JSON_TOKEN_LBRACE) {
        parse_result res = parse_object(tokens, &token);
        if (res.result == SUCCESS) {
            token++;
            if (tokens[token].kind != JSON_TOKEN_EOF) {
                token--;
                parse_result result;
                char *buffer = malloc(MAX_TEXT_SIZE);
                snprintf(buffer, MAX_TEXT_SIZE,
                    "Expected end of file at line: %llu pos: %llu", tokens[token].line, tokens[token].pos);
                result.result = FAILED;
                result.text = buffer;
                return result;
            }
        }
        return res;
    }

    if (tokens[0].kind == JSON_TOKEN_LBRACKET) {
        parse_result res = parse_array(tokens, &token);
        if (res.result == SUCCESS) {
            token++;
            if (tokens[token].kind != JSON_TOKEN_EOF) {
                token--;
                parse_result result;
                char *buffer = malloc(MAX_TEXT_SIZE);
                snprintf(buffer, MAX_TEXT_SIZE,
                    "Expected end of file at line: %llu pos: %llu", tokens[token].line, tokens[token].pos);
                result.result = FAILED;
                result.text = buffer;
                return result;
            }
        }
        return res;
    }

    parse_result result;
    result.result = FAILED;
    result.text = "Expected '{' or '[' to begin";
    return result;
}

parse_result parse_object(json_token *tokens, unsigned long long *token) {

    (*token)++; // skip the LBRACE

    parse_result result;

    int colon = 0;
    int key = 0;
    int comma = 0;
    int value = 0;
    while (tokens[*token].kind != JSON_TOKEN_EOF) {

        switch (tokens[*token].kind) {
            case JSON_TOKEN_STRING:
                key++;
                if (key >= 2 && colon == 0) {
                    result.result = FAILED;
                    result.text = "Expected colon";
                    return result;
                }
                comma = 0;
                break;
            case JSON_TOKEN_LBRACKET:
                value++;
                if (key != 1) {
                    result.result = FAILED;
                    result.text = "Expected a key";
                    return result;
                }
                if (colon != 1) {
                    result.result = FAILED;
                    result.text = "Colon expected";
                    return result;
                }
                parse_result array_result = parse_array(tokens, token);
                if (array_result.result == FAILED)
                    return array_result;
                break;
            case JSON_TOKEN_RBRACKET:
                result.result = FAILED;
                result.text = "] is invalid at this point";
                return result;
                break;
            case JSON_TOKEN_LBRACE:
                value++;
                if (key != 1) {
                    result.result = FAILED;
                    result.text = "Expected a key";
                    return result;
                }
                if (colon != 1) {
                    result.result = FAILED;
                    result.text = "Colon expected";
                    return result;
                }
                parse_result object_result = parse_object(tokens, token);
                if (object_result.result == FAILED)
                    return object_result;
                break;
            case JSON_TOKEN_RBRACE:
                if (comma >= 1) {
                    result.result = FAILED;
                    result.text = "Trailing comma";
                    return result;
                }
                result.result = SUCCESS;
                result.text = "";
                return result;
                break;
            case JSON_TOKEN_COMMA:
                comma++;
                if (key < 1) {
                    result.result = FAILED;
                    result.text = "Expected a key";
                    return result;
                }
                if (colon < 1) {
                    result.result = FAILED;
                    result.text = "Colon expected";
                    return result;
                }
                if (comma > 1) {
                    result.result = FAILED;
                    result.text = "Value expected";
                    return result;
                }
                if (key < 2 && value < 1) {
                    result.result = FAILED;
                    result.text = "Expected a value";
                    return result;
                }
                key = 0;
                value = 0;
                colon = 0;
                break;
            case JSON_TOKEN_COLON:
                colon++;
                if (key < 1) {
                    result.result = FAILED;
                    result.text = "Expected a key";
                    return result;
                }
                if (colon > 1) {
                    result.result = FAILED;
                    result.text = "Value expected";
                    return result;
                }
                break;
            case JSON_TOKEN_NUMBER:
                value++;
                if (key != 1) {
                    result.result = FAILED;
                    result.text = "Expected a key";
                    return result;
                }
                if (colon != 1) {
                    result.result = FAILED;
                    result.text = "Colon expected";
                    return result;
                }
                break;
            case JSON_TOKEN_BOOLEAN_TRUE:
                value++;
                if (key != 1) {
                    result.result = FAILED;
                    result.text = "Expected a key";
                    return result;
                }
                if (colon != 1) {
                    result.result = FAILED;
                    result.text = "Colon expected";
                    return result;
                }
                break;
            case JSON_TOKEN_BOOLEAN_FALSE:
                value++;
                if (key != 1) {
                    result.result = FAILED;
                    result.text = "Expected a key";
                    return result;
                }
                if (colon != 1) {
                    result.result = FAILED;
                    result.text = "Colon expected";
                    return result;
                }
                break;
            case JSON_TOKEN_NULL:
                value++;
                if (key != 1) {
                    result.result = FAILED;
                    result.text = "";
                    return result;
                }
                if (colon != 1) {
                    result.result = FAILED;
                    result.text = "Colon expected";
                    return result;
                }
                break;
            case JSON_TOKEN_EOF:
                result.result = FAILED;
                result.text = "EOF reached without closing brace";
                return result;
                break;
            case JSON_TOKEN_ILLEGAL:
                result.text = "Illegal token";
                if (tokens[*token].value.strValue != NULL)
                    result.text = tokens[*token].value.strValue;
                result.result = FAILED;
                return result;
                break;
        }

        (*token)++;
    }
    result.result = FAILED;
    result.text = "Missing ending brace in object";
    return result;
}

parse_result parse_array(json_token *tokens, unsigned long long *token) {

    (*token)++; // skip the LBRACKET

    parse_result result;

    int lastTokenValue = 0;
    int lastTokenComma = 0;

    // check the first token after LBRACKET for expected value

    switch (tokens[*token].kind) {
        case JSON_TOKEN_LBRACKET:
                parse_result array_result = parse_array(tokens, token);
                if (array_result.result == FAILED)
                    return array_result;
                break;
            case JSON_TOKEN_RBRACKET:
                result.result = SUCCESS;
                result.text = "";
                return result;
                break;
            case JSON_TOKEN_LBRACE:
                parse_result object_result = parse_object(tokens, token);
                if (object_result.result == FAILED)
                    return object_result;
                break;
            case JSON_TOKEN_RBRACE:
                result.result = FAILED;
                result.text = "} is invalid at this point";
                return result;
                break;
            case JSON_TOKEN_COMMA:
                    result.result = FAILED;
                    result.text = "Value expected";
                    return result;
                break;
            case JSON_TOKEN_COLON:
                result.result = FAILED;
                result.text = "Value expected";
                return result;
                break;
            case JSON_TOKEN_EOF:
                result.result = FAILED;
                result.text = "EOF reached without closing bracket";
                return result;
                break;
            case JSON_TOKEN_ILLEGAL:
                result.result = FAILED;
                result.text = "Illegal token";
                if (tokens[*token].value.strValue != NULL)
                    result.text = tokens[*token].value.strValue;
                return result;
                break;
            default:
                lastTokenValue = 1;
    }

    (*token)++;


    while (tokens[*token].kind != JSON_TOKEN_EOF) {

        switch (tokens[*token].kind) {
            case JSON_TOKEN_LBRACKET:
                if (lastTokenValue == 1) {
                    result.result = FAILED;
                    result.text = "Comma expected";
                    return result;
                }
                parse_result array_result = parse_array(tokens, token);
                if (array_result.result == FAILED)
                    return array_result;
                lastTokenValue = 1;
                lastTokenComma = 0;
                break;
            case JSON_TOKEN_RBRACKET:
                if (lastTokenComma == 1) {
                    result.result = FAILED;
                    result.text = "Trailing comma";
                    return result;
                }
                result.result = SUCCESS;
                result.text = "";
                return result;
                break;
            case JSON_TOKEN_LBRACE:
                if (lastTokenValue == 1) {
                    result.result = FAILED;
                    result.text = "Comma expected";
                    return result;
                }
                parse_result object_result = parse_object(tokens, token);
                lastTokenValue = 1;
                lastTokenComma = 0;
                if (object_result.result == FAILED)
                    return object_result;
                break;
            case JSON_TOKEN_RBRACE:
                result.result = FAILED;
                result.text = "} is invalid at this point";
                return result;
                break;
            case JSON_TOKEN_COMMA:
                if (lastTokenComma == 1) {
                    result.result = FAILED;
                    result.text = "Value expected";
                    return result;
                }
                lastTokenValue = 0;
                lastTokenComma = 1;
                break;
            case JSON_TOKEN_COLON:
                result.result = FAILED;
                if (lastTokenComma == 1) {
                    result.text = "Value expected";
                    return result;
                }
                if (lastTokenValue == 1) {
                    result.text = "Comma expected";
                    return result;
                }
                break;
            case JSON_TOKEN_STRING:
                if (lastTokenValue == 1) {
                    result.result = FAILED;
                    result.text = "Comma expected";
                    return result;
                }
                lastTokenValue = 1;
                lastTokenComma = 0;
                break;
            case JSON_TOKEN_NUMBER:
                if (lastTokenValue == 1) {
                    result.result = FAILED;
                    result.text = "Comma expected";
                    return result;
                }
                lastTokenValue = 1;
                lastTokenComma = 0;
                break;
            case JSON_TOKEN_BOOLEAN_TRUE:
                if (lastTokenValue == 1) {
                    result.result = FAILED;
                    result.text = "Comma expected";
                    return result;
                }
                lastTokenValue = 1;
                lastTokenComma = 0;
                break;
            case JSON_TOKEN_BOOLEAN_FALSE:
                if (lastTokenValue == 1) {
                    result.result = FAILED;
                    result.text = "Comma expected";
                    return result;
                }
                lastTokenValue = 1;
                lastTokenComma = 0;
                break;
            case JSON_TOKEN_NULL:
                if (lastTokenValue == 1) {
                    result.result = FAILED;
                    result.text = "Comma expected";
                    return result;
                }
                lastTokenValue = 1;
                lastTokenComma = 0;
                break;
            case JSON_TOKEN_EOF:
                result.result = FAILED;
                result.text = "EOF reached without closing bracket";
                return result;
                break;
            case JSON_TOKEN_ILLEGAL:
                result.result = FAILED;
                result.text = "Illegal token";
                if (tokens[*token].value.strValue != NULL)
                    result.text = tokens[*token].value.strValue;
                return result;
                break;
        }

        (*token)++;
    }
    result.result = FAILED;
    result.text = "Missing ending bracket in array";
    return result;
}

json_token* lexor(char *text, unsigned long long textLength) {
    json_token *tokens = malloc(sizeof(json_token) * (textLength + 1));
    memset(tokens, 0, sizeof(json_token) * (textLength + 1));
    unsigned long long currentToken = 0;
    unsigned long long textCurrentPos = 1;
    unsigned long long textLine = 1;

    int eof = 0;

    char *iter = text; // lav ny pointer der peger på starten af text (eg. "hello" = 'h')
    skip_spaces(&iter, &textCurrentPos, &textLine); // giver addressen på iter som parameter, så ændringer i skip_spaces kan ses i lexor funktion

    while (iter) {
        json_token token;
        token.pos = textCurrentPos;
        token.line = textLine;

        if (*iter == '{') {
            token.kind = JSON_TOKEN_LBRACE;
            token.value.strValue = NULL;
        } else if (*iter == '[') {
            token.kind = JSON_TOKEN_LBRACKET;
            token.value.strValue = NULL;
        } else if (*iter == '}') {
            token.kind = JSON_TOKEN_RBRACE;
            token.value.strValue = NULL;
        } else if (*iter == ']') {
            token.kind = JSON_TOKEN_RBRACKET;
            token.value.strValue = NULL;
        } else if (*iter == '"') {
            token.kind = JSON_TOKEN_STRING;
            parse_result stringResult = parse_string(&iter, &textCurrentPos); // passes the iterater and progresses it

            // hvis fejl sker i parse_string, returner ILLEGAL_TOKEN
            if (stringResult.result == FAILED) {
                token.kind = JSON_TOKEN_ILLEGAL;
                token.value.strValue = stringResult.text;
                tokens[currentToken] = token;
                textCurrentPos++;
                currentToken++;
                iter++;
                break;
            }

            token.value.strValue = stringResult.text;
        } else if (*iter == ':') {
            token.kind = JSON_TOKEN_COLON;
            token.value.strValue = NULL;
        } else if (*iter == ',') {
            token.kind = JSON_TOKEN_COMMA;
            token.value.strValue = NULL;
        } else {
            token = parse_value(&iter, &textCurrentPos, &textLine);
            if (token.kind == JSON_TOKEN_ILLEGAL) {
                tokens[currentToken] = token;
                textCurrentPos++;
                currentToken++;
                iter++;
                break;
            } else if (token.kind == JSON_TOKEN_EOF) {
                tokens[currentToken] = token;
                eof = 1;
                break;
            }
        }


        tokens[currentToken] = token;
        textCurrentPos++;
        currentToken++;
        iter++;

        skip_spaces(&iter, &textCurrentPos, &textLine);
    }

    if (!eof) {
        tokens[currentToken].kind = JSON_TOKEN_EOF;
        tokens[currentToken].value.strValue = NULL;
        tokens[currentToken].pos = textCurrentPos;
    }

    return tokens;
}


json_token parse_value(char **iter, unsigned long long *textCurrentPos, unsigned long long *textLine) {

    if (**iter == '\0') {
        json_token token;
        token.kind = JSON_TOKEN_EOF;
        token.value.strValue = NULL;
        token.pos = *textCurrentPos;
        token.line = *textLine;
        return token;
    }

    if (**iter == '-' || isdigit(**iter)) {
        return parse_numeric(iter, textCurrentPos, textLine);
    } else if (**iter == 'f' || **iter == 't' || **iter == 'n') { // parsing literal (boolean or null)
        return parse_literal(iter, textCurrentPos, textLine);
    } else {
        json_token token;
        token.kind = JSON_TOKEN_ILLEGAL;
        token.value.strValue = NULL;
        token.pos = *textCurrentPos;
        token.line = *textLine;
        return token;
    }
}

json_token parse_numeric(char **iter, unsigned long long *textCurrentPos, unsigned long long *textLine) {
    json_token token;
    int isNegative = 0; // is it negative
    int digitsParsed = 0; // how many digits parsed?
    int theFirstDigit = 0;
    int howManyE = 0; // does the number contain e?
    int howManyDot = 0; // does the number contain dot?
    int decimalPlaces = 0; // is it a decimal? and if so, what place are we at?


    if (**iter == '-') {
        isNegative = 1;
        (*textCurrentPos)++;
        (*iter)++;
    }
    double number = 0;

    if (isdigit(**iter)) {
        theFirstDigit = (**iter - '0');
        number = theFirstDigit;
        digitsParsed++;
        (*textCurrentPos)++;
        (*iter)++;
    }

    while (isdigit(**iter) || **iter == '.' || **iter == '-' || **iter == '+' || **iter == 'e' || **iter == 'E') {
        if (isdigit(**iter) && !decimalPlaces) {
            digitsParsed++;
            number = number * 10 + (**iter - '0'); // Converts the chars to integer with minus operation
        }
        if (isdigit(**iter) && decimalPlaces) {
            digitsParsed++;
            decimalPlaces = decimalPlaces * 10;
            number = number + (double)(**iter - '0') / (double)decimalPlaces;
        }
        if (**iter == '.') {
            howManyDot++;
            if (decimalPlaces == 0)
                decimalPlaces = 10;

            (*textCurrentPos)++;
            (*iter)++;
            if (!isdigit(**iter)) {
                token.kind = JSON_TOKEN_ILLEGAL;
                token.value.strValue = "Unexpected end of numeric";
                token.pos = *textCurrentPos;
                token.line = *textLine;
                return token;
            }
            if (digitsParsed < 1) {
                token.kind = JSON_TOKEN_ILLEGAL;
                token.value.strValue = "Value expected";
                token.pos = *textCurrentPos;
                token.line = *textLine;
                return token;
            }
            if (howManyE > 0) {
                token.kind = JSON_TOKEN_ILLEGAL;
                token.value.strValue = "e or E must come after dot";
                token.pos = *textCurrentPos;
                token.line = *textLine;
                return token;
            }
            if (howManyDot > 1) {
                token.kind = JSON_TOKEN_ILLEGAL;
                token.value.strValue = "Cannot have multiple dots in numeric";
                token.pos = *textCurrentPos;
                token.line = *textLine;
                return token;
            }
            digitsParsed++;
            number = number + (double)(**iter - '0') / (double)decimalPlaces;
        }
        if (**iter == 'e' || **iter == 'E') {
            howManyE++;
            if (howManyE > 1) {
                token.kind = JSON_TOKEN_ILLEGAL;
                token.value.strValue = "Cannot have multiple E's in number";
                token.pos = *textCurrentPos;
                token.line = *textLine;
                return token;
            }
            if (digitsParsed < 1) {
                token.kind = JSON_TOKEN_ILLEGAL;
                token.value.strValue = "e notation requires number before e";
                token.pos = *textCurrentPos;
                token.line = *textLine;
                return token;
            }

            (*textCurrentPos)++;
            (*iter)++;

            parse_result res = handle_scientific_notation(iter, textCurrentPos, textLine, &number);

            if (res.result == FAILED) {
                token.kind = JSON_TOKEN_ILLEGAL;
                token.value.strValue = res.text;
                token.line = *textLine;
                token.pos = *textCurrentPos;
                return token;
            }

        }
        (*textCurrentPos)++;
        (*iter)++;
    }

    // A minus was found, but no digits
    if (digitsParsed == 0) {
        token.kind = JSON_TOKEN_ILLEGAL;
        token.value.strValue = "Cannot have a minus, without following value";
        token.pos = *textCurrentPos;
        token.line = *textLine;
        return token;
    }

    // the first number is a 0 but has trailing numbers
    if (digitsParsed > 1 && theFirstDigit == 0 && !decimalPlaces) {
        token.kind = JSON_TOKEN_ILLEGAL;
        token.value.strValue = "0 cannot have trailing numbers";
        token.pos = *textCurrentPos;
        token.line = *textLine;
        return token;
    }

    if (isNegative) {
        number = -number;
    }

    // We decrement the iterable and the pos, because it is incremented in the lexor function (so as to not skip following char eg a ',')
    (*iter)--;
    (*textCurrentPos)--;

    if (howManyE > 0)
        token.isENotation = 1;
    else
        token.isENotation = 0;

    token.kind = JSON_TOKEN_NUMBER;
    token.value.numValue = number;
    token.pos = *textCurrentPos;
    token.line = *textLine;

    return token;
}

parse_result handle_scientific_notation(char **iter, unsigned long long *textCurrentPos, unsigned long long *textLine, double *number) {
    parse_result result;
    int isNegative = 0;
    int symbolCount = 0;
    int digitsParsed = 0;

    long long notationValue = 0;

    while (isdigit(**iter) || **iter == '-' || **iter == '+') {
        if (**iter == '-') {
            if (digitsParsed > 0) {
                result.result = FAILED;
                result.text = "Minus or plus notation must come before digits";
                return result;
            }
            isNegative = 1;
            symbolCount++;
        } else if (**iter == '+') {
            if (digitsParsed > 0) {
                result.result = FAILED;
                result.text = "Minus or plus notation must come before digits";
                return result;
            }
            symbolCount++;
        }
        if (symbolCount > 1) {
            result.result = FAILED;
            result.text = "Cannot have multiple add or neg symbols after e notation";
            return result;
        }

        if (isdigit(**iter)) {
            notationValue = notationValue * 10 + (**iter - '0');
            digitsParsed++;
        }

        (*iter)++;
        (*textCurrentPos)++;
    }

    if (isNegative) {
        notationValue = -notationValue;
    }

    (*textCurrentPos)--;
    (*iter)--;

    if (digitsParsed < 1) {
        result.result = FAILED;
        result.text = "No digits after e notation";
        return result;
    }

    *number = *number * pow(10, notationValue);

    result.result = SUCCESS;
    result.text = "";
    return result;
}

json_token parse_literal(char **iter, unsigned long long *textCurrentPos, unsigned long long *textLine) {
    json_token token;

    if (**iter == 't') {
        (*iter)++;
        (*textCurrentPos)++;
        for (int i = 0; i < 3; i++) {
            if (i == 0 && **iter != 'r') {
                token.kind = JSON_TOKEN_ILLEGAL;
                token.value.strValue = NULL;
                token.pos = *textCurrentPos - 1;
                token.line = *textLine;
                return token;
            }
            if (i == 1 && **iter != 'u') {
                token.kind = JSON_TOKEN_ILLEGAL;
                token.value.strValue = NULL;
                token.pos = *textCurrentPos - 1;
                token.line = *textLine;
                return token;
            }
            if (i == 2 && **iter != 'e') {
                token.kind = JSON_TOKEN_ILLEGAL;
                token.value.strValue = NULL;
                token.pos = *textCurrentPos - 1;
                token.line = *textLine;
                return token;
            }

            (*textCurrentPos)++;
            (*iter)++;
        }

        token.kind = JSON_TOKEN_BOOLEAN_TRUE;
        token.value.strValue = NULL;
        token.pos = *textCurrentPos;
        token.line = *textLine;

        (*textCurrentPos)--;
        (*iter)--;

        return token;
    }

    if (**iter == 'n') {
        (*iter)++;
        (*textCurrentPos)++;
        for (int i = 0; i < 3; i++) {

            if (i == 0 && **iter != 'u') {
                token.kind = JSON_TOKEN_ILLEGAL;
                token.value.strValue = NULL;
                token.pos = *textCurrentPos - 1;
                token.line = *textLine;
                return token;
            }
            if (i == 1 && **iter != 'l') {
                token.kind = JSON_TOKEN_ILLEGAL;
                token.value.strValue = NULL;
                token.pos = *textCurrentPos - 1;
                token.line = *textLine;
                return token;
            }
            if (i == 2 && **iter != 'l') {
                token.kind = JSON_TOKEN_ILLEGAL;
                token.value.strValue = NULL;
                token.pos = *textCurrentPos - 1;
                token.line = *textLine;
                return token;
            }
            (*textCurrentPos)++;
            (*iter)++;
        }

        token.kind = JSON_TOKEN_NULL;
        token.value.strValue = NULL;
        token.pos = *textCurrentPos;
        token.line = *textLine;

        (*textCurrentPos)--;
        (*iter)--;

        return token;
    }
    if (**iter == 'f') {
        (*iter)++;
        (*textCurrentPos)++;
        for (int i = 0; i < 4; i++) {
            if (i == 0 && **iter != 'a') {
                token.kind = JSON_TOKEN_ILLEGAL;
                token.value.strValue = NULL;
                token.pos = *textCurrentPos - 1;
                token.line = *textLine;
                return token;
            }
            if (i == 1 && **iter != 'l') {
                token.kind = JSON_TOKEN_ILLEGAL;
                token.value.strValue = NULL;
                token.pos = *textCurrentPos - 1;
                token.line = *textLine;
                return token;
            }
            if (i == 2 && **iter != 's') {
                token.kind = JSON_TOKEN_ILLEGAL;
                token.value.strValue = NULL;
                token.pos = *textCurrentPos - 1;
                token.line = *textLine;
                return token;
            }
            if (i == 3 && **iter != 'e') {
                token.kind = JSON_TOKEN_ILLEGAL;
                token.value.strValue = NULL;
                token.pos = *textCurrentPos - 1;
                token.line = *textLine;
                return token;
            }
            (*textCurrentPos)++;
            (*iter)++;
        }

        token.kind = JSON_TOKEN_BOOLEAN_FALSE;
        token.value.strValue = NULL;
        token.pos = *textCurrentPos;
        token.line = *textLine;

        (*textCurrentPos)--; // decrement, because it is incremented in the main lexor function
        (*iter)--;

        return token;
    }

    token.kind = JSON_TOKEN_ILLEGAL;
    token.value.strValue = NULL;
    token.pos = *textCurrentPos;
    token.line = *textLine;
    return token;

}

parse_result parse_string(char **iter, unsigned long long *textCurrentPos) {
    parse_result result;
    (*iter)++; // skips the '"'
    (*textCurrentPos)++;

    int i = 0;
    char *buffer = malloc(sizeof(char) * 500);
    memset(buffer, 0, sizeof(char) * 500);

    while (**iter != '"') {
        if (**iter == '\0') {
            result.result = FAILED;
            result.text = "Unexpected end of string";
            return result;
        }
        if (**iter == '\n' || **iter == '\t') {
            result.result = FAILED;
            result.text = "Unexpected newline or tab in string";
            return result;
        }
        if (**iter == '\\'){
            parse_result res = parse_escape_sequence(iter, textCurrentPos, buffer, &i);
            if (res.result == FAILED)
                return res;
            continue;
        }
        buffer[i++] = **iter;
        (*iter)++;
        (*textCurrentPos)++;
    }
    buffer[i] = '\0';

    // Add debug print

    // printf("parse_string returned: [%s]\n", buffer);

    result.result = SUCCESS;
    result.text = buffer;
    return result;
    // we DON'T skip the last '"' because that is being done in the lexor function
}

parse_result parse_escape_sequence(char **iter, unsigned long long *textCurrentPos, char *buffer, int *i) {
    parse_result result;
    buffer[(*i)++] = **iter;
    (*iter)++;
    (*textCurrentPos)++;
    if (**iter == 'u') {
        buffer[(*i)++] = **iter;
        (*iter)++;
        (*textCurrentPos)++;
        parse_result unicode = parse_unicode(iter, textCurrentPos, buffer, i);
        return unicode;
    }
    if (**iter == '"' || **iter == '\\' || **iter == 'b' || **iter == 'r' || **iter == 'n' || **iter == 'f'
       || **iter == 't' || **iter == '/') {
        buffer[(*i)++] = **iter;
        (*iter)++;
        (*textCurrentPos)++;
        result.result = SUCCESS;
        result.text = "";
        return result;
    }
    result.result = FAILED;
    result.text = "Not a valid escape sequence";
    return result;
}

parse_result parse_unicode(char **iter, unsigned long long *textCurrentPos, char *buffer, int *i) {
    parse_result result;
    int hexNumber = 1;

    while (hexNumber < 5) {
        int isHex = isxdigit(**iter);
        if (!isHex) {
            result.result = FAILED;
            result.text = "Invalid unicode sequence in string";
            return result;
        }
        buffer[(*i)++] = **iter;
        hexNumber++;
        (*iter)++;
        (*textCurrentPos)++;
    }
    result.result = SUCCESS;
    result.text = "";
    return result;
}

char* loadfile(char *text) {
    FILE *file = fopen(text, "rb"); // using "rb" instead of "r" to make CRLF work
    if (!file) return NULL;
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);
    char *buffer = malloc(size + 1);
    buffer[size] = '\0';
    fread(buffer, 1, size, file);
    fclose(file);
    return buffer;
}


void skip_spaces(char **text, unsigned long long *textCurrentPos, unsigned long long *textLine) {
    while (**text == ' ' || **text == '\n' || **text == '\r' || **text == '\t') {
        if (**text == '\n') {
            (*textCurrentPos) = 0;
            (*textLine)++;
        }
        if (**text == '\r')
            (*textCurrentPos) = 0;
        (*text)++;
        (*textCurrentPos)++;
    }
}
