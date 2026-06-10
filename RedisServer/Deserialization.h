//
// Created by mort4 on 10-06-2026.
//

#ifndef REDISSERVER_DESERIALIZATION_H
#define REDISSERVER_DESERIALIZATION_H
#include "Tests.h"

typedef enum  {
    SUCCESS,
    FAILURE
} Result;

typedef struct ArrayElement {
    Type type;
    union {
        char* stringResponse;
        int intValue;
        struct ArrayElement* array;
    };
    int arrayElementLength;
} ArrayElement;

typedef struct {
    int length;
    ArrayElement* array;
} ArrayResult;

typedef struct {
    int isInteger;
    int isArray;
    union {
        char* content;
        int intValue;
        ArrayResult* array;
    };
    Result result;
} DeserializationResult;

char* deserializeSimpleString(char** ch);

char* deserializeBulkStrings(char** ch);

char* concatenate(char* string1, char* string2);

char* deserializeError(char** ch);

int deserializeInteger(char** ch);

ArrayResult deserializeArrayElements(char** ch);

char* arrayConcatenate(char* string1, char* string2, int isString);

char* arrayConcatenateStart(char* string1, char* string2, int isString);

char* arrayConcatenateEnd(char* string1, char* string2, int isString);

char* arrayConcatenateOneElement(char* string1, char* string2, int isString);

char* arrayResponseConcatenator(char* result, ArrayElement* array, int index, int isStart, int isEnd, int hasOneElement);

char* deserializeEmbeddedArray(ArrayElement* array, int length);

char* deserializeArray(char** ch);

char* deserializeRequest(char** ch);

#endif //REDISSERVER_DESERIALIZATION_H
