#include <memoria.h>

// ------------------------------------------------------------------------------------------
// -- Funciones de analisis --
// ------------------------------------------------------------------------------------------

bool mayorTamanio (t_segmento* hueco1, t_segmento* hueco2) 
{
    return obtenerTamanioHueco(hueco1) > obtenerTamanioHueco(hueco2);
}


bool menorTamanio (t_segmento* hueco1, t_segmento* hueco2) 
{
    return obtenerTamanioHueco(hueco1) < obtenerTamanioHueco(hueco2);
}


bool menorBase (t_segmento* hueco1, t_segmento* hueco2) 
{
    return hueco1->base < hueco2->base;
}


int obtenerTamanioHueco(t_segmento* huecoLibre) 
{
    return huecoLibre->limite - huecoLibre->base;
}


bool esElMismoPID(t_procesoMemoria* proceso, void* pid)
{
    return (pidABuscar == proceso->pid);
}


int calcularTamioTotal()
{
    int i = 0;
    int tamanioTotal = 0;
    while(i < list_size(huecosLibres)) {
        t_segmento* huecoLibre = list_get(huecosLibres, i);
        tamanioTotal += obtenerTamanioHueco(huecoLibre);
        i++;
    }

    return tamanioTotal;
}



// ------------------------------------------------------------------------------------------
// -- Huecos libres --
// ------------------------------------------------------------------------------------------

t_segmento modificarListaHuecosLibres(t_segmento* hueco, uint32_t tamanioSegmento, int index) 
{
    t_segmento segmentoNuevo;
    segmentoNuevo.base = hueco->base;
    segmentoNuevo.limite = tamanioSegmento + segmentoNuevo.base;

    hueco->base = segmentoNuevo.limite;
    if(hueco->base == hueco->limite) list_remove(huecosLibres, index);
    return segmentoNuevo;
}


t_segmento buscarHuecoLibre(uint32_t tamanioSegmento)
{
    t_segmento* hueco = NULL;

    if(!strcmp(algoritmo_asignacion,"WORST")) {
        list_sort(huecosLibres, mayorTamanio);
        hueco = list_get(huecosLibres, 0);
        uint32_t tamanioHueco = hueco->limite - hueco->base;
        if(tamanioHueco >= tamanioSegmento) return modificarListaHuecosLibres(hueco, tamanioSegmento, 0);
    }
    else {
        if(!strcmp(algoritmo_asignacion,"BEST")) {
            list_sort(huecosLibres, menorTamanio);
        }
        else if(!strcmp(algoritmo_asignacion,"FIRST")) {
            list_sort(huecosLibres, menorBase);
        }
        int i = 0;
        while(i < list_size(huecosLibres)) {
            hueco = list_get(huecosLibres, i);
            uint32_t tamanioHueco = hueco->limite - hueco->base;
            if(tamanioHueco >= tamanioSegmento) return modificarListaHuecosLibres(hueco, tamanioSegmento, i);
            i++;
        }
    }
    int tamaniosTotales = calcularTamioTotal();
    if(tamaniosTotales >= tamanioSegmento) {
        hueco->id = -1;
        return *hueco;
    }
    else {
        hueco->id = -2;
        return *hueco;
    }
}


void crearNuevoHueco(uint32_t limite, uint32_t base)
{
    t_segmento* nuevoHueco = malloc(sizeof(t_segmento)); 
    nuevoHueco->base = base;
    nuevoHueco->limite = limite;
    list_add_sorted(huecosLibres, nuevoHueco, menorBase); 
    if(list_size(huecosLibres) == 1) return;
    t_segmento* huecoLibrePosterior = list_get(huecosLibres, 1); 
    if(huecoLibrePosterior->base == nuevoHueco->limite) { 
        nuevoHueco->limite = huecoLibrePosterior->limite;
        t_segmento* huecoContinuo = list_remove(huecosLibres, 1);
        //free(huecoContinuo);
    }
    return;
}


void agregarNuevoHueco(t_segmento* segmentoALiberar) 
{
    int j = 0;
    for(int i = 0; i < list_size(huecosLibres); i++) {
        t_segmento* huecoLibre = list_get(huecosLibres, i); 
        if(huecoLibre->base == segmentoALiberar->limite) {
            huecoLibre->base = segmentoALiberar->base;
            segmentoALiberar->limite = huecoLibre->limite;
            j++;
        }
        else if (huecoLibre->limite == segmentoALiberar->base) {
            huecoLibre->limite = segmentoALiberar->limite;
            segmentoALiberar->base = huecoLibre->base;
            j++;
        }
        if (j == 2) return;
    }
    list_add_sorted(huecosLibres, segmentoALiberar, menorBase);
    return;
}



// ------------------------------------------------------------------------------------------
// -- Segmento --
// ------------------------------------------------------------------------------------------

