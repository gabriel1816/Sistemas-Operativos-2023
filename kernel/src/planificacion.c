#include <planificacion.h>



t_list *colaNuevos;
t_list *colaListos;
t_list *colaBloqueados;
t_pcb *pcbAEjecutar;
t_list *colaTerminados;
t_list *procesosEnSistema; 
t_list *colaBloqueadosFileSystem;
t_list *colaBloqueadosRecursos;

bool puedeIniciarCompactacion;
int procesosRW = 0;


static pthread_mutex_t procesosNuevosMutex;
static pthread_mutex_t procesosListosMutex;
static pthread_mutex_t procesosBloqueadosMutex;
static pthread_mutex_t procesosBloqueadosFileSystemMutex;
static pthread_mutex_t procesoAEjecutarMutex;
static pthread_mutex_t procesosTerminadosMutex;
static pthread_mutex_t procesoMutex;
static pthread_mutex_t procesosEnSistemaMutex;
static pthread_mutex_t puedeIniciarCompactacionMutex;



void iniciar_listas_y_semaforos() 
{
    colaNuevos = list_create();
    colaListos = list_create();
    colaBloqueados = list_create();
    pcbAEjecutar = NULL;
    colaTerminados = list_create();
    procesosEnSistema = list_create();
    colaBloqueadosFileSystem = list_create();
    colaBloqueadosRecursos = list_create();
    tabla_global_archivos_abiertos = list_create();

    pthread_mutex_init(&procesosNuevosMutex, NULL);
    pthread_mutex_init(&procesosListosMutex, NULL);
    pthread_mutex_init(&procesosBloqueadosMutex, NULL);
    pthread_mutex_init(&procesoAEjecutarMutex, NULL);
    pthread_mutex_init(&procesosTerminadosMutex, NULL);
    pthread_mutex_init(&procesoMutex, NULL);
    pthread_mutex_init(&procesosEnSistemaMutex, NULL);
    pthread_mutex_init(&procesosBloqueadosFileSystemMutex, NULL);
    pthread_mutex_init(&puedeIniciarCompactacionMutex, NULL);

    sem_init(&compactacion, 0, 0);
    sem_init(&semFileSystem, 0, 0);
    sem_init(&semMultiprogramacion, 0, grado_multiprogramacion);
    sem_init(&semProcesoNuevo, 0, 0);
    sem_init(&semProcesoListo, 0, 0);
}


char* estado_to_string(enum_estados estado)
{
    switch (estado)
    {
    case NUEVO:
        return "NUEVO";        
    case LISTO:
        return "LISTO";
    case EJECUTANDO:
        return "EJECUTANDO";        
    case BLOQUEADO:
        return "BLOQUEADO";
    case TERMINADO:
        return "TERMINADO";
    case SEG_FAULT:
        return "EJECUTANDO";
    default:
        return "ERROR";
    }
}


