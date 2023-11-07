#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include "config2.h"

int main(){
    int worker_fd, status, idx, valread, valsend, server_fd, opt = 1, total_num_of_blocks, num_of_blocks_per_chunk;
    long file_size;
    struct sockaddr_in serv_addr, address;
    char buffer[1024] = "Hello from worker", filename[200] = { 0 }, temp[200] = { 0 }, worker_ip[INET_ADDRSTRLEN];
    

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
    valread = read(worker_fd, buffer, 1024);
    sscanf(buffer, "%d", &idx);
    printf("\n\nWorker ID: %d\n", idx);
    memset(buffer, '\0', sizeof(buffer));

    // Send Hello to server
    sprintf(buffer, "Hello from worker-%d", idx+1);
    valsend = send(worker_fd, buffer, strlen(buffer), 0);
    memset(buffer, '\0', sizeof(buffer));

    // Wait for message from server to start worker as server
    printf("Waiting for server to start server...\n");
    valread = read(worker_fd, filename, 1024);
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

    inet_ntop(AF_INET, &address, worker_ip, INET_ADDRSTRLEN);

    printf("\nWorker Server Online(%s)...\n\n", worker_ip);

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
    if( idx == WORKERS - 1)
        num_of_blocks_per_chunk = total_num_of_blocks/WORKERS;
    else
        num_of_blocks_per_chunk = total_num_of_blocks/WORKERS+total_num_of_blocks%WORKERS;

    printf("File size: %ld\n", file_size);
    printf("Total number of blocks: %d\n",total_num_of_blocks);
    printf("Per chunk: %d\n", num_of_blocks_per_chunk);

    // Closing the listening socket
    shutdown(server_fd, SHUT_RDWR);
    printf("\nWorker Server Offline...\n");
}