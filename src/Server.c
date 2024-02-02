#include "Server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

// constructor pentru Server
struct Server Server_Construct(int domain, int service, int protocol, unsigned long interface, int port, int backlog) 
{
    struct Server server;

    server.domain = domain;
    server.service = service;
    server.protocol = protocol;
    server.interface = interface;
    server.port = port;
    server.backlog = backlog;

    server.address.sin_family = domain;
    server.address.sin_port = htons(port);
    server.address.sin_addr.s_addr = htonl(interface);

    server.socket = socket(domain, service, protocol);

    if (server.socket == 0) 
    {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    if (bind(server.socket, (struct sockaddr *)&server.address, sizeof(server.address)) < 0) 
    {
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    }

    if (listen(server.socket, server.backlog) < 0) 
    {
        perror("Failed to listen on socket");
        exit(EXIT_FAILURE);
    }

    return server;
}

// functie pentru pornirea Serverului
void Server_Start(struct Server *server) 
{
    printf("Server is starting on port %d\n", server->port);

    while (1) 
    {
        int client_socket;
        struct sockaddr_in client_address;
        int addrlen = sizeof(client_address);

        // accepta o conexiune noua
        client_socket = accept(server->socket, (struct sockaddr *)&client_address, (socklen_t*)&addrlen);
        if (client_socket < 0) 
        {
            perror("Failed to accept client connection");
            continue;
        }

        // fork un proces nou ca sa dea handle la client request
        if (fork() == 0) 
        {
            // proces copil
            close(server->socket); // inchidem socket ul de listen in procesul copil
            HandleClientRequest(client_socket); // handle client request
            exit(0); // exit proces copil
        } else 
        {
            // proces parinte
            close(client_socket); // inchidem socket-ul client in procesul parinte
        }
    }
}