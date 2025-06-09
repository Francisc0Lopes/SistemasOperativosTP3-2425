#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include "sockets.h"
#include "erroraux.h"


int tcp_server_socket_init(int serverPort){
	
    int serverSocket;
    struct sockaddr_in serverAddr;
    int auxadd = 1;
    
    if((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0){ //Criar o socket
        perror_msg("ERRO ao criar o servidor socket");
        return -1;
    }
    
    if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &auxadd, sizeof(auxadd)) < 0){
        perror_msg("ERRO na configuração do socket");
        close(serverSocket);
        return -1;
    }
    
	//preparar as informacoes do servidor
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(serverPort);
    
    
    
    
    if(bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){	//Associa o socket ao endereco e porta
        perror_msg("ERRO associar socket a porta/endereco");
        close(serverSocket);
        return -1;
    }
    
    if(listen(serverSocket, 10) < 0){	//Coloca o servidor a escutar(até 10 conexoes pendentes)
        perror_msg("ERRO escuta socket");
        close(serverSocket);
        return -1;
    }
    
    return serverSocket;
}




int tcp_server_socket_accept(int serverSocket){
    struct sockaddr_in clientaddress;
    socklen_t client_length = sizeof(clientaddress);
    int client =  accept(serverSocket, (struct sockaddr*)&clientaddress, &client_length);//Aceita um nova conexao


	if(client<0){
        perror_msg("ERRO ao aceitar a conexao");
        return -1;
    }
    
    return client;
}



int tcp_client_socket_init(const char *host, int port){
    int clientSocket;
    struct sockaddr_in serverAddr;
    struct hostent *server;
    
    if((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0){//Criar socket
        perror_msg("ERRO ao criar o socket client");
        return -1;
    }
    
    if((server = gethostbyname(host)) == NULL){//Obter o IP do servidor
        fprintf(stderr, "ERRO hostname invalido %s\n", host);
        close(clientSocket);
        return -1;
    }
    
    // Preparar a estrutura do servidor
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    memcpy(&serverAddr.sin_addr.s_addr, server->h_addr, server->h_length);
    serverAddr.sin_port = htons(port);
    
	
    if(connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){//Conectar ao servidor
        perror_msg("ERRO de conexão ao servidor");
        close(clientSocket);
        return -1;
    }
    
    return clientSocket;
}



int un_server_socket_init(const char *serverEndPoint){
    int serverSocket;
    struct sockaddr_un serverAddr;
    
    if((serverSocket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0){//Criar o socket
        perror_msg("ERRO ao criar o socket");
        return -1;
    }
    


    // Preparar a estrutura do servidor
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sun_family = AF_UNIX;
    strncpy(serverAddr.sun_path, serverEndPoint, sizeof(serverAddr.sun_path)-1);
    
    
	unlink(serverEndPoint);//Remover o ficheiro que ja existe

    
    if(bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){//Associar o socket ao caminho
        perror_msg("ERRO associar socket a porta/endereco UNIX");
        close(serverSocket);
        return -1;
    }
    
    if(listen(serverSocket, 10) < 0){//Coloca o servidor a escutar(10 conexoes)
        perror_msg("ERRO escuta socket UNIX");
        close(serverSocket);
        return -1;
    }
    
    return serverSocket;
}


int un_server_socket_accept(int serverSocket){
    int clientSocket;
    struct sockaddr_un clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    
    if((clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen)) < 0){//Aceita a conexão
        perror_msg("ERRO ao aceitar a conexao UNIX");
        return -1;
    }
    
    return clientSocket;
}



int un_client_socket_init(const char *serverEndPoint){
    int clientSocket;
    struct sockaddr_un serverAddr;
    
    if((clientSocket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0){//Criar o socket
        perror_msg("ERRO ao criar o socket UNIX");
        return -1;
    }
    

    // Preparar a estrutura do servidor
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sun_family = AF_UNIX;
    strncpy(serverAddr.sun_path, serverEndPoint, sizeof(serverAddr.sun_path) - 1);
    
    if(connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){ //Conectar ao servidor
        perror_msg("ERRO de conexao ao servidor");
        close(clientSocket);
        return -1;
    }
    
    return clientSocket;
}






int get_socket_endpoint(int sock, char *buffer, int bufferSize){
    struct sockaddr_storage addr;
    socklen_t addrLen = sizeof(addr);
    char hostStr[NI_MAXHOST];
    char portStr[NI_MAXSERV];
    
    if(getpeername(sock, (struct sockaddr*)&addr, &addrLen) < 0){
        perror_msg("ERRO obter nome peer");
        return -1;
    }
    
    if(addr.ss_family == AF_INET || addr.ss_family == AF_INET6){
        if (getnameinfo((struct sockaddr*)&addr, addrLen, hostStr, sizeof(hostStr), portStr, sizeof(portStr), NI_NUMERICHOST | NI_NUMERICSERV) != 0){
            perror_msg("ERRO no getnameinfo");
            return -1;
        }
        snprintf(buffer, bufferSize, "IP=%s, Port=%s", hostStr, portStr);
    }
    else if(addr.ss_family == AF_UNIX){
        strncpy(buffer, "UNIX socket", bufferSize);
    }
    else{
        strncpy(buffer, "Tipo de socket desconhecido", bufferSize);
    }
    
    return 0;
}
