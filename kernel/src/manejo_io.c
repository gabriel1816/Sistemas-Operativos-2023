#include "planificacion.h"



void ejecutar_instruccion_io(t_pcb* pcb, int tiempo)
{
    agregar_pcb_en_cola_bloqueados_IO(pcb); 
    log_info(logger_kernel, "PID: %d - Ejecutando IO: %d", pcb->pid, tiempo);

    pthread_t hilo_IO;

    pthread_create(&hilo_IO, NULL, (void*) atender_io, (void*)(int)tiempo);
    pthread_detach(hilo_IO);
}


void atender_io(int tiempo)
{
    t_pcb* pcb = sacar_pcb_cola_bloqueados_IO();
    usleep(tiempo*1000000);
    agregar_pcb_en_cola_listos(pcb);
}