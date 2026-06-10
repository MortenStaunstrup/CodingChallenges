#include <stdio.h>
#include <stdlib.h>

#include "Deserialization.h"
#include "Tests.h"



int main(int argc, char* argv[]) {


    /*char* request = "*1\r\n*1\r\n*0\r\n";
    char* arrayRes = deserializeRequest(&request);
    */

    RunDeserializationTests();

    return 0;
}