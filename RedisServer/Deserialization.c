//
// Created by mort4 on 10-06-2026.
//

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Deserialization.h"

DeserializationResult deserializeSimpleString(char** ch) {
    DeserializationResult result = {0};
    if (**ch != '+') {
        printf("deserializeSimpleString: expected '+' starting char\n");
        exit(1);
    }

    (*ch)++;
    int length = 0;
    int capacity = 16;
    char *string = NULL;

    string = (char*)malloc(capacity * sizeof(char));
    if (string == NULL) {
        printf("deserializeSimpleString: malloc failed\n");
        exit(1);
    }

    while (**ch != '\r' && **ch != '\n' && **ch != '\0') {
        if (length + 1 >= capacity) {
            capacity *= 2;

            char *temp = realloc(string, capacity);
            if (temp == NULL) {
                free(string);
                printf("deserializeSimpleString: realloc failed\n");
                exit(1);
            }
            string = temp;
        }

        string[length] = (char)**ch;
        length++;
        (*ch)++;
    }

    if (**ch != '\r') {
        char* errorString = "deserializeSimpleString: expected CRLF ending in string";
        printf("%s\n", errorString);
        result.errorMessage = errorString;
        result.result = FAILED;
        return result;
    }
    (*ch)++;
    if (**ch != '\n') {
        char* errorString = "deserializeSimpleString: expected CRLF ending in string";
        printf("%s\n", errorString);
        result.errorMessage = errorString;
        result.result = FAILED;
        return result;
    }
    (*ch)++;

    string[length] = '\0';
    result.result = SUCCESS;
    result.content = string;
    return result;
}

DeserializationResult deserializeBulkStrings(char** ch) {
    DeserializationResult result = {0};
    if (**ch != '$') {
        printf("deserializeBulkStrings: expected '$' starting char\n");
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
            char* errorString = "deserializeBulkStrings: Invalid null string format, missing 1";
            printf("%s\n", errorString);
            result.errorMessage = errorString;
            result.result = FAILED;
            return result;
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
            char* errorString = "deserializeBulkStrings: parsed length, but expected \\r after";
            printf("%s\n", errorString);
            result.errorMessage = errorString;
            result.result = FAILED;
            return result;
        }
        (*ch)++;
        if (**ch != '\n') {
            char* errorString = "deserializeBulkStrings: parsed length, but expected \\r after";
            printf("%s\n", errorString);
            result.errorMessage = errorString;
            result.result = FAILED;
            return result;
        }
        (*ch)++;

        // Parse data

        string = (char*)malloc(length * sizeof(char));
        if (string == NULL) {
            printf("deserializeBulkStrings: malloc failed\n");
            exit(1);
        }

        int currentLength = 0;
        while (**ch != '\0' && currentLength < length) {
            string[currentLength] = (char)**ch;
            currentLength++;
            (*ch)++;
        }
        if (currentLength != length) {
            char* errorString = "deserializeBulkStrings: error, parsing ended before expected length of string";
            printf("%s\n", errorString);
            result.errorMessage = errorString;
            result.result = FAILED;
            return result;
        }
        string[length] = '\0';
    }


    // Parse ending CRLF

    if (**ch != '\r') {
        char* errorString = "deserializeBulkStrings: expected CRLF ending in string";
        printf("%s\n", errorString);
        result.errorMessage = errorString;
        result.result = FAILED;
        return result;
    }
    (*ch)++;
    if (**ch != '\n') {
        char* errorString = "deserializeBulkStrings: expected CRLF ending in string";
        printf("%s\n", errorString);
        result.errorMessage = errorString;
        result.result = FAILED;
        return result;
    }
    (*ch)++;

    result.result = SUCCESS;
    result.content = string;
    return result;
}

char* concatenateWithSpace(char* string1, char* string2) {
    char* result = (char*)malloc(strlen(string1) + strlen(string2) + 2);
    if (result == NULL) {
        printf("concatenateWithSpace: malloc failed\n");
        exit(1);
    }
    strcpy(result, string1);
    strcat(result, " ");
    strcat(result, string2);
    result[strlen(result)] = '\0';
    return result;
}


