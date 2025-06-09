#ifndef SOCKETS_H
#define SOCKETS_H

int tcp_server_socket_init(int serverPort);//inicia o socket para TCP (server)

int tcp_server_socket_accept(int serverSocket);//Aceita a conexao do socket TCP (server)

int tcp_client_socket_init(const char *host, int port);//Inicia o socket TCP cliente e conecta ao servidor

int un_server_socket_init(const char *serverEndPoint);//inicia o socket para UNIX (server)

int un_server_socket_accept(int serverSocket);//Aceita a conexao do socket UNIX (server)

int un_client_socket_init(const char *serverEndPoint);//Inicia o socket UNIX cliente e conecta ao servidor

int get_socket_endpoint(int sock, char *buffer, int bufferSize);//Ip address e porto de um socket

#endif /* SOCKETS_H */
