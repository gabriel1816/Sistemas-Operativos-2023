#include "planificacion.h"

static pthread_t hiloPlanificadorCortoPlazo;
uint32_t pidABuscar;



void crear_hilo_planificador_corto_plazo() 
{
    pthread_create(&hiloPlanificadorCortoPlazo, NULL, (void*) planificar_pcbs_planificador_corto_plazo, NULL);
    pthread_detach(hiloPlanificadorCortoPlazo);
}


void mandar_de_vuelta_PCB(t_pcb* pcb)
{
    int conexionDispatch = crear_conexion(ip_cpu, puerto_cpu, logger_kernel);
    enviar_pcb(pcb, conexionDispatch, logger_kernel);
    log_info(logger_kernel, "El PCB con ID %d se reenvio a  CPU", pcb->pid);
    recibir_pcb_de_CPU(conexionDispatch); 
    close(conexionDispatch);
}


void ejecutar_PCB(t_pcb* pcb)
{
    int conexionDispatch = crear_conexion(ip_cpu, puerto_cpu, logger_kernel);

    meter_pcb_en_ejecucion(pcb);
    enviar_pcb(pcb, conexionDispatch, logger_kernel);
    log_info(logger_kernel, "El PCB con ID %d se envio a  CPU", pcb->pid);
    recibir_pcb_de_CPU(conexionDispatch); 
    close(conexionDispatch);
}


void planificar_pcbs_planificador_corto_plazo() 
{
    log_debug(logger_kernel, "Inicia Planificador Corto Plazo");
    while (1) {
        sem_wait(&semProcesoListo);
        //log_debug(logger_kernel, "Planificador corto plazo notificado proceso listo");
        //int conexionDispatch = crear_conexion(ip_cpu, puerto_cpu, logger_kernel);
        if (!strcmp(algoritmo_planificacion, "HRRN")) { 
            ordenarPorRR();
        }
        t_pcb* pcb = sacar_pcb_cola_listos();
        if ((pcb->llegada_a_listo != NULL)){
        temporal_destroy(pcb->llegada_a_listo); 
        }
        ejecutar_PCB(pcb);
    }
}


void ponerEstimadoRafaga(t_pcb* pcbViejo, t_pcb* pcbNuevo)
{
    pcbNuevo->estimado_rafaga = pcbViejo->estimado_rafaga;
}


