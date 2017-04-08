#include <unistd.h>
#include <netinet/in.h>
#include "counter-common.h"

ssize_t bulkRead(int fd, char *buffer, size_t length)
{
    ssize_t totalBytesRead = 0;
    while (length > 0)
    {
        ssize_t currentBytesRead = read(fd, buffer, length);
        if (currentBytesRead < 0)
            ERR("read");
        else if (currentBytesRead == 0)
            break;
        
        buffer += currentBytesRead;
        totalBytesRead += currentBytesRead;
        length -= currentBytesRead;
    }

    return totalBytesRead;
}

ssize_t bulkWrite(int fd, char *buffer, size_t length)
{
    ssize_t totalBytesWritten = 0;
    while (length > 0)
    {
        ssize_t currentBytesWritten = write(fd, buffer, length);
        if (currentBytesWritten < 0)
            ERR("read");
        else if (currentBytesWritten == 0)
            break;
        
        buffer += currentBytesWritten;
        totalBytesWritten += currentBytesWritten;
        length -= currentBytesWritten;
    }

    return totalBytesWritten;
}

int networkReadNumber(int fd, uint32_t *number)
{
    ssize_t bytesRead = bulkRead(fd, (char*)number, sizeof(uint32_t));
    if (bytesRead == sizeof(uint32_t))
        *number = ntohl(*number);
    return (bytesRead == sizeof(uint32_t)) ? 0 : -1;
}

int networkWriteNumber(int fd, uint32_t number)
{
    number = htonl(number);
    ssize_t bytesWritten = bulkWrite(fd, (char*)&number, sizeof(uint32_t));
    return (bytesWritten == sizeof(uint32_t)) ? 0 : -1;
}