#ifndef CLIENT_H
#define CLIENT_H

typedef struct {
    const char *host;
    int port;
    const char *program;
    const char *args;
    const char *filename;
    int is_unix;
} StressTestArgs;


void send_request(const char *host, int port, const char *program, const char *args, const char *filename, int is_unix);

void* stress_test_thread(void *arg);


void stress_test(const char *host, int port, int connections, const char *file, const char *program, const char *args, int is_unix);


#endif // CLIENT_H
