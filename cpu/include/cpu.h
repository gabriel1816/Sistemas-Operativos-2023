#ifndef cpu_H_
#define cpu_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/dictionary.h>
#include <semaphore.h>
#include <pcb.h>



#define CONFIG_FILE_PATH "cfg/cpu.config"
#define LOG_FILE_PATH "cfg/cpu.log"



extern int conexion_memoria;
extern int cliente_dispatch;
extern t_log* logger_cpu;
extern char *ip_cpu;
extern char *ip_memoria;
extern char *puerto_escucha;
extern char *puerto_memoria;
extern int retardo_instruccion;
extern t_temporal* temporizador;
extern uint32_t tamanio_maximo_segmentos;
extern t_config* config_cpu;



// CPU
void atender_clientes(void *conexion);

// UTILS CPU
void levantar_config();
void insertar_nuevo_valor_registro(t_registros *registros, char *registro, char *nuevo_valor);
char* consultar_valor_registro(t_registros *registros, char *registro);
int consultar_tamanio_registro(t_registros registros, char *registro);
void imprimir_registros_por_pantalla(t_registros registros);
void solicitar_MOV_OUT(uint32_t pid, int32_t direccion_fisica, char* registro);
char* solicitar_MOV_IN(uint32_t pid, int32_t direccion_fisica, uint32_t tamanio_registro);
void liberar_registros(t_registros *registros);
void leer_instruccion_y_ejecutar_pcb(t_instruccion *instruccion_aux, t_pcb *pcb, int conexion_kernel);
void comenzar_ciclo_instruccion(t_pcb *pcb, int conexion_kernel);
t_instruccion *siguiente_instruccion(t_pcb *pcb);
void enviar_contador(t_temporal *temporizador, int conexion_kernel);
void procesarsolicitud(int32_t direccionfisica, int conexion_kernel);

// MMU
int32_t traducir_direccion(uint32_t direccion_logica, t_pcb *pcb, int conexion_kernel, uint32_t cantidad_bytes);
uint32_t obtener_numero_segmento(uint32_t direccion_logica);
uint32_t obtener_desplazamineto_segmento(uint32_t direccion_logica);

#endif /* SERVER_H_ */