DeserializationResult deserializeError(char** ch) {
    DeserializationResult result = {0};
    if (**ch != '-') {
        printf("deserializeError: expected '-' starting char\n");
        exit(1);
    }
    (*ch)++;

    // Parse Error type
    int errorLength = 0;
    int capacity = 16;
    char *errorString = NULL;

    errorString = (char*)malloc(capacity * sizeof(char));
    if (errorString == NULL) {
        printf("deserializeError: malloc failed\n");
        exit(1);
    }

    while (**ch != '\r' && **ch != '\n' && **ch != '\0' && **ch != ' ') {
        if (errorLength + 1 >= capacity) {
            capacity *= 2;

            char *temp = realloc(errorString, capacity);
            if (temp == NULL) {
                free(errorString);
                printf("deserializeError: realloc failed\n");
                exit(1);
            }
            errorString = temp;
        }

        errorString[errorLength] = (char)**ch;
        errorLength++;
        (*ch)++;
    }
    if (errorLength == 0) {
        char* errorStringMessage = "deserializeError: Expected error type of length greater than 0";
        printf("%s\n", errorStringMessage);
        result.errorMessage = errorStringMessage;
        result.result = FAILED;
        return result;
    }
    if (**ch != ' ') {
        char* errorStringMessage = "deserializeError: Expected ' ' space between error type and error message";
        printf("%s\n", errorStringMessage);
        result.errorMessage = errorStringMessage;
        result.result = FAILED;
        return result;
    }
    errorString[errorLength] = '\0';

    (*ch)++;
    if (**ch == ' ') {
        char* errorStringMessage = "deserializeError: space between error type and error message too large";
        printf("%s\n", errorStringMessage);
        result.errorMessage = errorStringMessage;
        result.result = FAILED;
        return result;
    }

    // Parse error message
    int messageLength = 0;
    int messageCapacity = 16;
    char *messageString = NULL;

    messageString = (char*)malloc(capacity * sizeof(char));
    if (messageString == NULL) {
        printf("deserializeError: malloc failed\n");
        exit(1);
    }

    while (**ch != '\r' && **ch != '\n' && **ch != '\0') {
        if (messageLength + 1 >= messageCapacity) {
            messageCapacity *= 2;

            char *tmp2 = realloc(messageString, messageCapacity);
            if (tmp2 == NULL) {
                free(messageString);
                printf("deserializeError: realloc failed\n");
                exit(1);
            }
            messageString = tmp2;
        }

        messageString[messageLength] = (char)**ch;
        messageLength++;
        (*ch)++;
    }

    if (messageLength == 0) {
        char* errorStringMessage = "deserializeError: Expected error message length greater than 0";
        printf("%s\n", errorStringMessage);
        result.errorMessage = errorStringMessage;
        result.result = FAILED;
        return result;
    }
    messageString[messageLength] = '\0';

    // Parse ending CRLF
    if (**ch != '\r') {
        char* errorStringMessage = "deserializeError: expected CRLF ending in string";
        printf("%s\n", errorStringMessage);
        result.errorMessage = errorStringMessage;
        result.result = FAILED;
        return result;
    }
    (*ch)++;
    if (**ch != '\n') {
        char* errorStringMessage = "deserializeError: expected CRLF ending in string";
        printf("%s\n", errorStringMessage);
        result.errorMessage = errorStringMessage;
        result.result = FAILED;
        return result;
    }
    (*ch)++;

    result.result = SUCCESS;
    result.content = concatenateWithSpace(errorString, messageString);
    return result;
}

