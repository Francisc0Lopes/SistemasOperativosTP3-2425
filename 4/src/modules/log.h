#ifndef LOG_H
#define LOG_H

typedef enum {INFO, ERROR, DEBUG} LOG_LEVEL;

int log_init(const char *pathname);//inicia o ficheiro onde serão guardados os registos

int log_message(LOG_LEVEL level, const char *msg);//Cria e escreve no ficheiro um registo

int log_message_with_end_point(LOG_LEVEL level, const char *msg, int sock);//Cria e escreve no ficheiro um registo

int log_close();//Fecha o ficheiro

#endif /*LOG_H*/
