#ifndef FILE_SYSTEMS_H_
#define FILE_SYSTEMS_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/bitarray.h>
#include <stdint.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <math.h>
#include <commons/string.h>
#include "shared_utils.h"

extern t_log* logger_fileSystem;
extern t_config *config_FileSystem;
extern char* puerto_escucha;
extern char* puerto_llamado;
extern char* ip;

extern uint32_t conexion_memoria;
extern t_bitarray* bitmap;

extern int cliente_dispatch;
extern void* copia_bloques;
extern char *ip_file_system;
extern char *ip_memoria;
extern char *puerto_escucha;
extern char *puerto_memoria;
extern char *path_superbloque;
extern char *path_bitmap;
extern char *path_bloques;
extern char *path_fcb;
extern int retardo;
extern sem_t semBinSolicitudes;

typedef struct 
{
  int tamanio_bloque;
  int cant_bloques; 
}t_superBloque;

extern t_superBloque *superBloque; 
typedef struct
{
  char *nombre_archivo;
  int tamanio_archivo;
  uint32_t puntero_directo;
  uint32_t puntero_indirecto;
  int cant_bloques_asignados;
} t_fcb;

extern t_list* lista_fcbs;  // Declaración de la variable global

t_superBloque* levantar_superBloque(char* path);
void iniciar_bitmap(char* path, t_superBloque* superbloque);
void atender_kernel(void * conexion);
void crear_bloques(char* path_bloques, t_superBloque* superbloque, int retardo, t_log* logger_fileSystem);
void buscar_archivo_pedido(char* nombreArchivo, intptr_t una_conexion);
void truncar_archivo_pedido(char* nombreArchivo,char* tamanio, intptr_t una_conexion);
void leer_archivo_pedido(t_pedido_file_system*, intptr_t una_conexion);  
void escribir_archivo_pedido(t_pedido_file_system*, intptr_t una_conexion); 
void desconectar_filesystem(); 
void levantar_config();
char* itoa(uint32_t num);
bool abrir_archivo(char* nombre_archivo_recibido);
t_list* levantar_fcbs(const char* directorio_fcbs);
t_fcb* buscar_fcb_por_nombre(t_list* lista_fcbs, char* nombre_archivo);
int crear_archivo(char* path_fcb, char *nombre, intptr_t una_conexion); 
void acortar_archivo(t_fcb* fcb, uint32_t tamanio); 
void agrandar_archivo(t_fcb* fcb, uint32_t tamanio); 
int calcular_bloques_adicionales(uint32_t nuevo_tamanio, t_fcb *fcb); 
int calcular_bloques_a_liberar(uint32_t nuevo_tamanio, t_fcb *fcb);
void agrandar_archivo(t_fcb* fcb, uint32_t tamanio_nuevo);
void asignar_bloques(t_fcb* puntero_indirecto, uint32_t bloques_adicionales);
void escribir_bloque_en_archivo(uint32_t bloque, uint32_t pos_en_bloque, uint32_t valor);
uint32_t buscar_bloque_libre(t_bitarray* bitmap);
int liberar_bloque_bitmap(uint32_t bloque_a_liberar);
void liberar_bloques(t_fcb* fcb, int bloques_a_liberar);
void notificar_truncado(t_instruccion* instruccion, intptr_t conexionKernel); 
void leer_archivo(char *nombre, int puntero_archivo, int cantidad_bytes_para_leer, int direccion_fisica, int pid); 
char* leer_bytes_bloque(uint32_t puntero_bloque, int bytes_por_leer, int bytes_a_leer);
void escribir_archivo(char* nombre, int puntero_archivo, char* datos_a_escribir, int cantidad_bytes_para_escribir); 


#define LOG_FILE_PATH "cfg/file_system.log"
#define CONFIG_FILE_PATH "cfg/file_system.config"
#endif /* SERVER_H_ */