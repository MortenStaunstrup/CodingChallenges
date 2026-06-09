//
// Created by mort4 on 09-06-2026.
//

#ifndef REDISSERVER_TESTS_H
#define REDISSERVER_TESTS_H

typedef enum {
    SSTRING,
    ERROR,
    INTEGER,
    BSTRING,
    ARRAY
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

TypeResponse ResponseType(char* ch);

CreateDeserializationTests CreateDeserializationTestsFunction();

void RunDeserializationTests();

#endif //REDISSERVER_TESTS_H
