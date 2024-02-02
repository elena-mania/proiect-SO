#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include "ClientHandler.h"

// structura pentru server
struct Server 
{
    int domain;
    int service;
    int protocol;
    unsigned long interface;
    int port;
    int backlog;
    struct sockaddr_in address;
    int socket;
};

// functie pentru construiere si initializare sv
struct Server Server_Construct(int domain, int service, int protocol, unsigned long interface, int port, int backlog);

// functie pentru pornirea serverului
void Server_Start(struct Server *server);

#endif 
