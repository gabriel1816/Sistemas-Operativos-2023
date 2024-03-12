#include "planificacion.h"

static pthread_t hiloRecurso;
t_dictionary *dic_recursos;



void iniciar_recursos(char** key_recursos, char** instancias) 
{
    dic_recursos = dictionary_create();
    for(int i = 0; i< string_array_size(key_recursos); i++)
	{
		t_recurso* recurso = crear_recurso(atoi(instancias[i]));
        dictionary_put(dic_recursos, key_recursos[i], recurso);
	}
}


t_recurso* crear_recurso(int instancias)
{
    t_recurso* recurso = malloc(sizeof(t_recurso));
    recurso->cola_bloqueado = list_create();
    recurso->instancias = instancias;
    return recurso;
}


void atender_signal(t_pcb* pcb, char* key_recurso)
{
    if(!dictionary_has_key(dic_recursos, key_recurso)){
        terminar_proceso(pcb, "INVALID_RESOURCE");
        return;
    }
    t_recurso* recurso = dictionary_get(dic_recursos, key_recurso);
    recurso->instancias++;
    log_info(logger_kernel, "PID: %d - Signal: %s - Instancias: %d", pcb->pid, key_recurso, recurso->instancias);
    if(recurso->instancias <= 0){
        t_pcb* pcb_bloqueado = list_remove(recurso->cola_bloqueado, 0);
        sacar_pcb_cola_bloqueados_Recurso(pcb_bloqueado);
        agregar_pcb_en_cola_listos(pcb_bloqueado);
    }
    ejecutar_PCB(pcb);
}


void atender_wait(t_pcb* pcb, char* key_recurso)
{
    if(!dictionary_has_key(dic_recursos, key_recurso)){
        terminar_proceso(pcb, "INVALID_RESOURCE");
        return;
    }
    t_recurso* recurso = dictionary_get(dic_recursos, key_recurso);
    recurso->instancias--;
    log_info(logger_kernel, "PID: %d - Wait: %s - Instancias: %d", pcb->pid, key_recurso, recurso->instancias);
    if(recurso->instancias < 0){
        list_add(recurso->cola_bloqueado, pcb);
        log_info(logger_kernel, "PID: %d - Bloqueado por: %s", pcb->pid, key_recurso);
        agregar_pcb_cola_bloqueados_Recurso(pcb);
        return;
    }
    ejecutar_PCB(pcb);
}