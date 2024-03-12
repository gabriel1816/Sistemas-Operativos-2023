#include "file_system.h"

t_log *logger_fileSystem;
char *puerto_escucha;
char *puerto_llamado;
char *ip;
void *copia_bloques;

char *ip_file_system;
char *ip_memoria;
char *puerto_memoria;
char *path_superbloque;
char *path_bitmap;
char *path_bloques;
char *path_fcb;
int retardo;
t_config *config_FileSystem;
t_list* lista_fcbs;

t_bitarray *bitmap;
t_superBloque *superBloque;
uint32_t conexion_memoria;
sem_t semBinSolicitudes;

int main(void) {
    logger_fileSystem = log_create(LOG_FILE_PATH, "FILE_SYSTEM", 1, LOG_LEVEL_INFO);
    levantar_config();
    mkdir(path_fcb, 0777);
    conexion_memoria = crear_conexion(ip_memoria, puerto_memoria, logger_fileSystem);
    send(conexion_memoria, "filesystem", strlen("filesystem") + 1, 0); 

    superBloque = levantar_superBloque(path_superbloque);
    iniciar_bitmap(path_bitmap, superBloque);
    crear_bloques(path_bloques, superBloque, retardo, logger_fileSystem);
    lista_fcbs = levantar_fcbs(path_fcb);
    sem_init(&semBinSolicitudes, 0, 1);
    int socket_file_system_escucha = iniciar_servidor(logger_fileSystem, "FILE_SYSTEM", ip_file_system, puerto_escucha);
    
    pthread_t hilo_escucha;

   while(1) { 
    int conexion = esperar_cliente(socket_file_system_escucha, logger_fileSystem);
    pthread_create(&hilo_escucha, NULL, (void *)atender_kernel, (void *)(intptr_t)conexion);
    pthread_join(hilo_escucha, NULL);
    }
    log_info(logger_fileSystem, "Se cerrara la conexion");

    free(superBloque);
    terminar_programa(socket_file_system_escucha, logger_fileSystem, config_FileSystem);

    return 0;
}

