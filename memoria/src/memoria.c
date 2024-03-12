#include "memoria.h"

t_log* logger_memoria;
t_config* config_memoria;
char* puerto_escucha;
uint32_t tam_memoria;
uint32_t tam_segmento_0;
uint32_t cant_segmento;
uint32_t retardo_memoria;
uint32_t retardo_compactacion;
char* algoritmo_asignacion;
char* ip_memoria;
int cliente_cpu, cliente_kernel, cliente_filesystem;
void* memoriaReal;

t_list* procesosEnMemoria;
t_list* huecosLibres;
uint32_t pidABuscar;
t_segmento segmento0;
t_segmento segmentoVacio;

int main(void) {

    levantar_config();
	logger_memoria = log_create(LOG_FILE_PATH , "MEMORIA", true, LOG_LEVEL_INFO);

    inicializarMemoria();
	int socket_servidor = iniciar_servidor(logger_memoria, "MEMORIA", ip_memoria, puerto_escucha);
    
	pthread_t hilo_escucha;	
	pthread_create(&hilo_escucha, NULL, f_aux_escucha, (void*) &socket_servidor);

	pthread_join(hilo_escucha, NULL);

    finalizar_memoria();

	return 0;
}

void* f_aux_escucha(void* socket_memoria) {
		escuchar(*(int*)socket_memoria);
		return NULL;
}	

void atencion_a_CPU(void* conexion) {
	int una_conexion = (int) conexion; 
	log_info(logger_memoria, "Se conecto la cpu a la memoria");
    bool continuar = true;
    t_paquete* paquete;
    while (continuar)
    {
        paquete = recibir_paquete(una_conexion, logger_memoria);
        uint32_t offset = 0;
        t_instruccion* pedidoMemoria = crear_instruccion_para_el_buffer(paquete->buffer, &offset);
        
        uint32_t pid = (uint32_t)strtoul(pedidoMemoria->parametros[0], NULL, 10);
        int32_t direccion_fisica = (int32_t)strtoul(pedidoMemoria->parametros[1], NULL, 10);
        char* valor_registro = malloc(sizeof(char)*17);
        t_paquete* paquete_rta;
        uint32_t tamanio_registro;
        t_buffer* buffer;
        t_list* parametros_rta = list_create();
        t_instruccion* rta_memoria;
        switch (pedidoMemoria->identificador) {
            case MOV_IN:
                tamanio_registro = (uint32_t)strtoul(pedidoMemoria->parametros[2], NULL, 10);
                void* inicio_lectura = memoriaReal + direccion_fisica;
                memcpy(valor_registro, inicio_lectura, sizeof(char)*tamanio_registro);
                
                list_add(parametros_rta, valor_registro);
                rta_memoria = crear_instruccion(MOV_IN, parametros_rta);
                buffer = crear_buffer_para_t_instruccion(rta_memoria);
                paquete_rta = crear_paquete(buffer, CODIGO_INSTRUCCION_MEMORIA);
                log_info(logger_memoria, "PID: %u - Acción: LEER - Dirección física: %d - Tamaño: %u - Origen: CPU", pid, direccion_fisica, tamanio_registro);
                break;

            case MOV_OUT: 
                valor_registro = pedidoMemoria->parametros[2];
                tamanio_registro = sizeof(char)*strlen(valor_registro);
                memcpy((memoriaReal+direccion_fisica), valor_registro, tamanio_registro);
                rta_memoria = crear_instruccion(MOV_OUT, parametros_rta);
                buffer = crear_buffer_para_t_instruccion(parametros_rta);
                paquete_rta = crear_paquete(buffer, MOV_OUT_OK);
                log_info(logger_memoria, "PID: %u - Acción: ESCRIBIR - Dirección física: %d - Tamaño: %u - Origen: CPU", pid, direccion_fisica, tamanio_registro);
                valor_registro = "";
                break;

            default:
                log_info(logger_memoria, "Mensaje no reconocido");
                break;
        }
        usleep(retardo_memoria*1000);
        enviar_paquete(una_conexion, paquete_rta, logger_memoria);
	    destruir_paquete(paquete_rta);
        destruir_paquete(paquete);
        destruir_instruccion(rta_memoria);
        //free(valor_registro);
    }
    log_info(logger_memoria, "Se cerrará la conexión (CPU - Memoria)");
    log_info(logger_memoria, "Se desconectó un cliente");
    close(una_conexion);
}

