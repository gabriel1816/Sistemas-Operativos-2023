#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/txt.h>
#include <commons/collections/list.h> // llamado a la libreria de commos para el manejo de listas
#include <commons/string.h>
#include "shared_utils.h"

extern char* ipKernel;
extern char* puertoKernel;
extern t_config* config_consola;
extern t_log* logger_consola;

t_identificador convertir_instruccion(char* str);
t_list* crear_lista_instrucciones(void);
t_identificador mapear_identificador(char* identificador);
void destruir_tokens(char** tokens);
void iniciar_logger();
void levantar_config(char* pathConfig); 
t_list* parsear_instrucciones(FILE* archivo_instrucciones);
void recibir_mensaje_para_terminar(int conexion_kernel, t_log* logger_consola);

#define LOG_FILE_PATH "cfg/consola.log"

#endif /*CONSOLA_H_*/
