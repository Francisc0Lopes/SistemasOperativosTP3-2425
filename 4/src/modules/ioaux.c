#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "ioaux.h"
#include "erroraux.h"

// Lê uma linha do socket até encontrar '\n' ou atingir o tamanho máximo do buffer
ssize_t read_line(int sockfd, char *buffer, size_t size){
    size_t i = 0;
    ssize_t n;
    char c;

    if(size <= 0 || buffer == NULL){
        errno = EINVAL; // Argumento inválido
        return -1;
    }



    while(i < size - 1){
        n = read(sockfd, &c, 1);
        if(n == -1){
            if(errno == EINTR){
                continue;  // Sinal interrompeu a leitura, tentar novamente
            }
            return -1;     // Erro na leitura
        }
        if(n == 0){ // EOF
            if(i == 0){
                return 0;  // Nenhum dado lido
            }
            break;        // Dados lidos parcialmente
        }

        buffer[i++] = c;

        if(c == '\n'){ // Fim da linha
            break;
        }
    }

    buffer[i] = '\0'; // Finalizar a string
    return i;
}









// Lê exatamente `n` bytes do socket, a menos que o EOF seja alcançado
ssize_t read_n_bytes(int sockfd, void *buffer, size_t n){
    size_t remaining = n;
    ssize_t nread;
    char *buf = buffer;

    while(remaining > 0){
        nread = read(sockfd, buf, remaining);

        if(nread == 0){
            return n - remaining;  // EOF, retorna bytes lidos até agora
        }

        if(nread == -1){
            if(errno == EINTR){
                continue;  // Sinal interrompeu a leitura, tentar novamente
            }
            return -1;     // Erro na leitura
        }
        remaining -= nread;
        buf += nread;
    }

    return n; // Retorna o número total de bytes lidos
}





// Escreve exatamente `n` bytes no socket
ssize_t write_n_bytes(int sockfd, const void *buffer, size_t n){
    size_t remaining = n;
    ssize_t nwritten;
    const char *buff = buffer;

    while(remaining > 0){
        nwritten = write(sockfd, buff, remaining);

        if(nwritten == -1){
            if(errno == EINTR){ // Sinal interrompeu a escrita, tentar novamente
                continue;
            }
            return -1;     // Erro na escrita
        }

        remaining -= nwritten;
        buff += nwritten;
    }

    return n; // Retorna o número total de bytes escritos
}

