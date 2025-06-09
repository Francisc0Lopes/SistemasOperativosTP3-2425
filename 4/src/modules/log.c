#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "log.h"
#include "sockets.h"
#include "erroraux.h"

static FILE *logFile = NULL;
static pthread_mutex_t logMutex = PTHREAD_MUTEX_INITIALIZER;





static const char *log_level_to_string(LOG_LEVEL level){	//PARA OS LOGS FICA STATIC SO USADO AQUI
    switch (level){
        case INFO:  return "INFO";
        case ERROR: return "ERROR";
        case DEBUG: return "DEBUG";
        default:    return "UNKNOWN";
    }
}



int log_init(const char *pathname){	//inicia o ficheiro onde serão guardados os registos
    pthread_mutex_lock(&logMutex);
    
    if(logFile != NULL){		// Se já houver um arquivo aberto, fecha-o
        fclose(logFile);
    }
    
    logFile = fopen(pathname, "a");
    if(logFile == NULL){	// Verifica se houve erro ao abrir
        pthread_mutex_unlock(&logMutex); 	// Liberta o mutex antes de retornar
        perror_msg("ERRO ao abrir o log file");
        return -1;
    }
    
    pthread_mutex_unlock(&logMutex);	// Liberta o mutex
    return 0;
}





int log_message(LOG_LEVEL level, const char *msg){	//Cria e escreve no ficheiro um registo
    time_t now;
    struct tm timeinfo;
    char timestamp[30];
    
    if(logFile == NULL){ 	// Verifica se o arquivo de log foi iniciado
        fprintf(stderr, "ERRO no log file\n");
        return -1;
    }
    
    time(&now);	// Obtém o tempo atual
    localtime_r(&now, &timeinfo); 	// Converte para hora local
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);
    
    pthread_mutex_lock(&logMutex);	// Bloqueia o mutex para escrita no arquivo
    fprintf(logFile, "[%s] %s - %s\n", log_level_to_string(level), timestamp, msg);
    fflush(logFile); 	// Garante que os dados sejam gravados imediatamente
    pthread_mutex_unlock(&logMutex);	// Liberta o mutex
    
    return 0;
}






int log_message_with_end_point(LOG_LEVEL level, const char *msg, int sock) {	//Cria e escreve no ficheiro um registo
    time_t now;
    struct tm timeinfo;
    char timestamp[30];
    char endpoint[100];
    
    if(logFile == NULL){ // Verifica o log 
        fprintf(stderr, "ERRO no logfile\n");
        return -1;
    }
    
    if(get_socket_endpoint(sock, endpoint, sizeof(endpoint)) < 0){ // Tenta obter o endpoint do socket
        strcpy(endpoint, "Endpoint desconhecido");
    }
    
    time(&now); 	// Obtém o tempo atual
    localtime_r(&now, &timeinfo);	// Converte para hora local
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);
    
    pthread_mutex_lock(&logMutex);	// Bloqueia o mutex
    fprintf(logFile, "[%s] %s - %s: %s\n", log_level_to_string(level), timestamp, msg, endpoint);
    fflush(logFile);	// Grava
    pthread_mutex_unlock(&logMutex);	//Liberta o mutex
    
    return 0;
}






int log_close(){	//Fecha o ficheiro
    pthread_mutex_lock(&logMutex); 	// Bloqueia o mutex
    
    if(logFile != NULL){ 	// Se o ficheiro estiver aberto
        fclose(logFile);		//fecha
        logFile = NULL;		//Certificar que fechado
    }
    
    pthread_mutex_unlock(&logMutex);	// Liberta o mutex
    return 0;
}
