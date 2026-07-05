//
// Created by mort4 on 25-06-2026.
//

#ifndef REDISSERVER_STRUCTS_H
#define REDISSERVER_STRUCTS_H

typedef enum {
    TYPE_SSTRING,
    TYPE_ERROR,
    TYPE_INTEGER,
    TYPE_BSTRING,
    TYPE_ARRAY
} Type;

typedef struct {
    int amountOfTests;
    char** tests;
    char** expected;
    int* expectFailed;
} CreateDeserializationTests;

typedef struct {
    int validType;
    Type type;
} TypeResponse;


typedef enum  {
    FAILED,
    SUCCESS
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
        ArrayResult array;
    };
    Result result;
    char* errorMessage;
} DeserializationResult;

typedef struct {
    Result result;
    char* errorMessage;
    char* content;
    int contentLength;
} SerializationRequestResult;

typedef struct {
    Result result;
    char* errorMessage;
    char* content;
    int contentLength;
} ClientRequestResult;

typedef struct {
    Result result;
    char* errorMessage;
    char** commands;
    int commandsCount;
} ClientCommandsResult;

#endif //REDISSERVER_STRUCTS_H