void recibir_pcb_de_CPU(int conexion) 
{
    t_pcb *pcb_en_ejecucion, *pcb_actualizado;
    pcb_actualizado = recibir_pcb(conexion, logger_kernel);
    int64_t temporizador = recibir_temporizador(conexion);
    t_instruccion* ultima_instruccion = list_get(pcb_actualizado->instrucciones, pcb_actualizado->program_counter - 1);
    if (pcb_actualizado->estado == SEG_FAULT) {
        pcb_en_ejecucion = sacar_pcb_de_ejecucion(temporizador);
        ponerEstimadoRafaga(pcb_en_ejecucion, pcb_actualizado);
        destruir_pcb(pcb_en_ejecucion); 
        terminar_proceso(pcb_actualizado, "SEG_FAULT");
    }
    
    switch(ultima_instruccion->identificador) {
        case EXIT:                  
            pcb_en_ejecucion = sacar_pcb_de_ejecucion(temporizador);
            ponerEstimadoRafaga(pcb_en_ejecucion, pcb_actualizado);
            destruir_pcb(pcb_en_ejecucion); 
            terminar_proceso(pcb_actualizado, "SUCCESS"); //Poner PCB en Cola Terminados -> Cerrar Conexion con Consola.
            break;

        case I_O:
            pcb_en_ejecucion = sacar_pcb_de_ejecucion(temporizador);
            ponerEstimadoRafaga(pcb_en_ejecucion, pcb_actualizado);
            destruir_pcb(pcb_en_ejecucion);
            ejecutar_instruccion_io(pcb_actualizado, atoi(ultima_instruccion->parametros[0]));
            break;

        case YIELD:            
            pcb_en_ejecucion = sacar_pcb_de_ejecucion(temporizador);
            ponerEstimadoRafaga(pcb_en_ejecucion, pcb_actualizado);
            destruir_pcb(pcb_en_ejecucion);
            agregar_pcb_en_cola_listos(pcb_actualizado); 
            break;
        
        case SIGNAL:
            pcb_en_ejecucion = sacar_pcb_de_ejecucion(temporizador);
            ponerEstimadoRafaga(pcb_en_ejecucion, pcb_actualizado);
            destruir_pcb(pcb_en_ejecucion);
            atender_signal(pcb_actualizado, ultima_instruccion->parametros[0]);
            break;

        case WAIT:
            pcb_en_ejecucion = sacar_pcb_de_ejecucion(temporizador);
            ponerEstimadoRafaga(pcb_en_ejecucion, pcb_actualizado);
            destruir_pcb(pcb_en_ejecucion);
            atender_wait(pcb_actualizado, ultima_instruccion->parametros[0]);
            break;		

        case CREATE_SEGMENT:
            pcb_en_ejecucion = sacar_pcb_de_ejecucion(temporizador);
            ponerEstimadoRafaga(pcb_en_ejecucion, pcb_actualizado);
            meter_pcb_en_ejecucion(pcb_actualizado);
            crear_segmento_en_memoria(ultima_instruccion, pcb_actualizado, temporizador);
            break; 

        case DELETE_SEGMENT:
    
            pcb_en_ejecucion = sacar_pcb_de_ejecucion(temporizador);
            ponerEstimadoRafaga(pcb_en_ejecucion, pcb_actualizado);
            meter_pcb_en_ejecucion(pcb_actualizado);
            borrar_segmento_en_memoria(ultima_instruccion, pcb_actualizado, temporizador);
            break; 

        case F_OPEN:
            
            pcb_en_ejecucion = sacar_pcb_de_ejecucion(temporizador);
            char* archivo = ultima_instruccion->parametros[0];
            if(esta_en_tabla(archivo))
            {
                agregar_a_tabla_proceso(pcb_actualizado, archivo);
                atender_wait_archivo(pcb_actualizado, archivo);
                break;
            }
            else {
                log_info(logger_kernel, "PID: %d - Abrir Archivo: %s", pcb_actualizado->pid, archivo);
                if(!consultar_existencia_archivo(ultima_instruccion)) {
                log_info(logger_kernel, "PID: %d - Crear Archivo: %s", pcb_actualizado->pid, archivo);    
                    solicitar_creacion_archivo(ultima_instruccion);
                }
                agregar_a_tabla_global(archivo);
                agregar_a_tabla_proceso(pcb_actualizado, archivo);
                atender_wait_archivo(pcb_actualizado, archivo);

            }
            agregar_pcb_en_cola_listos(pcb_actualizado); 
            break;

        case F_CLOSE:
            char* archivo_a_cerrar = ultima_instruccion->parametros[0];
            sacar_de_tabla_proceso(pcb_actualizado, archivo_a_cerrar);
            log_info(logger_kernel, "PID: %d - Cerrar Archivo: %s", pcb_actualizado->pid, archivo_a_cerrar);
            if(hay_procesos_en_espera(pcb_actualizado, archivo_a_cerrar)) {
                atender_signal_archivo(pcb_actualizado, archivo_a_cerrar);
            }
            else {
                sacar_de_tabla_global(pcb_actualizado, archivo_a_cerrar); // ver si ponemos mutex
            }
            pcb_en_ejecucion = sacar_pcb_de_ejecucion(temporizador); 
            agregar_pcb_en_cola_listos(pcb_actualizado); 
            break;

        case F_SEEK:
            pcb_en_ejecucion = sacar_pcb_de_ejecucion(temporizador);
            actualizar_puntero_tabla_global(ultima_instruccion->parametros[0], ultima_instruccion->parametros[1]);
            agregar_pcb_en_cola_listos(pcb_actualizado);
            log_info(logger_kernel, "PID: %d - Actualizar puntero Archivo: %s - Puntero %s", pcb_actualizado->pid, ultima_instruccion->parametros[0], ultima_instruccion->parametros[1]); 
            break;

        case F_TRUNCATE:
            log_info(logger_kernel, "PID: %d - Archivo: %s - Tamaño: %s ", pcb_actualizado->pid, ultima_instruccion->parametros[0], ultima_instruccion->parametros[1]);
            pcb_en_ejecucion = sacar_pcb_de_ejecucion(temporizador);
            ponerEstimadoRafaga(pcb_en_ejecucion, pcb_actualizado);
            destruir_pcb(pcb_en_ejecucion);
            log_info(logger_kernel, "PID: %d - Bloqueado por: %s", pcb_actualizado->pid, ultima_instruccion->parametros[0]);
            agregar_pcb_en_cola_bloqueados_FileSystem(pcb_actualizado);            
            break;

        case F_READ:
           if(pcb_actualizado->estado == SEG_FAULT) {
                pcb_en_ejecucion = sacar_pcb_de_ejecucion(temporizador);
                ponerEstimadoRafaga(pcb_en_ejecucion, pcb_actualizado);
                destruir_pcb(pcb_en_ejecucion); 
                //log_info(logger_kernel, "Finaliza el proceso %u - Motivo: SEG_FAULT", pcb_actualizado->pid);
                terminar_proceso(pcb_actualizado, "SEG_FAULT");
            }
            else {
                pcb_en_ejecucion = sacar_pcb_de_ejecucion(temporizador);
                ponerEstimadoRafaga(pcb_en_ejecucion, pcb_actualizado);
                destruir_pcb(pcb_en_ejecucion);
                set_puede_iniciar_compactacion(false);
                log_info(logger_kernel, "PID: %d - Bloqueado por: %s", pcb_actualizado->pid, ultima_instruccion->parametros[0]);
                agregar_pcb_en_cola_bloqueados_FileSystem(pcb_actualizado); 
            }
            break;

        case F_WRITE:
            if(pcb_actualizado->estado == SEG_FAULT) {
                pcb_en_ejecucion = sacar_pcb_de_ejecucion(temporizador);
                ponerEstimadoRafaga(pcb_en_ejecucion, pcb_actualizado);
                destruir_pcb(pcb_en_ejecucion); 
                //log_info(logger_kernel, "Finaliza el proceso %u - Motivo: SEG_FAULT", pcb_actualizado->pid);
                terminar_proceso(pcb_actualizado, "SEG_FAULT");
            }
            else {
                pcb_en_ejecucion = sacar_pcb_de_ejecucion(temporizador);
                ponerEstimadoRafaga(pcb_en_ejecucion, pcb_actualizado);
                destruir_pcb(pcb_en_ejecucion);
                set_puede_iniciar_compactacion(false);
                log_info(logger_kernel, "PID: %d - Bloqueado por: %s", pcb_actualizado->pid, ultima_instruccion->parametros[0]);
                agregar_pcb_en_cola_bloqueados_FileSystem(pcb_actualizado); 
            }
            break;

        case MOV_IN:
        case MOV_OUT:
            pcb_en_ejecucion = sacar_pcb_de_ejecucion(temporizador);
            ponerEstimadoRafaga(pcb_en_ejecucion, pcb_actualizado);
            destruir_pcb(pcb_en_ejecucion); 
            //log_info(logger_kernel, "Finaliza el proceso %u - Motivo: SEG_FAULT", pcb_actualizado->pid);
            terminar_proceso(pcb_actualizado, "SEG_FAULT");
            break;

        default:
            break;
    }
}


