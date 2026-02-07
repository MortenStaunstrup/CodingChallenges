#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <time.h>
#include <ctype.h>
#pragma comment(lib, "ws2_32.lib")

#define LB_PORT 80
#define MAX_PORT_AMOUNT 20
#define BUFFER_SIZE 4096

// use gcc main.c -o <name> -lws2_32 to compile

// Used to pass data into thread
struct thread_data {
    SOCKET client;
    int backend_port;
};

struct health_data {
    char* check;
    int backend_port;
};

enum portResult {
    NOT_PORT_ARG,
    NO_PORTS,
    FORMAT_ERROR,
    SUCCESS
};

struct port_result {
    enum portResult result;
    int ports[MAX_PORT_AMOUNT];
    int amount;
};

// Globals so both threads can use them
int healthyPorts[MAX_PORT_AMOUNT] = {0};
int healthyPortCount = 0;
int allPorts[MAX_PORT_AMOUNT] = {0};
int allPortCount = 0;
HANDLE healthyMutex;

int secondsToWait = 0; // default, will be set in main

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


// Return 1 = Could not connect to server, or server failed health check
// Return 0 = Server sends 200 result and passes health check
// Return -1 = Critical error, should not happen
int send_health_check(struct health_data data) {
    char* check = data.check;
    int backend_port = data.backend_port;
    int length = strlen(data.check);

    char buffer[BUFFER_SIZE];

    SOCKET backend = connect_to_backend(backend_port);
    if (backend == INVALID_SOCKET) {
        printf("Failed to connect to backend %d for health check\n", backend_port);
        return 1;
    }
    if (send(backend, check, length, 0) == SOCKET_ERROR) {
        printf("Failed to send data for health check %d\n", SOCKET_ERROR);
        closesocket(backend);
        return 1;
    }
    if (recv(backend, buffer, BUFFER_SIZE, 0) > 0) {
        char resBuffer[20];
        strncpy(resBuffer, buffer, 3);
        resBuffer[3] = '\0';
        if (strcmp(resBuffer, "200") == 0) {
            printf("Backend: %d passed health check\n", backend_port);
            closesocket(backend);
            return 0;
        }
        printf("Backend: %d failed to pass health check\n", backend_port);
        closesocket(backend);
        return 1;
    }

    closesocket(backend);
    return -1;
}

void init_ports(int *ports, int *portCount, int* userPorts, int userPortCount) {
    for (int i = 0; i < userPortCount; i++) {
        ports[i] = userPorts[i];
        (*portCount)++;
    }
}

int remove_unhealthy_port(int port) {
    int portFound = 0;
    int i = 0;
    while (i < healthyPortCount) {
        if (healthyPorts[i] == port) {
            healthyPorts[i] = 0;
            healthyPortCount--;
            portFound = 1;
            i++;
            break;
        }
        i++;
    }

    if (!portFound) {
        return 1;
    }
    while (healthyPorts[i] != 0) {
        healthyPorts[i - 1] = healthyPorts[i];
        i++;
    }
    return 0;
}

int add_healthy_port(int port) {
    int i = 0;
    while (i < healthyPortCount) {
        if (healthyPorts[i] == port) {
            return 10;
        }
        i++;
    }
    healthyPorts[healthyPortCount] = port;
    healthyPortCount++;
    return 0;
}

// Thread for periodic health check
DWORD WINAPI healthcheck_thread(LPVOID lpParam) {
    while (1) {
        Sleep(secondsToWait * 1000);

        // Mutex for health check operation
        WaitForSingleObject(healthyMutex, INFINITE);

        for (int i = 0; i < allPortCount; i++) {
            struct health_data sendData;
            sendData.backend_port = allPorts[i];
            sendData.check = "GET /health";

            int result = send_health_check(sendData);
            if (result != 0) {
                int removeResult = remove_unhealthy_port(sendData.backend_port);
                if (removeResult == 1) {
                    printf("Still no connection to server: %d\n", sendData.backend_port);
                } else if (removeResult == 0) {
                    printf("Successfully removed unhealthy server: %d\n", sendData.backend_port);
                }
            } else {
                int addResult = add_healthy_port(sendData.backend_port);
                if (addResult == 10) {
                    printf("Server still healthy: %d\n", sendData.backend_port);
                } else if (addResult == 0) {
                    printf("Successfully added healthy server: %d\n", sendData.backend_port);
                }
            }
        }

        ReleaseMutex(healthyMutex);
    }
    return 0;
}

int checkForSArg(char* argv) {
    char resBuffer[20];
    strncpy(resBuffer, argv, 2);
    resBuffer[2] = '\0';

    if (strcmp(resBuffer, "-s") == 0) {
        char* p = argv + 2;
        int seconds = 0;
        while (*p != '\0') {
            if (!isdigit(*p)) {
                return -2;
            }
            seconds = seconds * 10 + *p - '0';
            p++;
        }
        printf("Seconds to wait between health checks: %u\n", seconds);
        return seconds;
    }
    return -1;
}

struct port_result checkForPArg(char* argv) {
    char resBuffer[20];
    strncpy(resBuffer, argv, 2);
    resBuffer[2] = '\0';
    struct port_result result = {0};
    result.amount = 0;
    result.result = NOT_PORT_ARG;

