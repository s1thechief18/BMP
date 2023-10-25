#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include "config.h"

struct SetupArgs{
    int client_fd;
    char *filename;
    long file_size;
};

void* setup(void *args);

int main()
{
    int client_fd, status;
    long file_size;
    char filename[1024] = { 0 };
    struct sockaddr_in serv_addr;

    // Creating socket file descriptor
    if((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Socket creation failed!");
        exit(-1);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // ********** Server IP Address here **********
    if( inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0 ){
        perror("Invalid address");
        exit(-1);
    }

    if ((status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
        perror("Connection failed");
        exit(-1);
    }

    printf("Server connected successfully\n\n");

    // // Reading test message from server
    // char buffer[1024];
    // int valread;
    // valread = read(client_fd, buffer, 1024);
    // printf("Message from server: %s\n\n", buffer);

    // Setup
    struct SetupArgs setupArgs;
    setupArgs.client_fd = client_fd;

    setup(&setupArgs);
    file_size = setupArgs.file_size;
    strcpy(filename,setupArgs.filename);
    printf("\nFile size: %ld\n\n", file_size);

    // Closing the socket
    close(client_fd);
}


void* setup(void *args){
    int status, valread, client_fd;
    long file_size=0;
    char buffer[1024] = { 0 };
    struct SetupArgs *setupArgs = (struct SetupArgs*)args;
    client_fd = setupArgs->client_fd;

    // Enter filename to be send
    char filename[1024];
    memset(buffer,'\0',sizeof(buffer));
    valread = read(client_fd, buffer, 1024);
    printf("%s\n", buffer);
    scanf("%s", filename);
    send(client_fd, filename, strlen(filename), 0);

    // Recieve file size
    valread = read(client_fd, buffer, 1024);
    sscanf(buffer,"%ld", &file_size);
    setupArgs->file_size = file_size;
    setupArgs->filename = filename;

    return NULL;
}