void atencion_a_filesystem(void* conexion) {
	int una_conexion = (int) conexion; 
	log_info(logger_memoria, "Se conecto el file system a la memoria");
    bool continuar = true;
    t_paquete* paquete;

    while (continuar)
    {
        paquete = recibir_paquete(una_conexion, logger_memoria);
        t_pedido_file_system* pedidoFilesystem = crear_pedido_file_system_para_el_buffer(paquete->buffer);
        uint32_t pid = pedidoFilesystem->pid;
        int32_t direccion_fisica = pedidoFilesystem->direccion_fisica;
        t_paquete* paquete_rta;
        uint32_t tamanio_lectura;
        t_instruccion* rta_memoria;
        t_list* parametros = list_create();
        t_buffer* buffer;
    
        switch (pedidoFilesystem->instruccion->identificador) {
            case F_READ_MEMORIA: // escribir en la direccion fisica que me mandan lo de lectura
                char* lectura = malloc(sizeof(pedidoFilesystem->instruccion->parametros[0]));
                lectura = pedidoFilesystem->instruccion->parametros[0];          
                tamanio_lectura = sizeof(char)*strlen(lectura);
                memcpy((memoriaReal+direccion_fisica), lectura, tamanio_lectura);
                rta_memoria = crear_instruccion(F_READ_MEMORIA, parametros);
                buffer = crear_buffer_para_t_instruccion(rta_memoria);
                paquete_rta = crear_paquete(buffer, F_READ_OK);
                log_info(logger_memoria, "PID: %u - Acción: ESCRIBIR - Dirección física: %d - Tamaño: %u - Origen: FS", pid, direccion_fisica, tamanio_lectura);
                lectura = "";
                break;
            case F_WRITE: // mando puntero de la direccion fisica que me mandan y su tamanio
                // tamanio = pedidoMemoria->parametros[1];
                tamanio_lectura = (uint32_t)strtoul(pedidoFilesystem->instruccion->parametros[2], NULL, 10);
                char* escritura = malloc(tamanio_lectura);
                void* inicio_escritura = memoriaReal + direccion_fisica;
                memcpy(escritura, inicio_escritura, sizeof(char)*tamanio_lectura); 
                list_add(parametros, escritura);
                rta_memoria = crear_instruccion(F_WRITE, parametros);
                buffer = crear_buffer_para_t_instruccion(rta_memoria);
                paquete_rta = crear_paquete(buffer, CODIGO_INSTRUCCION_MEMORIA);
                log_info(logger_memoria, "PID: %u - Acción: LEER - Dirección física: %d - Tamaño: %u - Origen: FS", pid, direccion_fisica, tamanio_lectura);
                break;
            default:
                log_info(logger_memoria, "Mensaje no reconocido");
                break;
        }
        
        usleep(retardo_memoria*1000);
        enviar_paquete(una_conexion, paquete_rta, logger_memoria);
	    destruir_paquete(paquete_rta);
        destruir_paquete(paquete);
    }
    log_info(logger_memoria, "Se cerrará la conexión (FileSystem - Memoria)");
    log_info(logger_memoria, "Se desconectó un cliente");
    close(una_conexion);
}

