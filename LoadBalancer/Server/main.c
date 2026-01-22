#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define PORT 80
#define BUFFER_SIZE 1024

// use -lws2_32 when gcc to compiling program

int main(void) {
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
    address.sin_port = htons(PORT);

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

    printf("Server listening on port %d\n", PORT);

    // Accepts a client connection
    // Blocks until a client connects
    new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
    if (new_socket == INVALID_SOCKET) {
        printf("accept failed: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    printf("Connection accepted\n");

    // Continuously reads from the client and prints the received data
    // Clears buffer for next message
    int valread;
    while((valread = recv(new_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        printf("Client: %s", buffer);
        memset(buffer, 0, BUFFER_SIZE);
    }

    // Closes the open sockets and clean up Winsock resources
    closesocket(new_socket);
    closesocket(server_fd);
    WSACleanup();

    // Only processes one client, then terminates after disconnect
    // Only handles one client at a time

    return 0;

}