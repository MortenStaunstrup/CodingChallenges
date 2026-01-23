#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define BUFFER_SIZE 1024

// use -lws2_32 when gcc to compiling program

char *concat(const char *str1, const char *str2) {
    char *result = malloc(strlen(str1) + strlen(str2) + 1);
    strcpy(result, str1);
    strcat(result, str2);
    result[strlen(str1) + strlen(str2)] = '\0';
    return result;
}

void convert_to_string(int number, char str[]) {
    sprintf(str, "%d", number);
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
    int port = *argv[1] - '0' + 79;
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
                "Content-Length: 28\r\n"
                "Content-Type: text/plain\r\n"
                "\r\n"
                "Hello from backend server ";

    char numbStr[20];
    sprintf(numbStr, "%d", port);
    char* http_response = concat(init, numbStr);

    printf("Server listening on port %d\n", port);

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
        memset(buffer, 0, BUFFER_SIZE);


        printf("Sending message to client\n");
        if (send(new_socket, http_response, (int)strlen(http_response), 0) == SOCKET_ERROR) {
            printf("Message failed: %d\n", WSAGetLastError());
            break;
        }
        printf("Closing socket connection to client");
        closesocket(new_socket);
    }
    // Closes the open sockets and clean up Winsock resources
    closesocket(new_socket);
    closesocket(server_fd);
    WSACleanup();

    free(http_response);
    return 0;
}