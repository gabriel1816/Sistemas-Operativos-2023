#include "planificacion.h"

t_dictionary *dic_archivos;


void agregar_a_tabla_global(char* archivo) 
{
    t_archivo_abierto* archivo_abierto = malloc(sizeof(t_archivo_abierto));
    archivo_abierto->nombre_archivo = malloc(sizeof(archivo));
    strcpy(archivo_abierto->nombre_archivo, archivo);
    archivo_abierto->puntero = 0;
    archivo_abierto->archivo = crear_recurso(1);
    dictionary_put(dic_archivos, archivo, archivo_abierto->archivo);
    list_add(tabla_global_archivos_abiertos, archivo_abierto);
}


void agregar_a_tabla_proceso(t_pcb* pcb, char* archivo) 
{
    t_archivo_abierto* archivo_abierto = malloc(sizeof(t_archivo_abierto));
    archivo_abierto->nombre_archivo = archivo;
    archivo_abierto->puntero = 0;
    list_add(pcb->tabla_archivos, archivo_abierto);
}


bool esta_en_tabla(char* archivo) 
{
    for(int i = 0; i < list_size(tabla_global_archivos_abiertos); i++) {
        t_archivo_abierto* nuevo_archivo = list_get(tabla_global_archivos_abiertos, i);
        if(string_equals_ignore_case(nuevo_archivo->nombre_archivo, archivo)) {
            return true;  
        }
    }
    return false;
}


void sacar_de_tabla_proceso(t_pcb* pcb_actualizado, char* archivo) 
{
    eliminar_archivo_por_nombre(pcb_actualizado->tabla_archivos, archivo);
}


void sacar_de_tabla_global(t_pcb* pcb_actualizado, char* archivo) 
{
    eliminar_archivo_por_nombre(tabla_global_archivos_abiertos, archivo);
}


void eliminar_archivo_por_nombre(t_list* lista, char* nombre) 
{
    for(int i = 0; i < list_size(lista); i++) {
        t_archivo_abierto* archivo = list_get(lista, i);

        if(strcmp(archivo->nombre_archivo, nombre) == 0) {
            t_archivo_abierto* archivo_eliminado = list_remove(lista, i);
            break;
        }
    }
}


void actualizar_puntero_tabla_global(char* nombre_archivo, char* puntero) 
{
    int punteroN = atoi(puntero);
    for(int i = 0; i < list_size(tabla_global_archivos_abiertos); i++) {
        t_archivo_abierto* archivo = list_get(tabla_global_archivos_abiertos, i);
        if(string_equals_ignore_case(archivo->nombre_archivo, nombre_archivo)) {
            archivo->puntero = punteroN;
            break;
        }
    }
}

void iniciar_archivos()
{
    dic_archivos = dictionary_create();
}


void desbloquear_proceso(t_pcb* pcb, char* key_archivo) 
{
    int indice_a_remover = -1;
    t_recurso* archivo = dictionary_get(dic_archivos, key_archivo);
    for(int i = 0; i < list_size(archivo->cola_bloqueado); i++) {
        t_pcb* pcb_bloqueado = list_get(archivo->cola_bloqueado, i);
            if(pcb_bloqueado == pcb) {
            indice_a_remover = i;
            break;
        }
    }
    if(indice_a_remover != -1) {
        t_pcb* pcb_bloqueado = list_remove(archivo->cola_bloqueado, indice_a_remover);
        agregar_pcb_en_cola_listos(pcb_bloqueado);
    }
}  


bool hay_procesos_en_espera(t_pcb* pcb, char* key_archivo) 
{
    t_recurso* archivo = dictionary_get(dic_archivos, key_archivo);
    if(archivo->instancias < 0) return true;
    return false;
}


void atender_signal_archivo(t_pcb* pcb, char* key_archivo)
{
    if(!dictionary_has_key(dic_archivos, key_archivo)){
        terminar_proceso(pcb, "SIGNAL DE ARCHIVO INEXISTENTE");
        return;
    }

    t_recurso* archivo = dictionary_get(dic_archivos, key_archivo);

    archivo->instancias++;

    log_info(logger_kernel, "PID: %d - Signal: %s - Instancias: %d", pcb->pid, key_archivo, archivo->instancias);

    if(archivo->instancias <= 0){
        t_pcb* pcb_bloqueado = list_remove(archivo->cola_bloqueado, 0);
        sacar_pcb_cola_bloqueados_RecursoArchivo(pcb_bloqueado);
        agregar_pcb_en_cola_listos(pcb_bloqueado);
    }
}


void atender_wait_archivo(t_pcb* pcb, char* key_archivo)
{

    if(!dictionary_has_key(dic_archivos, key_archivo)){
        terminar_proceso(pcb, "WAIT DE ARCHIVO INEXISTENTE");
        return;
    }
    t_recurso* archivo = dictionary_get(dic_archivos, key_archivo);

    archivo->instancias--;

    if(archivo->instancias < 0){
        list_add(archivo->cola_bloqueado, pcb);
        log_info(logger_kernel, "PID: %d - Bloqueado por FOPEN: %s", pcb->pid, key_archivo);
        agregar_pcb_cola_bloqueados_RecursoArchivo(pcb);
        return;
    }
}