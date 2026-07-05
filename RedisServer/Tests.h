//
// Created by mort4 on 09-06-2026.
//

#ifndef REDISSERVER_TESTS_H
#define REDISSERVER_TESTS_H

#include "Structs.h"

TypeResponse ResponseType(char* ch);

CreateDeserializationTests CreateDeserializationTestsFunction();

ClientRequestResult deserializeRequest(char** ch);

void RunDeserializationTests();

#endif //REDISSERVER_TESTS_H
