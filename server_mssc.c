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

struct RoutineArgs{
    int idx;
    int num_of_blocks;
    int num_of_blocks_last; // improve this
    int server_fd;
    struct sockaddr_in address;
    int addrlen;
    char *filename;
}; 

struct SetupArgs{
    int server_fd;
    struct sockaddr_in address;
    int addrlen;
    long file_size;
    char *filename;
}; 

void* routine(void* args){
    int new_socket, valread, server_fd, addrlen, bytes_read;
    char buffer[BLOCKSIZE] = { 0 };
    char filename[200];
    char temp[200];
    char *connected = "Client connected successfully.";
    struct sockaddr_in address;
    
    struct RoutineArgs *routineArgs = (struct RoutineArgs*)args;
    server_fd = routineArgs->server_fd;
    address = routineArgs->address;
    addrlen = routineArgs->addrlen;
    strcpy(filename, routineArgs->filename);


    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Accept failed");
        exit(-1);
    }

    // Reading hello from client
    valread = read(new_socket, buffer, 1024);
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

void* setup(void* args){
    int new_socket, valread, server_fd, addrlen;
    long file_size=0;
    char buffer[1024] = { 0 };
    char temp[200] = { 0 };
    char *connected = "Server connected successfully.";
    char socket_req[1024];
    struct sockaddr_in address;
    
    struct SetupArgs *setupArgs = (struct SetupArgs*)args;
    server_fd = setupArgs->server_fd;
    address = setupArgs->address;
    addrlen = setupArgs->addrlen;
    file_size = setupArgs->file_size;


    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Accept failed");
        exit(-1);
    }

    // server connected successfully
    send(new_socket, connected, strlen(connected), 0);


    valread = read(new_socket, buffer, 1024);
    printf("%s\n\n", buffer);

    // Receive filename to be send
    char filename[1024]={0};
    char *request_filename="Enter filename to be send:";
    send(new_socket, request_filename, strlen(request_filename), 0);
    printf("Waiting to receive filename...\n");
    valread = read(new_socket, filename, 1024);
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
    send(new_socket, socket_req, strlen(socket_req), 0);

    close(new_socket);
    fclose(file);

    return NULL;
}

int main()
{
    int server_fd, new_socket, valread, total_num_of_blocks, num_of_blocks_per_chunk, num_of_blocks_per_chunk_last;
    long file_size=0;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = { 0 };
    char *hello = "Hello from server";
    double time_spent = 0.0;
    clock_t begin, end;

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

    printf("Server Online...\n");

    struct SetupArgs setupArgs;
    setupArgs.server_fd = server_fd;
    setupArgs.address = address;
    setupArgs.addrlen = addrlen;

    // setup
    setup(&setupArgs);
    file_size = setupArgs.file_size;

    // calculating total numbers of blocks to be read
    total_num_of_blocks = ceil(file_size/(float)BLOCKSIZE);

    // calculating numbers of blocks to be read per chunk
    num_of_blocks_per_chunk = total_num_of_blocks/NUMSOCKET;
    num_of_blocks_per_chunk_last = total_num_of_blocks/NUMSOCKET+total_num_of_blocks%NUMSOCKET;

    // temp
    printf("File size: %ld\n", file_size);
    printf("Total number of blocks: %d\n",total_num_of_blocks);
    printf("Per chunk: %d\n", num_of_blocks_per_chunk);
    printf("Per chunk last: %d\n\n", num_of_blocks_per_chunk_last);


    // creating multiple sockets
    struct RoutineArgs routineArgs[NUMSOCKET];
    pthread_t th[NUMSOCKET];

    // start clock
    begin = clock();
    
    for(int i=0; i<NUMSOCKET; i++){
        routineArgs[i].server_fd = server_fd;
        routineArgs[i].address = address;
        routineArgs[i].addrlen = addrlen;
        routineArgs[i].filename = setupArgs.filename;
        routineArgs[i].idx = i;
        routineArgs[i].num_of_blocks = num_of_blocks_per_chunk;
        routineArgs[i].num_of_blocks_last = num_of_blocks_per_chunk_last;
        if(i!=NUMSOCKET-1){
            routineArgs[i].num_of_blocks_last = num_of_blocks_per_chunk;
        }else{
            routineArgs[i].num_of_blocks_last = num_of_blocks_per_chunk_last;
        }
        if(pthread_create(&th[i], NULL, &routine, &routineArgs[i]) !=0) {
            perror("Thread creation error");
            exit(-1);
        }
    }

    for(int i=0; i<NUMSOCKET; i++){
        if(pthread_join(th[i], NULL) !=0) {
            perror("Thread joining error");
            exit(-1);
        }
    }

    // end clock
    end = clock();

    // closing the listening socket
    shutdown(server_fd, SHUT_RDWR);

    //With shutdown, you will still be able to receive pending data the peer already sent
    // SHUT_RDWR will block both sending and receiving(like close)

    printf("\nTime: %f\n\n", (double)(end-begin)/CLOCKS_PER_SEC);

    printf("Server Offline...\n");

    return 0;
}
