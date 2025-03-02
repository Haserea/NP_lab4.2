#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <time.h>
#include "echo.h"

void free_socket(SOCKET* socket) {
    if (*socket != INVALID_SOCKET) {
        closesocket(*socket);
        *socket = INVALID_SOCKET;
    }
}
#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT 12345

void usage(const char* exe_name) {
    printf("Usage:\n");
    printf("\t%s -p <port>\n", exe_name);
}

int start(int argc, char* argv[]) {
    int port = DEFAULT_PORT;

    if (argc >= 2) {
        sscanf(argv[1], "-p %d", &port);
    }

    return init_server(port);
}

int init_server(short port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return -1;
    }

    SOCKET server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        WSACleanup();
        return -1;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        printf("Bind failed\n");
        free_socket(&server_socket);
        WSACleanup();
        return -2;
    }

    printf("Server is running on port %d\n", port);

    static void process_requests(SOCKET server_socket);
    process_requests(server_socket);

    free_socket(&server_socket);
    return 0;
}

static void process_requests(SOCKET server_socket) {
    struct sockaddr_in client_addr;
    int len = sizeof(client_addr);
    char buffer[1024];

    while (1) {
        memset(buffer, 0, sizeof(buffer));

        int bytes_received = recvfrom(server_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &len);
        if (bytes_received <= 0) {
            printf("Receiving data error\n");
            continue;
        }

        printf("Received request from %s\n", inet_ntoa(client_addr.sin_addr));

        time_t raw_time;
        time(&raw_time);
        struct tm* time_info = localtime(&raw_time);

        char response[1024] = { 0 };
        strftime(response, sizeof(response), "Server time: %Y-%m-%d %H:%M:%S", time_info);

        int ret = sendto(server_socket, response, strlen(response), 0, (struct sockaddr*)&client_addr, len);

        if (ret <= 0) {
            printf("Sending response error\n");
        }
        else {
            printf("Sent server time to client: %s\n", response);
        }
    }
}