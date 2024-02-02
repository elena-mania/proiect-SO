#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H


// enum pentru metode HTTP 
typedef enum 
{
    HTTP_GET,
    HTTP_HEAD,
    HTTP_POST,
    HTTP_PUT,
    HTTP_UNSUPPORTED
} HttpMethod;

// structura care sa tina request-urile HTTP parsate
typedef struct 
{
    HttpMethod method;
    char path[1024];
    char body[4096];  
    char contentType[256];  
} HttpRequest;

// functie care sa parseze request-ul HTTP
void ParseHttpRequest(const char *request, HttpRequest *httpRequest);

// functionalitatile server-ului 
void HandleGetRequest(const HttpRequest *httpRequest, int client_socket);
void HandleHeadRequest(const HttpRequest *httpRequest, int client_socket);
void HandlePostRequest(const HttpRequest *httpRequest, int client_socket);
void HandlePutRequest(const HttpRequest *httpRequest, int client_socket);

#endif 