bool esElMismoPID(t_procesoMemoria* proceso, void* pid)
{
    return (pidABuscar == proceso->pid);
}



// ------------------------------------------------------------------------------------------
// -- Funciones para realizar solicitudes a la memoria --
// ------------------------------------------------------------------------------------------

void recibirYActualizarProcesos() 
{
    t_paquete* paquete = get_paquete(conexion_memoria, logger_kernel); //recibe mensaje de que la compactación salio bien
    uint32_t offset = 0;
    while (offset < paquete->buffer->size)
    {
        t_buffer* buffer_proceso = malloc(sizeof(t_buffer));
        uint32_t size_restante = paquete->buffer->size - offset;
        buffer_proceso->size = size_restante;
        buffer_proceso->stream = malloc(size_restante);
        memcpy(buffer_proceso->stream, paquete->buffer->stream + offset, size_restante);

        t_procesoMemoria *proceso = deserializar_buffer_para_proceso_memoria(buffer_proceso);
        offset +=  sizeof(uint32_t) +                         // pid
                   sizeof(uint32_t) +                         // tamanio_tabla
                   (sizeof(int32_t) + sizeof(uint32_t) * 2) * proceso->tamanio_tabla;  // (id + base + segmento) * cuan grande es la tabla
        
        pidABuscar = proceso->pid;
        t_pcb* pcb_a_actualizar = find_pcb_cola_procesos_en_sistema(proceso->pid);
        pcb_a_actualizar->tabla_segmentos = proceso->tabla_segmento;
        pcb_a_actualizar->tamanio_tabla = proceso->tamanio_tabla;

        free(buffer_proceso->stream);
        free(buffer_proceso);
    }
    return;
}


