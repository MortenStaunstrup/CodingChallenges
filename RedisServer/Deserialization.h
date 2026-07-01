//
// Created by mort4 on 10-06-2026.
//

#ifndef REDISSERVER_DESERIALIZATION_H
#define REDISSERVER_DESERIALIZATION_H
#include "Tests.h"



DeserializationResult deserializeSimpleString(char** ch);

DeserializationResult deserializeBulkStrings(char** ch);

char* concatenateWithSpace(char* string1, char* string2);

DeserializationResult deserializeError(char** ch);

DeserializationResult deserializeInteger(char** ch);

DeserializationResult deserializeArrayElements(char** ch);

char* arrayConcatenate(char* string1, char* string2, int isString);

char* arrayConcatenateStart(char* string1, char* string2, int isString);

char* arrayConcatenateEnd(char* string1, char* string2, int isString);

char* arrayConcatenateOneElement(char* string1, char* string2, int isString);

char* arrayResponseConcatenator(char* result, ArrayElement* array, int index, int isStart, int isEnd, int hasOneElement);

char* deserializeEmbeddedArray(ArrayElement* array, int length);

DeserializationResult deserializeArray(char** ch);

DeserializeRequestResult deserializeRequest(char** ch);

#endif //REDISSERVER_DESERIALIZATION_H
