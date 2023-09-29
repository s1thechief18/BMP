#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#define PORT 8080

struct SetupArgs{
    int num_of_socket;
}; 


void* routine(){
    int status, valread, client_fd;
    struct sockaddr_in serv_addr;
    char hello[200];
    char buffer[1024] = { 0 };

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error\n");
        exit(-1);
    }

    sprintf(hello, "Hello from client{sock_num: %d, thread_id: %lu}", client_fd, (unsigned long)pthread_self());

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Conver IPv4 to IPv6 addresses from text to binary form
    // if ( inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0 ) {
    //     perror("Invalid address");
    //     exit(-1);
    // }
    if ( inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0 ) {
        perror("Invalid address");
        exit(-1);
    }

    if ((status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
        perror("Connection failed");
        exit(-1);
    }

    send(client_fd, hello, strlen(hello), 0);

    // pthread_self gives currrent thread ID
    printf("Hello message send{sock_num: %d, thread_id: %lu}\n", client_fd, (unsigned long)pthread_self());

    // closing the connected socket
    close(client_fd);

    return NULL;
}

void* setup(void *args){
    int status, valread, client_fd, num_of_socket;
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
    if ( inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0 ) {
        perror("Invalid address");
        exit(-1);
    }

    if ((status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
        perror("Connection failed");
        exit(-1);
    }

    send(client_fd, connected, strlen(connected), 0);

    valread = read(client_fd, buffer, 1024);
    printf("%s\n\n", buffer);

    valread = read(client_fd, buffer, 1024);
    sscanf(buffer,"%d", &num_of_socket);
    setupArgs->num_of_socket = num_of_socket;

    printf("Number of sockets required: %d\n", num_of_socket);

    // closing the connected socket
    close(client_fd);

    return NULL;
}

int main()
{
    int status, valread, client_fd, num_of_socket=2;
    struct sockaddr_in serv_addr;
    char* hello = "Hello from client";
    char buffer[1024] = { 0 };
    pthread_t th[num_of_socket];

    struct SetupArgs setupArgs;

    // setup
    setup(&setupArgs);
    num_of_socket = setupArgs.num_of_socket;
    printf("Main:- Number of sockets required: %d\n", num_of_socket);

    for(int i=0; i<num_of_socket; i++){
        if(pthread_create(&th[i], NULL, &routine, NULL) !=0 ){
            perror("Thread creation error");
            exit(-1);
        }
    }

    for(int i=0; i<num_of_socket; i++){
        if(pthread_join(th[i], NULL) != 0){
            perror("Thread joining error");
            exit(-1);
        }
    }
}

