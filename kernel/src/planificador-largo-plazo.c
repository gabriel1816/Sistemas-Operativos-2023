#include <planificacion.h>

static pthread_t hiloPlanificadorLargoPlazo;



void crear_hilo_planificador_largo_plazo() 
{

    pthread_create(&hiloPlanificadorLargoPlazo, NULL, (void*) chequear_grado_de_multiprogramacion, NULL);
    pthread_detach(hiloPlanificadorLargoPlazo);
}


void chequear_grado_de_multiprogramacion() 
{
    while (1) {
        sem_wait(&semProcesoNuevo);
        sem_wait(&semMultiprogramacion);
        log_info(logger_kernel, "Proceso entro al sistema");
        t_pcb* pcb = sacar_pcb_de_cola_nuevo();
        agregar_pcb_en_cola_listos(pcb);
    }
}


t_procesoMemoria* solicitarCreacionDeProcesoEnMemoria(uint32_t pid) 
{ 
    t_list* parametros = list_create();
    char* pidString = string_itoa(pid);
    list_add(parametros, pidString);
    t_instruccion* pedidoMemoria = crear_instruccion(CREATE_PROCESS, parametros);
    enviarPedidoAMemoria(pedidoMemoria, conexion_memoria, logger_kernel);
    return recibirProcesoDeMemoria(conexion_memoria, logger_kernel);
}