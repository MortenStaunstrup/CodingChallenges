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

void RunDeserializationTests() {

    CreateParseTestResult res = CreateDeserializationTests();

    for (int i = 0; i < res.amountOfTests; i++) {
        printf("Test case: %s\nExpected: %s\n", res.tests[i], res.expected[i]);
        TypeResponse t = ResponseType(res.tests[i]);
        if (!t.validType) {
            printf("Invalid type\n");
        }
    }

    free(res.tests);
    free(res.expected);
    free(res.expectFailed);
}

char* deserializeSimpleString(char** ch) {
    if (**ch != '+') {
        printf("parseSimpleString: expected '+' starting char\n");
        exit(1);
    }

    (*ch)++;
    int length = 0;
    int capacity = 16;
    char *string = NULL;

    string = (char*)malloc(capacity * sizeof(char));
    if (string == NULL) {
        printf("parseSimpleString: malloc failed\n");
        exit(1);
    }

    while (**ch != '\r' && **ch != '\n' && **ch != '\0') {
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

        string[length] = (char)**ch;
        length++;
        (*ch)++;
    }

    if (**ch != '\r') {
        printf("parseSimpleString: expected CRLF ending in string\n");
        exit(1);
    }
    (*ch)++;
    if (**ch != '\n') {
        printf("parseSimpleString: expected CRLF ending in string\n");
        exit(1);
    }
    (*ch)++;

    string[length] = '\0';
    return string;
}

