#include <stdio.h>
#include <stdlib.h>
#include "Tests.h"

#include <string.h>

#include "Deserialization.h"


void RunDeserializationTests() {

    CreateDeserializationTests testResult = CreateDeserializationTestsFunction();

    for (int i = 0; i < testResult.amountOfTests; i++) {
        DeserializeRequestResult result = deserializeRequest(&testResult.tests[i]);
        int testCase = i + 1;
        switch (result.result) {
            case SUCCESS:
                if (testResult.expectFailed[i] == 1) {
                    printf("Test %d: FAILED\nExpected parsing failure response, but got SUCCESS\n", testCase);
                } else if (result.content == NULL || strcmp(testResult.expected[i], result.content) == 0) {
                    printf("Test %d: PASSED\n", testCase);
                } else {
                    printf("Test %d: FAILED\nExpected \"%s\" but got \"%s\"", testCase, testResult.expected[i], result.content);
                }
                break;
            case FAILED:
                if (testResult.expectFailed[i] == 1) {
                    printf("Test %d: PASSED with failure message: %s\n", testCase, result.errorMessage);
                } else {
                    printf("Test %d: FAILED\nDid not expect parsing to fail. Error message: %s\n", testCase, result.errorMessage);
                }
                break;
        }
    }

    free(testResult.tests);
    free(testResult.expected);
    free(testResult.expectFailed);
}


TypeResponse ResponseType(char* ch) {
    TypeResponse t;
    t.validType = 1;
    switch (*ch) {
        case '+':
            t.type = TYPE_SSTRING;
            break;
        case '-':
            t.type = TYPE_ERROR;
            break;
        case ':':
            t.type = TYPE_INTEGER;
            break;
        case '$':
            t.type = TYPE_BSTRING;
            break;
        case '*':
            t.type = TYPE_ARRAY;
            break;
        default:
            t.validType = 0;
    }

    return t;

}

CreateDeserializationTests CreateDeserializationTestsFunction() {
    CreateDeserializationTests res;
    res.amountOfTests = 16;
    char** tests;
    char** expectedParsedResponses;
    int* expectedFailedResponse;
    tests = (char**)malloc(res.amountOfTests * sizeof(char*));
    expectedParsedResponses = (char**)malloc(res.amountOfTests * sizeof(char*));
    expectedFailedResponse = (int*)malloc(res.amountOfTests * sizeof(int));

    tests[0] = "$-1\r\n";
    expectedParsedResponses[0] = NULL;

    tests[1] = "*1\r\n$4\r\nping\r\n";
    expectedParsedResponses[1] = "[\"ping\"]";

    tests[2] = "*2\r\n$4\r\necho\r\n$11\r\nhello world\r\n";
    expectedParsedResponses[2] = "[\"echo\", \"hello world\"]";

    tests[3] = "*2\r\n$3\r\nget\r\n$3\r\nkey\r\n";
    expectedParsedResponses[3] = "[\"get\", \"key\"]";

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

    tests[13] = "$9\r\nHey\r\nMy G\r\n";
    expectedParsedResponses[13] = "Hey\r\nMy G";

    tests[14] = "$21\r\nHello my name is Carl\r\n";
    expectedParsedResponses[14] = "Hello my name is Carl";

    // Is "11" because there are 11 bytes in this string, "éøæûü" all count as 2 bytes each
    tests[15] = "$11\r\néRøæûü\r\n";
    expectedParsedResponses[15] = "éRøæûü";

    res.tests = tests;
    res.expected = expectedParsedResponses;
    res.expectFailed = expectedFailedResponse;
    return res;
}