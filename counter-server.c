#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define PORT 5678
#define BACKLOG 10

#define ERR(source) ( fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), \
                        perror(source), exit(EXIT_FAILURE) )

int main(int argc, char **argv)
{
    int socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd < 0)
        ERR("socket");
    
    struct sockaddr_in tcpAddress;
    memset(&tcpAddress, 0, sizeof(struct sockaddr_in));
    tcpAddress.sin_family = AF_INET;
    tcpAddress.sin_port = htons(PORT);
    tcpAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(socketFd, &tcpAddress, sizeof(struct sockaddr_in)) < 0)
        ERR("bind");
    
    if (listen(socketFd, BACKLOG) < 0)
        ERR("listen");
    
    printf("Listening for connections on %d\n", PORT);

    int clientFd = 0;
    if ((clientFd = accept(socketFd, NULL, NULL)) < 0)
        ERR("accept");
    
    printf("Client connected\n");

    uint32_t number;
    if (read(clientFd, (char*)&number, sizeof(uint32_t)) < 0)
        ERR("read");
    
    number = ntohl(number);
    printf("Received %d from client. Closing connection\n", number);

    if (close(clientFd) < 0)
        ERR("close");

    printf("Closing server\n");
    if (close(socketFd) < 0)
        ERR("close");
    printf("Exiting\n");
    return EXIT_SUCCESS;
}