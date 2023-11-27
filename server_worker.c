#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include "config2.h"

struct RoutineArgs{
    int idx;
    int num_of_blocks;
    int num_of_blocks_last; // improve this
    int new_socket;
    char filename[200];
};

void* routine(void* args);

int main(){
    int worker_fd, client_fd, status, idx, valread, valsend, server_fd, opt = 1, total_num_of_blocks, num_of_blocks_per_chunk, num_of_blocks_per_chunk_last;
    long file_size;
    struct sockaddr_in serv_addr, address;
    int addrlen = sizeof(address);
    char buffer[BUFFERSIZE] = "Hello from worker", filename[200] = { 0 }, temp[200] = { 0 }, worker_ip[INET_ADDRSTRLEN];
    

    if ((worker_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error\n");
        exit(-1);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // ********** Server IP Address here **********
    if ( inet_pton(AF_INET, SERVERIP, &serv_addr.sin_addr) <= 0 ) {
        perror("Invalid address");
        exit(-1);
    }

    if ((status = connect(worker_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
        perror("Connection failed");
        exit(-1);
    }

    printf("Server connected successfully!\n");

    // Read worker no.
    valread = read(worker_fd, buffer, BUFFERSIZE);
    sscanf(buffer, "%d", &idx);
    printf("\n\nWorker ID: %d\n", idx);
    memset(buffer, '\0', sizeof(buffer));
    valsend = send(worker_fd, "Go", 2, 0);
    valread = read(worker_fd, buffer, BUFFERSIZE);
    strcpy(worker_ip, buffer);
    memset(buffer, '\0', sizeof(buffer));
    printf("Worker IP: %s\n\n", worker_ip);

    // Send Hello to server
    sprintf(buffer, "Hello from worker-%d", idx+1);
    valsend = send(worker_fd, buffer, strlen(buffer), 0);
    memset(buffer, '\0', sizeof(buffer));

    // Wait for message from server to start worker as server
    printf("Waiting for server to start server...\n");
    valread = read(worker_fd, filename, BUFFERSIZE);
    close(worker_fd);

    // Give enough time to server to shut down properly
    sleep(2);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Socket failed");
        exit(-1);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR , &opt, sizeof(opt))) {
        perror("Setsockopt");
        exit(-1);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(-1);
    }

    // Server Online
    if (listen(server_fd, 100) < 0) {
        perror("Listen failed");
        exit(-1);
    }

    printf("\nWorker Server Online...\n\n");

    // Calculating filesize
    printf("Filename: %s\n", filename);
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

    // Calculating total numbers of blocks to be read
    total_num_of_blocks = ceil(file_size/(float)BLOCKSIZE);

    // Calculating numbers of blocks to be read per chunk
    num_of_blocks_per_chunk = total_num_of_blocks/WORKERS;
    num_of_blocks_per_chunk_last = total_num_of_blocks/WORKERS+total_num_of_blocks%WORKERS;

    printf("File size: %ld\n", file_size);
    printf("Total number of blocks: %d\n",total_num_of_blocks);
    printf("Per chunk: %d\n", num_of_blocks_per_chunk);
    printf("Per chunk last: %d\n", num_of_blocks_per_chunk_last);


    // Waiting for client to accept the connection
    if ((client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(-1);
    }

    printf("\nClient connected successfully\n");

    // Transfer file to client
    struct RoutineArgs routineArgs;
    routineArgs.new_socket = client_fd;
    strcpy(routineArgs.filename, filename);
    routineArgs.idx = idx;
    routineArgs.num_of_blocks = num_of_blocks_per_chunk;
    routineArgs.num_of_blocks_last = num_of_blocks_per_chunk_last;
    routine(&routineArgs);

    // Closing the listening socket
    shutdown(server_fd, SHUT_RDWR);
    printf("\nWorker Server Offline...\n");
}

void* routine(void* args){
    int new_socket, valread, bytes_read;
    char buffer[BLOCKSIZE] = { 0 };
    char filename[200];
    char temp[200];
    char *connected = "Client connected successfully.";
    
    struct RoutineArgs *routineArgs = (struct RoutineArgs*)args;
    new_socket = routineArgs->new_socket;
    strcpy(filename, routineArgs->filename);

    // Reading hello from client
    valread = read(new_socket, buffer, BUFFERSIZE);
    // printf("%s\n", buffer);
    memset(buffer,'\0',sizeof(buffer));

    // Sending index of block
    char idx_str[10];
    memset(idx_str,'\0',10);
    sprintf(idx_str,"%09d",routineArgs->idx);
    send(new_socket,idx_str,strlen(idx_str),0);

    // Opening file
    strcpy(temp, INPUTPATH);
    strcat(temp, filename);

    FILE *file = fopen(temp,"rb");
    if(!file){
        perror("Error in opening input folder!");
        exit(-1);
    }

    // Seting cursor
    fseek(file,(routineArgs->idx)*BLOCKSIZE*(routineArgs->num_of_blocks),SEEK_SET);

    // Reading Block and sending block
    for(int i=0; i<(routineArgs->num_of_blocks_last); i++){
        bytes_read = fread(buffer, sizeof(char), BLOCKSIZE, file);
        send(new_socket,buffer, bytes_read, 0);
        // printf("%d - %s\n",routineArgs->idx, buffer);
        memset(buffer,'\0',sizeof(buffer)); // try this after commenting
    }

    close(new_socket);

    return NULL;
}