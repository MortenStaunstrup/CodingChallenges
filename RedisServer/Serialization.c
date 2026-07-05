//
// Created by mort4 on 25-06-2026.
//

#include "Serialization.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Deserialization.h"

// Things get sent to the server as an 'Array of bulk-strings' as per.: https://redis.io/docs/latest/develop/reference/protocol-spec/

// Todo need logic for when string is larger than initsize
SerializationRequestResult SerializeSimpleString(char** ch) {
    SerializationRequestResult result = {0};

    int initSize = 50;
    char* simpleString = malloc(sizeof(char) * initSize);
    int index = 0;

    simpleString[index++] = '+';

    while (**ch != '\0') {
        if (**ch == '\r' || **ch == '\n') {
            result.result = FAILED;
            result.errorMessage = "Simple string cannot contain CRLF";
            return result;
        }
        simpleString[index++] = **ch;
        (*ch)++;
    }
    simpleString[index++] = '\r';
    simpleString[index++] = '\n';
    simpleString[index] = '\0';
    result.result = SUCCESS;
    result.content = simpleString;
    result.contentLength = index;
    return result;
}


char* concatenate(char* string1, char* string2) {
    char* result = (char*)malloc(strlen(string1) + strlen(string2) + 2);
    if (result == NULL) {
        printf("concatenate: malloc failed\n");
        exit(1);
    }
    strcpy(result, string1);
    strcat(result, string2);
    result[strlen(result)] = '\0';
    return result;
}


SerializationRequestResult SerializeBulkString(char** ch) {
    SerializationRequestResult result = {0};

    int initSize = 50;
    char* bulkString = malloc(sizeof(char) * initSize);
    int index = 0;

    while (**ch != '\0') {
        bulkString[index++] = **ch;
        (*ch)++;
    }
    bulkString[index] = '\0';

    char length[25];
    sprintf(length, "%d", index);

    char* firstCon = concatenate("$", length);
    char* secondCon = concatenate(firstCon, "\r\n");
    char* thirdCon = concatenate(secondCon, bulkString);
    char* finalResult = concatenate(thirdCon, "\r\n");
    result.result = SUCCESS;
    result.content = finalResult;
    result.contentLength = index + 6 + ((int)strlen(length) - 1);
    return result;
}

SerializationRequestResult SerializeInteger(char** ch) {
    SerializationRequestResult result = {0};

    char integerValue[25];
    int index = 0;

    if (**ch == '+' && **ch == '-') {
        integerValue[index++] = **ch;
    }

    while (**ch != '\0') {
        if (isdigit(**ch)) {
            integerValue[index++] = **ch;
        } else {
            result.result = FAILED;
            result.errorMessage = "Integer contains non numeric character";
            return result;
        }
        (*ch)++;
    }

    char* firstCon = concatenate(":", integerValue);
    char* secondCon = concatenate(firstCon, "\r\n");
    result.result = SUCCESS;
    result.content = secondCon;
    return result;
}

SerializationRequestResult SerializeError(char** ch) {
    SerializationRequestResult result = {0};

    char error[50];
    int index = 0;

    int containsSpace = 0;
    int containsErrorMessage = 0;

    while (**ch != '\0') {
        if (**ch == ' ') {
            containsSpace = 1;
            error[index++] = **ch;
        } else {
            error[index++] = **ch;
            if (containsSpace) {
                containsErrorMessage = 1;
            }
        }
        (*ch)++;
    }

    if (!containsSpace || !containsErrorMessage) {
        result.result = FAILED;
        result.errorMessage = "Error does not contain error message";
        return result;
    }

    char* firstCon = concatenate("-", error);
    char* secondCon = concatenate(firstCon, "\r\n");
    result.result = SUCCESS;
    result.content = secondCon;
    return result;
}

SerializationRequestResult SerializeArray(char** ch) {
    SerializationRequestResult result = {0};
}