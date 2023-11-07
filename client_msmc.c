#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include "config2.h"

struct SetupArgs{
    char *filename;
    long file_size;
};

void* setup(void *args);

int main(){
    int status, valread, client_fd, total_num_of_blocks, num_of_blocks_per_chunk, num_of_blocks_per_chunk_last;
    long file_size = 0;
    struct sockaddr_in serv_addr;
    char* hello = "Hello from client";
    char buffer[1024] = { 0 };
    char filename[1024] = { 0 };

    // Setup
    struct SetupArgs setupArgs;
    setup(&setupArgs);
    file_size = setupArgs.file_size;
    strcpy(filename,setupArgs.filename);
    printf("File size: %ld\n\n", file_size);
}

void* setup(void *args){
    int status, valread, client_fd;
    long file_size=0;
    struct sockaddr_in serv_addr;
    char *connected="Client connected successfully.";
    char buffer[1024] = { 0 };
    struct SetupArgs *setupArgs = (struct SetupArgs*)args;

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error\n");
        exit(-1);
    }


    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Conver IPv4 to IPv6 addresses from text to binary form
    // if ( inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0 ) {
    //     perror("Invalid address");
    //     exit(-1);
    // }

    // ********** Server IP Address here **********
    if ( inet_pton(AF_INET, SERVERIP, &serv_addr.sin_addr) <= 0 ) {
        perror("Invalid address");
        exit(-1);
    }

    if ((status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
        perror("Connection failed");
        exit(-1);
    }

    // client connected successfully
    send(client_fd, connected, strlen(connected), 0);

    valread = read(client_fd, buffer, 1024);
    printf("%s\n\n", buffer);

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

    // closing the connected socket
    close(client_fd);
    
    return NULL;
}