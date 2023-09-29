#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>

#define PORT 8080
#define BLOCKSIZE 10

struct RoutineArgs{
    int idx;
    int server_fd;
    struct sockaddr_in address;
    int addrlen;
    char *buffer;
    char *filename;
}; 

struct SetupArgs{
    int server_fd;
    struct sockaddr_in address;
    int addrlen;
    int num_of_socket;
    char *filename;
}; 

void* routine(void* args){
    int new_socket, valread, server_fd, addrlen;
    char buffer[1024] = { 0 };
    char *connected = "Client connected successfully.";
    struct sockaddr_in address;
    
    struct RoutineArgs *routineArgs = (struct RoutineArgs*)args;
    server_fd = routineArgs->server_fd;
    address = routineArgs->address;
    addrlen = routineArgs->addrlen;


    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Accept failed");
        exit(-1);
    }

    valread = read(new_socket, buffer, 1024);
    printf("%s\n", buffer);
    memset(buffer,'\0',sizeof(buffer));

    // Sending index of block
    char idx_str[10];
    memset(idx_str,'\0',10);
    sprintf(idx_str,"%09d",routineArgs->idx);
    send(new_socket,idx_str,strlen(idx_str),0);

    // Opening file
    FILE *file = fopen(routineArgs->filename,"rb");
    // Seting cursor
    fseek(file,(routineArgs->idx)*BLOCKSIZE,SEEK_SET);
    // Reading Block
    int bytes_read = fread(routineArgs->buffer, sizeof(char), BLOCKSIZE, file);

    // sending Block
    send(new_socket,routineArgs->buffer, bytes_read, 0);

    close(new_socket);

    return NULL;
}

void* setup(void* args){
    int new_socket, valread, server_fd, addrlen, num_of_socket;
    char buffer[1024] = { 0 };
    char *connected = "Server connected successfully.";
    char socket_req[1024];
    struct sockaddr_in address;
    
    struct SetupArgs *setupArgs = (struct SetupArgs*)args;
    server_fd = setupArgs->server_fd;
    address = setupArgs->address;
    addrlen = setupArgs->addrlen;
    num_of_socket = setupArgs->num_of_socket;


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
    FILE *file = fopen(filename,"rb");

    if(file==NULL){
        printf("Unable to open file!\n");
        return NULL;
    }

    // Calculating the size of file
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    long cursor = 0;
    fseek(file, 0, SEEK_SET);
    printf("File size: %ld\n\n", file_size); 

    // Calculating number of sockets required
    num_of_socket = ceil(file_size/(float)BLOCKSIZE);
    setupArgs->num_of_socket = num_of_socket;

    // sending number of sockets required
    sprintf(socket_req, "%d", num_of_socket);
    send(new_socket, socket_req, strlen(socket_req), 0);

    close(new_socket);
    fclose(file);

    return NULL;
}

int main()
{
    int server_fd, new_socket, valread, num_of_socket=2;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = { 0 };
    char *hello = "Hello from server";

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
    setupArgs.num_of_socket = num_of_socket;

    // setup
    setup(&setupArgs);
    num_of_socket = setupArgs.num_of_socket;
    printf("Number of sockets required: %d\n", num_of_socket);

    // creating array of buffer
    char buffers[num_of_socket][BLOCKSIZE];
    for(int i=0; i<num_of_socket; i++){
        memset(buffers[i],'\0',sizeof(buffers[i]));
    }

    struct RoutineArgs routineArgs[num_of_socket];
    

    pthread_t th[num_of_socket];

    for(int i=0; i<num_of_socket; i++){
        routineArgs[i].server_fd = server_fd;
        routineArgs[i].address = address;
        routineArgs[i].addrlen = addrlen;
        routineArgs[i].filename = setupArgs.filename;
        routineArgs[i].idx = i;
        routineArgs[i].buffer = buffers[i];
        if(pthread_create(&th[i], NULL, &routine, &routineArgs[i]) !=0) {
            perror("Thread creation error");
            exit(-1);
        }
    }

    for(int i=0; i<num_of_socket; i++){
        if(pthread_join(th[i], NULL) !=0) {
            perror("Thread joining error");
            exit(-1);
        }
    }

    // closing the listening socket
    shutdown(server_fd, SHUT_RDWR);

    //With shutdown, you will still be able to receive pending data the peer already sent
    // SHUT_RDWR will block both sending and receiving(like close)

    printf("Server Offline...\n");

    return 0;
}
