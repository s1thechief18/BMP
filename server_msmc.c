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
    int server_fd;
    struct sockaddr_in address;
    int addrlen;
    long file_size;
    char filename[200];
    char workerAddrs[WORKERS][INET_ADDRSTRLEN];
}; 

struct SetupWorkersArgs{
    int worker_fd;
    int idx;
    char worker_ip[INET_ADDRSTRLEN];
};

void* setup(void* args);
void* setup_workers(void* args);

int main(){
    int server_fd, worker_fd, valsend;
    long file_size = 0;
    struct sockaddr_in address;
    int opt = 1;
    char buffer[BUFFERSIZE] = { 0 }, filename[200] = { 0 };
    int addrlen = sizeof(address);

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

    // Setup workers
    struct SetupWorkersArgs setupWorkersArgs[WORKERS];
    char workerAddrs[WORKERS][INET_ADDRSTRLEN];
    int worker_fds[WORKERS];
    pthread_t th[WORKERS];
    for(int i=0; i<WORKERS; i++){
        if ((worker_fds[i] = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(-1);
        }

        // Extract IP address of worker from address
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &address.sin_addr, client_ip, INET_ADDRSTRLEN);
        printf("\nWorker-%d IP address: %s\n", i+1, client_ip);
        strcpy(workerAddrs[i], client_ip);

        setupWorkersArgs[i].worker_fd = worker_fds[i];
        strcpy(setupWorkersArgs[i].worker_ip, client_ip);
        // strcpy(setupWorkersArgs[i].filename, setupArgs.filename);
        setupWorkersArgs[i].idx = i; 

        if(pthread_create(&th[i], NULL, &setup_workers, &setupWorkersArgs[i]) !=0 ){
            perror("Thread creation error");
            exit(-1);
        }
    }

    for(int i=0; i<WORKERS; i++){
        if(pthread_join(th[i], NULL) != 0){
            perror("Thread joining error");
            exit(-1);
        }
    }

    printf("\n\n");

    struct SetupArgs setupArgs;
    setupArgs.server_fd = server_fd;
    setupArgs.address = address;
    setupArgs.addrlen = addrlen;
    for(int i=0; i<WORKERS; i++)
        strcpy(setupArgs.workerAddrs[i], workerAddrs[i]);

    // Setup client
    setup(&setupArgs);
    file_size = setupArgs.file_size;
    strcpy(filename, setupArgs.filename); 

    // Send message to worker to start worker as server
    for(int i=0; i<WORKERS; i++){
        valsend = send(worker_fds[i], filename, strlen(filename), 0);
    }


    // closing the listening socket
    shutdown(server_fd, SHUT_RDWR);

    printf("Server Offline...\n");
}

void* setup(void* args){
    int new_socket, valread, server_fd, addrlen;
    long file_size=0;
    char buffer[BUFFERSIZE] = { 0 };
    char temp[200] = { 0 };
    char *connected = "Server connected successfully.";
    char socket_req[BUFFERSIZE];
    struct sockaddr_in address;
    char workerAddrsStr[BUFFERSIZE] = { 0 };
    
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

    // client connect successfully
    valread = read(new_socket, buffer, BUFFERSIZE);
    printf("%s\n\n", buffer);

    // Receive filename to be send
    char filename[BUFFERSIZE]={0};
    char *request_filename="Enter filename to be send:";
    send(new_socket, request_filename, strlen(request_filename), 0);
    printf("Waiting to receive filename...\n");
    valread = read(new_socket, filename, BUFFERSIZE);
    printf("Filename to be send: %s\n\n", filename);
    strcpy(setupArgs->filename, filename);

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

    // sending workeraddrs
    valread = read(new_socket, buffer, BUFFERSIZE);
    for(int i=0; i<WORKERS; i++){
        strcat(workerAddrsStr, setupArgs->workerAddrs[i]);
        if(i != WORKERS-1)
            strcat(workerAddrsStr," ");
    }
    send(new_socket, workerAddrsStr, strlen(workerAddrsStr), 0); 

    close(new_socket);
    fclose(file);

    return NULL;
}

void* setup_workers(void* args){
    int worker_fd, idx, valread, valsend;
    char filename[200];
    char buffer[BUFFERSIZE];
    char worker_ip[INET_ADDRSTRLEN];

    struct SetupWorkersArgs *setupWorkersArgs = (struct SetupWorkersArgs*)args;
    worker_fd = setupWorkersArgs->worker_fd;
    idx =  setupWorkersArgs->idx;
    strcpy(worker_ip, setupWorkersArgs->worker_ip);

    // Send worker no.
    sprintf(buffer, "%d\n", idx);
    valsend = send(worker_fd, buffer, strlen(buffer), 0);
    memset(buffer, '\0', sizeof(buffer));
    valread = read(worker_fd, buffer, 2);
    memset(buffer, '\0', sizeof(buffer));
    strcpy(buffer, worker_ip);
    valsend = send(worker_fd, buffer, strlen(buffer), 0);
    memset(buffer, '\0', sizeof(buffer));

    // Read Hello from worker
    valread = read(worker_fd, buffer, BUFFERSIZE);
    printf("%s\n", buffer);

    return NULL;
}