char* deserializeBulkStrings(char** ch) {
    if (**ch != '$') {
        printf("parseBulkStrings: expected '$' starting char\n");
        exit(1);
    }

    (*ch)++;

    int isNull = 0;
    if (**ch == '-') {
        (*ch)++;
        if (**ch == '1') {
            isNull = 1;
            (*ch)++;
        } else {
            printf("parseBulkStrings: Invalid null string format, missing 1\n");
            exit(1);
        }
    }

    int length = 0;
    // Find length of BulkString
    while (isdigit(**ch)) {
        length = length * 10 + (**ch - '0');
        (*ch)++;
    }

    char *string = NULL;

    if (!isNull) {
        // Parse CRLF after length
        if (**ch != '\r') {
            printf("parseBulkStrings: parsed length, but expected \\r after\n");
            exit(1);
        }
        (*ch)++;
        if (**ch != '\n') {
            printf("parseBulkStrings: parsed length and \\r, but expected \\n after\n");
            exit(1);
        }
        (*ch)++;

        // Parse data

        string = (char*)malloc(length * sizeof(char));
        if (string == NULL) {
            printf("parseBulkStrings: malloc failed\n");
            exit(1);
        }

        int currentLength = 0;
        while (**ch != '\r' && **ch != '\n' && **ch != '\0') {
            if (currentLength >= length) {
                printf("parseBulkStrings: error, parsing longer string than anticipated\n");
                exit(1);
            }
            string[currentLength] = (char)**ch;
            currentLength++;
            (*ch)++;
        }
        if (currentLength != length) {
            printf("parseBulkStrings: error, parsing ended before expected length of string\n");
            exit(1);
        }

        string[length] = '\0';
    }


    // Parse ending CRLF

    if (**ch != '\r') {
        printf("parseBulkStrings: expected CRLF ending in string\n");
        exit(1);
    }
    (*ch)++;
    if (**ch != '\n') {
        printf("parseBulkStrings: expected CRLF ending in string\n");
        exit(1);
    }
    (*ch)++;

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


char* deserializeError(char** ch) {
    if (**ch != '-') {
        printf("parseError: expected '-' starting char\n");
        exit(1);
    }
    (*ch)++;

    // Parse Error type
    int errorLength = 0;
    int capacity = 16;
    char *errorString = NULL;

    errorString = (char*)malloc(capacity * sizeof(char));
    if (errorString == NULL) {
        printf("parseError: malloc failed\n");
        exit(1);
    }

    while (**ch != '\r' && **ch != '\n' && **ch != '\0' && **ch != ' ') {
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

        errorString[errorLength] = (char)**ch;
        errorLength++;
        (*ch)++;
    }
    if (errorLength == 0) {
        printf("parseError: Expected error type of length greater than 0\n");
        exit(1);
    }
    if (**ch != ' ') {
        printf("parseError: Expected ' ' space between error type and error message\n");
        exit(1);
    }
    errorString[errorLength] = '\0';

    (*ch)++;
    if (**ch == ' ') {
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

    while (**ch != '\r' && **ch != '\n' && **ch != '\0') {
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

        messageString[messageLength] = (char)**ch;
        messageLength++;
        (*ch)++;
    }

    if (messageLength == 0) {
        printf("parseError: Expected error message length greater than 0\n");
        exit(1);
    }
    messageString[messageLength] = '\0';

    // Parse ending CRLF
    if (**ch != '\r') {
        printf("parseError: expected CRLF ending in string\n");
        exit(1);
    }
    (*ch)++;
    if (**ch != '\n') {
        printf("parseError: expected CRLF ending in string\n");
        exit(1);
    }
    (*ch)++;
    char* result = concatenate(errorString, messageString);
    return result;
}

int deserializeInteger(char** ch) {
    if (**ch != ':') {
        printf("parseInteger: expected ':' starting char\n");
        exit(1);
    }
    (*ch)++;
    int isNegative = 0;

    if (**ch == '-' || **ch == '+') {
        if (**ch == '-') {
            isNegative = 1;
        }
        ch++;
    }

    int parsedInteger = 0;
    // Parse integer
    int integer = 0;
    // Find length of BulkString
    while (isdigit(**ch)) {
        parsedInteger = 1;
        integer = integer * 10 + (**ch - '0');
        (*ch)++;
    }
    if (!parsedInteger) {
        printf("parseInteger: expected integer\n");
        exit(1);
    }
    if (isNegative) {
        integer = -integer;
    }

    // Parse ending CRLF
    if (**ch != '\r') {
        printf("parseInteger: expected CRLF ending in string\n");
        exit(1);
    }
    (*ch)++;
    if (**ch != '\n') {
        printf("parseInteger: expected CRLF ending in string\n");
        exit(1);
    }
    (*ch)++;
    return integer;
}

ArrayResult deserializeArrayElements(char** ch) {
    if (**ch != '*') {
        printf("deserializeArrayElements: expected '*' starting char\n");
        exit(1);
    }
    (*ch)++;

    int isNull = 0;
    if (**ch == '-') {
        (*ch)++;
        if (**ch == '1') {
            isNull = 1;
            (*ch)++;
        } else {
            printf("deserializeArrayElements: Invalid null string format, missing 1\n");
            exit(1);
        }
    }

    int digitFound = 0;
    int arrayLength = 0;
    ArrayElement* array;

    if (!isNull) {
        while (isdigit(**ch)) {
            digitFound = 1;
            arrayLength = arrayLength * 10 + (**ch - '0');
            (*ch)++;
        }
        if (!digitFound) {
            printf("deserializeArrayElements: expected int telling array size\n");
            exit(1);
        }

        // Parse array length denominator and first element CRLF
        if (**ch != '\r') {
            printf("deserializeArrayElements: expected CRLF ending in string\n");
            exit(1);
        }
        (*ch)++;
        if (**ch != '\n') {
            printf("deserializeArrayElements: expected CRLF ending in string\n");
            exit(1);
        }
        (*ch)++;

        array = (ArrayElement*)malloc(sizeof(ArrayElement) * arrayLength);

        // Parse datatypes
        for (int i = 0; i < arrayLength; i++) {
            if (**ch == '\0') {
                printf("deserializeArrayElements: unexpected end of array\n");
                exit(1);
            }
            TypeResponse type = ResponseType(*ch);
            if (!type.validType) {
                printf("deserializeArrayElements: array contains non valid type\n");
                exit(1);
            }

            switch (type.type) {
                case SSTRING:
                    char* sstring = deserializeSimpleString(ch);
                    array[i].stringResponse = sstring;
                    array[i].type = SSTRING;
                    break;
                case BSTRING:
                    char* bstring = deserializeBulkStrings(ch);
                    array[i].stringResponse = bstring;
                    array[i].type = BSTRING;
                    break;
                case ERROR:
                    char* errorString = deserializeError(ch);
                    array[i].stringResponse = errorString;
                    array[i].type = ERROR;
                    break;
                case INTEGER:
                    int integer = deserializeInteger(ch);
                    array[i].intValue = integer;
                    array[i].type = INTEGER;
                    break;
                case ARRAY:
                    ArrayResult resArray = deserializeArrayElements(ch);
                    array[i].array = resArray.array;
                    array[i].type = ARRAY;
                    array[i].arrayElementLength = resArray.length;
                    break;
                default:
                    printf("deserializeArrayElements: unexpected type\n");
                    exit(1);
            }
        }
    }


    ArrayResult result;
    result.array = array;
    result.length = arrayLength;
    return result;

}

void printDeserializedArrayResult(ArrayElement* array, int length) {
    printf("[");
    for (int i = 0; i < length; i++) {
        switch (array[i].type) {
            case SSTRING:
                printf("%s", array[i].stringResponse);
                break;
            case BSTRING:
                printf("%s", array[i].stringResponse);
                break;
            case ERROR:
                printf("%s", array[i].stringResponse);
                break;
            case INTEGER:
                printf("%d", array[i].intValue);
                break;
            case ARRAY:
                printDeserializedArrayResult(array[i].array, array[i].arrayElementLength);
                break;
            default:
                printf("printDeserializedArray: unexpected type\n");
                exit(1);
        }
        if (i < length - 1)
            printf(", ");
    }
    printf("]");
}

char* arrayConcatenate(char* string1, char* string2, int isString) {
    char* st2;
    char* result;
    int isNull = 0;
    if (string2 == NULL) {
        st2 = "nil";
        isNull = 1;
        result = malloc(strlen(string1) + strlen(st2) + 3);
    } else {
        if (isString) {
            result = malloc(strlen(string1) + strlen(string2) + 5);
        } else {
            result = malloc(strlen(string1) + strlen(string2) + 3);
        }
        st2 = string2;
    }
    if (result == NULL) {
        printf("concatenateArray: malloc failed\n");
        exit(1);
    }
    if (isString && !isNull) {
        strcpy(result, string1);
        strcat(result, "\"");
        strcat(result, st2);
        strcat(result, "\"");
    } else {
        strcpy(result, string1);
        strcat(result, st2);
    }
    strcat(result, ", ");
    result[strlen(result)] = '\0';
    return result;
}

char* arrayConcatenateStart(char* string1, char* string2, int isString) {
    char* st2;
    char* result;
    int isNull = 0;
    if (string2 == NULL) {
        st2 = "nil";
        isNull = 1;
        result = malloc(1 + strlen(st2) + 4);
    } else {
        if (isString) {
            result = malloc(1 + strlen(string2) + 6);
        } else {
            result = malloc(1 + strlen(string2) + 4);
        }
        st2 = string2;
    }
    if (result == NULL) {
        printf("concatenateArray: malloc failed\n");
        exit(1);
    }
    strcpy(result, "[");
    if (isString && !isNull) {
        strcat(result, "\"");
        strcat(result, st2);
        strcat(result, "\"");
    } else {
        strcat(result, st2);
    }
    strcat(result, ", ");
    result[strlen(result)] = '\0';
    return result;
}

char* arrayConcatenateEnd(char* string1, char* string2, int isString) {
    char* st2;
    char* result;
    int isNull = 0;
    if (string2 == NULL) {
        st2 = "nil";
        isNull = 1;
        result = malloc(strlen(string1) + strlen(st2) + 2);
    } else {
        if (isString) {
            result = malloc(strlen(string1) + strlen(string2) + 4);
        } else {
            result = malloc(strlen(string1) + strlen(string2) + 2);
        }
        st2 = string2;
    }
    if (result == NULL) {
        printf("concatenateArray: malloc failed\n");
        exit(1);
    }
    if (isString && !isNull) {
        strcpy(result, string1);
        strcat(result, "\"");
        strcat(result, st2);
        strcat(result, "\"");
    } else {
        strcpy(result, string1);
        strcat(result, st2);
    }
    strcat(result, "]");
    result[strlen(result)] = '\0';
    return result;
}

char* arrayConcatenateOneElement(char* string1, char* string2, int isString) {
    char* st2;
    char* result;
    int isNull = 0;
    if (string2 == NULL) {
        st2 = "nil";
        isNull = 1;
        result = malloc(strlen(string1) + strlen(st2) + 3);
    } else {
        if (isString) {
            result = malloc(strlen(string1) + strlen(string2) + 5);
        } else {
            result = malloc(strlen(string1) + strlen(string2) + 3);
        }
        st2 = string2;
    }
    if (result == NULL) {
        printf("concatenateArrayOneElement: malloc failed\n");
        exit(1);
    }

    strcpy(result, "[");
    if (isString && !isNull) {
        strcat(result, "\"");
        strcat(result, st2);
        strcat(result, "\"");
    } else {
        strcat(result, st2);
    }
    strcat(result, "]");
    result[strlen(result)] = '\0';
    return result;
}

char* deserializeEmbeddedArray(ArrayElement* array, int length);

char* arrayResponseConcatenator(char* result, ArrayElement* array, int index, int isStart, int isEnd, int hasOneElement) {
    if (isStart && isEnd) {
        printf("ArrayResponseConcatenator called with both arrayStart and arrayEnd flags");
        exit(1);
    }
    if (isEnd && hasOneElement) {
        printf("ArrayResponseConcatenator called with both arrayEnd and hasOneElement flag");
        exit(1);
    }
    if (isStart && hasOneElement) {
        printf("ArrayResponseConcatenator called with both arrayStart and hasOneElement flag");
        exit(1);
    }

    if (index < 0) {
        printf("Index cannot be less than 0\n");
        exit(1);
    }

    if (isStart) {
        switch (array[0].type) {
            case SSTRING:
                char* concat = arrayConcatenateStart(result, array[0].stringResponse, 1);
                result = concat;
                break;
            case BSTRING:
                char* concatB = arrayConcatenateStart(result, array[0].stringResponse, 1);
                result = concatB;
                break;
            case ERROR:
                char* concatE = arrayConcatenateStart(result, array[0].stringResponse, 0);
                result = concatE;
                break;
            case INTEGER:
                char intBuffer[20];
                sprintf(intBuffer, "%d", array[0].intValue);
                char* concatI = arrayConcatenateStart(result, intBuffer, 0);
                result = concatI;
                break;
            case ARRAY:
                char* embeddedArray = deserializeEmbeddedArray(array[0].array, array[0].arrayElementLength);
                char* concatA = arrayConcatenateStart(result, embeddedArray, 0);
                result = concatA;
                break;
            default:
                printf("arrayResponseConcatenator: unexpected type\n");
                exit(1);
        }
    } else if (hasOneElement) {
        switch (array[0].type) {
            case SSTRING:
                char* concat = arrayConcatenateOneElement(result, array[0].stringResponse, 1);
                result = concat;
                break;
            case BSTRING:
                char* concatB = arrayConcatenateOneElement(result, array[0].stringResponse, 1);
                result = concatB;
                break;
            case ERROR:
                char* concatE = arrayConcatenateOneElement(result, array[0].stringResponse, 0);
                result = concatE;
                break;
            case INTEGER:
                char intBuffer[20];
                sprintf(intBuffer, "%d", array[0].intValue);
                char* concatI = arrayConcatenateOneElement(result, intBuffer, 0);
                result = concatI;
                break;
            case ARRAY:
                char* embeddedArray = deserializeEmbeddedArray(array[0].array, array[0].arrayElementLength);
                char* concatA = arrayConcatenateOneElement(result, embeddedArray, 0);
                result = concatA;
                break;
            default:
                printf("arrayResponseConcatenator: unexpected type\n");
                exit(1);
        }
    } else if (isEnd) {
        switch (array[index].type) {
            case SSTRING:
                char* concat = arrayConcatenateEnd(result, array[index].stringResponse, 1);
                result = concat;
                break;
            case BSTRING:
                char* concatB = arrayConcatenateEnd(result, array[index].stringResponse, 1);
                result = concatB;
                break;
            case ERROR:
                char* concatE = arrayConcatenateEnd(result, array[index].stringResponse, 0);
                result = concatE;
                break;
            case INTEGER:
                char intBuffer[20];
                sprintf(intBuffer, "%d", array[1].intValue);
                char* concatI = arrayConcatenateEnd(result, intBuffer, 0);
                result = concatI;
                break;
            case ARRAY:
                char* embeddedArray = deserializeEmbeddedArray(array[index].array, array[index].arrayElementLength);
                char* concatA = arrayConcatenateEnd(result, embeddedArray, 0);
                result = concatA;
                break;
            default:
                printf("deserializeEmbeddedArray: unexpected type\n");
                exit(1);
        }
    }

    if (!isEnd && !isStart && !hasOneElement) {
        switch (array[index].type) {
            case SSTRING:
                char* concat = arrayConcatenate(result, array[index].stringResponse, 1);
                result = concat;
                break;
            case BSTRING:
                char* concatB = arrayConcatenate(result, array[index].stringResponse, 1);
                result = concatB;
                break;
            case ERROR:
                char* concatE = arrayConcatenate(result, array[index].stringResponse, 0);
                result = concatE;
                break;
            case INTEGER:
                char intBuffer[20];
                sprintf(intBuffer, "%d", array[1].intValue);
                char* concatI = arrayConcatenate(result, intBuffer, 0);
                result = concatI;
                break;
            case ARRAY:
                char* embeddedArray = deserializeEmbeddedArray(array[index].array, array[index].arrayElementLength);
                char* concatA = arrayConcatenate(result, embeddedArray, 0);
                result = concatA;
                break;
            default:
                printf("deserializeEmbeddedArray: unexpected type\n");
                exit(1);
        }
    }

    return result;
}

char* deserializeEmbeddedArray(ArrayElement* array, int length) {
    if (length == 0) {
        char* emptyArray = malloc(2*sizeof(char) + 1);
        strcpy(emptyArray, "[]");
        emptyArray[2] = '\0';
        return emptyArray;
    }
    int initSize = 50;

    char* result = malloc(initSize*sizeof(char));

    // First array element concat

    if (length == 1) {
        result = arrayResponseConcatenator(result, array, 0, 0, 0, 1);
    } else {
        result = arrayResponseConcatenator(result, array, 0, 1, 0, 0);
    }


    for (int i = 1; i < length; i++) {
        if (i == length - 1) {
            result = arrayResponseConcatenator(result, array, i, 0, 1, 0);
        } else {
            result = arrayResponseConcatenator(result, array, i, 0, 0, 0);
        }
    }

    return result;
}

// TODO Need logic for reallocation if init size in deserialize array functions gets exceeded.

char* deserializeArray(char** ch) {
    if (**ch != '*') {
        printf("deserializeArray: expected '*' to start array sequence\n");
        exit(1);
    }
    int initSize = 50;

    ArrayResult arrayRes = deserializeArrayElements(ch);
    int arrayLength = arrayRes.length;
    ArrayElement* array = arrayRes.array;

    if (arrayLength == 0) {
        char* emptyArray = malloc(2*sizeof(char) + 1);
        strcpy(emptyArray, "[]");
        emptyArray[2] = '\0';
        return emptyArray;
    }

    char* result = malloc(initSize*sizeof(char));

    // First array element concat
    if (arrayLength == 1) {
        result = arrayResponseConcatenator(result, array, 0, 0, 0, 1);
    } else {
        result = arrayResponseConcatenator(result, array, 0, 1, 0, 0);
    }


    for (int i = 1; i < arrayLength; i++) {
        if (i == arrayLength - 1) {
            result = arrayResponseConcatenator(result, array, i, 0, 1, 0);
        } else {
            result = arrayResponseConcatenator(result, array, i, 0, 0, 0);
        }
    }

    return result;

}

int main(int argc, char* argv[]) {

    //RunTests()

    //char* request = "*0\r\n$5\r\nhello\r\n$-1\r\n$5\r\nworld\r\n";

    char* request = "*1\r\n*1\r\n*1\r\n";
    char* arrayRes = deserializeArray(&request);

    printf("%s\n", arrayRes);

    free(arrayRes);

    return 0;
}