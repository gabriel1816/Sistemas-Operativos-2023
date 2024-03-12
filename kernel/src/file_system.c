#include "planificacion.h"


static pthread_t hiloPlanificadorFileSystem;



void iniciar_file_system() 
{
    pthread_create(&hiloPlanificadorFileSystem, NULL, (void*) file_system, NULL);
    pthread_detach(hiloPlanificadorFileSystem);
}


void file_system()
{
    while (1)
    {
        sem_wait(&semFileSystem);
        t_pcb *proceso; 
        proceso = sacar_pcb_cola_bloqueados_FileSystem();
        t_list* instrucciones = proceso->instrucciones;
        t_instruccion* ultima_instruccion = list_get(instrucciones, proceso->program_counter - 1);
        char* direccion_fisica_char;
        int32_t direccion_fisica;
        switch (ultima_instruccion->identificador)
        {
            case F_TRUNCATE:
                actualizar_tamanio_archivo(ultima_instruccion);
                break;

            case F_READ:
                leer_y_grabar_en_memoria(ultima_instruccion, proceso);               
                set_puede_iniciar_compactacion(true);
                break;

            case F_WRITE:
                escribir(ultima_instruccion, proceso);
                set_puede_iniciar_compactacion(true);
                break;
                
            default:
                break;
        }
        agregar_pcb_en_cola_listos(proceso);
    }
}

int extraer_puntero_archivo(char * archivo) {
    for(int i = 0; i < list_size(tabla_global_archivos_abiertos); i++) {
        t_archivo_abierto* nuevo_archivo = list_get(tabla_global_archivos_abiertos, i);
        if(string_equals_ignore_case(nuevo_archivo->nombre_archivo, archivo)) {
            return nuevo_archivo->puntero;  
        }
    }
}