#include <winsock2.h>
#include "echo.h"

int main(int argc, char* argv[]) {
    int result = start(argc, argv);

    WSACleanup();

    return result;
}