#ifndef _SERVER_H
#define _SERVER_H
#include <pthread.h>

//LIGACAO DO CLIENTE
typedef struct {
    int socket;     
    int isTcp;      // 1 se for TCP, 0 se for UNIX
} ClientConnection;

//CONFIGURAÇÃO DO SERVIDOR
typedef struct {
    int tcpPort;               // Porta TCP para o servidor
    char *unixSocketPath;      // Caminho para o socket UNIX
    char *logFile;             // Caminho para o arquivo de log
} ServerConfig;

//ESTATISTICAS DO SERVIDOR
typedef struct {
    int activeConnections;     // Número de conexões ativas
    int totalTcpConnections;   // Total de conexões TCP atendidas
    int totalUnixConnections;  // Total de conexões UNIX atendidas
    pthread_mutex_t lock;      // Mutex para sincronização
} ServerStats;

int server_init(ServerConfig *config); //Iniciar o servidor

int server_start();//Start ao servidor

int server_stop();//Parar o servidor

int server_get_stats(ServerStats *stats);//Estatísticas do servidor



#endif /* _SERVER_H */
