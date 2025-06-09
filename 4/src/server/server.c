#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "server.h"
#include "../modules/sockets.h"
#include "../modules/log.h"
#include "../modules/ioaux.h"
#include "../modules/erroraux.h"
#include "../modules/threadpool.h"

#define MAX_HEADER_LINE 1024 
#define MAX_BUFFER_SIZE 8192 

// Variáveis globais de configuração, estatísticas e estado do servidor
static ServerConfig config;
static ServerStats stats;
static int tcpServerSocket = -1;
static int unixServerSocket = -1;
static volatile int running = 0; // Indica se o servidor está ativo
static pthread_t acceptThread;
static pthread_t menuThread;
static threadpool_t *tp = NULL; // Thread pool global

// Protótipos das funções privadas
static void *accept_thread(void *arg);
static void *menu_thread(void *arg);
static void *client_handler_thread(void *arg);
static int process_client_request(int clientSocket);
static int parse_header(int clientSocket, char **program, char **args, char **filename, size_t *filesize);
static int execute_program(int clientSocket, const char *program, const char *args, size_t filesize);

int main(int argc, char *argv[]){ // Função principal do servidor
    ServerConfig config={
        .tcpPort = 8000, // Porta TCP por defeito
        .unixSocketPath = "server.sock", // Caminho do socket UNIX
        .logFile = "server.log" // Ficheiro de log
    };

    if(server_init(&config) < 0){ // Inicializa o servidor
        fprintf(stderr, "ERRO ao iniciar o servidor\n");
        return EXIT_FAILURE;
    }
    if(server_start() < 0){ // Inicia o servidor (abre sockets e cria threads)
        fprintf(stderr, "ERRO ao arrancar o servidor\n");
        server_stop();
        return EXIT_FAILURE;
    }

    // Mantém o servidor ativo até ser encerrado
    while(running){
        sleep(1);
    }

    return EXIT_SUCCESS;
}

int server_init(ServerConfig *cfg){ // Inicializa a configuração e recursos do servidor
    config.tcpPort = cfg->tcpPort;
    config.unixSocketPath = strdup(cfg->unixSocketPath); // Copia o caminho do socket UNIX
    config.logFile = strdup(cfg->logFile); // Copia o caminho do ficheiro de log

    // Inicializa as estatísticas do servidor
    stats.activeConnections = 0;
    stats.totalTcpConnections = 0;
    stats.totalUnixConnections = 0;
    pthread_mutex_init(&stats.lock, NULL); // Inicializa o mutex para acesso seguro às estatísticas

    // Inicializa o ficheiro de log
    if(log_init(config.logFile) < 0){
        fprintf(stderr, "ERRO ao iniciar o ficheiro de log\n");
        return -1;
    }

    // Inicializa o thread pool (exemplo: fila 32, 4 a 16 threads)
    if(threadpool_init(&tp, 32, 4, 16) != 0){
        fprintf(stderr, "ERRO ao iniciar o thread pool\n");
        return -1;
    }

    return 0;
}

int server_start(){ // Inicia os sockets do servidor e as threads principais
    char msg[256];

    tcpServerSocket = tcp_server_socket_init(config.tcpPort); // Inicia socket TCP
    if(tcpServerSocket < 0){
        log_message(ERROR, "ERRO ao iniciar o socket TCP");
        return -1;
    }

    snprintf(msg, sizeof(msg), "Servidor TCP iniciado na porta %d", config.tcpPort);
    log_message(INFO, msg);

    unixServerSocket = un_server_socket_init(config.unixSocketPath); // Inicia socket UNIX
    if(unixServerSocket < 0){
        log_message(ERROR, "ERRO ao iniciar o socket UNIX");
        close(tcpServerSocket);
        return -1;
    }

    snprintf(msg, sizeof(msg), "Servidor UNIX iniciado em %s", config.unixSocketPath);
    log_message(INFO, msg);

    running = 1; // Ativa a flag de execução do servidor

    // Cria a thread de aceitação de ligações
    if(pthread_create(&acceptThread, NULL, accept_thread, NULL) != 0){
        log_message(ERROR, "ERRO ao criar a thread de aceitação");
        close(tcpServerSocket);
        close(unixServerSocket);
        running = 0;
        return -1;
    }

    // Cria a thread do menu interativo
    if(pthread_create(&menuThread, NULL, menu_thread, NULL) != 0){
        log_message(ERROR, "ERRO ao criar a thread do menu");
        close(tcpServerSocket);
        close(unixServerSocket);
        running = 0;
        pthread_cancel(acceptThread);
        pthread_join(acceptThread, NULL);
        return -1;
    }

    return 0;
}

