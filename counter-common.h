#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define ERR(source) ( fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), \
                        perror(source), exit(EXIT_FAILURE) )

ssize_t bulkRead(int fd, char *buffer, size_t length);
ssize_t bulkWrite(int fd, char *buffer, size_t length);
int networkReadNumber(int fd, uint32_t *number);
int networkWriteNumber(int fd, uint32_t number);
