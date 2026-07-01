//
// Created by mort4 on 25-06-2026.
//

#ifndef REDISSERVER_SERIALIZATION_H
#define REDISSERVER_SERIALIZATION_H

#include "Structs.h"

SerializationRequestResult SerializeSimpleString(char** ch);

SerializationRequestResult SerializeBulkString(char** ch);

SerializationRequestResult SerializeInteger(char** ch);

SerializationRequestResult SerializeError(char** ch);

SerializationRequestResult SerializeArray(char** ch);

SerializationRequestResult HandleRequest(char** ch);

char* concatenate(char* string1, char* string2);

#endif //REDISSERVER_SERIALIZATION_H
