#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include "counter-common.h"

#define PORT 5678
#define BACKLOG 10
#define RETRIES 3


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
    uint32_t highestNumber = 0;
    int totalNumbersSent = 0;

    while (1)
    {
        printf("Accepting clients\n");

        int clientFd = 0;
        if ((clientFd = accept(socketFd, NULL, NULL)) < 0)
            ERR("accept");
        
        printf("\tClient connected, reading numbers\n");

        while (1)
        {
            uint32_t number;
            if (networkReadNumber(clientFd, &number) < 0)
            {
                printf("Client disconnected\n");
                break;
            }
            totalNumbersSent++;
            
            printf("\tReceived %d from client\n", number);

            if (number > highestNumber)
            {
                highestNumber = number;
                printf("\tNew highest number\n");
            }
            
            printf("\tResponding with %d\n", highestNumber);

            if (networkWriteNumber(clientFd, highestNumber) < 0)
                ERR("write");
        }
        
        printf("Closing client socket\n");
        if (close(clientFd) < 0)
            ERR("close");
    }

    printf("Closing server\n");
    if (close(socketFd) < 0)
        ERR("close");
    printf("Exiting\n");
    return EXIT_SUCCESS;
}