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

struct RoutineArgs{
    int idx;
    int num_of_blocks;
    int num_of_blocks_last; // improve this
    int client_fd;
    char *filename;
};

void* setup(void* args);
void* routine(void* args);

int main()
{
    int server_fd, opt = 1, client_fd, total_num_of_blocks, num_of_blocks_per_chunk, num_of_blocks_per_chunk_last;
    long file_size;
    char filename[1024] =  { 0 };
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
    strcat(filename, setupArgs.filename);

    // calculating total numbers of blocks to be read
    total_num_of_blocks = ceil(file_size/(float)BLOCKSIZE);

    // calculating numbers of blocks to be read per chunk
    num_of_blocks_per_chunk = total_num_of_blocks/NUMTHREAD;
    num_of_blocks_per_chunk_last = total_num_of_blocks/NUMTHREAD+total_num_of_blocks%NUMTHREAD;

    printf("Total number of blocks: %d\n",total_num_of_blocks);
    printf("Per chunk: %d\n", num_of_blocks_per_chunk);
    printf("Per chunk last: %d\n\n", num_of_blocks_per_chunk_last);

    // Creating multiple threads
    struct RoutineArgs routineArgs[NUMTHREAD];
    pthread_t th[NUMTHREAD];

    for(int i=0; i<NUMTHREAD; i++){
        routineArgs[i].client_fd = client_fd;
        routineArgs[i].filename = filename;
        routineArgs[i].idx = i;
        routineArgs[i].num_of_blocks = num_of_blocks_per_chunk;
        if(i!=NUMTHREAD-1){
            routineArgs[i].num_of_blocks_last = num_of_blocks_per_chunk;
        }else{
            routineArgs[i].num_of_blocks_last = num_of_blocks_per_chunk_last;
        }

        if(pthread_create(&th[i], NULL, &routine, &routineArgs[i]) !=0) {
            perror("Thread creation error");
            exit(-1);
        }
    }

    for(int i=0; i<NUMTHREAD; i++){
        if(pthread_join(th[i], NULL) !=0) {
            perror("Thread joining error");
            exit(-1);
        }
    }

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

void* routine(void* args){
    int client_fd, valread, bytes_read;
    char buffer[BLOCKSIZE] = { 0 };
    char filename[200];
    char temp[200];
    
    struct RoutineArgs *routineArgs = (struct RoutineArgs*)args;
    client_fd = routineArgs->client_fd;
    strcpy(filename, routineArgs->filename);

    // Reading hello from client
    valread = read(client_fd, buffer, 1024);
    printf("%s\n", buffer);
    memset(buffer,'\0',sizeof(buffer));

    // Sending index of block
    char idx_str[10];
    memset(idx_str,'\0',10);
    sprintf(idx_str,"%09d",routineArgs->idx);
    send(client_fd,idx_str,strlen(idx_str),0);

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
        send(client_fd,buffer, bytes_read, 0);
        // printf("%d - %s\n",routineArgs->idx, buffer);
        memset(buffer,'\0',sizeof(buffer)); // try this after commenting
    }

    return NULL;
}