#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum Type {
    SSTRING,
    ERROR,
    INTEGER,
    BSTRING,
    ARRAY,
    null
} Type;

typedef struct TypeResponse {
    int validType;
    Type type;
} TypeResponse;

typedef struct CreateParseTestResult {
    int amountOfTests;
    char** tests;
    char** expectedParsedResponses;
} CreateParseTestResult;

int CheckForNullResponse(char* ch) {
    // If not a bulk string or array
    if (*ch != '$' && *ch != '*') {
        exit(401);
    }
    char* p = ch + 1;

    int i = 0;
    while (i != 4) {
        if (i == 0 && *p != '-')
            return 0;
        else if (i == 1 && *p != '1')
            return 0;
        else if (i == 2 && *p != '\r')
            return 0;
        else if (i == 3 && *p != '\n')
            return 0;
        i++;
        p++;
    }
    return 1;
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
            int nullResponse = CheckForNullResponse(ch);
            if (nullResponse)
                t.type = null;
            else
                t.type = BSTRING;
            break;
        case '*':
            t.type = ARRAY;
            break;
        default:
            t.type = ERROR;
            t.validType = 0;
    }

    return t;

}

CreateParseTestResult CreateExpectedTypeTests() {
    CreateParseTestResult res;
    res.amountOfTests = 8;
    char** tests;
    char** expectedParsedResponses;
    tests = (char**)malloc(res.amountOfTests * sizeof(char*));
    expectedParsedResponses = (char**)malloc(res.amountOfTests * sizeof(char*));

    tests[0] = "$-1\r\n";
    expectedParsedResponses[0] = NULL;

    tests[1] = "*1\r\n$4\r\nping\r\n";
    expectedParsedResponses[1] = "ping";

    tests[2] = "*2\r\n$4\r\necho\r\n$11\r\nhello world\r\n";
    expectedParsedResponses[2] = "echo hello world";

    tests[3] = "*2\r\n$3\r\nget\r\n$3\r\nkey\r\n";
    expectedParsedResponses[3] = "get key";

    tests[4] = "+OK\r\n";
    expectedParsedResponses[4] = "OK";

    tests[5] = "-Error message\r\n";
    expectedParsedResponses[5] = "message";

    tests[6] = "$0\r\n\r\n";
    expectedParsedResponses[6] = "";

    tests[7] = "+hello world\r\n";
    expectedParsedResponses[7] = "hello world";

    res.tests = tests;
    res.expectedParsedResponses = expectedParsedResponses;
    return res;
}

void RunTests() {

    CreateParseTestResult res = CreateExpectedTypeTests();

    for (int i = 0; i < res.amountOfTests; i++) {
        printf("Test case: %s\nENDTEST\n", res.tests[i]);

        TypeResponse response = ResponseType(res.tests[i]);

        if (response.validType == 0) {
            printf("Invalid type\n");
            exit(1);
        }

        switch (response.type) {
            case SSTRING:
                printf("Simple string response\n");
                break;
            case ERROR:
                printf("Error response\n");
                break;
            case INTEGER:
                printf("Integer response\n");
                break;
            case null:
                printf("Null response\n");
                break;
            case BSTRING:
                printf("Bulk string response\n");
                break;
            case ARRAY:
                printf("Array response\n");
                break;
            default:
                printf("Unexpected response type");
                exit(1);
        }

    }

    free(res.tests);
    free(res.expectedParsedResponses);
}

char* parseSimpleString(char* ch) {
    if (*ch != '+') {
        printf("parseSimpleString: expected '+' starting char\n");
        exit(1);
    }

    char* p = ch + 1;
    int length = 0;
    int capacity = 16;
    char *string = NULL;

    string = (char*)malloc(capacity * sizeof(char));
    if (string == NULL) {
        printf("parseSimpleString: malloc failed\n");
        exit(1);
    }

    while (*p != '\r' && *p != '\n' && *p != '\0') {
        if (length + 1 >= capacity) {
            capacity *= 2;

            char *temp = realloc(string, capacity);
            if (temp == NULL) {
                free(string);
                printf("parseSimpleString: realloc failed\n");
                return NULL;
            }
            string = temp;
        }

        string[length] = (char)*p;
        length++;
        p++;
    }

    if (*p != '\r') {
        printf("parseSimpleString: expected CRLF ending in string\n");
        exit(1);
    }
    p++;
    if (*p != '\n') {
        printf("parseSimpleString: expected CRLF ending in string\n");
        exit(1);
    }

    string[length] = '\0';
    return string;
}

