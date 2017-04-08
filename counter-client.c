#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include "counter-common.h"

#define PORT "5678"
#define RETRIES 3
#define SLEEP_DURATION 2

void usage(char *fileName)
{
    fprintf(stderr, "Usage: %s address\n", fileName);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    if (argc != 2)
        usage(argv[0]);
    char *serverAddress = argv[1];

    srand(time(NULL));

    struct addrinfo hints = {};
    hints.ai_family = AF_INET;
    struct addrinfo* addrInfo;
    if (getaddrinfo(serverAddress, PORT, &hints, &addrInfo) < 0)
        ERR("getaddrinfo");
    
    struct sockaddr_in tcpAddress = *(struct sockaddr_in*)addrInfo[0].ai_addr;
    freeaddrinfo(addrInfo);

    int socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd < 0)
        ERR("socket");

    printf("Connecting to the server at %s (%d)\n", serverAddress, ntohl(tcpAddress.sin_addr.s_addr));
    if (connect(socketFd, &tcpAddress, sizeof(struct sockaddr_in)) < 0)
        ERR("connect");
    
    printf("Connected\n");
    
    for (int i=0; i < RETRIES; i++)
    {
        printf("Try %d\n", i + 1);

        uint32_t initialNumber = 1 + rand() % 1000;
        printf("\tSending %d to the server\n", initialNumber);
        if (networkWriteNumber(socketFd, initialNumber) < 0)
        {
            printf("\tCannot send number\n");
            break;
        }
        
        uint32_t responseNumber;
        if (networkReadNumber(socketFd, &responseNumber) < 0)
        {
            printf("\tCannot read number, connection broken\n");
            break;
        }

        if (responseNumber == initialNumber)
            printf("\tHIT!\n");
        else
            printf("\tReceived %d from the server\n", responseNumber);
        
        if (i != RETRIES - 1)
        {
            printf("Sleeping %d seconds\n", SLEEP_DURATION);
            sleep(SLEEP_DURATION);
        }
    }

    printf("%d retries reached\n", RETRIES);
    if (close(socketFd) < 0)
        ERR("close");
    printf("Exiting\n");
    return EXIT_SUCCESS;
}