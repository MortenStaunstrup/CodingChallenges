#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "Deserialization.h"

#define BUFFER_SIZE 1024

// Command to run for testing client requests: docker run -it --rm redis:7 redis-cli -h host.docker.internal -p 6379 <COMMANDS>

int main(int argc, char* argv[]) {
    WSADATA wsa;
    SOCKET server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char* buffer = malloc(sizeof(char) * BUFFER_SIZE);
    int port = 6379;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("WSAStartup failed: %d\n", WSAGetLastError());
        free(buffer);
        return 1;
    }

    // Create TCP socket using IPv4
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        printf("Could not create socket: %d\n", WSAGetLastError());
        WSACleanup();
        free(buffer);
        return 1;
    }

    // Set socket option
    // Allows reuse of address (for quick restarts)
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) < 0) {
        printf("setsockopt failed: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        free(buffer);
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
        free(buffer);
        return 1;
    }

    // Listens for connections
    if (listen(server_fd, 3) == SOCKET_ERROR) {
        printf("listen failed: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        free(buffer);
        return 1;
    }

    while (1) {
        // Accepts a client connection
        new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (new_socket == INVALID_SOCKET) {
            printf("accept failed: %d\n", WSAGetLastError());
            break;
        }

        // Turn IPv4 address into ip address
        char *ip = inet_ntoa(address.sin_addr);

        printf("Connection accepted\n");
        printf("Connected device ip: %s\n", ip);

        while (1) {
            int valread = recv(new_socket, buffer, BUFFER_SIZE, 0);

            if (valread == SOCKET_ERROR) {
                printf("recv failed: %d\n", WSAGetLastError());
                break;
            }

            if (valread == 0) {
                printf("Connection closed\n");
                break;
            }
            printf("Raw received: %s\n", buffer);

            char* p = buffer;
            DeserializeRequestResult deserializationResult = deserializeRequest(&p);
            if (deserializationResult.result == SUCCESS) {
                printf("Message: %s\n", deserializationResult.content);
                send(new_socket, deserializationResult.content, sizeof(deserializationResult.content) - 1, 0);
            } else {
                printf("Error message: %s\n", deserializationResult.errorMessage);
                printf("Message: %s\n", deserializationResult.errorMessage);
                send(new_socket, deserializationResult.errorMessage, sizeof(deserializationResult.errorMessage) - 1, 0);
            }
        }

    }
    closesocket(new_socket);
    closesocket(server_fd);
    WSACleanup();
    free(buffer);
    return 0;
}