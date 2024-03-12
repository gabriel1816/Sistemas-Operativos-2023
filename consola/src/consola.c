#include "consola.h"

char* ipKernel;
char* puertoKernel;
t_config* config_consola;
t_log* logger_consola;


int main(int argc, char** argv) 
{ 
    iniciar_logger();
    if(argc < 2) {
    log_error(logger_consola, "No se pasaron la cantidad correcta de parámetros"); 
        return 1;
    }
    char* path = argv[2]; 
    char* pathConfig = argv[1];
    levantar_config(pathConfig);
    
    FILE* archivo_instrucciones = fopen(path, "rt"); 
	if (archivo_instrucciones == NULL) {
        log_error(logger_consola, "No fue posible abrir el archivo"); 
        return 1; 
    }
    	
    t_list * lista_instrucciones = list_create();
    lista_instrucciones = parsear_instrucciones(archivo_instrucciones);
    fclose(archivo_instrucciones);

    int conexion_kernel = crear_conexion(ipKernel, puertoKernel, logger_consola);
    if( enviar_instrucciones(conexion_kernel, lista_instrucciones, logger_consola)) {
        log_info(logger_consola, "se mandaron las instrucciones correctamente");
    }
    else {
        log_error(logger_consola, "No se pudieron enviar las instrucciones");
    }   
   
    log_info(logger_consola, "por recibir");
    recibir_mensaje_para_terminar(conexion_kernel, logger_consola);
    log_info(logger_consola, "recibido");
    list_destroy_and_destroy_elements(lista_instrucciones, (void*)destruir_instruccion);
    log_info(logger_consola, "destruyo");
    terminar_programa_consola(conexion_kernel, logger_consola, config_consola);
    log_info(logger_consola, "termino");
    return 0;
}

