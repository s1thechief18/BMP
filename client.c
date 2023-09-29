#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>

#define PORT 8080

int main()
{
    int status, valread, client_fd;
    struct sockaddr_in serv_addr;
    char* hello = "Hello from client";
    char buffer[1024] = { 0 };

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error\n");
        exit(-1);
    }
    

    printf("Client fd: %d\n", client_fd);

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
    printf("Hello message send\n");
    valread = read(client_fd, buffer, 1024);
    printf("%s\n", buffer);

    // closing the connected socket
    close(client_fd);
}

