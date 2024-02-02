#include "Server.h"

int main() {
    // initializam serverul cu port 8080
    struct Server myServer = Server_Construct(AF_INET, SOCK_STREAM, 0, INADDR_ANY, 8080, 10);

    // pornim serverul
    Server_Start(&myServer);

    return 0;
}
