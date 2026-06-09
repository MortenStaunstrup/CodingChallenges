#include <stdio.h>
#include <stdlib.h>
#include "Tests.h"


void RunDeserializationTests() {

    CreateDeserializationTests res = CreateDeserializationTestsFunction();

    for (int i = 0; i < res.amountOfTests; i++) {
        TypeResponse t = ResponseType(res.tests[i]);
        if (!t.validType) {
            printf("Invalid type\n");
        }
    }

    free(res.tests);
    free(res.expected);
    free(res.expectFailed);
}


TypeResponse ResponseType(char* ch) {
    TypeResponse t;
    t.validType = 1;
    switch (*ch) {
        case '+':
            t.type = SSTRING;
            break;
        case '-':
            t.type = ERROR;
            break;
        case ':':
            t.type = INTEGER;
            break;
        case '$':
            t.type = BSTRING;
            break;
        case '*':
            t.type = ARRAY;
            break;
        default:
            t.validType = 0;
    }

    return t;

}

CreateDeserializationTests CreateDeserializationTestsFunction() {
    CreateDeserializationTests res;
    res.amountOfTests = 13;
    char** tests;
    char** expectedParsedResponses;
    int* expectedFailedResponse;
    tests = (char**)malloc(res.amountOfTests * sizeof(char*));
    expectedParsedResponses = (char**)malloc(res.amountOfTests * sizeof(char*));
    expectedFailedResponse = (int*)malloc(res.amountOfTests * sizeof(int));

    tests[0] = "$-1\r\n";
    expectedParsedResponses[0] = NULL;

    tests[1] = "*1\r\n$4\r\nping\r\n";
    expectedParsedResponses[1] = "ping";

    tests[2] = "*2\r\n$4\r\necho\r\n$11\r\nhello world\r\n";
    expectedParsedResponses[2] = "echo hello world";

    tests[3] = "*2\r\n$3\r\nget\r\n$3\r\nkey\r\n";
    expectedParsedResponses[3] = "[get, key]";

    tests[4] = "+OK\r\n";
    expectedParsedResponses[4] = "OK";

    tests[5] = "-Error message\r\n";
    expectedParsedResponses[5] = "Error message";

    tests[6] = "$0\r\n\r\n";
    expectedParsedResponses[6] = "";

    tests[7] = "+hello world\r\n";
    expectedParsedResponses[7] = "hello world";

    tests[8] = "$0\r\r\n";
    expectedParsedResponses[8] = NULL;
    expectedFailedResponse[8] = 1;

    tests[9] = "$-1";
    expectedParsedResponses[9] = NULL;
    expectedFailedResponse[9] = 1;

    tests[10] = "sfdgsfdg4e6346346";
    expectedParsedResponses[10] = NULL;
    expectedFailedResponse[10] = 1;

    tests[11] = "+Whatup\r\n";
    expectedParsedResponses[11] = "Whatup";

    tests[12] = "-Error\r\n";
    expectedParsedResponses[12] = NULL;
    expectedFailedResponse[12] = 1;

    res.tests = tests;
    res.expected = expectedParsedResponses;
    res.expectFailed = expectedFailedResponse;
    return res;
}