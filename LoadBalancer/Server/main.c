#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

typedef enum Method {
    get,
    post,
    put,
    delete,
    invalid
}Method;

typedef struct request {
    Method Method;
    char* Endpoint;
}request;

#define BUFFER_SIZE 1024

// use -lws2_32 when gcc to compiling program

char *concat(const char *str1, const char *str2) {
    char *result = malloc(strlen(str1) + strlen(str2) + 1);
    strcpy(result, str1);
    strcat(result, str2);
    result[strlen(str1) + strlen(str2)] = '\0';
    return result;
}

int tryParseClient(char requestBuffer[BUFFER_SIZE], int index) {
    char client[9];
    client[8] = '\0';
    strncpy(client, requestBuffer + index, 8);
    if ((strcmp(client, "Client: ")) == 0) {
        return 0;
    } else {
        return -1;
    }
}

// 1. GET CHECK FUNCTION
int checkForGet(char requestBuffer[BUFFER_SIZE], int index) {
    char get[4];
    get[3] = '\0';
    strncpy(get, requestBuffer + index, 3);
    printf("checkForGet: substring='%s'\n", get);

    if ((strcmp(get, "GET")) == 0) {
        printf("checkForGet: matched 'GET', returning 3\n");
        return 3;
    } else {
        printf("checkForGet: did not match 'GET', returning -1\n");
        return -1;
    }
}


// 2. DELETE CHECK FUNCTION
int checkForDelete(char requestBuffer[BUFFER_SIZE], int index) {
    char get[7];
    get[6] = '\0';
    strncpy(get, requestBuffer + index, 6);
    printf("checkForDelete: substring='%s'\n", get);

    if ((strcmp(get, "DELETE")) == 0) {
        printf("checkForDelete: matched 'DELETE', returning 6\n");
        return 6;
    } else {
        printf("checkForDelete: did not match 'DELETE', returning -1\n");
        return -1;
    }
}


// 3. POST CHECK FUNCTION
int checkForPost(char requestBuffer[BUFFER_SIZE], int index) {
    char get[5];
    get[4] = '\0';
    strncpy(get, requestBuffer + index, 4);
    printf("checkForPost: substring='%s'\n", get);

    if ((strcmp(get, "POST")) == 0) {
        printf("checkForPost: matched 'POST', returning 4\n");
        return 4;
    } else {
        printf("checkForPost: did not match 'POST', returning -1\n");
        return -1;
    }
}


// 4. PUT CHECK FUNCTION
int checkForPut(char requestBuffer[BUFFER_SIZE], int index) {
    char get[4];
    get[3] = '\0';
    strncpy(get, requestBuffer + index, 3);
    printf("checkForPut: substring='%s'\n", get);

    if ((strcmp(get, "PUT")) == 0) {
        printf("checkForPut: matched 'PUT', returning 3\n");
        return 3;
    } else {
        printf("checkForPut: did not match 'PUT', returning -1\n");
        return -1;
    }
}


// 5. PARSE METHOD FUNCTION
Method tryParseMethod(char requestBuffer[BUFFER_SIZE], int index) {
    char method[50]; // unused, but kept here per your code

    printf("tryParseMethod: requestBuffer[index]=%c\n", requestBuffer[index]);

    if (requestBuffer[index] == 'G') {
        printf("tryParseMethod: found 'G', checking for 'GET'\n");
        int getres = checkForGet(requestBuffer, index);
        if (getres != -1) {
            printf("tryParseMethod: recognized GET method\n");
            return get;
        }
        printf("tryParseMethod: not GET, returning invalid\n");
        return invalid;
    }

    if (requestBuffer[index] == 'P') {
        printf("tryParseMethod: found 'P', checking for 'PUT' and 'POST'\n");
        int putres = checkForPut(requestBuffer, index);
        if (putres != -1) {
            printf("tryParseMethod: recognized PUT method\n");
            return put;
        }
        int postres = checkForPost(requestBuffer, index);
        if (postres != -1) {
            printf("tryParseMethod: recognized POST method\n");
            return post;
        }
        printf("tryParseMethod: not PUT or POST, returning invalid\n");
        return invalid;
    }

    if (requestBuffer[index] == 'D') {
        printf("tryParseMethod: found 'D', checking for 'DELETE'\n");
        int deleteres = checkForDelete(requestBuffer, index);
        if (deleteres != -1) {
            printf("tryParseMethod: recognized DELETE method\n");
            return delete;
        }
        printf("tryParseMethod: not DELETE, returning invalid\n");
        return invalid;
    }

    printf("tryParseMethod: did not find valid method letter at index, returning invalid\n");
    return invalid;
}

