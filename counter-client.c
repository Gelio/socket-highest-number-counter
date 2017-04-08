#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <netinet/tcp.h>
#include <unistd.h>

#define PORT "5678"

#define ERR(source) ( fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), \
                        perror(source), exit(EXIT_FAILURE) )

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
    
    printf("Connected");
    
    uint32_t number = 1 + rand() % 1000;
    number = htonl(number);
    if (write(socketFd, (char*)&number, sizeof(uint32_t)) < 0)
        ERR("write");
    
    printf("Number %d sent, closing connection\n", ntohl(number));

    if (close(socketFd) < 0)
        ERR("close");
    printf("Exiting\n");
    return EXIT_SUCCESS;
}