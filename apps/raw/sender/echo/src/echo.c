#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT 12345

void usage(const char* exe_name) {
    printf("Usage:\n");
    printf("\t%s -h <host> -p <port>\n", exe_name);
}

int start(int argc, char* argv[]) {
    char host[2048] = "";
    int port = DEFAULT_PORT;

    if (argc >= 3) {
        sscanf(argv[1], "-h %s", host);
        sscanf(argv[2], "-p %d", &port);
    }
    else {
        printf("Enter server address (-h <host> -p <port>): ");
        char input[2048];
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;

        if (sscanf(input, "-h %s -p %d", host, &port) != 2) {
            printf("Invalid input format\n");
            usage(argv[0]);
            return -1;
        }
    }

    return init_client(host, port);
}

int init_client(const char* host, short port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return -1;
    }

    SOCKET client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        WSACleanup();
        return -1;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    if (inet_pton(AF_INET, host, &server_address.sin_addr) <= 0) {
        printf("Invalid address/ Address not supported\n");
        closesocket(client_socket);
        WSACleanup();
        return -1;
    }

    printf("Sending message to server: %s:%d\n", host, port);
    static void process_connection(SOCKET client_socket, struct sockaddr_in server_address);
    process_connection(client_socket, server_address);

    closesocket(client_socket);
    WSACleanup();
    return 0;
}

static void process_connection(SOCKET client_socket, struct sockaddr_in server_address) {
    char buffer[1024] = "";

    const char* request = "GET_TIME";
    int ret = sendto(client_socket, request, strlen(request), 0, (struct sockaddr*)&server_address, sizeof(server_address));
    if (ret <= 0) {
        printf("Sending data error\n");
        return;
    }

    printf("====> Sent request: [%d bytes]\n", ret);

    memset(buffer, 0, sizeof(buffer));

    struct sockaddr_in from_address;
    int from_len = sizeof(from_address);
    ret = recvfrom(client_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&from_address, &from_len);

    if (ret <= 0) {
        printf("Receiving data error\n");
        return;
    }

    printf("<==== Server response: %s [%d bytes]\n", buffer, ret);
}