void atencion_a_kernel(void* conexion) {
	int una_conexion = (int) conexion; 
    log_info(logger_memoria, "Se conecto el kernel a la memoria");
    bool continuar = true;
    t_paquete* paquete;

    while (continuar)
    {
        paquete = recibir_paquete(una_conexion, logger_memoria);
        uint32_t offset = 0;
        t_instruccion* pedidoMemoria = crear_instruccion_para_el_buffer(paquete->buffer, &offset);

        int32_t idSegmento;
        uint32_t pid, tamanioSegmento;

        switch (pedidoMemoria->identificador) {
            case CREATE_PROCESS:
                pid = (uint32_t)strtoul(pedidoMemoria->parametros[0], NULL, 10);
                crearProcesoEnMemoria(pid, una_conexion);
                break;
            case END_PROCESS: 
                pid = (uint32_t)strtoul(pedidoMemoria->parametros[0], NULL, 10);
                finalizarProceso(pid, una_conexion);
                break;
            case CREATE_SEGMENT: 
                idSegmento = (int32_t)strtoul(pedidoMemoria->parametros[0], NULL, 10);
                tamanioSegmento =(uint32_t)strtoul(pedidoMemoria->parametros[1], NULL, 10);
                pid = (uint32_t)strtoul(pedidoMemoria->parametros[2], NULL, 10);
                crear_segmento(pid, idSegmento, tamanioSegmento, una_conexion);
                break;
            case DELETE_SEGMENT: 
                idSegmento =(int32_t)strtoul(pedidoMemoria->parametros[0], NULL, 10);
                pid = (uint32_t)strtoul(pedidoMemoria->parametros[1], NULL, 10);
                borrar_segmento(pid, idSegmento, una_conexion);
                break;
            default:
                log_info(logger_memoria, "Mensaje no reconocido");
                break;
        }
        
        destruir_paquete(paquete);
        destruir_instruccion(pedidoMemoria);
    }
    log_info(logger_memoria, "Se cerrará la conexión (Kernel - Memoria)");
    log_info(logger_memoria, "Se desconectó un cliente");
    terminar_programa(conexion, logger_memoria, config_memoria);
}


void escuchar(int socket_memoria_escucha) {
    pthread_t thread_cpu;
    pthread_t thread_filesystem;
    pthread_t thread_kernel;

    while (1) {
        int cliente = esperar_cliente(socket_memoria_escucha, logger_memoria);
        char tipo_cliente[20];
        recv(cliente, tipo_cliente, sizeof(tipo_cliente), 0);
        
        if (strcmp(tipo_cliente, "kernel") == 0) {
            pthread_create(&thread_kernel, NULL, atencion_a_kernel, (void*)cliente);
            
        } else if (strcmp(tipo_cliente, "filesystem") == 0) {
            pthread_create(&thread_filesystem, NULL, atencion_a_filesystem, (void*)cliente);
        } else if (strcmp(tipo_cliente, "cpu") == 0) {
            pthread_create(&thread_cpu, NULL, atencion_a_CPU, (void*)cliente);
        } else {
            log_error(logger_memoria, "Cliente desconocido");
            break;
        }
    }

    pthread_join(thread_cpu, NULL);
    pthread_join(thread_filesystem, NULL);
    pthread_join(thread_kernel, NULL);

    return NULL;
}

void levantar_config() {

    config_memoria = iniciar_config(CONFIG_FILE_PATH);
	puerto_escucha = config_get_string_value(config_memoria, "PUERTO_ESCUCHA") ;
	ip_memoria = config_get_string_value(config_memoria, "IP_MEMORIA") ;
	tam_memoria = config_get_int_value(config_memoria, "TAM_MEMORIA") ;
	tam_segmento_0 = config_get_int_value(config_memoria, "TAM_SEGMENTO_0");
	cant_segmento = config_get_int_value(config_memoria, "CANT_SEGMENTOS");
	retardo_memoria = config_get_int_value(config_memoria, "RETARDO_MEMORIA");
	retardo_compactacion = config_get_int_value(config_memoria, "RETARDO_COMPACTACION");
	algoritmo_asignacion = config_get_string_value(config_memoria, "ALGORITMO_ASIGNACION");
}