char* tryParseEndpoint(char requestBuffer[BUFFER_SIZE], int index) {
    int endpointCount = 0;
    int initSlash = 0;
    int endPointStart = index;
    while (requestBuffer[index] != ' ') {
        if (!initSlash && requestBuffer[index] != '/') {
            return NULL;
        }
        initSlash = 1;

        endpointCount++;
        index++;
    }
    if (endpointCount == 0) {
        printf("Could not parse endpoint");
        return NULL;
    }
    char* endpoint = malloc(endpointCount + 1);
    if (endpoint == NULL) {
        printf("tryParseEndpoint: malloc failed\n");
        return NULL;
    }
    for (int i = 0; i < endpointCount; i++) {
        endpoint [i] = requestBuffer[endPointStart + i];
    }
    endpoint[endpointCount] = '\0';
    return endpoint;
}

request parseRequest(char requestBuffer[BUFFER_SIZE]) {
    printf("Parsing request\n");
    request req;
    req.Endpoint = "";
    req.Method = invalid;
    int i = 0;

    int methodResult = tryParseMethod(requestBuffer, i);
    if (methodResult == invalid) {
        printf("Method not found\n");
        return req;
    }
    req.Method = methodResult;
    switch (req.Method) {
        case get:
            i+=4;
            break;
        case post:
            i+=5;
            break;
        case put:
            i+=4;
            break;
        case delete:
            i+=7;
            break;
        default:
            return req;
    }
    char* endpointResult = tryParseEndpoint(requestBuffer, i);
    if (endpointResult == NULL) {
        printf("Endpoint error\n");
        req.Method = invalid;
        return req;
    }
    req.Endpoint = endpointResult;

    return req;
}

int main(int argc, char* argv[]) {
    if (argc > 2 || argc < 2) {
        printf("Only 1 arg needed (server number 1 or 2)\n");
        return 1;
    }
    if (*argv[1] - '0' > 2 || *argv[1] - '0' < 1) {
        printf("You can only start server number 1 or 2\n");
        return 1;
    }
    int port = *argv[1] - '0' + 8079;
    WSADATA wsa;
    SOCKET server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("WSAStartup failed: %d\n", WSAGetLastError());
        return 1;
    }

    // Create TCP socket using IPv4
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        printf("Could not create socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Set socket option
    // Allows reuse of address (for quick restarts)
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) < 0) {
        printf("setsockopt failed: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    // Fill out address structure
    // IPv4
    address.sin_family = AF_INET;
    // Any local IP
    address.sin_addr.S_un.S_addr = INADDR_ANY;
    // Port number in network byte order
    address.sin_port = htons(port);

    // Binds the socket to the port and IP, and checks for error
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        printf("bind failed: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    // Listens for connections
    if (listen(server_fd, 3) == SOCKET_ERROR) {
        printf("listen failed: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    const char* init =
                "HTTP/1.1 200 OK\r\n"
                "Content-Length: 30\r\n"
                "Content-Type: text/plain\r\n"
                "\r\n"
                "Hello from backend server ";

    char numbStr[20];
    sprintf(numbStr, "%d", port);
    char* http_response = concat(init, numbStr);

    printf("Server listening on port %d\n", port);

    int isHealthCheck = 0;
    while (1) {
        // Accepts a client connection
        new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (new_socket == INVALID_SOCKET) {
            printf("accept failed: %d\n", WSAGetLastError());
            closesocket(server_fd);
            WSACleanup();
            free(http_response);
            return 1;
        }

        // Turn IPv4 address into ip address
        char *ip = inet_ntoa(address.sin_addr);

        printf("Connection accepted\n");
        printf("Connected device ip: %s\n", ip);

        int valread = recv(new_socket, buffer, BUFFER_SIZE, 0);
        if (valread == 0) {
            printf("Connection closed\n");
            break;
        }
        printf("Client: %s", buffer);

        request request = parseRequest(buffer);
        printf("Printing request endpoint: %s\n", request.Endpoint);
        if (strcmp(request.Endpoint, "/health") == 0) {
            isHealthCheck = 1;
            const char* healthResponse =
                "200 OK";
            printf("Sending healthcheck to loadbalancer\n");
            if (send(new_socket, healthResponse, (int)strlen(healthResponse), 0) == SOCKET_ERROR) {
                printf("Message failed: %d\n", WSAGetLastError());
                break;
            }
        }
        free(request.Endpoint);
        memset(buffer, 0, BUFFER_SIZE);

        if (!isHealthCheck) {
            printf("Sending message to client\n");
            if (send(new_socket, http_response, (int)strlen(http_response), 0) == SOCKET_ERROR) {
                printf("Message failed: %d\n", WSAGetLastError());
                break;
            }
        }
        printf("Closing socket connection to client");
        isHealthCheck = 0;
        closesocket(new_socket);
    }
    // Closes the open sockets and clean up Winsock resources
    closesocket(new_socket);
    closesocket(server_fd);
    WSACleanup();

    free(http_response);
    return 0;
}