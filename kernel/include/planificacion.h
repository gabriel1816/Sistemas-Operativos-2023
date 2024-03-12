#ifndef PLANIFICACION_H_
#define PLANIFICACION_H_


#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <commons/string.h>
#include <commons/collections/list.h> // ver si es necesario
#include "pcb.h"


#define CONFIG_FILE_PATH "cfg/kernel.config"
#define LOG_FILE_PATH "cfg/kernel.log"



extern t_log* logger_kernel;
extern char* ip_kernel;
extern char* ip_filesystem ;
extern char* ip_memoria ;
extern char* ip_cpu ;
extern char* puerto_memoria ;
extern char* puerto_filesystem; 
extern char* puerto_escucha ;
extern char* puerto_cpu;
extern char* ip;
extern char* algoritmo_planificacion;
extern int grado_multiprogramacion;
extern double hrrn_alfa;
extern int estimacion_inicial;
extern char** recursos;
extern int* instancias_recursos;
extern uint32_t conexion_memoria, conexion_filesystem, conexion_cpu;
extern int cliente_dispatch;
extern t_list* tabla_global_archivos_abiertos;
extern sem_t semMultiprogramacion;
extern sem_t semProcesoNuevo;
extern sem_t semProcesoListo;
extern sem_t semFileSystem;
extern sem_t compactacion;
extern t_list *procesosEnSistema;



// ------------------------------------------------------------------------------------------
// -- Archivos --
// ------------------------------------------------------------------------------------------
void agregar_a_tabla_global(char* archivo);
void agregar_a_tabla_proceso(t_pcb* pcb, char* archivo);
bool esta_en_tabla(char* archivo);
void sacar_de_tabla_proceso(t_pcb* pcb_actualizado, char* archivo);
void sacar_de_tabla_global(t_pcb* pcb_actualizado, char* archivo);
void eliminar_archivo_por_nombre(t_list* lista, char* nombre);
void actualizar_puntero_tabla_global(char* nombre_archivo, char* puntero);
void iniciar_archivos();
void desbloquear_proceso(t_pcb* pcb, char* key_archivo);
bool hay_procesos_en_espera(t_pcb* pcb, char* key_archivo);
void atender_signal_archivo(t_pcb* pcb, char* key_archivo);
void atender_wait_archivo(t_pcb* pcb, char* key_archivo);



// ------------------------------------------------------------------------------------------
// -- FileSystem --
// ------------------------------------------------------------------------------------------
void iniciar_file_system();
void file_system();
int extraer_puntero_archivo(char * archivo);


// ------------------------------------------------------------------------------------------
// -- Kernel --
// ------------------------------------------------------------------------------------------
void levantar_config();
void atender_consolas_nuevas(void* conexion);



// ------------------------------------------------------------------------------------------
// -- Manejo IO --
// ------------------------------------------------------------------------------------------
void ejecutar_instruccion_io(t_pcb* pcb, int tiempo);
void atender_io(int tiempo);



// ------------------------------------------------------------------------------------------
// -- Planificacion --
// ------------------------------------------------------------------------------------------
void iniciar_listas_y_semaforos();
char* estado_to_string(enum_estados estado);
void cambiar_estado_pcb(t_pcb* pcb, enum_estados estado);
double responseRatio (t_pcb* pcb);
bool mayorResponseRatio (t_pcb* pcb1, t_pcb* pcb2);
void stop_all_timers();
void resume_all_timers();
void ordenarPorRR();
void actualizarEstimadoRafaga (t_pcb* pcb, int64_t tiempoEjecucion);
t_pcb*  sacar_pcb_cola_bloqueados_IO();
void agregar_pcb_en_cola_bloqueados_IO(t_pcb* proceso);
t_pcb*  sacar_pcb_cola_bloqueados_FileSystem();
void agregar_pcb_en_cola_bloqueados_FileSystem(t_pcb* proceso);
void  sacar_pcb_cola_bloqueados_Recurso(t_pcb* proceso);
void agregar_pcb_cola_bloqueados_RecursoArchivo(t_pcb* proceso);
void  sacar_pcb_cola_bloqueados_RecursoArchivo(t_pcb* proceso);
void agregar_pcb_cola_bloqueados_Recurso(t_pcb* proceso);
void set_puede_iniciar_compactacion(bool estado);
void loggear_cola_listos();
t_pcb* sacar_pcb_cola_listos();
void agregar_pcb_en_cola_listos(t_pcb* proceso);
void agregar_pcb_cola_nuevos(t_pcb* proceso);
t_pcb* sacar_pcb_de_cola_nuevo();
void meter_pcb_en_ejecucion(t_pcb* proceso);
t_pcb* sacar_pcb_de_ejecucion(int64_t tiempoEjecucion);
void terminar_proceso(t_pcb* pcb, char* motivo);
void eliminarProcesoDeMemoria(uint32_t pid);
bool esElMismoPID_PCB(t_pcb* proceso, void* pid);
t_pcb* find_pcb_cola_procesos_en_sistema(int32_t pid);
void  sacar_pcb_cola_procesos_en_sistema(t_pcb* proceso);
void agregar_pcb_en_cola_procesos_en_sistema(t_pcb* proceso);
void esperar_fin_FS();


// ------------------------------------------------------------------------------------------
// -- Planificacion corto plazo --
// ------------------------------------------------------------------------------------------
void crear_hilo_planificador_corto_plazo();
void mandar_de_vuelta_PCB(t_pcb* pcb);
void ejecutar_PCB(t_pcb* pcb);
void planificar_pcbs_planificador_corto_plazo();
void ponerEstimadoRafaga(t_pcb* pcbViejo, t_pcb* pcbNuevo);
void recibir_pcb_de_CPU(int conexion);
bool esElMismoPID(t_procesoMemoria* proceso, void* pid);
void recibirYActualizarProcesos();
void actualizar_tabla_segmento_y_reenviar_CPU (t_paquete *paquete, t_pcb* pcb);
void recibirRespuestaPorCreacionDeSegmento(t_pcb* pcb, int64_t temporizador, t_instruccion* pedidoMemoria);
t_instruccion* agregarPidAInstruccion(t_instruccion* instruccion, char* pid);
void crear_segmento_en_memoria(t_instruccion* instruccion, t_pcb* pcb, int64_t temporizador);
void borrar_segmento_en_memoria(t_instruccion* instruccion, t_pcb* pcb, int64_t temporizador);
void desconectar_memoria();
bool consultar_existencia_archivo(t_instruccion* instruccion);
void solicitar_creacion_archivo(t_instruccion* instruccion);
void actualizar_tamanio_archivo(t_instruccion* instruccion);
void leer_y_grabar_en_memoria(t_instruccion* instruccion , t_pcb* pcb);
void escribir(t_instruccion* instruccion, t_pcb* pcb);
void desconectar_filesystem();
int64_t recibir_temporizador(int conexion_cpu);



// ------------------------------------------------------------------------------------------
// -- Planificador largo plazo --
// ------------------------------------------------------------------------------------------
t_procesoMemoria* solicitarCreacionDeProcesoEnMemoria(uint32_t pid);
void chequear_grado_de_multiprogramacion();
void crear_hilo_planificador_largo_plazo();



// ------------------------------------------------------------------------------------------
// -- Recursos --
// ------------------------------------------------------------------------------------------
void iniciar_recursos(char** key_recursos, char** instancias);
t_recurso* crear_recurso(int instancias);
void atender_signal(t_pcb* pcb, char* key_recurso);
void atender_wait(t_pcb* pcb, char* key_recurso);



#endif