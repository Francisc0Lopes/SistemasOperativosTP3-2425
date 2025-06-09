#ifndef IOAUX_H
#define IOAUX_H

#include <sys/types.h> 
#include <stddef.h>    

ssize_t read_line(int sockfd, char *buffer, size_t size);


ssize_t read_n_bytes(int sockfd, void *buffer, size_t n);


ssize_t write_n_bytes(int sockfd, const void *buffer, size_t n);

#endif /* IOAUX_H */