void actualizar_tabla_segmento_y_reenviar_CPU (t_paquete *paquete, t_pcb* pcb)
{
    t_procesoMemoria* procesoDeMemoria = deserializar_buffer_para_proceso_memoria(paquete->buffer);
    pcb->tabla_segmentos = procesoDeMemoria->tabla_segmento;
    pcb->tamanio_tabla = procesoDeMemoria->tamanio_tabla;
    mandar_de_vuelta_PCB(pcb);
}


void recibirRespuestaPorCreacionDeSegmento(t_pcb* pcb, int64_t temporizador, t_instruccion* pedidoMemoria) 
{
    t_paquete *paquete = get_paquete(conexion_memoria, logger_kernel);
    switch(paquete->codigo_operacion) {
        case CODIGO_INSTRUCCION_MEMORIA: 
            actualizar_tabla_segmento_y_reenviar_CPU (paquete, pcb);
        break;

        case ERROR_MEMORIA:
            t_pcb* pcb_en_ejecucion = sacar_pcb_de_ejecucion(temporizador);
            terminar_proceso(pcb_en_ejecucion, "OUT_OF_MEMORY");
        break;

        case COMPACTACION:
            t_list* parametros = list_create();
            t_instruccion* pedidoCompactacion = crear_instruccion(COMPACTACION, parametros);
            log_info(logger_kernel, "Compactación: Esperando Fin de Operaciones de FS");
            esperar_fin_FS();
            log_info(logger_kernel, "Compactación: Se solicitó compactación");
            log_info(logger_kernel, "Cantidad procesos en sistema %u", list_size(procesosEnSistema));
            enviarPedidoAMemoria(pedidoCompactacion, conexion_memoria, logger_kernel); //solicita compactacion
            recibirYActualizarProcesos(conexion_memoria);
            log_info(logger_kernel, "Se finalizó el proceso de compactación");
            enviarPedidoAMemoria(pedidoMemoria, conexion_memoria, logger_kernel); //envia de nuevo solicitud crear segmento
            recibirRespuestaPorCreacionDeSegmento(pcb, temporizador, pedidoMemoria);
        break;

    }
    destruir_paquete(paquete);
}


t_instruccion* agregarPidAInstruccion(t_instruccion* instruccion, char* pid) 
{    
    return instruccion;
}


void crear_segmento_en_memoria(t_instruccion* instruccion, t_pcb* pcb, int64_t temporizador) 
{
    t_list* parametros = list_create();
    uint32_t pid = pcb->pid;
    char* pidString = string_itoa(pid);
    list_add(parametros, instruccion->parametros[0]);
    list_add(parametros, instruccion->parametros[1]);
    list_add(parametros, pidString);
    t_instruccion* pedidoMemoria = crear_instruccion(CREATE_SEGMENT, parametros);
    enviarPedidoAMemoria(pedidoMemoria, conexion_memoria, logger_kernel);
    log_info(logger_kernel, "PID: %u - Crear Segmento - Id: %s - Tamaño: %s", pid, instruccion->parametros[0], instruccion->parametros[1]);
    recibirRespuestaPorCreacionDeSegmento(pcb, temporizador, pedidoMemoria);
}


