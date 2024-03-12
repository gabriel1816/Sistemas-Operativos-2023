#include "consola.h"


t_identificador mapear_identificador(char* identificador){

    t_identificador id;

    if(string_equals_ignore_case(identificador, "F_READ")){  
        id = F_READ;
    }
    else if(string_equals_ignore_case(identificador, "F_WRITE")){
        id = F_WRITE;
    }
    else if(string_equals_ignore_case(identificador, "SET")){
        id = SET;
    }
    else if(string_equals_ignore_case(identificador, "MOV_IN")){
        id = MOV_IN;
    }
    else if(string_equals_ignore_case(identificador, "MOV_OUT")){
        id = MOV_OUT;
    }
    else if(string_equals_ignore_case(identificador, "F_TRUNCATE")){
        id = F_TRUNCATE;
    }
    else if(string_equals_ignore_case(identificador, "F_SEEK")){
        id = F_SEEK;
    }
    else if(string_equals_ignore_case(identificador, "CREATE_SEGMENT")){
        id = CREATE_SEGMENT;
    }
    else if(string_equals_ignore_case(identificador, "I/O")){
        id = I_O;
    }
    else if(string_equals_ignore_case(identificador, "WAIT")){
        id = WAIT;
    }
    else if(string_equals_ignore_case(identificador, "SIGNAL")){
        id = SIGNAL;
    }
    else if(string_equals_ignore_case(identificador, "F_OPEN")){
        id = F_OPEN;
    }
    else if(string_equals_ignore_case(identificador, "F_CLOSE")){
        id = F_CLOSE;
    }
    else if(string_equals_ignore_case(identificador, "DELETE_SEGMENT")){
        id = DELETE_SEGMENT;
    }
    else if(string_equals_ignore_case(identificador, "EXIT")){
        id = EXIT;
    }
    else if(string_equals_ignore_case(identificador, "YIELD")){
        id = YIELD;
    }
    return id;
 }
 void iniciar_logger() {
    logger_consola = log_create(LOG_FILE_PATH, "CONSOLA", 1, LOG_LEVEL_INFO);
    log_info(logger_consola, "Modulo Consola iniciado");
 }

void levantar_config(char* pathConfig) {

    config_consola = config_create(pathConfig); 
    ipKernel = config_get_string_value(config_consola, "IP_KERNEL");
	puertoKernel = config_get_string_value(config_consola, "PUERTO_KERNEL");

}

t_list* parsear_instrucciones(FILE* archivo_instrucciones) {
    char* line = malloc(sizeof(char) * 1024);
    size_t len = sizeof(line);  
    t_list* lista_instrucciones = list_create();
    while ((getline(&line, &len, archivo_instrucciones)) != -1) {
        t_list* lines = list_create();
        char* t = strtok(line, "\n");   
        char** tokens = string_split(t, " "); 
        int i = 1;
        while(tokens[i] != NULL){
            list_add(lines, (void*) tokens[i]);
            i++;
        }
        t_identificador identificador = mapear_identificador(tokens[0]); 
        t_instruccion* instruccion = crear_instruccion(identificador, lines);  
        list_add(lista_instrucciones, instruccion);     
       
        free(tokens);
        list_destroy(lines); 
    }
    free(line);
    return lista_instrucciones;
}

void recibir_mensaje_para_terminar(int conexion_kernel, t_log* logger_consola) {
    t_paquete* paquete_recibido = recibir_paquete(conexion_kernel, logger_consola); 
    uint32_t offset = 0;
    t_instruccion* instruccion = crear_instruccion_para_el_buffer(paquete_recibido->buffer, &offset);
    if (instruccion->identificador == EXIT) {
        log_info(logger_consola, "Finaliza ejecución de la consola");
    }
    else {
        log_error(logger_consola, "No finalizó el proceso");
    }
    return;
}
