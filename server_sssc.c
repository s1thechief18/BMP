#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#include "config.h"

struct SetupArgs{
    int client_fd;
    long file_size;
    char *filename;
}; 

void* setup(void* args);

int main()
{
    int server_fd, opt = 1, client_fd, file_size;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Socket creation failed!");
        exit(-1);
    }

    // Forcefully attaching socket to the port 8080
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))){
        perror("Setsockopt failed!");
        exit(-1);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Attaching socket to the port 8080
    if(bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0){
        perror("Bind failed");
        exit(-1);
    }

    // Server ONLINE
    if(listen(server_fd, 100) < 0){
        perror("Listen failed!");
        exit(-1);
    }

    printf("Server Online...\n\n\n");

    if((client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0){
        perror("Accept failed");
        exit(-1);
    }

    printf("Client connected successfully\n\n");

    // // Sending test message to client
    // char *buffer = "Hello";
    // send(client_fd, buffer, strlen(buffer), 0);

    // Setup
    struct SetupArgs setupArgs;
    setupArgs.client_fd = client_fd;

    setup(&setupArgs);
    file_size = setupArgs.file_size;

    // Closing the client socket
    close(client_fd);

    // Closing the listening socket
    shutdown(server_fd, SHUT_RDWR);
}


void* setup(void* args){
    int valread, client_fd, addrlen;
    long file_size=0;
    char buffer[1024] = { 0 };
    char temp[200] = { 0 };
    char socket_req[1024];
    
    struct SetupArgs *setupArgs = (struct SetupArgs*)args;
    client_fd = setupArgs->client_fd;
    file_size = setupArgs->file_size;

    // Receive filename to be send
    char filename[1024]={0};
    char *request_filename="Enter filename to be send:";
    send(client_fd, request_filename, strlen(request_filename), 0);
    printf("Waiting to receive filename...\n");
    valread = read(client_fd, filename, 1024);
    printf("Filename to be send: %s\n\n", filename);
    setupArgs->filename = filename;

    // Opening file
    strcpy(temp, INPUTPATH);
    strcat(temp, filename);

    FILE *file = fopen(temp,"rb");
    if(!file){
        perror("Error in opening input folder!!");
        exit(-1);
    }

    // Calculating the size of file
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    long cursor = 0;
    fseek(file, 0, SEEK_SET);
    printf("File size: %ld\n\n", file_size); 
    setupArgs->file_size=file_size;

    // sending file size
    sprintf(socket_req, "%ld", file_size);
    send(client_fd, socket_req, strlen(socket_req), 0);

    fclose(file);

    return NULL;
}