#include "HttpServer.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <jansson.h>
#include <sys/time.h>
#include <errno.h>

const char* GetMimeType(const char* path) 
{
    const char* dot = strrchr(path, '.');
    if (!dot || dot == path) return "text/plain";
    if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0) return "text/html";
    if (strcmp(dot, ".png") == 0) return "image/png";
    if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(dot, ".css") == 0) return "text/css";
    if (strcmp(dot, ".js") == 0) return "application/javascript";
    return "text/plain";
}

void ParseHttpRequest(const char *request, HttpRequest *httpRequest) 
{
    char method[10];
    sscanf(request, "%s %s", method, httpRequest->path);

    if (strcmp(method, "GET") == 0) 
    {
        httpRequest->method = HTTP_GET;
    } else if (strcmp(method, "HEAD") == 0) 
    {
        httpRequest->method = HTTP_HEAD;
    } else if (strcmp(method, "POST") == 0) 
    {
        httpRequest->method = HTTP_POST;
    } else if (strcmp(method, "PUT") == 0) 
    {
        httpRequest->method = HTTP_PUT;
    } else 
    {
        httpRequest->method = HTTP_UNSUPPORTED;
    }

    char *bodyStart = strstr(request, "\r\n\r\n");
    if (bodyStart) 
    {
        strcpy(httpRequest->body, bodyStart + 4);
    }

    // cazul default/unknown
    strcpy(httpRequest->contentType, "text/plain");

    char *line = strtok(request, "\r\n");
    while (line != NULL) {
        if (strstr(line, "Content-Type:") == line) 
        {
            // extragem content type-u
            char *contentType = line + strlen("Content-Type: ");
            while (isspace(*contentType)) contentType++; // fara spatii albe
            strncpy(httpRequest->contentType, contentType, sizeof(httpRequest->contentType));
            httpRequest->contentType[sizeof(httpRequest->contentType) - 1] = '\0';
            break;
        }
        line = strtok(NULL, "\r\n");
    }
}

void HandleGetRequest(const HttpRequest *httpRequest, int client_socket) 
{
    FILE *logFile = fopen("server_log.txt", "a");
    if (logFile == NULL) {
        perror("Unable to open log file");
        return;
    }

    struct timeval start, end;
    long bytes_sent = 0;
    gettimeofday(&start, NULL);

    char filePath[256];
    
    snprintf(filePath, sizeof(filePath), "www%s", httpRequest->path);
    fprintf(logFile, "Handling GET request for: %s\n", httpRequest->path);

    FILE *file = fopen(filePath, "rb");
    if (file == NULL) 
    {
        char response[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found.";
        ssize_t result = write(client_socket, response, strlen(response));
        if (result < 0) {
            fprintf(logFile, "Error writing to socket: %s\n", strerror(errno));
        }
        bytes_sent += result;
    } 
    else 
    {
        const char* mimeType = GetMimeType(filePath);
        char responseHeader[1024];
      

        int header_len = snprintf(responseHeader, sizeof(responseHeader), "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", mimeType);
        ssize_t result = write(client_socket, responseHeader, header_len);
        if (result < 0) {
            fprintf(logFile, "Error writing header to socket: %s\n", strerror(errno));
            fclose(file);
            fclose(logFile);
            return;
        }
        bytes_sent += result;

        char buffer[1024];
        size_t bytesRead;
        while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) 
        {
            result = send(client_socket, buffer, bytesRead, 0);
            if (result < 0) {
                fprintf(logFile, "Error sending to socket: %s\n", strerror(errno));
                fclose(file);
                fclose(logFile);
                return;
            }
            bytes_sent += result;
        }
    
        fclose(file);
    }

    gettimeofday(&end, NULL);
    double time_taken = (end.tv_sec - start.tv_sec) * 1e6;
time_taken = (time_taken + (end.tv_usec - start.tv_usec)) * 1e-6;
double throughput = bytes_sent / time_taken;

fprintf(logFile, "Bytes Sent: %ld, Time Taken: %f seconds, Throughput: %f bytes/sec\n", bytes_sent, time_taken, throughput);
fprintf(logFile, "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-");
fclose(logFile);

}


void HandleHeadRequest(const HttpRequest *httpRequest, int client_socket) 
{
    char filePath[256];
    snprintf(filePath, sizeof(filePath), "www%s", httpRequest->path);
    printf("Handling HEAD request for: %s\n", httpRequest->path);

    FILE *file = fopen(filePath, "r");
    if (file == NULL) 
    {
        char response[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n";
        write(client_socket, response, strlen(response));
    } 
    else 
    {
        const char* mimeType = GetMimeType(filePath);
        char responseHeader[1024];
        snprintf(responseHeader, sizeof(responseHeader), "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", mimeType);
        write(client_socket, responseHeader, strlen(responseHeader));
        fclose(file);
    }
}

void HandlePostRequest(const HttpRequest *httpRequest, int client_socket) 
{
     char filePath[256];
    snprintf(filePath, sizeof(filePath), "www%s", httpRequest->path);
    printf("Handling POST request for: %s\n", httpRequest->path);

    if (strcmp(httpRequest->contentType, "application/json") == 0) {
        FILE *file = fopen(filePath, "a");
        if (file == NULL) {
            char response[] = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
            write(client_socket, response, strlen(response));
        } else {
            fputs(httpRequest->body, file);
            fclose(file);

            char response[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nJSON data appended.";
            write(client_socket, response, strlen(response));
        }
    } else
    {
        char filePath[256];
        snprintf(filePath, sizeof(filePath), "www%s", httpRequest->path);
        printf("Handling POST request for: %s\n", httpRequest->path);

        FILE *file = fopen(filePath, "a");
        if (file == NULL) 
        {
            char response[] = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
            write(client_socket, response, strlen(response));
        } 
        else 
        {
            fputs(httpRequest->body, file);
            fclose(file);

            char response[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nData appended.";
            write(client_socket, response, strlen(response));
        }
    }
}

void HandlePutRequest(const HttpRequest *httpRequest, int client_socket) 
{
     char filePath[256];
    snprintf(filePath, sizeof(filePath), "www%s", httpRequest->path);
    printf("Handling PUT request for: %s\n", httpRequest->path);

    if (strcmp(httpRequest->contentType, "application/json") == 0) {
        FILE *file = fopen(filePath, "w");
        if (file == NULL) {
            char response[] = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
            write(client_socket, response, strlen(response));
        } else {
            fputs(httpRequest->body, file);
            fclose(file);

            char response[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nJSON data stored.";
            write(client_socket, response, strlen(response));
        }
    } else
     {
        char filePath[256];
        snprintf(filePath, sizeof(filePath), "www%s", httpRequest->path);
        printf("Handling PUT request for: %s\n", httpRequest->path);

        FILE *file = fopen(filePath, "w");
        if (file == NULL) 
        {
            char response[] = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
            write(client_socket, response, strlen(response));
        } 
        else 
        {
            fputs(httpRequest->body, file);
            fclose(file);

            char response[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nData stored.";
            write(client_socket, response, strlen(response));
        }
    }
}