DeserializationResult deserializeInteger(char** ch) {
    DeserializationResult result = {0};
    if (**ch != ':') {
        printf("deserializeInteger: expected ':' starting char\n");
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
        char* errorStringMessage = "deserializeInteger: expected integer";
        printf("%s\n", errorStringMessage);
        result.errorMessage = errorStringMessage;
        result.result = FAILED;
        return result;
    }
    if (isNegative) {
        integer = -integer;
    }

    // Parse ending CRLF
    if (**ch != '\r') {
        char* errorStringMessage = "deserializeInteger: expected CRLF ending in string";
        printf("%s\n", errorStringMessage);
        result.errorMessage = errorStringMessage;
        result.result = FAILED;
        return result;
    }
    (*ch)++;
    if (**ch != '\n') {
        char* errorStringMessage = "deserializeInteger: expected CRLF ending in string";
        printf("%s\n", errorStringMessage);
        result.errorMessage = errorStringMessage;
        result.result = FAILED;
        return result;
    }
    (*ch)++;

    result.result = SUCCESS;
    result.isInteger = 1;
    result.intValue = integer;
    return result;
}

DeserializationResult deserializeArrayElements(char** ch) {
    DeserializationResult deserialization_result = {0};
    if (**ch != '*') {
        char* errorString = "deserializeArrayElements: expected '*' starting char";
        printf("%s\n", errorString);
        deserialization_result.result = FAILED;
        deserialization_result.errorMessage = errorString;
        return deserialization_result;
    }
    (*ch)++;

    int isNull = 0;
    if (**ch == '-') {
        (*ch)++;
        if (**ch == '1') {
            isNull = 1;
            (*ch)++;
        } else {
            char* errorString = "deserializeArrayElements: Invalid null string format, missing 1";
            printf("%s\n", errorString);
            deserialization_result.result = FAILED;
            deserialization_result.errorMessage = errorString;
            return deserialization_result;
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
            char* errorString = "deserializeArrayElements: expected int telling array size";
            printf("%s\n", errorString);
            deserialization_result.result = FAILED;
            deserialization_result.errorMessage = errorString;
            return deserialization_result;
        }

        // Parse array length denominator and first element CRLF
        if (**ch != '\r') {
            char* errorString = "deserializeArrayElements: expected CRLF ending in string";
            printf("%s\n", errorString);
            deserialization_result.result = FAILED;
            deserialization_result.errorMessage = errorString;
            return deserialization_result;
        }
        (*ch)++;
        if (**ch != '\n') {
            char* errorString = "deserializeArrayElements: expected CRLF ending in string";
            printf("%s\n", errorString);
            deserialization_result.result = FAILED;
            deserialization_result.errorMessage = errorString;
            return deserialization_result;
        }
        (*ch)++;

        array = (ArrayElement*)malloc(sizeof(ArrayElement) * arrayLength);

        // Parse datatypes
        for (int i = 0; i < arrayLength; i++) {
            if (**ch == '\0') {
                char* errorString = "deserializeArrayElements: unexpected end of array";
                printf("%s\n", errorString);
                deserialization_result.result = FAILED;
                deserialization_result.errorMessage = errorString;
                return deserialization_result;
            }
            TypeResponse type = ResponseType(*ch);
            if (!type.validType) {
                char* errorString = "deserializeArrayElements: array contains non valid type";
                printf("%s\n", errorString);
                deserialization_result.result = FAILED;
                deserialization_result.errorMessage = errorString;
                return deserialization_result;
            }

            switch (type.type) {
                case TYPE_SSTRING:
                    DeserializationResult simpleResult = deserializeSimpleString(ch);
                    if (simpleResult.result == FAILED) {
                        return simpleResult;
                    }
                    array[i].stringResponse = simpleResult.content;
                    array[i].type = TYPE_SSTRING;
                    break;
                case TYPE_BSTRING:
                    DeserializationResult bulkResult = deserializeBulkStrings(ch);
                    if (bulkResult.result == FAILED) {
                        return bulkResult;
                    }
                    array[i].stringResponse = bulkResult.content;
                    array[i].type = TYPE_BSTRING;
                    break;
                case TYPE_ERROR:
                    DeserializationResult errorResult = deserializeError(ch);
                    if (errorResult.result == FAILED) {
                        return errorResult;
                    }
                    array[i].stringResponse = errorResult.content;
                    array[i].type = TYPE_ERROR;
                    break;
                case TYPE_INTEGER:
                    DeserializationResult integerResult = deserializeInteger(ch);
                    if (integerResult.result == FAILED) {
                        return integerResult;
                    }
                    if (!integerResult.isInteger) {
                        integerResult.result = FAILED;
                        integerResult.errorMessage = "deserializeInteger: result does not indicate integer type";
                        return integerResult;
                    }
                    array[i].intValue = integerResult.intValue;
                    array[i].type = TYPE_INTEGER;
                    break;
                case TYPE_ARRAY:
                    DeserializationResult resArray = deserializeArrayElements(ch);
                    if (resArray.result == FAILED) {
                        return resArray;
                    }
                    if (!resArray.isArray) {
                        resArray.result = FAILED;
                        resArray.errorMessage = "deserializeArrayElements: result does not indicate array type";
                        return resArray;
                    }
                    array[i].array = resArray.array.array;
                    array[i].type = TYPE_ARRAY;
                    array[i].arrayElementLength = resArray.array.length;
                    break;
                default:
                    char* errorStringMessage = "deserializeArrayElements: unexpected type";
                    printf("%s\n", errorStringMessage);
                    deserialization_result.result = FAILED;
                    deserialization_result.errorMessage = errorStringMessage;
                    return deserialization_result;
            }
        }
    }

    ArrayResult result;
    result.array = array;
    result.length = arrayLength;

    deserialization_result.result = SUCCESS;
    deserialization_result.isArray = 1;
    deserialization_result.array = result;

    return deserialization_result;

}