    if (strcmp(resBuffer, "-p") == 0) {
        char* p = argv + 2;
        int currentPort = 0;
        int isLastCharComma = 0;

        while (*p != '\0') {
            if (isdigit(*p)) {
                isLastCharComma = 0;
                currentPort = currentPort * 10 + (*p - '0');
            } else if (*p == ',') {
                isLastCharComma = 1;
                if (currentPort == 0) {
                    result.result = FORMAT_ERROR;
                    printf("Missing port arg\n");
                    return result;
                }
                printf("Adding port %u to ports\nContinuing...\n", currentPort);
                result.ports[result.amount] = currentPort;
                result.amount = result.amount + 1;
                currentPort = 0;
            } else {
                printf("Unexpected character %c in -p arg\n", *p);
                result.result = FORMAT_ERROR;
                result.amount = 0;
                return result;
            }
            p++;
        }
        if (isLastCharComma) {
            printf("Expected port after comma in -p arg\n");
            result.result = FORMAT_ERROR;
            result.amount = 0;
            return result;
        }
        if (result.amount == 0 && currentPort == 0) {
            printf("Cannot use -p arg without specifying ports\n");
            result.result = NO_PORTS;
            return result;
        }
        if (currentPort == 0) {
            printf("Port 0 not a valid port\n");
            result.result = FORMAT_ERROR;
            return result;
        }
        printf("Adding port %u to ports\n", currentPort);
        result.result = SUCCESS;
        result.ports[result.amount] = currentPort;
        result.amount = result.amount + 1;
    }
    return result;
}

void help() {
    printf("Usage: balancer.exe -p<server ports> -s<seconds>\n");
    printf("-p usage: balancer.exe -p8080,7032 specify as many ports as needed\n");
    printf("-s usage: balancer.exe -s200 specify seconds before loadbalancer sends health check to servers\n");
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        help();
        return 0;
    }
    if (argc == 2 && strcmp(argv[1], "-h") == 0) {
        help();
        return 0;
    }
    if (argc < 3) {
        printf("Missing args\n");
        return 1;
    }

    int* userPorts;
    int userPortCount = 0;

    printf("checking for -p arg\n");
    for (int i = 1; i < argc; i++) {
        struct port_result res = checkForPArg(argv[i]);
        if (res.result == SUCCESS) {
            userPorts = res.ports;
            userPortCount = res.amount;
            break;
        }
        if (res.result != NOT_PORT_ARG) {
            if (res.result == FORMAT_ERROR) {
                printf("Format error in -p arg, see -h for help\n");
                exit(1);
            }
            if (res.result == NO_PORTS) {
                printf("No ports specified in -p arg\n");
                exit(1);
            }
        }
    }

    if (userPortCount == 0) {
        printf("-p arg not present\n");
        exit(1);
    }

    printf("checking for -s arg\n");
    for (int i = 1; i < argc; i++) {
        int result = checkForSArg(argv[i]);
        if (result > -1) {
            secondsToWait = result;
            break;
        }
    }

    if (secondsToWait == 0) {
        printf("-s arg not present or number <= 0\n");
        exit(1);
    }


    WSADATA wsa;
    SOCKET lb_sock, client;
    struct sockaddr_in lb_addr, client_addr;
    int c = 0;

    init_ports(allPorts, &allPortCount, userPorts, userPortCount);

    // Innit WSA
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }


    int initHealthyPortIdx = 0;
    for (int i = 0; i<allPortCount; i++) {
        struct health_data data;
        data.backend_port = allPorts[i];
        data.check =
            "GET /health";
        int result = send_health_check(data);
        if (result == 0) {
            healthyPorts[initHealthyPortIdx] = data.backend_port;
            initHealthyPortIdx++;
            healthyPortCount++;
        }
    }

    healthyMutex = CreateMutex(NULL, FALSE, NULL);

    // Start healthcheck thread
    HANDLE hHealthThread = CreateThread(NULL, 0, healthcheck_thread, NULL, 0, NULL);
    if (!hHealthThread) {
        printf("Failed to create healthcheck thread\n");
        WSACleanup();
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

        WaitForSingleObject(healthyMutex, INFINITE);
        if (healthyPortCount == 0) {
            printf("No healthy backend available. Closing client\n");
            closesocket(client);
            ReleaseMutex(healthyMutex);
            continue;
        }

        if (currPortIdx >= healthyPortCount)
            currPortIdx = 0;

        ReleaseMutex(healthyMutex);
        int backend_port = healthyPorts[currPortIdx];
        currPortIdx++;


        // Thread magic idk tbh
        struct thread_data *td = malloc(sizeof(struct thread_data));
        td->client = client;
        td->backend_port = backend_port;

        // HANDLE is opaque term for objects managed by Windows (processes, threads, files etc)
        // Here the function handle_client gets called, where the new thread handles the client request and subsequent response
        HANDLE thread = CreateThread(NULL, 0, handle_client, td, 0, NULL);
        if (thread != NULL) {
            CloseHandle(thread); // Let thread run independently
        }
    }
    closesocket(lb_sock);
    WSACleanup();
    return 0;
}