char* parseBulkStrings(char* ch) {
    if (*ch != '$') {
        printf("parseBulkStrings: expected '$' starting char\n");
        exit(1);
    }

    char* p = ch + 1;
    if (*p == '-') {
        //TODO create logic for null bulk string
    }
    int length = 0;
    // Find length of BulkString
    while (isdigit(*p)) {
        length = length * 10 + *p++ - '0';
    }

    // Parse CRLF after length
    if (*p != '\r') {
        printf("parseBulkStrings: parsed length, but expected \\r after\n");
        exit(1);
    }
    p++;
    if (*p != '\n') {
        printf("parseBulkStrings: parsed length and \\r, but expected \\n after\n");
        exit(1);
    }
    p++;

    // Parse data
    char *string = NULL;

    string = (char*)malloc(length * sizeof(char));
    if (string == NULL) {
        printf("parseBulkStrings: malloc failed\n");
        exit(1);
    }

    int currentLength = 0;
    while (*p != '\r' && *p != '\n' && *p != '\0') {
        if (currentLength >= length) {
            printf("parseBulkStrings: error, parsing longer string than anticipated\n");
            exit(1);
        }
        string[currentLength] = (char)*p;
        currentLength++;
        p++;
    }
    if (currentLength != length) {
        printf("parseBulkStrings: error, parsing ended before expected length of string\n");
        exit(1);
    }

    // Parse ending CRLF

    if (*p != '\r') {
        printf("parseBulkStrings: expected CRLF ending in string\n");
        exit(1);
    }
    p++;
    if (*p != '\n') {
        printf("parseBulkStrings: expected CRLF ending in string\n");
        exit(1);
    }

    string[length] = '\0';
    return string;

}

char* concatenate(char* string1, char* string2) {
    char* result = (char*)malloc(strlen(string1) + strlen(string2) + 2);
    if (result == NULL) {
        printf("concatenate: malloc failed\n");
        exit(1);
    }
    strcpy(result, string1);
    strcat(result, " ");
    strcat(result, string2);
    result[strlen(result)] = '\0';
    return result;
}


char* parseError(char* ch) {
    if (*ch != '-') {
        printf("parseError: expected '-' starting char\n");
        exit(1);
    }
    char* p = ch + 1;

    // Parse Error type
    int errorLength = 0;
    int capacity = 16;
    char *errorString = NULL;

    errorString = (char*)malloc(capacity * sizeof(char));
    if (errorString == NULL) {
        printf("parseError: malloc failed\n");
        exit(1);
    }

    while (*p != '\r' && *p != '\n' && *p != '\0' && *p != ' ') {
        if (errorLength + 1 >= capacity) {
            capacity *= 2;

            char *temp = realloc(errorString, capacity);
            if (temp == NULL) {
                free(errorString);
                printf("parseError: realloc failed\n");
                return NULL;
            }
            errorString = temp;
        }

        errorString[errorLength] = (char)*p;
        errorLength++;
        p++;
    }
    if (errorLength == 0) {
        printf("parseError: Expected error type of length greater than 0\n");
        exit(1);
    }
    if (*p != ' ') {
        printf("parseError: Expected ' ' space between error type and error message\n");
        exit(1);
    }
    errorString[errorLength] = '\0';

    p++;
    if (*p == ' ') {
        printf("parseError: space between error type and error message too large\n");
        exit(1);
    }

    // Parse error message
    int messageLength = 0;
    int messageCapacity = 16;
    char *messageString = NULL;

    messageString = (char*)malloc(capacity * sizeof(char));
    if (messageString == NULL) {
        printf("parseError: malloc failed\n");
        exit(1);
    }

    while (*p != '\r' && *p != '\n' && *p != '\0') {
        if (messageLength + 1 >= messageCapacity) {
            messageCapacity *= 2;

            char *tmp2 = realloc(messageString, messageCapacity);
            if (tmp2 == NULL) {
                free(messageString);
                printf("parseError: realloc failed\n");
                return NULL;
            }
            messageString = tmp2;
        }

        messageString[messageLength] = (char)*p;
        messageLength++;
        p++;
    }

    if (messageLength == 0) {
        printf("parseError: Expected error message length greater than 0\n");
        exit(1);
    }
    messageString[messageLength] = '\0';

    // Parse ending CRLF
    if (*p != '\r') {
        printf("parseError: expected CRLF ending in string\n");
        exit(1);
    }
    p++;
    if (*p != '\n') {
        printf("parseError: expected CRLF ending in string\n");
        exit(1);
    }

    char* result = concatenate(errorString, messageString);
    return result;
}

int parseInteger(char* ch) {
    if (*ch != ':') {
        printf("parseInteger: expected ':' starting char\n");
        exit(1);
    }
    char* p = ch + 1;
    int isNegative = 0;

    if (*p == '-' || *p == '+') {
        if (*p == '-') {
            isNegative = 1;
        }
        p++;
    }

    int parsedInteger = 0;
    // Parse integer
    int integer = 0;
    // Find length of BulkString
    while (isdigit(*p)) {
        parsedInteger = 1;
        integer = integer * 10 + *p++ - '0';
    }
    if (!parsedInteger) {
        printf("parseInteger: expected integer\n");
        exit(1);
    }
    if (isNegative) {
        integer = -integer;
    }

    // Parse ending CRLF
    if (*p != '\r') {
        printf("parseInteger: expected CRLF ending in string\n");
        exit(1);
    }
    p++;
    if (*p != '\n') {
        printf("parseInteger: expected CRLF ending in string\n");
        exit(1);
    }
    return integer;
}

int main(int argc, char* argv[]) {

    //RunTests()

    int res = parseInteger(":45645\r\n");
    printf("%d\n", res);

    return 0;
}