void modificarSegmentoContinuo (t_segmento* huecoLibre) 
{
    int index = 0;
    t_procesoMemoria* procesoMemoria;
    while(index<list_size(procesosEnMemoria)) {
        procesoMemoria = list_get(procesosEnMemoria, index);
        t_segmento* tablaSegmentos = procesoMemoria->tabla_segmento;
        int indexTabla = 0;
        while(indexTabla < procesoMemoria->tamanio_tabla){
            if(tablaSegmentos[indexTabla].id != -1 && tablaSegmentos[indexTabla].base == huecoLibre->limite){  
                uint32_t tamanioSegmento = tablaSegmentos[indexTabla].limite - tablaSegmentos[indexTabla].base;
                uint32_t limiteViejo = tablaSegmentos[indexTabla].limite;
                uint32_t baseVieja = tablaSegmentos[indexTabla].base;
                tablaSegmentos[indexTabla].base = huecoLibre->base;
                tablaSegmentos[indexTabla].limite = tablaSegmentos[indexTabla].base + tamanioSegmento;
                void* destino = memoriaReal + tablaSegmentos[indexTabla].base;
                void* origen = memoriaReal + baseVieja;
                memmove(destino, origen, tamanioSegmento); //muevo lo que estaba en el segmento antes al nuevo
                list_remove_element(huecosLibres, huecoLibre);
                crearNuevoHueco(limiteViejo, tablaSegmentos[indexTabla].limite);
                return;
            }
            indexTabla++;
        }
        index++;
    }
    return;
}


void iniciarCompactacion() 
{
    list_sort(huecosLibres, menorBase);
    t_segmento* huecoLibre = list_get(huecosLibres, 0);
    while(huecoLibre->limite != tam_memoria) {
        modificarSegmentoContinuo(huecoLibre);
        huecoLibre = list_get(huecosLibres, 0);
    }
    return;
}


void mostrarResultadoCompactacion() 
{
    log_info(logger_memoria, "-------------Resultado Compactacion-------------");
    for(int i = 0; i < list_size(procesosEnMemoria); i++) {
        t_procesoMemoria* procesoMemoria = list_get(procesosEnMemoria, i);
        t_segmento* tablaSegmentos = procesoMemoria->tabla_segmento;
        int indexTabla = 0;
        while(indexTabla < procesoMemoria->tamanio_tabla) {
            if(tablaSegmentos[indexTabla].id != -1) {  
                t_segmento segmento = tablaSegmentos[indexTabla];
                log_info(logger_memoria, "PID: %u - Segmento: %i - Base: %u - Tamaño %i", procesoMemoria->pid, segmento.id, segmento.base, obtenerTamanioHueco(&segmento));
            }
            indexTabla++;
        }
    }
}


void crear_segmento(uint32_t pid, int32_t idSegmento, uint32_t tamanioSegmento, intptr_t una_conexion) 
{
    pidABuscar = pid;
    t_procesoMemoria* proceso = list_find(procesosEnMemoria, esElMismoPID); 
    t_segmento hueco = buscarHuecoLibre(tamanioSegmento);
    switch(hueco.id) {
        case -1: //COMPACTACION
            log_info(logger_memoria, "-------------Solicitud Compactacion-------------");
            enviarMensajeCompactacion(una_conexion); 
            recibirAprobacion(una_conexion);
            log_info(logger_memoria, "-------------Compactacion aceptada-------------");
            iniciarCompactacion();
            mostrarResultadoCompactacion();
            usleep(retardo_compactacion*1000);
            enviarProcesosEnMemoria(una_conexion); 
            
        break;

        case -2:
            enviarErrorOutOfMemory(una_conexion);
        break;

        default:
            t_segmento* nuevoSegmento = malloc(sizeof(t_segmento));
            nuevoSegmento->id = idSegmento;
            nuevoSegmento->base = hueco.base;
            nuevoSegmento->limite = hueco.limite;
            proceso->tabla_segmento[idSegmento] = *nuevoSegmento;
            log_info(logger_memoria, "PID: %u - Crear Segmento: %d - Base: %u - TAMAÑO: %u", pid, idSegmento, hueco.base, obtenerTamanioHueco(nuevoSegmento));
            enviarProcesoEnMemoriaADestino(proceso, una_conexion);
        break;
    }
    return;
}


void borrar_segmento(uint32_t pid, int32_t idSegmento, intptr_t una_conexion) 
{
    pidABuscar = pid;
    t_procesoMemoria* proceso = list_find(procesosEnMemoria, esElMismoPID);
    borrar_segmento_en_memoria(proceso, idSegmento);
    enviarProcesoEnMemoriaADestino(proceso, una_conexion);
}


void borrar_segmento_en_memoria(t_procesoMemoria* proceso, uint32_t idSegmento) 
{
    t_segmento* segmentoALiberar = &proceso->tabla_segmento[idSegmento];
    log_info(logger_memoria, "PID: %u - Eliminar Segmento: %d - Base: %u - TAMAÑO: %u", proceso->pid, idSegmento, segmentoALiberar->base, obtenerTamanioHueco(segmentoALiberar));
    agregarNuevoHueco(segmentoALiberar);
    proceso->tabla_segmento[idSegmento].id = -1;
    //free(segmentoALiberar);
}