#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#define LB_PORT 8080
#define MAX_PORT_AMOUNT 20
#define BUFFER_SIZE 4096


// WHOLE LOAD BALANCER MADE WITH AI!!!!!!!!

struct thread_data {
    SOCKET client;
    int backend_port;
};


SOCKET connect_to_backend(int port) {
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) return INVALID_SOCKET;

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    // 127.0.0.1 is localhost
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        closesocket(s);
        return INVALID_SOCKET;
    }
    return s;
}

// Thread functions in windows have to return a DWORD
// Windows threading API expects thread functions to use the WINAPI keyword
DWORD WINAPI handle_client(LPVOID lpParam) {
    struct thread_data *td = (struct thread_data*)lpParam;
    SOCKET client = td->client;
    int backend_port = td->backend_port;
    free(td);

    SOCKET backend = connect_to_backend(backend_port);
    if (backend == INVALID_SOCKET) {
        closesocket(client);
        printf("Failed to connect to backend %d\n", backend_port);
        return 1;
    }

    // Relay request from client to backend
    char buffer[BUFFER_SIZE];
    int bytes = recv(client, buffer, BUFFER_SIZE, 0);
    if (bytes > 0) {
        send(backend, buffer, bytes, 0);

        // Relay response from backend to client
        bytes = recv(backend, buffer, BUFFER_SIZE, 0);
        if (bytes > 0) {
            send(client, buffer, bytes, 0);
        }
    }
    closesocket(backend);
    closesocket(client);
    return 0;
}

void init_ports(int *ports, int *portCount) {
    int currPort = 80;
    for (int i = 0; i < 2; i++) {
        ports[i] = currPort++;
        (*portCount)++;
    }
}

int main(void) {
    WSADATA wsa;
    SOCKET lb_sock, client;
    struct sockaddr_in lb_addr, client_addr;
    int c, backend_idx = 0;
    int ports[MAX_PORT_AMOUNT] = {0};
    int portCount = 0;

    init_ports(ports, &portCount);

    // Innit WSA
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    // Create loadbalancer socket
    lb_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (lb_sock == INVALID_SOCKET) {
        printf("socket() failed\n");
        WSACleanup();
        return 1;
    }

    // Define options for loadbalancer socket
    lb_addr.sin_family = AF_INET;
    lb_addr.sin_addr.s_addr = INADDR_ANY;
    lb_addr.sin_port = htons(LB_PORT);

    // Bind the options to socket
    if (bind(lb_sock, (struct sockaddr*)&lb_addr, sizeof(lb_addr)) < 0) {
        printf("bind failed\n");
        closesocket(lb_sock);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    listen(lb_sock, 4);
    printf("Load balancer listening on http://localhost:%d\n", LB_PORT);

    int currPortIdx = 0;
    while (1) {
        // Accept client connection
        c = sizeof(client_addr);
        client = accept(lb_sock, (struct sockaddr*)&client_addr, &c);
        if (client == INVALID_SOCKET) {
            printf("accept failed\n");
            continue;
        }

        if (currPortIdx > portCount)
            currPortIdx = 0;

        int backend_port = ports[currPortIdx++];


        // Thread magic idk tbh
        struct thread_data *td = malloc(sizeof(struct thread_data));
        td->client = client;
        td->backend_port = backend_port;

        HANDLE thread = CreateThread(NULL, 0, handle_client, td, 0, NULL);
        if (thread != NULL) {
            CloseHandle(thread); // Let thread run independently
        }
    }
    closesocket(lb_sock);
    WSACleanup();
    return 0;
}