/*
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
*/
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
            case TYPE_SSTRING:
                char* concat = arrayConcatenateStart(result, array[0].stringResponse, 1);
                result = concat;
                break;
            case TYPE_BSTRING:
                char* concatB = arrayConcatenateStart(result, array[0].stringResponse, 1);
                result = concatB;
                break;
            case TYPE_ERROR:
                char* concatE = arrayConcatenateStart(result, array[0].stringResponse, 0);
                result = concatE;
                break;
            case TYPE_INTEGER:
                char intBuffer[20];
                sprintf(intBuffer, "%d", array[0].intValue);
                char* concatI = arrayConcatenateStart(result, intBuffer, 0);
                result = concatI;
                break;
            case TYPE_ARRAY:
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
            case TYPE_SSTRING:
                char* concat = arrayConcatenateOneElement(result, array[0].stringResponse, 1);
                result = concat;
                break;
            case TYPE_BSTRING:
                char* concatB = arrayConcatenateOneElement(result, array[0].stringResponse, 1);
                result = concatB;
                break;
            case TYPE_ERROR:
                char* concatE = arrayConcatenateOneElement(result, array[0].stringResponse, 0);
                result = concatE;
                break;
            case TYPE_INTEGER:
                char intBuffer[20];
                sprintf(intBuffer, "%d", array[0].intValue);
                char* concatI = arrayConcatenateOneElement(result, intBuffer, 0);
                result = concatI;
                break;
            case TYPE_ARRAY:
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
            case TYPE_SSTRING:
                char* concat = arrayConcatenateEnd(result, array[index].stringResponse, 1);
                result = concat;
                break;
            case TYPE_BSTRING:
                char* concatB = arrayConcatenateEnd(result, array[index].stringResponse, 1);
                result = concatB;
                break;
            case TYPE_ERROR:
                char* concatE = arrayConcatenateEnd(result, array[index].stringResponse, 0);
                result = concatE;
                break;
            case TYPE_INTEGER:
                char intBuffer[20];
                sprintf(intBuffer, "%d", array[1].intValue);
                char* concatI = arrayConcatenateEnd(result, intBuffer, 0);
                result = concatI;
                break;
            case TYPE_ARRAY:
                char* embeddedArray = deserializeEmbeddedArray(array[index].array, array[index].arrayElementLength);
                char* concatA = arrayConcatenateEnd(result, embeddedArray, 0);
                result = concatA;
                break;
            default:
                printf("arrayResponseConcatenator: unexpected type\n");
                exit(1);
        }
    }

    if (!isEnd && !isStart && !hasOneElement) {
        switch (array[index].type) {
            case TYPE_SSTRING:
                char* concat = arrayConcatenate(result, array[index].stringResponse, 1);
                result = concat;
                break;
            case TYPE_BSTRING:
                char* concatB = arrayConcatenate(result, array[index].stringResponse, 1);
                result = concatB;
                break;
            case TYPE_ERROR:
                char* concatE = arrayConcatenate(result, array[index].stringResponse, 0);
                result = concatE;
                break;
            case TYPE_INTEGER:
                char intBuffer[20];
                sprintf(intBuffer, "%d", array[1].intValue);
                char* concatI = arrayConcatenate(result, intBuffer, 0);
                result = concatI;
                break;
            case TYPE_ARRAY:
                char* embeddedArray = deserializeEmbeddedArray(array[index].array, array[index].arrayElementLength);
                char* concatA = arrayConcatenate(result, embeddedArray, 0);
                result = concatA;
                break;
            default:
                printf("arrayResponseConcatenator: unexpected type\n");
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

DeserializationResult deserializeArray(char** ch) {
    if (**ch != '*') {
        printf("deserializeArray: expected '*' to start array sequence\n");
        exit(1);
    }
    int initSize = 50;

    DeserializationResult arrayRes = deserializeArrayElements(ch);
    if (arrayRes.result == FAILED) {
        return arrayRes;
    }
    int arrayLength = arrayRes.array.length;
    ArrayElement* array = arrayRes.array.array;

    if (arrayLength == 0) {
        char* emptyArray = malloc(2*sizeof(char) + 1);
        strcpy(emptyArray, "[]");
        emptyArray[2] = '\0';
        arrayRes.result = SUCCESS;
        arrayRes.content = emptyArray;
        return arrayRes;
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

    arrayRes.content = result;
    arrayRes.result = SUCCESS;
    return arrayRes;
}

DeserializeRequestResult deserializeRequest(char** ch) {
    DeserializeRequestResult result = {0};
    DeserializationResult deserialization_result = {0};
    switch (**ch) {
        case '*':
            deserialization_result = deserializeArray(ch);
            if (deserialization_result.result == FAILED) {
                result.result = FAILED;
                result.errorMessage = deserialization_result.errorMessage;
            } else {
                result.result = SUCCESS;
                result.content = deserialization_result.content;
            }
            return result;
            break;
        case '+':
            deserialization_result = deserializeSimpleString(ch);
            if (deserialization_result.result == FAILED) {
                result.result = FAILED;
                result.errorMessage = deserialization_result.errorMessage;
            } else {
                result.result = SUCCESS;
                result.content = deserialization_result.content;
            }
            return result;
            break;
        case '-':
            deserialization_result = deserializeError(ch);
            if (deserialization_result.result == FAILED) {
                result.result = FAILED;
                result.errorMessage = deserialization_result.errorMessage;
            } else {
                result.result = SUCCESS;
                result.content = deserialization_result.content;
            }
            return result;
            break;
        case '$':
            deserialization_result = deserializeBulkStrings(ch);
            if (deserialization_result.result == FAILED) {
                result.result = FAILED;
                result.errorMessage = deserialization_result.errorMessage;
            } else {
                result.result = SUCCESS;
                result.content = deserialization_result.content;
            }
            return result;
            break;
        case ':':
            deserialization_result = deserializeInteger(ch);
            if (deserialization_result.result == FAILED) {
                result.result = FAILED;
                result.errorMessage = deserialization_result.errorMessage;
            } else {
                result.result = SUCCESS;
                if (!deserialization_result.isInteger) {
                    printf("deserializeRequest: Deserialization of integer, did not return 'isInteger' of type true\n");
                    exit(1);
                }
                int res = deserialization_result.intValue;
                int length = snprintf(NULL, 0, "%d", res);
                char* stringVersion = malloc(length + 1);
                sprintf(stringVersion, "%d", res);
                if (stringVersion == NULL) {
                    printf("deserializeRequest: error malloc deserializeInteger\n");
                    exit(1);
                }
                result.content = stringVersion;
            }

            return result;
            break;
        default:
            result.result = FAILED;
            result.errorMessage = "Type does not exist";
            return result;
            break;
    }
}