void cambiar_estado_pcb(t_pcb* pcb, enum_estados estado) 
{
    if(estado!=pcb->estado){
        log_info(logger_kernel, "PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb->pid, estado_to_string(pcb->estado), estado_to_string(estado));
    }
    pthread_mutex_lock(&procesoMutex);
    pcb->estado = estado;
    pthread_mutex_unlock(&procesoMutex);
}



// ------------------------------------------------------------------------------------------
// -- HRRN --
// ------------------------------------------------------------------------------------------

double responseRatio (t_pcb* pcb) 
{
    // RR = (PROX.RAFAGA + ESPERA) / PROX.RAFAGA
    double rr = ((pcb->estimado_rafaga) + temporal_gettime(pcb->llegada_a_listo)) / (pcb->estimado_rafaga);
    return rr;
}


bool mayorResponseRatio (t_pcb* pcb1, t_pcb* pcb2)
{
    return responseRatio (pcb1) > responseRatio (pcb2);
}


void stop_all_timers()
{
    for(int i = 0; i < list_size(colaListos); i++){
        t_pcb* pcb = list_get(colaListos, i);
        temporal_stop(pcb->llegada_a_listo);
    }
}


void resume_all_timers()
{
    for(int i = 0; i < list_size(colaListos); i++){
        t_pcb* pcb = list_get(colaListos, i);
        temporal_resume(pcb->llegada_a_listo);
    }
}


void ordenarPorRR()
{
    pthread_mutex_lock(&procesosListosMutex);
    stop_all_timers();
    list_sort(colaListos, (void *)mayorResponseRatio);
    resume_all_timers();
    pthread_mutex_unlock(&procesosListosMutex);
}


void actualizarEstimadoRafaga (t_pcb* pcb, int64_t tiempoEjecucion) 
{
    // S = α . rafagaAnterior + (1 - α) . estimadoAnterior
    double calculoEstimadoRafaga = (hrrn_alfa * tiempoEjecucion) + ((1 - hrrn_alfa) * (pcb->estimado_rafaga));
    pcb->estimado_rafaga = calculoEstimadoRafaga;
}



// ------------------------------------------------------------------------------------------
// -- Cola BLOQUEADOS --
// ------------------------------------------------------------------------------------------

t_pcb*  sacar_pcb_cola_bloqueados_IO()
{
    pthread_mutex_lock(&procesosBloqueadosMutex);
    t_pcb* proceso = list_remove(colaBloqueados, 0);
    sacar_pcb_cola_procesos_en_sistema(proceso);
    pthread_mutex_unlock(&procesosBloqueadosMutex);
    return proceso;
}


void agregar_pcb_en_cola_bloqueados_IO(t_pcb* proceso) 
{
    pthread_mutex_lock(&procesosBloqueadosMutex);
    list_add(colaBloqueados, proceso); //Por ahora es solo FIFO, en caso de que sea HRRN se debería agregar a la cola dependiendo de su orden. 
    cambiar_estado_pcb(proceso, BLOQUEADO);
    agregar_pcb_en_cola_procesos_en_sistema(proceso);
    pthread_mutex_unlock(&procesosBloqueadosMutex);
    log_info(logger_kernel, "PID: %d - Bloqueado por: IO", proceso->pid);
}


t_pcb*  sacar_pcb_cola_bloqueados_FileSystem()
{
    pthread_mutex_lock(&procesosBloqueadosFileSystemMutex);
    t_pcb* proceso = list_remove(colaBloqueadosFileSystem, 0);
    sacar_pcb_cola_procesos_en_sistema(proceso);
    pthread_mutex_unlock(&procesosBloqueadosFileSystemMutex);
    return proceso;
}


void agregar_pcb_en_cola_bloqueados_FileSystem(t_pcb* proceso) 
{
    pthread_mutex_lock(&procesosBloqueadosFileSystemMutex);
    list_add(colaBloqueadosFileSystem, proceso);
    cambiar_estado_pcb(proceso, BLOQUEADO);
    agregar_pcb_en_cola_procesos_en_sistema(proceso);
    pthread_mutex_unlock(&procesosBloqueadosFileSystemMutex);
    sem_post(&semFileSystem);
}

bool espero_compactacion = false;

void set_puede_iniciar_compactacion(bool estado)
{

    bool comp = false;
    pthread_mutex_lock(&puedeIniciarCompactacionMutex);

    if(procesosRW <= 0) comp = true;

    if(estado == false) procesosRW++;
    else procesosRW--;

    if(procesosRW <= 0 && comp == false){
        if(espero_compactacion) sem_post(&compactacion);
    }
    
    pthread_mutex_unlock(&puedeIniciarCompactacionMutex);
    

    
    
}

bool get_puede_iniciar_compactacion()
{
    bool retorno = false;
    pthread_mutex_lock(&puedeIniciarCompactacionMutex);
    if(procesosRW < 1) retorno = true;
    pthread_mutex_unlock(&puedeIniciarCompactacionMutex);
    return retorno;
}

void esperar_fin_FS(){
    if(!get_puede_iniciar_compactacion()){
        espero_compactacion = true;
        sem_wait(&compactacion);
    } 
}


void  sacar_pcb_cola_bloqueados_Recurso(t_pcb* proceso)
{
    pthread_mutex_lock(&procesosBloqueadosFileSystemMutex);
    list_remove_element(colaBloqueadosRecursos, proceso);
    sacar_pcb_cola_procesos_en_sistema(proceso);
    pthread_mutex_unlock(&procesosBloqueadosFileSystemMutex);
    return;
}


void agregar_pcb_cola_bloqueados_RecursoArchivo(t_pcb* proceso) 
{
    pthread_mutex_lock(&procesosBloqueadosFileSystemMutex);
    list_add(colaBloqueadosRecursos, proceso);
    cambiar_estado_pcb(proceso, BLOQUEADO);
    agregar_pcb_en_cola_procesos_en_sistema(proceso);
    pthread_mutex_unlock(&procesosBloqueadosFileSystemMutex);
}


void  sacar_pcb_cola_bloqueados_RecursoArchivo(t_pcb* proceso)
{
    pthread_mutex_lock(&procesosBloqueadosFileSystemMutex);
    list_remove_element(colaBloqueadosRecursos, proceso);
    sacar_pcb_cola_procesos_en_sistema(proceso);
    pthread_mutex_unlock(&procesosBloqueadosFileSystemMutex);
    return;
}


void agregar_pcb_cola_bloqueados_Recurso(t_pcb* proceso) 
{
    pthread_mutex_lock(&procesosBloqueadosFileSystemMutex);
    list_add(colaBloqueadosRecursos, proceso);
    cambiar_estado_pcb(proceso, BLOQUEADO);
    agregar_pcb_en_cola_procesos_en_sistema(proceso);
    pthread_mutex_unlock(&procesosBloqueadosFileSystemMutex);
}

// ------------------------------------------------------------------------------------------
// -- Cola LISTOS --
// ------------------------------------------------------------------------------------------

void loggear_cola_listos()
{
    char * stringLog = string_new();
    for(int i = 0; i < list_size(colaListos); i++){
        t_pcb* pcbReady = list_get(colaListos, i);
        char* pid = string_itoa(pcbReady->pid);
        string_append(&stringLog, pid);
        string_append(&stringLog, " ");
    }
    log_info(logger_kernel, "Cola Ready %s: [ %s]", algoritmo_planificacion, stringLog);
}


t_pcb* sacar_pcb_cola_listos() 
{
    pthread_mutex_lock(&procesosListosMutex);
    t_pcb* proceso = list_remove(colaListos, 0);
    temporal_stop(proceso->llegada_a_listo);
    sacar_pcb_cola_procesos_en_sistema(proceso);  
    pthread_mutex_unlock(&procesosListosMutex); 
    return proceso;
}


void agregar_pcb_en_cola_listos(t_pcb* proceso) 
{
    pthread_mutex_lock(&procesosListosMutex);
    list_add(colaListos, proceso); 
    cambiar_estado_pcb(proceso, LISTO);
    loggear_cola_listos();
    proceso->llegada_a_listo = temporal_create(); // para calcular el tiempo de LISTO a EJECUTANDO en HRRN
    agregar_pcb_en_cola_procesos_en_sistema(proceso);  
    pthread_mutex_unlock(&procesosListosMutex);
    sem_post(&semProcesoListo);
}



// ------------------------------------------------------------------------------------------
// -- Cola NUEVOS --
// ------------------------------------------------------------------------------------------

void agregar_pcb_cola_nuevos(t_pcb* proceso) 
{
    pthread_mutex_lock(&procesosNuevosMutex);
    list_add(colaNuevos, proceso);
    agregar_pcb_en_cola_procesos_en_sistema(proceso);
    pthread_mutex_unlock(&procesosNuevosMutex);
    sem_post(&semProcesoNuevo);
    
}


t_pcb* sacar_pcb_de_cola_nuevo() 
{
    pthread_mutex_lock(&procesosNuevosMutex);
    t_pcb* proceso = list_remove(colaNuevos, 0);
    sacar_pcb_cola_procesos_en_sistema(proceso);
    pthread_mutex_unlock(&procesosNuevosMutex);
    return proceso;
}



// ------------------------------------------------------------------------------------------
// -- En ejecucion --
// ------------------------------------------------------------------------------------------

void meter_pcb_en_ejecucion(t_pcb* proceso) 
{
    pthread_mutex_lock(&procesoAEjecutarMutex);
    pcbAEjecutar = proceso;
    cambiar_estado_pcb(proceso, EJECUTANDO);
    agregar_pcb_en_cola_procesos_en_sistema(proceso);
    pthread_mutex_unlock(&procesoAEjecutarMutex);
}


t_pcb* sacar_pcb_de_ejecucion(int64_t tiempoEjecucion) 
{
    pthread_mutex_lock(&procesoAEjecutarMutex);
    t_pcb* proceso = pcbAEjecutar;
    proceso;
    pcbAEjecutar = NULL;
    if (!strcmp(algoritmo_planificacion, "HRRN")) { 
            actualizarEstimadoRafaga(proceso, tiempoEjecucion); // Para HRRN
        }
    sacar_pcb_cola_procesos_en_sistema(proceso);
    pthread_mutex_unlock(&procesoAEjecutarMutex);
    
  return proceso;
}



// ------------------------------------------------------------------------------------------
// -- En estado EXIT --
// ------------------------------------------------------------------------------------------

void terminar_proceso(t_pcb* pcb, char* motivo)
{
    cambiar_estado_pcb(pcb, TERMINADO);
    log_info(logger_kernel, "Finaliza el proceso %d - Motivo: %s ", pcb->pid, motivo);//log obligatorio
    sem_post(&semMultiprogramacion);
    sacar_pcb_cola_procesos_en_sistema(pcb);
    eliminarProcesoDeMemoria(pcb->pid);
    t_instruccion* instruccion = list_get(pcb->instrucciones, pcb->program_counter - 1);
    t_buffer *buffer = crear_buffer_para_t_instruccion(instruccion);
    t_paquete *paquete = crear_paquete(buffer, CODIGO_INSTRUCCION);
    enviar_paquete((int)pcb->pid, paquete, logger_kernel);
    destruir_pcb(pcb);
}


void eliminarProcesoDeMemoria(uint32_t pid) 
{ 
    t_list* parametros = list_create();
    char* pidString = string_itoa(pid);
    list_add(parametros, pidString);
    t_instruccion* pedidoMemoria = crear_instruccion(END_PROCESS, parametros);
    enviarPedidoAMemoria(pedidoMemoria, conexion_memoria, logger_kernel);
}


// ------------------------------------------------------------------------------------------
// -- Cola procesos en sistema --
// ------------------------------------------------------------------------------------------

uint32_t pidPCB;


bool esElMismoPID_PCB(t_pcb* proceso, void* pid)
{
    return (pidPCB == proceso->pid);
}


t_pcb* find_pcb_cola_procesos_en_sistema(int32_t pid)
{
    pthread_mutex_lock(&procesosEnSistemaMutex);
    pidPCB = pid;
    t_pcb* retorno = list_find(procesosEnSistema, esElMismoPID_PCB);
    pthread_mutex_unlock(&procesosEnSistemaMutex);
    return retorno;
}


void sacar_pcb_cola_procesos_en_sistema(t_pcb* proceso)
{
    pthread_mutex_lock(&procesosEnSistemaMutex);
    list_remove_element(procesosEnSistema, proceso);
    pthread_mutex_unlock(&procesosEnSistemaMutex);
    return;
}


void agregar_pcb_en_cola_procesos_en_sistema(t_pcb* proceso) 
{
    pthread_mutex_lock(&procesosEnSistemaMutex);
    list_add(procesosEnSistema, proceso); 
    pthread_mutex_unlock(&procesosEnSistemaMutex);
}