void borrar_segmento_en_memoria(t_instruccion* instruccion, t_pcb* pcb, int64_t temporizador) 
{
    t_list* parametros = list_create();
    uint32_t pid = pcb->pid;
    char* pidString = string_itoa(pid);
    list_add(parametros, instruccion->parametros[0]);
    list_add(parametros, pidString);
    t_instruccion* pedidoMemoria = crear_instruccion(DELETE_SEGMENT, parametros);
    enviarPedidoAMemoria(pedidoMemoria, conexion_memoria, logger_kernel);
    t_paquete *paquete = get_paquete(conexion_memoria, logger_kernel);
    log_info(logger_kernel, "PID: %u - Eliminar Segmento - Id Segmento: %s", pid, instruccion->parametros[0]);
    actualizar_tabla_segmento_y_reenviar_CPU (paquete, pcb);
}


void desconectar_memoria() 
{
    send(conexion_memoria, "TERMINAR", strlen("TERMINAR"), 0);
}



// ------------------------------------------------------------------------------------------
// -- Funciones para realizar solicitudes al FileSystem --
// ------------------------------------------------------------------------------------------

bool consultar_existencia_archivo(t_instruccion* instruccion) 
{
    t_pedido_file_system * pedido_filesystem = malloc(sizeof(pedido_filesystem));
    pedido_filesystem->direccion_fisica = 0;
    pedido_filesystem->pid = 0;
    pedido_filesystem->instruccion = instruccion; 

    enviar_pedido_file_system(pedido_filesystem, conexion_filesystem, logger_kernel);
    
    // Recibir la respuesta
    const size_t maxRespuestaLongitud = 256;
    char* respuesta = (char*)malloc(maxRespuestaLongitud);
    ssize_t bytesRecibidos = recv(conexion_filesystem, respuesta, maxRespuestaLongitud, 0);
    respuesta[bytesRecibidos] = '\0';

    bool archivoExiste = (strcmp(respuesta, "SI") == 0);

    free(respuesta);
 
    return archivoExiste;
}


void solicitar_creacion_archivo(t_instruccion* instruccion) 
{
    char* nombre_archivo = strdup(instruccion->parametros[0]);
    t_list* lista_parametros = list_create();
    list_add(lista_parametros, nombre_archivo);
    t_instruccion* instruccion_crear = crear_instruccion(F_CREATE, lista_parametros);
    t_pedido_file_system * pedido_filesystem = malloc(sizeof(pedido_filesystem));
    pedido_filesystem->direccion_fisica = 0;
    pedido_filesystem->pid = 0;
    pedido_filesystem->instruccion = instruccion_crear; 
    enviar_pedido_file_system(pedido_filesystem, conexion_filesystem, logger_kernel);

    const size_t maxRespuestaLongitud = 256;
    char* respuesta = (char*)malloc(maxRespuestaLongitud);
    ssize_t bytesRecibidos = recv(conexion_filesystem, respuesta, maxRespuestaLongitud, 0);
    respuesta[bytesRecibidos] = '\0';

    bool archivoCreado = (strcmp(respuesta, "SI") == 0);
    free(respuesta);
    list_destroy(lista_parametros);
}


void actualizar_tamanio_archivo(t_instruccion* instruccion) 
{
    t_pedido_file_system * pedido_filesystem = malloc(sizeof(pedido_filesystem));
    pedido_filesystem->direccion_fisica = 0;
    pedido_filesystem->pid = 0;
    pedido_filesystem->instruccion = instruccion; 
    enviar_pedido_file_system(pedido_filesystem, conexion_filesystem, logger_kernel);

    t_paquete* paquete_recibido = recibir_paquete(conexion_filesystem, logger_kernel); 
    uint32_t offset = 0;
    t_instruccion* instruccion_recibida = crear_instruccion_para_el_buffer(paquete_recibido->buffer, &offset);
    if (instruccion_recibida->identificador != F_TRUNCATE) {
        log_error(logger_kernel, "No fue posible truncarlo");
    }
}