int server_stop(){ // Para o servidor e liberta recursos
    running = 0; // Desativa a flag de execução

    if(tcpServerSocket >= 0){
        close(tcpServerSocket);
        tcpServerSocket = -1;
    }

    if(unixServerSocket >= 0){
        close(unixServerSocket);
        unlink(config.unixSocketPath); // Remove o ficheiro do socket UNIX
        unixServerSocket = -1;
    }

    pthread_join(acceptThread, NULL); // Aguarda o término da thread de aceitação
    pthread_join(menuThread, NULL);   // Aguarda o término da thread do menu

    // Destroi o thread pool (espera todos os trabalhos terminarem)
    if(tp) {
        threadpool_destroy(tp);
        tp = NULL;
    }

    log_message(INFO, "Servidor a encerrar");
    log_close(); // Fecha o ficheiro de log

    free((void*)config.unixSocketPath); // Liberta strings alocadas
    free((void*)config.logFile);
    pthread_mutex_destroy(&stats.lock); // Destroi o mutex das estatísticas

    return 0;
}

int server_get_stats(ServerStats *out_stats){ // Devolve uma cópia das estatísticas do servidor
    if(!out_stats){
        return -1;
    }

    pthread_mutex_lock(&stats.lock);
    memcpy(out_stats, &stats, sizeof(ServerStats));
    pthread_mutex_unlock(&stats.lock);

    return 0;
}

static void *accept_thread(void *arg){ // Thread que aceita conexões dos clientes
    fd_set readfds;
    int max_fd;
    ClientConnection *client;

    while(running){
        FD_ZERO(&readfds);
        FD_SET(tcpServerSocket, &readfds);
        FD_SET(unixServerSocket, &readfds);
        max_fd = (tcpServerSocket > unixServerSocket) ? tcpServerSocket : unixServerSocket;

        if(select(max_fd + 1, &readfds, NULL, NULL, NULL) < 0){
            if(running){
                log_message(ERROR, "ERRO no select()");
            }
            continue;
        }

        if(FD_ISSET(tcpServerSocket, &readfds)){
            int clientSocket = tcp_server_socket_accept(tcpServerSocket); // Aceita conexão TCP
            if(clientSocket < 0){
                log_message(ERROR, "ERRO ao aceitar conexão TCP");
                continue;
            }

            client = malloc(sizeof(ClientConnection));
            client->socket = clientSocket;
            client->isTcp = 1;

            // Submete o trabalho ao thread pool
            if(threadpool_submit(tp, client_handler_thread, client) != 0){
                log_message(ERROR, "ERRO ao submeter trabalho ao thread pool");
                close(clientSocket);
                free(client);
            } else {
                pthread_mutex_lock(&stats.lock);
                stats.activeConnections++;
                stats.totalTcpConnections++;
                pthread_mutex_unlock(&stats.lock);
            }
        }

        if(FD_ISSET(unixServerSocket, &readfds)){
            int clientSocket = un_server_socket_accept(unixServerSocket); // Aceita conexão UNIX
            if(clientSocket < 0){
                log_message(ERROR, "ERRO ao aceitar conexão UNIX");
                continue;
            }

            client = malloc(sizeof(ClientConnection));
            client->socket = clientSocket;
            client->isTcp = 0;

            // Submete o trabalho ao thread pool
            if(threadpool_submit(tp, client_handler_thread, client) != 0){
                log_message(ERROR, "ERRO ao submeter trabalho ao thread pool");
                close(clientSocket);
                free(client);
            } else {
                pthread_mutex_lock(&stats.lock);
                stats.activeConnections++;
                stats.totalUnixConnections++;
                pthread_mutex_unlock(&stats.lock);
            }
        }
    }

    return NULL;
}

static void *menu_thread(void *arg){ // Thread que mostra o menu de controlo do servidor
    char input[256];
    ServerStats currentStats;

    while(running){
        printf("\nMenu do Servidor\n");
        printf("1. Ver estatísticas\n");
        printf("2. Desligar servidor\n");
        printf("--> ");

        if(fgets(input, sizeof(input), stdin) == NULL){
            continue;
        }

        switch(input[0]){
            case '1':
                server_get_stats(&currentStats);
                printf("\nEstatísticas:\n");
                printf("Ligações ativas: %d\n", currentStats.activeConnections);
                printf("Total ligações TCP: %d\n", currentStats.totalTcpConnections);
                printf("Total ligações UNIX: %d\n", currentStats.totalUnixConnections);
                break;
            case '2':
                printf("A encerrar servidor...\n");
                server_stop();
                break;
            default:
                printf("Opção inválida\n");
        }
    }

    return NULL;
}

