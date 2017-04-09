#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>
#include <errno.h>
#include "counter-common.h"

#define PORT 5678
#define BACKLOG 10
#define RETRIES 3

void setSignalHandler(int signal, void (*handler)(int))
{
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = handler;
    if (sigaction(signal, &action, NULL) < 0)
        ERR("sigaction");
}

volatile sig_atomic_t shouldQuit = 0;
void handleSigInt(int signal)
{
    shouldQuit = 1;
}

typedef struct client_t {
    int fd;
    struct client_t *next;
} client_t;

void removeClient(client_t **clientHead, client_t **clientTail, client_t *client)
{
    if (client == *clientHead)
    {
        *clientHead = client->next;
        if (client == *clientTail)  // client == head == tail
            *clientTail = NULL;
    }
    else
    {
        client_t *currentClient = *clientHead;
        while (currentClient != NULL)
        {
            if (currentClient->next == client)
            {
                currentClient->next = client->next;

                if (client == *clientTail)
                    *clientTail = currentClient;
                break;
            }
            currentClient = currentClient->next;
        }
    }

    free(client);
}

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
    
    setSignalHandler(SIGINT, handleSigInt);
    uint32_t highestNumber = 0;
    int totalNumbersSent = 0;
    client_t *clientHead = NULL,
        *clientTail = NULL;

    printf("Listening for connections on %d\n", PORT);
    while (!shouldQuit)
    {
        fd_set readFds;
        FD_ZERO(&readFds);
        FD_SET(socketFd, &readFds);

        int maxFd = socketFd;
        client_t *currentClient = clientHead;
        while (currentClient != NULL)
        {
            FD_SET(currentClient->fd, &readFds);
            if (currentClient->fd > maxFd)
                maxFd = currentClient->fd;
            
            currentClient = currentClient->next;
        }

        if (select(maxFd + 1, &readFds, NULL, NULL, NULL) < 0)
        {
            if (errno == EINTR)
                continue;
            ERR("select");
        }
        
        // Handle new connections
        if (FD_ISSET(socketFd, &readFds))
        {
            int clientFd = accept(socketFd, NULL, NULL);
            if (clientFd < 0)
            {
                if (errno == EINTR)
                    continue;
                ERR("accept");
            }

            client_t *newClient = malloc(sizeof(client_t));
            if (newClient == NULL)
                ERR("malloc");
            newClient->fd = clientFd;
            newClient->next = NULL;

            if (clientTail == NULL)
                clientTail = clientHead = newClient;
            else
            {
                clientTail->next = newClient;
                clientTail = newClient;
            }
            
            printf("New client connected\n");
        }

        // Read numbers
        currentClient = clientHead;
        while (currentClient != NULL)
        {
            if (FD_ISSET(currentClient->fd, &readFds))
            {
                uint32_t number;
                if (networkReadNumber(currentClient->fd, &number) < 0)
                {
                    printf("Client disconnected\n");
                    if (TEMP_FAILURE_RETRY(close(currentClient->fd)) < 0)
                        ERR("close");
                    
                    client_t *nextClient = currentClient->next;
                    removeClient(&clientHead, &clientTail, currentClient);
                    currentClient = nextClient;
                    continue;
                }
                totalNumbersSent++;
                
                printf("Received %d from client\n", number);

                if (number > highestNumber)
                {
                    highestNumber = number;
                    printf("New highest number\n");
                }
                
                printf("Responding with %d\n", highestNumber);

                if (networkWriteNumber(currentClient->fd, highestNumber) < 0)
                {
                    printf("Client disconnected\n");
                    if (TEMP_FAILURE_RETRY(close(currentClient->fd)) < 0)
                        ERR("close");
                    
                    client_t *nextClient = currentClient->next;
                    removeClient(&clientHead, &clientTail, currentClient);
                    currentClient = nextClient;
                    continue;
                }
            }

            currentClient = currentClient->next;
        }
        printf("\n");
    }

    printf("Received %d numbers in total\n", totalNumbersSent);
    printf("Closing connections to all the clients\n");
    
    client_t *currentClient = clientHead;
    while (currentClient != NULL)
    {
        if (close(currentClient->fd) < 0)
            ERR("close");
        client_t *nextClient = currentClient->next;
        removeClient(&clientHead, &clientTail, currentClient);
        currentClient = nextClient;
    }

    printf("Closing server socket\n");
    if (close(socketFd) < 0)
        ERR("close");
    printf("Exiting\n");
    return EXIT_SUCCESS;
}