#ifndef SHARED_UTILS_H
#define SHARED_UTILS_H

#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>	
#include <netdb.h>
#include <commons/log.h>
#include <string.h>
#include <assert.h>
#include <commons/config.h>
#include <pthread.h>
#include <semaphore.h>
#include <inttypes.h>
#include <dirent.h>

#define PUERTO "4444"


extern t_log *logger;
typedef enum
{
    NUEVO_PROCESO,
    RECIBIR_PCB,
    MENSAJE
} op_code;

typedef struct
{
    int size;
    void *stream;
} t_buffer;

typedef enum
{
    NEW,
    READY,
    EXEC,
    BLOCK,
    TERMINAR
} enum_estados;

typedef struct
{
    uint32_t codigo_operacion;
    t_buffer *buffer;
} t_paquete;

typedef enum // le asignamos un código a cada operacion
{
    SET,
    MOV_OUT,
    WAIT,
    I_O,
    SIGNAL,
    MOV_IN,
    F_OPEN,
    YIELD,
    F_TRUNCATE,
    F_SEEK,
    CREATE_SEGMENT,
    F_WRITE,
    F_READ,
    F_CREATE,
    DELETE_SEGMENT,
    F_CLOSE,
    EXIT,
    CREATE_PROCESS,
    F_READ_MEMORIA,
    DIRECCION_FISICA,
    END_PROCESS

} t_identificador;

typedef struct
{
    t_identificador identificador;
    uint32_t cant_parametros;
    uint32_t p1_length;
    uint32_t p2_length;
    uint32_t p3_length;
    uint32_t p4_length;
    char **parametros;
} t_instruccion;

typedef struct
{
    t_list *cola_bloqueado;
    int instancias;
} t_recurso;

typedef struct
{
    int32_t id;
    uint32_t base;
    uint32_t limite;
} t_segmento;

typedef struct
{
    uint32_t pid;
    uint32_t tamanio_tabla;
    t_segmento *tabla_segmento;

} t_procesoMemoria;

typedef struct 
{
    uint32_t pid;
    int32_t direccion_fisica;
    t_instruccion* instruccion;
}t_pedido_file_system;



// ------------------------------------------------------------------------------------------
// -- Inicio / Handshake / Cierre --
// ------------------------------------------------------------------------------------------
int iniciar_servidor(t_log *logger, const char *name, char *ip, char *puerto);
t_config *iniciar_config(char *path);
int crear_conexion(char* ip, char* puerto, t_log* logger);
int esperar_cliente(int socket_servidor, t_log* logger);
void liberar_conexion(int socket_cliente);
void terminar_programa(int conexion, t_log *logger, t_config *config);
void terminar_programa_consola(int conexion, t_log *logger, t_config *config);



// ------------------------------------------------------------------------------------------
// -- Mensajes --
// ------------------------------------------------------------------------------------------
void enviar_mensaje(char *mensaje, int socket_cliente);
void recibir_mensaje(int socket_cliente, t_log* logger);



// ------------------------------------------------------------------------------------------
// -- Instruccion --
// ------------------------------------------------------------------------------------------
uint32_t espacio_de_array_parametros(t_instruccion *instruccion);
t_buffer *crear_buffer_para_t_instruccion(t_instruccion *instruccion);
t_instruccion *crear_instruccion_para_el_buffer(t_buffer *buffer, uint32_t *offset);
void agregar_parametro_a_instruccion(t_list *parametros, t_instruccion *instruccion);
t_instruccion* crear_instruccion(t_identificador identificador, t_list* parametros);
void destruir_instruccion(t_instruccion *instruccion);
t_list *get_instrucciones(t_paquete *paquete);
t_buffer *crear_buffer__para_t_lista_instrucciones(t_list *lista_instrucciones);
t_list *crear_lista_instrucciones_para_el_buffer( t_buffer *buffer);
bool enviar_instrucciones(int socket, t_list *lista_instrucciones, t_log *logger);



// ------------------------------------------------------------------------------------------
// -- Paquete --
// ------------------------------------------------------------------------------------------
void *serializar_paquete(t_paquete *paquete, int bytes);
t_paquete *crear_paquete(t_buffer *buffer, int codigo_operacion);
uint32_t paquete_size(t_paquete *paquete);
bool enviar_paquete(int socket, t_paquete *paquete, t_log *logger);
t_paquete *recibir_paquete(int socket, t_log *logger);
void destruir_paquete(t_paquete *paquete);
t_paquete *get_paquete(int socket, t_log *logger);
int recibir_operacion(int socket_cliente);



// ------------------------------------------------------------------------------------------
// -- Buffer --
// ------------------------------------------------------------------------------------------
t_buffer *buffer_create();
void *recibir_buffer(int *size, int socket_cliente);
t_buffer *null_buffer();
void liberar_buffer(t_buffer *buffer);
void destruir_buffer(t_buffer *buffer);
int retardo_en_segundos(int retardo);



// ------------------------------------------------------------------------------------------
// -- FileSystem --
// ------------------------------------------------------------------------------------------
t_buffer *crear_buffer_para_nombreArchivo(char *nombreArchivo);
char* crear_nombreArchivo_para_el_buffer(t_buffer *buffer, uint32_t *offset);
t_buffer* crear_buffer_para_t_pedido_file_system(t_pedido_file_system* pedido_filesystem);
void enviar_pedido_file_system(t_pedido_file_system* pedido_filesystem, int conexion, t_log* logger_kernel);
t_pedido_file_system* crear_pedido_file_system_para_el_buffer(t_buffer* buffer);



// ------------------------------------------------------------------------------------------
// -- Memoria --
// ------------------------------------------------------------------------------------------
t_procesoMemoria* deserializar_buffer_para_proceso_memoria(t_buffer* buffer);
t_buffer *crear_buffer_para_proceso_memoria(t_procesoMemoria* proceso);
void enviarPedidoAMemoria(t_instruccion* pedidoMemoria, uint32_t socket, t_log* loggercito);
t_procesoMemoria* recibirProcesoDeMemoria(uint32_t socket, t_log *logger);



#define CODIGO_OPERACION 1
#define CODIGO_INSTRUCCION 20
#define CODIGO_PCB 10
#define CODIGO_INSTRUCCION_MEMORIA 30
#define ERROR_MEMORIA 31
#define COMPACTACION 32
#define MOV_OUT_OK 33
#define F_READ_OK 34
#define CODIGO_NOMBRE_ARCHIVO 40
#define CODIGO_INFORMACION_FILE_SYSTEM 41

#endif /* UTILS_H_ */
