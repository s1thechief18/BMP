#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 8080

struct RoutineArgs{
    int server_fd;
    struct sockaddr_in address;
    int addrlen;
}; 

void* routine(void* args){
    int new_socket, valread, server_fd, addrlen;
    char buffer[1024] = { 0 };
    char *hello = "Hello from server";
    struct sockaddr_in address;
    
    struct RoutineArgs *routineArgs = (struct RountineArgs*)args;
    server_fd = routineArgs->server_fd;
    address = routineArgs->address;
    addrlen = routineArgs->addrlen;


    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Accept failed");
        exit(-1);
    }
    

    valread = read(new_socket, buffer, 1024);
    printf("%s\n", buffer);

    // send(new_socket, hello, strlen(hello), 0);
    // printf("Hello message sent\n");

    // closing the connected socket
    close(new_socket);

    return NULL;
}

int main()
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = { 0 };
    char *hello = "Hello from server";
    pthread_t th[10];

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

    struct RoutineArgs routineArgs;
    routineArgs.server_fd = server_fd;
    routineArgs.address = address;
    routineArgs.addrlen = addrlen;

    for(int i=0; i<10; i++){
        if(pthread_create(&th[i], NULL, &routine, &routineArgs) !=0) {
            perror("Thread creation error");
            exit(-1);
        }
    }

    for(int i=0; i<10; i++){
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