static void *client_handler_thread(void *arg){ // Thread que trata da comunicação com um cliente
    ClientConnection *client = (ClientConnection *)arg;
    int clientSocket = client->socket;
    int isTcp = client->isTcp;
    free(client);

    if(process_client_request(clientSocket) < 0){
        log_message(ERROR, "ERRO ao processar pedido do cliente");
    }

    pthread_mutex_lock(&stats.lock);
    stats.activeConnections--;
    pthread_mutex_unlock(&stats.lock);

    close(clientSocket);
    return NULL;
}

static int process_client_request(int clientSocket){
    char *program = NULL;
    char *args = NULL;
    char *filename = NULL;
    size_t filesize = 0;

    if(parse_header(clientSocket, &program, &args, &filename, &filesize) < 0){
        log_message(ERROR, "ERRO ao analisar cabeçalho");
        free(program);
        free(args);
        free(filename);
        return -1;
    }

    if(execute_program(clientSocket, program, args, filesize) < 0){
        log_message(ERROR, "ERRO ao executar programa");
        free(program);
        free(args);
        free(filename);
        return -1;
    }

    free(program);
    free(args);
    free(filename);
    return 0;
}

static int parse_header(int clientSocket, char **program, char **args, char **filename, size_t *filesize){
    char line[MAX_HEADER_LINE];
    char *key, *value;
    int headerEnd = 0;

    while(!headerEnd){
        if(read_line(clientSocket, line, sizeof(line)) <= 0){
            log_message(ERROR, "ERRO ao ler o header line");
            return -1;
        }

        if(strlen(line) == 0){
            headerEnd = 1;
            continue;
        }

        key = strtok(line, ":");
        value = strtok(NULL, "\n");
        if(value != NULL){
            while(*value == ' ' || *value == '\t'){
                value++;
            }
        }

        if(key != NULL && value != NULL){
            if(strcmp(key, "RUN") == 0){
                *program = strdup(value);
            }else if (strcmp(key, "ARGS") == 0){
                *args = strdup(value);
            }else if (strcmp(key, "FILE") == 0){
                *filename = strdup(value);
            }else if (strcmp(key, "DIM") == 0){
                *filesize = atol(value);
            }
        }
    }

    if(*program == NULL || *filename == NULL || *filesize == 0){
        log_message(ERROR, "ERRO Header inválido");
        return -1;
    }

    log_message(INFO, "Header parsed");
    return 0;
}

static int execute_program(int clientSocket, const char *program, const char *args, size_t filesize){
    int pipe_stdin[2], pipe_stdout[2];
    pid_t pid;

    if(pipe(pipe_stdin) < 0 || pipe(pipe_stdout) < 0){
        log_message(ERROR, "ERRO ao criar pipes");
        return -1;
    }

    pid = fork();
    if(pid < 0){
        log_message(ERROR, "ERRO no fork");
        return -1;
    }

    if(pid == 0){ // Processo filho
        dup2(pipe_stdin[0], STDIN_FILENO);
        dup2(pipe_stdout[1], STDOUT_FILENO);
        close(pipe_stdin[1]);
        close(pipe_stdout[0]);

        execlp(program, program, args, NULL);
        log_message(ERROR, "ERRO ao executar programa");
        exit(EXIT_FAILURE);
    }
    else{ // Processo pai
        close(pipe_stdin[0]);
        close(pipe_stdout[1]);

        // Enviar dados para o programa
        char buffer[1024];
        ssize_t bytes_read;
        while((bytes_read = read(clientSocket, buffer, sizeof(buffer))) > 0){
            write(pipe_stdin[1], buffer, bytes_read);
        }
        close(pipe_stdin[1]);

        // Ler saída do programa e enviar ao cliente
        while((bytes_read = read(pipe_stdout[0], buffer, sizeof(buffer))) > 0){
            write(clientSocket, buffer, bytes_read);
        }
        close(pipe_stdout[0]);

        waitpid(pid, NULL, 0);
    }

    return 0;
}