#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include "pcb.h"
#include <sys/select.h>
#include <stdint.h>



#define CONFIG_FILE_PATH "cfg/memoria.config"
#define LOG_FILE_PATH "cfg/memoria.log"


extern t_config* config_memoria;
extern t_log* logger_memoria;
extern char* puerto_escucha;
extern uint32_t tam_memoria;
extern uint32_t tam_segmento_0;
extern uint32_t cant_segmento;
extern uint32_t retardo_memoria;
extern uint32_t retardo_compactacion;
extern char* algoritmo_asignacion;
extern char* ip_memoria;
extern int cliente_cpu, cliente_kernel, cliente_filesystem;
extern void* memoriaReal;

extern t_list* procesosEnMemoria;
extern t_list* huecosLibres;
extern uint32_t pidABuscar;
extern t_segmento segmento0;
extern t_segmento segmentoVacio;


// ------------------------------------------------------------------------------------------
// -- Memoria --
// ------------------------------------------------------------------------------------------
void* f_aux_escucha(void* socket_memoria);
void atencion_a_CPU(void* conexion);
void atencion_a_filesystem(void* conexion);
void atencion_a_kernel(void* conexion);
void escuchar(int socket_memoria_escucha);
void levantar_config();



// ------------------------------------------------------------------------------------------
// -- Utils Memoria --
// ------------------------------------------------------------------------------------------
void inicializar_lista_huecos_libres();
void inicializarMemoria();
void finalizar_memoria();
void enviarProcesoEnMemoriaADestino(t_procesoMemoria* nuevoProceso, intptr_t socket);
void enviarErrorOutOfMemory(int una_conexion);
void enviarMensajeCompactacion(int una_conexion);
void enviarProcesosEnMemoria(int una_conexion);
void recibirAprobacion(int una_conexion);
void enviarMensajeFinalizacionProceso(int una_conexion);
bool mayorTamanio (t_segmento* hueco1, t_segmento* hueco2);
bool menorTamanio (t_segmento* hueco1, t_segmento* hueco2);
bool menorBase (t_segmento* hueco1, t_segmento* hueco2);
int obtenerTamanioHueco(t_segmento* huecoLibre);
bool esElMismoPID(t_procesoMemoria* proceso, void* pid);
int calcularTamioTotal();
t_segmento modificarListaHuecosLibres(t_segmento* hueco, uint32_t tamanioSegmento, int index);
t_segmento buscarHuecoLibre(uint32_t tamanioSegmento);
void crearNuevoHueco(uint32_t limite, uint32_t base);
void agregarNuevoHueco(t_segmento* segmentoALiberar);
void modificarSegmentoContinuo (t_segmento* huecoLibre);
void iniciarCompactacion();
void mostrarResultadoCompactacion();
void crear_segmento(uint32_t pid, int32_t idSegmento, uint32_t tamanioSegmento, intptr_t una_conexion);
void borrar_segmento(uint32_t pid, int32_t idSegmento, intptr_t una_conexion);
void borrar_segmento_en_memoria(t_procesoMemoria* proceso, uint32_t idSegmento);
void crearProcesoEnMemoria(uint32_t pid, intptr_t una_conexion);
void finalizarProceso(uint32_t pid, intptr_t una_conexion);



#endif /* SERVER_H_ */