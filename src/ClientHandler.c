#include "ClientHandler.h"
#include "HttpServer.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void HandleClientRequest(int client_socket) 
{
    char request_buffer[4096]; 
    read(client_socket, request_buffer, sizeof(request_buffer));

    HttpRequest httpRequest;
    ParseHttpRequest(request_buffer, &httpRequest);

    switch (httpRequest.method) 
    {
        case HTTP_GET:
            HandleGetRequest(&httpRequest, client_socket);
            break;
        case HTTP_HEAD:
            HandleHeadRequest(&httpRequest, client_socket);
            break;
        case HTTP_POST:
            HandlePostRequest(&httpRequest, client_socket);
            break;
        case HTTP_PUT:
            HandlePutRequest(&httpRequest, client_socket);
            break;
        default:
            // nu se intampla nimic in cazul default
            break;
    }

    // inchidem socket-ul client
    close(client_socket);
}
