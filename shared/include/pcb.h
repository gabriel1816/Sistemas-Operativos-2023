#ifndef PCB_H
#define PCB_H
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <commons/log.h>
#include <string.h>
#include <assert.h>
#include <commons/config.h>
#include <pthread.h>
#include "shared_utils.h"
#include <commons/temporal.h>



typedef enum {
    NUEVO,
    LISTO,
    EJECUTANDO,
    BLOQUEADO,
	TERMINADO,
	SEG_FAULT
}t_cod_estado;

typedef struct
{
	char AX[5], BX[5], CX[5], DX[5];
	char EAX[9], EBX[9], ECX[9],EDX[9];
	char RAX[17], RBX[17], RCX[17], RDX[17];
}t_registros;

typedef struct {
    t_recurso *archivo;
    char* nombre_archivo;
    int puntero;
} t_archivo_abierto;

typedef struct
{
	u_int32_t pid; //  Identificador del proceso (deberá ser un número entero, único en todo el sistema).
	t_list* instrucciones; // Lista de instrucciones a ejecutar
	u_int32_t program_counter; // Número de la próxima instrucción a ejecutar.
	t_cod_estado estado; // Estado del proceso
	t_registros registros; //Estructura que contendrá los valores de los registros de uso general de la CPU.
	t_segmento* tabla_segmentos; // Tabla de Segmentos: Contendrá ids, direcciones base y tamaños de los segmentos de datos del proceso.
	double estimado_rafaga; // Estimación utilizada para planificar los procesos en el algoritmo HRRN -> Cuando se crea el PCB se usa el del config. HRRN_ALFA 
	//t_temporal* temporizador;// Lo usamos para calcular el tiempo q estuvo en estado EJECUTANDO, en la rafaga anterior y el tiempo en el que proceso llegó a ready por última vez 
	t_temporal* llegada_a_listo;
	uint32_t tamanio_tabla;
	t_list* tabla_archivos; // Contendrá la lista de archivos abiertos del proceso
	int32_t direccion_fisica;
} t_pcb;



// ------------------------------------------------------------------------------------------
// -- Registros --
// ------------------------------------------------------------------------------------------
t_registros iniciar_registros();



// ------------------------------------------------------------------------------------------
// -- Creacion / Eliminacion --
// ------------------------------------------------------------------------------------------
t_pcb* crear_pcb(intptr_t pid, t_list* lista_instrucciones, t_cod_estado estado, int32_t estimado_rafaga);
void destruir_pcb(t_pcb* pcb);



// ------------------------------------------------------------------------------------------
// -- Envío de pcbs --
// ------------------------------------------------------------------------------------------
void enviar_pcb(t_pcb* pcb, int socket, t_log* logger);



// ------------------------------------------------------------------------------------------
// -- Serializar pcb --
// ------------------------------------------------------------------------------------------
t_buffer* crear_buffer_envio_pcb(t_pcb* pcb, t_log* logger);



// ------------------------------------------------------------------------------------------
// -- Para la recepción de pcbs --
// ------------------------------------------------------------------------------------------
t_pcb* recibir_pcb(int socket_cliente, t_log* logger);
t_pcb* deserializar_buffer_paquete_pcb(t_buffer* buffer,  t_log* logger);



#endif