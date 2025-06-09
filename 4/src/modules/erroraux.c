#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "erroraux.h"


void perror_msg(const char *msg) {
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
}


void error_exit(const char *msg) {
    perror_msg(msg);
    exit(EXIT_FAILURE);
}
