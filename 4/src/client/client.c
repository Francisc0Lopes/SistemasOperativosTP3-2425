#include "client.h"
#include "../modules/sockets.h"
#include "../modules/ioaux.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void send_request(const char *host, int port, const char *program, const char *args, const char *filename, int is_unix){
    int sock;

    // Inicializar o socket (TCP ou UNIX)
    if(is_unix){
        sock = un_client_socket_init(host); // Conexão UNIX
    }
    else{
        sock = tcp_client_socket_init(host, port); // Conexão TCP
    }


    if(sock < 0){
        perror("ERRO ao conectar");
        return;
    }


    // Abrir o arquivo
    int file_fd = open(filename, O_RDONLY);
    if(file_fd < 0){
        perror("ERRO ao abrir o file");
        close(sock);
        return;
    }

    // Obter o tamanho do arquivo
    struct stat file_stat;
    if(fstat(file_fd, &file_stat) < 0){
        perror("ERRO ao obter o tamanho do file");
        close(file_fd);
        close(sock);
        return;
    }



    // Enviar cabeçalho
    char header[1024];
    int header_len = snprintf(header, sizeof(header), "RUN: %s\nARGS: %s\nFILE: %s\nDIM: %ld\n\n", program, args, filename, file_stat.st_size);
    if(header_len < 0 || header_len >= sizeof(header)){
        fprintf(stderr, "ERRO formato header\n");
        close(file_fd);
        close(sock);
        return;
    }

    if(write_n_bytes(sock, header, header_len) == -1){
        perror("ERRO header send");
        close(file_fd);
        close(sock);
        return;
    }





    // Enviar conteúdo do arquivo
    char buffer[1024];
    ssize_t bytes_read;
    while((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0){
        if(write_n_bytes(sock, buffer, bytes_read) == -1){
            perror("ERRO no conteudo enviado");
            close(file_fd);
            close(sock);
            return;
        }
    }

    if(bytes_read == -1){
        perror("ERRO ao ler o file");
    }

    close(file_fd);






    // Receber resposta
    while((bytes_read = read_n_bytes(sock, buffer, sizeof(buffer))) > 0){
        if(write(STDOUT_FILENO, buffer, bytes_read) == -1){
            perror("ERRO stdout");
            close(sock);
            return;
        }
    }

    if(bytes_read == -1){
        perror("ERRO ler server");
    }

    close(sock);
    
    
}







void* stress_test_thread(void *arg){
    StressTestArgs *args = (StressTestArgs *)arg;
    send_request(args->host, args->port, args->program, args->args, args->filename, args->is_unix);
    free(args); // Libertar memória alocada
    return NULL;
}






void stress_test(const char *host, int port, int connections, const char *file, const char *program, const char *args, int is_unix){
    pthread_t *threads = malloc(connections * sizeof(pthread_t)); 
    if(!threads){
        perror("ERRO memoria threads");
        return;
    }

    for(int i = 0; i < connections; i++){
        StressTestArgs *args_struct = malloc(sizeof(StressTestArgs));
        if(!args_struct){
            perror("ERRO memoria argumentos da thread");
            continue;
        }

        args_struct->host = host;
        args_struct->port = port;
        args_struct->program = program;
        args_struct->args = args;
        args_struct->filename = file;
        args_struct->is_unix = is_unix;

        if(pthread_create(&threads[i], NULL, stress_test_thread, args_struct) != 0){
            perror("ERRO ao criar thread");
            free(args_struct);
        }
    }


    for (int i = 0; i < connections; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("ERRO ao join thread");
        }
    }

    free(threads); // Libertar memória alocada
}








int main(int argc, char *argv[]){
    if (argc < 7){
        fprintf(stderr, " %s <host ou unix_socket > <porto|unused> <conection> <programa> <argumentos> <files>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *host = argv[1];
    int port = atoi(argv[2]);
    int connections = atoi(argv[3]);
    const char *program = argv[4];
    const char *args = argv[5];
    const char *file = argv[6];

    int is_unix = (strcmp(host, "server.sock") == 0); // Verifica se é UNIX socket

    stress_test(host, port, connections, file, program, args, is_unix);

    return EXIT_SUCCESS;
}
