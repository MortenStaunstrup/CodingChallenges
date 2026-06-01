#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum Type {
    SSTRING,
    ERROR,
    INTEGER,
    BSTRING,
    ARRAY
} Type;

typedef struct TypeResponse {
    int validType;
    Type type;
} TypeResponse;

typedef struct CreateDeserializationTests {
    int amountOfTests;
    char** tests;
    char** expected;
    int* expectFailed;
} CreateParseTestResult;

typedef struct ArrayElement {
    Type type;
    union {
        char* stringResponse;
        int intValue;
        struct ArrayElement* array;
    };
    int arrayElementLength;
} ArrayElement;

typedef struct ArrayResult {
    int length;
    ArrayElement* array;
} ArrayResult;

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
            t.type = ERROR;
            t.validType = 0;
    }

    return t;

}

CreateParseTestResult CreateDeserializationTests() {
    CreateParseTestResult res;
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
    expectedParsedResponses[3] = "get key";

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

void RunDeserializationTests() {

    CreateParseTestResult res = CreateDeserializationTests();

    for (int i = 0; i < res.amountOfTests; i++) {
        printf("Test case: %s\nExpected: %s\n", res.tests[i], res.expected[i]);
        TypeResponse t = ResponseType(res.tests[i]);
        if (!t.validType) {
            printf("Invalid type\n");
            exit(1);
        }
    }

    free(res.tests);
    free(res.expected);
    free(res.expectFailed);
}

char* deserializeSimpleString(char* ch) {
    if (*ch != '+') {
        printf("parseSimpleString: expected '+' starting char\n");
        exit(1);
    }

    ch++;
    int length = 0;
    int capacity = 16;
    char *string = NULL;

    string = (char*)malloc(capacity * sizeof(char));
    if (string == NULL) {
        printf("parseSimpleString: malloc failed\n");
        exit(1);
    }

    while (*ch != '\r' && *ch != '\n' && *ch != '\0') {
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

        string[length] = (char)*ch;
        length++;
        ch++;
    }

    if (*ch != '\r') {
        printf("parseSimpleString: expected CRLF ending in string\n");
        exit(1);
    }
    ch++;
    if (*ch != '\n') {
        printf("parseSimpleString: expected CRLF ending in string\n");
        exit(1);
    }
    ch++;

    string[length] = '\0';
    return string;
}

char* deserializeBulkStrings(char* ch) {
    if (*ch != '$') {
        printf("parseBulkStrings: expected '$' starting char\n");
        exit(1);
    }

    ch++;

    int isNull = 0;
    if (*ch == '-') {
        ch++;
        if (*ch == '1') {
            isNull = 1;
            ch++;
        } else {
            printf("parseBulkStrings: Invalid null string format, missing 1\n");
            exit(1);
        }
    }

    int length = 0;
    // Find length of BulkString
    while (isdigit(*ch)) {
        length = length * 10 + *ch++ - '0';
    }

    char *string = NULL;

    if (!isNull) {
        // Parse CRLF after length
        if (*ch != '\r') {
            printf("parseBulkStrings: parsed length, but expected \\r after\n");
            exit(1);
        }
        ch++;
        if (*ch != '\n') {
            printf("parseBulkStrings: parsed length and \\r, but expected \\n after\n");
            exit(1);
        }
        ch++;

        // Parse data

        string = (char*)malloc(length * sizeof(char));
        if (string == NULL) {
            printf("parseBulkStrings: malloc failed\n");
            exit(1);
        }

        int currentLength = 0;
        while (*ch != '\r' && *ch != '\n' && *ch != '\0') {
            if (currentLength >= length) {
                printf("parseBulkStrings: error, parsing longer string than anticipated\n");
                exit(1);
            }
            string[currentLength] = (char)*p;
            currentLength++;
            ch++;
        }
        if (currentLength != length) {
            printf("parseBulkStrings: error, parsing ended before expected length of string\n");
            exit(1);
        }

        string[length] = '\0';
    }


    // Parse ending CRLF

    if (*ch != '\r') {
        printf("parseBulkStrings: expected CRLF ending in string\n");
        exit(1);
    }
    ch++;
    if (*ch != '\n') {
        printf("parseBulkStrings: expected CRLF ending in string\n");
        exit(1);
    }
    ch++;

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


char* deserializeError(char* ch) {
    if (*ch != '-') {
        printf("parseError: expected '-' starting char\n");
        exit(1);
    }
    ch++;

    // Parse Error type
    int errorLength = 0;
    int capacity = 16;
    char *errorString = NULL;

    errorString = (char*)malloc(capacity * sizeof(char));
    if (errorString == NULL) {
        printf("parseError: malloc failed\n");
        exit(1);
    }

    while (*ch != '\r' && *ch != '\n' && *ch != '\0' && *ch != ' ') {
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

        errorString[errorLength] = (char)*ch;
        errorLength++;
        ch++;
    }
    if (errorLength == 0) {
        printf("parseError: Expected error type of length greater than 0\n");
        exit(1);
    }
    if (*ch != ' ') {
        printf("parseError: Expected ' ' space between error type and error message\n");
        exit(1);
    }
    errorString[errorLength] = '\0';

    ch++;
    if (*ch == ' ') {
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

    while (*ch != '\r' && *ch != '\n' && *ch != '\0') {
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
        ch++;
    }

    if (messageLength == 0) {
        printf("parseError: Expected error message length greater than 0\n");
        exit(1);
    }
    messageString[messageLength] = '\0';

    // Parse ending CRLF
    if (*ch != '\r') {
        printf("parseError: expected CRLF ending in string\n");
        exit(1);
    }
    ch++;
    if (*ch != '\n') {
        printf("parseError: expected CRLF ending in string\n");
        exit(1);
    }
    ch++;
    char* result = concatenate(errorString, messageString);
    return result;
}

int deserializeInteger(char* ch) {
    if (*ch != ':') {
        printf("parseInteger: expected ':' starting char\n");
        exit(1);
    }
    ch++;
    int isNegative = 0;

    if (*ch == '-' || *ch == '+') {
        if (*ch == '-') {
            isNegative = 1;
        }
        ch++;
    }

    int parsedInteger = 0;
    // Parse integer
    int integer = 0;
    // Find length of BulkString
    while (isdigit(*ch)) {
        parsedInteger = 1;
        integer = integer * 10 + *ch++ - '0';
    }
    if (!parsedInteger) {
        printf("parseInteger: expected integer\n");
        exit(1);
    }
    if (isNegative) {
        integer = -integer;
    }

    // Parse ending CRLF
    if (*ch != '\r') {
        printf("parseInteger: expected CRLF ending in string\n");
        exit(1);
    }
    ch++;
    if (*ch != '\n') {
        printf("parseInteger: expected CRLF ending in string\n");
        exit(1);
    }
    ch++;
    return integer;
}

ArrayResult deserializeArray(char* ch) {
    if (*ch != '*') {
        printf("parseArray: expected '*' starting char\n");
        exit(1);
    }
    char* p = ch + 1;

    int digitFound = 0;
    int arrayLength = 0;
    while (isdigit(*p)) {
        digitFound = 1;
        arrayLength = arrayLength * 10 + *p++ - '0';
    }
    if (!digitFound) {
        printf("parseArray: expected int telling array size\n");
        exit(1);
    }

    // Parse array length denominator and first element CRLF
    if (*p != '\r') {
        printf("parseInteger: expected CRLF ending in string\n");
        exit(1);
    }
    p++;
    if (*p != '\n') {
        printf("parseInteger: expected CRLF ending in string\n");
        exit(1);
    }
    p++;

    ArrayElement* array = (ArrayElement*)malloc(sizeof(ArrayElement) * arrayLength);

    // Parse datatypes
    for (int i = 0; i < arrayLength; i++) {
        if (*ch == '\0') {
            printf("parseArray: unexpected end of array\n");
        }
        TypeResponse type = ResponseType(p);
        if (!type.validType) {
            printf("parseArray: array contains non valid type\n");
            exit(1);
        }

        switch (type.type) {
            case SSTRING:
                char* sstring = deserializeSimpleString(p);
                array[i].stringResponse = sstring;
                array[i].type = SSTRING;
                break;
            case BSTRING:
                char* bstring = deserializeBulkStrings(p);
                array[i].stringResponse = bstring;
                array[i].type = BSTRING;
                break;
            case ERROR:
                char* errorString = deserializeError(p);
                array[i].stringResponse = errorString;
                array[i].type = ERROR;
                break;
            case INTEGER:
                int integer = deserializeInteger(p);
                array[i].intValue = integer;
                array[i].type = INTEGER;
                break;
            case ARRAY:
                ArrayResult resArray = deserializeArray(p);
                array[i].array = resArray.array;
                array[i].type = SSTRING;
                array[i].arrayElementLength = resArray.length;
                break;
            default:
                printf("parseArray: unexpected type\n");
                exit(1);
        }
    }

    if (arrayLength == 0) {
        // Parse ending CRLF
        if (*p != '\r') {
            printf("parseArray: expected CRLF ending in string, when array empty\n");
            exit(1);
        }
        p++;
        if (*p != '\n') {
            printf("parseArray: expected CRLF ending in string, when array empty\n");
            exit(1);
        }
    }

    ArrayResult result;
    result.array = array;
    result.length = arrayLength;
    return result;

}


int main(int argc, char* argv[]) {

    //RunTests()


    ArrayElement* array = deserializeArray("*2\r\n+Hello\r\n$6 World\r\n");

    return 0;
}