void leer_y_grabar_en_memoria(t_instruccion* instruccion , t_pcb* pcb) 
{

    t_pedido_file_system * pedido_filesystem = malloc(sizeof(t_pedido_file_system));
    pedido_filesystem->direccion_fisica = pcb->direccion_fisica;
    pedido_filesystem->pid = pcb->pid;

    t_list* lista_parametros = list_create();
    list_add(lista_parametros, instruccion->parametros[0]);
    int puntero = extraer_puntero_archivo(instruccion->parametros[0]);
    list_add(lista_parametros, string_itoa(puntero));
    list_add(lista_parametros, instruccion->parametros[2]);
    t_instruccion* instruccion_crear = crear_instruccion(F_READ, lista_parametros);
    pedido_filesystem->instruccion = instruccion_crear; 

    
    log_info(logger_kernel, "PID: %u - Leer Archivo: %s - Puntero %d - Dirección Memoria %d - Tamaño %s", pcb->pid, pedido_filesystem->instruccion->parametros[0], puntero, pedido_filesystem->direccion_fisica, pedido_filesystem->instruccion->parametros[2]);
    enviar_pedido_file_system(pedido_filesystem, conexion_filesystem, logger_kernel);

    const size_t maxRespuestaLongitud = 256;
    char* respuesta = (char*)malloc(maxRespuestaLongitud);
    ssize_t bytesRecibidos = recv(conexion_filesystem, respuesta, maxRespuestaLongitud, 0);
    respuesta[bytesRecibidos] = '\0';

    bool archivoCreado = (strcmp(respuesta, "SI") == 0);

    free(respuesta);
}


void escribir(t_instruccion* instruccion, t_pcb* pcb) 
{
    t_pedido_file_system * pedido_filesystem = malloc(sizeof(t_pedido_file_system));
    pedido_filesystem->direccion_fisica = pcb->direccion_fisica;
    pedido_filesystem->pid = pcb->pid;
     t_list* lista_parametros = list_create();
    list_add(lista_parametros, instruccion->parametros[0]);
    int puntero = extraer_puntero_archivo(instruccion->parametros[0]);
    list_add(lista_parametros, string_itoa(puntero));
    list_add(lista_parametros, instruccion->parametros[2]);
    t_instruccion* instruccion_crear = crear_instruccion(F_WRITE, lista_parametros);
    pedido_filesystem->instruccion = instruccion_crear; 

    log_info(logger_kernel, "PID: %u - Escribir Archivo: %s - Puntero %d - Dirección Memoria %d - Tamaño %s", pcb->pid, pedido_filesystem->instruccion->parametros[0], puntero, pedido_filesystem->direccion_fisica, pedido_filesystem->instruccion->parametros[2]);

    enviar_pedido_file_system(pedido_filesystem, conexion_filesystem, logger_kernel);

    const size_t maxRespuestaLongitud = 256;
    char* respuesta = (char*)malloc(maxRespuestaLongitud);
    ssize_t bytesRecibidos = recv(conexion_filesystem, respuesta, maxRespuestaLongitud, 0);
    respuesta[bytesRecibidos] = '\0';

    bool archivoCreado = (strcmp(respuesta, "SI") == 0);

    free(respuesta);
}  


void desconectar_filesystem() 
{
    send(conexion_filesystem, "TERMINAR", strlen("TERMINAR"), 0);
}


int64_t recibir_temporizador(int conexion_cpu) 
{
    char respuesta[20];
    int bytes_recibidos = recv(conexion_cpu, respuesta, sizeof(respuesta) - 1, 0);
    if (bytes_recibidos >= 0) {
        respuesta[bytes_recibidos] = '\0'; // Agregar el terminador nulo al final del mensaje recibido
        }
        int64_t tiempoready;
        sscanf(respuesta, "%" SCNd64, &tiempoready);
        return tiempoready;
}