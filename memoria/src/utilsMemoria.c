#include <memoria.h>

// ------------------------------------------------------------------------------------------
// -- Inicializar y Finalizar --
// ------------------------------------------------------------------------------------------

void inicializar_lista_huecos_libres()
{
    t_segmento* huecoLibreInicial = malloc(sizeof(t_segmento));
    huecoLibreInicial->base = segmento0.limite;
    huecoLibreInicial->limite = tam_memoria;
    list_add(huecosLibres, huecoLibreInicial);
}


void inicializarMemoria() 
{
    memoriaReal = calloc(tam_memoria, sizeof(uint32_t)); //inicializa en 0 la memoria reservada
    procesosEnMemoria = list_create();
    huecosLibres = list_create();
    segmento0.id = 0;
    segmento0.base = 0;
    segmento0.limite = tam_segmento_0;
    inicializar_lista_huecos_libres();
    segmentoVacio.id = -1;
    segmentoVacio.base = 0;
    segmentoVacio.limite = 0;
}


void finalizar_memoria() 
{
    t_segmento* huecoUnico = list_remove(huecosLibres, 0);
    free(huecoUnico);
    free(memoriaReal);
    list_destroy(huecosLibres);
    list_destroy(procesosEnMemoria);
}



// ------------------------------------------------------------------------------------------
// -- Envios y Recibos --
// ------------------------------------------------------------------------------------------

void enviarProcesoEnMemoriaADestino(t_procesoMemoria* nuevoProceso, intptr_t socket)
{
    t_buffer* buffer = crear_buffer_para_proceso_memoria(nuevoProceso);
    t_paquete* paquete = crear_paquete(buffer, CODIGO_INSTRUCCION_MEMORIA);
    enviar_paquete(socket, paquete, logger_memoria);
	destruir_paquete(paquete);
}


void enviarErrorOutOfMemory(int una_conexion)
{
    t_list* parametros = list_create();
    t_instruccion* mensajeErrorOutOfMemory = crear_instruccion(ERROR_MEMORIA, parametros);

    t_buffer* buffer = crear_buffer_para_t_instruccion(mensajeErrorOutOfMemory); 
    t_paquete* paquete = crear_paquete(buffer, ERROR_MEMORIA);
    enviar_paquete(una_conexion, paquete, logger_memoria);
    destruir_paquete(paquete);
    destruir_instruccion(mensajeErrorOutOfMemory);
    return;
}


void enviarMensajeCompactacion(int una_conexion)
{
    t_list* parametros = list_create();
    t_instruccion* mensajeCompactacion = crear_instruccion(COMPACTACION, parametros);

    t_buffer* buffer = crear_buffer_para_t_instruccion(mensajeCompactacion); 
    t_paquete* paquete = crear_paquete(buffer, COMPACTACION);
    enviar_paquete(una_conexion, paquete, logger_memoria);
    destruir_paquete(paquete);
    destruir_instruccion(mensajeCompactacion);
    return;
}


void enviarProcesosEnMemoria(int una_conexion)
{
    t_buffer *buffer = malloc(sizeof(t_buffer));
    uint32_t size_total = 0;
    for (int i = 0; i < list_size(procesosEnMemoria); i++)
    {
        t_buffer *buffer_proceso = crear_buffer_para_proceso_memoria(list_get(procesosEnMemoria, i));
        size_total += buffer_proceso->size;
        destruir_buffer(buffer_proceso);
    }
    // creo el stream y copio los datos de cada buffer
    void *stream = malloc(size_total);
    buffer->size = size_total;
    uint32_t offset = 0;
    for (int i = 0; i < list_size(procesosEnMemoria); i++)
    {
        t_buffer *buffer_proceso = crear_buffer_para_proceso_memoria(list_get(procesosEnMemoria, i));
        uint32_t size = buffer_proceso->size;
        void *stream_proceso = buffer_proceso->stream;
        memcpy(stream + offset, stream_proceso, size);
        offset += size;
        destruir_buffer(buffer_proceso);
    }
    buffer->stream = stream;
    t_paquete* paquete = crear_paquete(buffer, COMPACTACION);
    enviar_paquete(una_conexion, paquete, logger_memoria);
    destruir_paquete(paquete);
    return;
}


void recibirAprobacion(int una_conexion)
{
    t_paquete* paquete = recibir_paquete(una_conexion, logger_memoria);
    destruir_paquete(paquete);
    return;
}


void enviarMensajeFinalizacionProceso(int una_conexion)
{
    t_list* parametros = list_create();
    t_instruccion* mensajeCompactacion = crear_instruccion(COMPACTACION, parametros);
    t_buffer* buffer = crear_buffer_para_t_instruccion(mensajeCompactacion); 
    t_paquete* paquete = crear_paquete(buffer, COMPACTACION);
    enviar_paquete(una_conexion, paquete, logger_memoria);
    destruir_paquete(paquete);
    destruir_instruccion(mensajeCompactacion);
    return;
}



// ------------------------------------------------------------------------------------------
// -- Proceso --
// ------------------------------------------------------------------------------------------

void crearProcesoEnMemoria(uint32_t pid, intptr_t una_conexion) 
{
    t_procesoMemoria* nuevoProceso = malloc(sizeof(t_procesoMemoria));
    t_segmento* tabla_segmento = calloc(cant_segmento * sizeof(t_segmento), sizeof(uint32_t)); //inicializa en 0 la memoria reservada
    tabla_segmento[0] = segmento0;
    nuevoProceso->pid = pid;
    nuevoProceso->tabla_segmento = tabla_segmento;
    nuevoProceso->tamanio_tabla = cant_segmento;
    list_add(procesosEnMemoria, nuevoProceso);
    for(int i = 1; i < cant_segmento; i++) {
        tabla_segmento[i] = segmentoVacio;
    }
    log_info(logger_memoria, "Creacion de Proceso PID: %u", pid);
    enviarProcesoEnMemoriaADestino(nuevoProceso, una_conexion);
}


void finalizarProceso(uint32_t pid, intptr_t una_conexion) 
{
    pidABuscar = pid;
    t_procesoMemoria* proceso = list_find(procesosEnMemoria, esElMismoPID); 
    
    t_segmento* tablaSegmentos = proceso->tabla_segmento;

    for(int i = 1; i < cant_segmento; i++) {
        int32_t idSegmento = tablaSegmentos[i].id;
        if(idSegmento != -1) {
            borrar_segmento_en_memoria(proceso, idSegmento);
        }
    }
    list_remove_by_condition(procesosEnMemoria, esElMismoPID); 
    free(proceso->tabla_segmento);
    free(proceso);
    log_info(logger_memoria, "Eliminacion de Proceso PID: %u", pid);
}