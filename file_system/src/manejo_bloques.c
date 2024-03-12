#include "file_system.h"

int calcular_bloques_adicionales(uint32_t nuevo_tamanio, t_fcb *fcb)
{

    double tamanio_archivo = (double)fcb->tamanio_archivo;
    double nuevo_tam = (double)nuevo_tamanio;
    double tamanio_bloque = (double)superBloque->tamanio_bloque;

    int bloques_actuales = (int)ceil(tamanio_archivo / tamanio_bloque);
    int bloques_nuevos = (int)ceil(nuevo_tam / tamanio_bloque);

    int bloquesAdicionales = bloques_nuevos - bloques_actuales;

    return bloquesAdicionales;
}

int calcular_bloques_a_liberar(uint32_t nuevo_tamanio, t_fcb *fcb)
{

    double tamanio_archivo = (double)fcb->tamanio_archivo;
    double nuevo_tam = (double)nuevo_tamanio;
    double tamanio_bloque = (double)superBloque->tamanio_bloque;

    int bloques_actuales = (int)ceil(tamanio_archivo / tamanio_bloque);
    int bloques_nuevos = (int)ceil(nuevo_tam / tamanio_bloque);

    int bloques_a_liberar = bloques_actuales - bloques_nuevos;

    return bloques_a_liberar;
}

void crear_bloques(char *path_bloques, t_superBloque *superbloque, int retardo, t_log *logger_fileSystem)
{

    int archivoBloques = open(path_bloques, O_CREAT | O_RDWR, 0644);
    int tamanioArchivoBloques = superbloque->tamanio_bloque * superbloque->cant_bloques;
    ftruncate(archivoBloques, tamanioArchivoBloques);
    void *mmapBloques = mmap(NULL, tamanioArchivoBloques, PROT_READ | PROT_WRITE, MAP_SHARED, archivoBloques, 0);
    copia_bloques = malloc(tamanioArchivoBloques);
    memcpy(copia_bloques, mmapBloques, tamanioArchivoBloques);

    /*
    while (1)
    {
        //sleep(retardo / 1000);
        memcpy(mmapBloques, copia_bloques, tamanioArchivoBloques);
    }
    */
    munmap(mmapBloques, tamanioArchivoBloques);
    close(archivoBloques);
}

void asignar_bloques(t_fcb *fcb, uint32_t bloques_adicionales)
{

    for (uint32_t i = 0; i < bloques_adicionales; i++)
    {
        uint32_t bloque_libre = buscar_bloque_libre(bitmap);
        if (bloque_libre != -1)
        { // suponiendo que buscar_bloque_libre devuelve -1 si no encuentra un bloque libre
            escribir_bloque_en_archivo(fcb->puntero_indirecto, fcb->cant_bloques_asignados - 1 + i, bloque_libre);
        }
        else
        {
            log_error(logger_fileSystem, "No hay suficientes bloques libres\n");
            break;
        }
    }
    fcb->cant_bloques_asignados += bloques_adicionales;
}

void escribir_bloque_en_archivo(uint32_t bloque, uint32_t pos_en_bloque, uint32_t valor)
{
    int archivoBloques = open(path_bloques, O_RDWR); // Abre el archivo para lectura y escritura

    if (archivoBloques == -1)
    {
        log_error(logger_fileSystem, "No se pudo abrir el archivo de bloques");
        return;
    }

    lseek(archivoBloques, bloque * superBloque->tamanio_bloque + pos_en_bloque * sizeof(uint32_t), SEEK_SET); // Busca la posición correcta en el bloque
    sleep(retardo / 1000);
    write(archivoBloques, &valor, sizeof(uint32_t));

    close(archivoBloques);
}

uint32_t buscar_bloque_libre(t_bitarray *bitmap)
{

    uint32_t bloque_libre = -1;

    uint32_t tamanio_bitmap = bitarray_get_max_bit(bitmap);
    for (int i = 0; i < tamanio_bitmap; i++)
    {

        if (!bitarray_test_bit(bitmap, i))
        { // el bit en la pos i del bitmap es 0 osea esta libre =>
            log_info(logger_fileSystem, "Acceso a Bitmap - Bloque: %d - Estado: %d", i, bitarray_test_bit(bitmap, i));
            bitarray_set_bit(bitmap, i); // lo seteo como ocupado (1)

            bloque_libre = i;
            break;
        }
    }
    return bloque_libre;
}

void liberar_bloques(t_fcb *fcb, int bloques_a_liberar)
{
    int cant_bloques_fcb = fcb->cant_bloques_asignados;
    int archivo_bloques = open(path_bloques, O_RDWR);

    while (bloques_a_liberar > 0 && cant_bloques_fcb > 1)
    {
        uint32_t bloque_leido;
        lseek(archivo_bloques, (int)fcb->puntero_indirecto * superBloque->tamanio_bloque + (cant_bloques_fcb - 1) * sizeof(uint32_t), SEEK_SET); // Busca la posición correcta en el bloque
        read(archivo_bloques, &bloque_leido, sizeof(uint32_t));
        sleep(retardo / 1000);
        liberar_bloque_bitmap(bloque_leido);
        cant_bloques_fcb--;
        bloques_a_liberar--;
    }
    if (cant_bloques_fcb == 1 && bloques_a_liberar == 1)
    {
        liberar_bloque_bitmap(fcb->puntero_directo);
        liberar_bloque_bitmap(fcb->puntero_indirecto);
    }
}

int liberar_bloque_bitmap(uint32_t bloque_a_liberar)
{
    log_info(logger_fileSystem, "Acceso a Bitmap - Bloque: %u - Estado: %d", bloque_a_liberar, bitarray_test_bit(bitmap, bloque_a_liberar));
    bitarray_clean_bit(bitmap, bloque_a_liberar);
}

void leer_archivo(char *nombre, int puntero_archivo, int cantidad_bytes_para_leer, int direccion_fisica, int pid) {
    t_fcb *fcb = buscar_fcb_por_nombre(lista_fcbs, nombre);

    if (fcb == NULL)
        return;

    if (fcb->tamanio_archivo < puntero_archivo + cantidad_bytes_para_leer)
    {
        log_error(logger_fileSystem, "No se puede leer el archivo %s porque los bytes a leer superan el tamaño del archivo", nombre);
        free(fcb->nombre_archivo);
        free(fcb);
        return;
    }
    
    int archivoBloques = open(path_bloques, O_RDWR);

    char* datos_leidos = malloc(cantidad_bytes_para_leer + 1);
    strcpy(datos_leidos, "");
    int bytes_para_leer_de_bloque_actual = 0;
    int puntero_en_bloque = 0;
    int bloque_leido;

    while (cantidad_bytes_para_leer > 0)
    {
        int bloque_actual = ceil((double)puntero_archivo/(double)superBloque->tamanio_bloque);
        puntero_en_bloque = puntero_archivo%superBloque->tamanio_bloque;
        if(puntero_en_bloque == 0) {
            bloque_actual += 1;
        }
        if ((superBloque->tamanio_bloque - puntero_en_bloque) > cantidad_bytes_para_leer) 
        { 
            bytes_para_leer_de_bloque_actual = cantidad_bytes_para_leer; 
            } 
        else { 
            bytes_para_leer_de_bloque_actual = superBloque->tamanio_bloque - puntero_en_bloque; 
            }
        
        if(bloque_actual == 1) {
            lseek(archivoBloques, (fcb->puntero_directo) * superBloque->tamanio_bloque + puntero_en_bloque, SEEK_SET);
            log_info(logger_fileSystem, "Acceso Bloque - Archivo: %s - Bloque Archivo: %d - Bloque File System %d ", nombre, bloque_actual, fcb->puntero_directo);
            char* datitos = malloc(bytes_para_leer_de_bloque_actual);
            ssize_t numBytesLeidos = read(archivoBloques, datitos, bytes_para_leer_de_bloque_actual);
            strcat(datos_leidos, datitos);
            puntero_archivo += bytes_para_leer_de_bloque_actual;
            cantidad_bytes_para_leer -= bytes_para_leer_de_bloque_actual;
            sleep(retardo / 1000);
        }
        else {
            log_info(logger_fileSystem, "Acceso Bloque - Archivo: %s - Bloque Archivo: 2 - Bloque File System %d ", nombre, fcb->puntero_indirecto);       
            lseek(archivoBloques, (int)fcb->puntero_indirecto * superBloque->tamanio_bloque + (bloque_actual - 2) * sizeof(uint32_t), SEEK_SET);
            read(archivoBloques, &bloque_leido, sizeof(uint32_t));
            log_info(logger_fileSystem, "Acceso Bloque - Archivo: %s - Bloque Archivo: %d - Bloque File System %d ", nombre, bloque_actual+1, bloque_leido);
            lseek(archivoBloques, bloque_leido * superBloque->tamanio_bloque + puntero_en_bloque, SEEK_SET);
            char* datitos = malloc(bytes_para_leer_de_bloque_actual);
            ssize_t numBytesLeidos = read(archivoBloques, datitos, bytes_para_leer_de_bloque_actual);
            sleep(retardo / 1000);
            strcat(datos_leidos, datitos);
            puntero_archivo += bytes_para_leer_de_bloque_actual;
            cantidad_bytes_para_leer -= bytes_para_leer_de_bloque_actual;
            
        }

    }
    sleep(retardo / 1000);
    close(archivoBloques);
    
    t_list* lista_parametros = list_create();
    list_add(lista_parametros, datos_leidos);
    
    t_pedido_file_system* pedido = malloc(sizeof(t_pedido_file_system));
    pedido->instruccion = crear_instruccion(F_READ_MEMORIA, lista_parametros);
    pedido->pid = pid;
    pedido->direccion_fisica = direccion_fisica;
    t_buffer *buffer = crear_buffer_para_t_pedido_file_system(pedido);
    t_paquete *paquete = crear_paquete(buffer, CODIGO_INFORMACION_FILE_SYSTEM);
    enviar_paquete(conexion_memoria, paquete, logger_fileSystem);
    list_destroy(lista_parametros);

    t_paquete *paquete2 = get_paquete(conexion_memoria, logger_fileSystem);
    //if(paquete->codigo_operacion == F_READ_OK) log_info(logger_fileSystem, "La lectura fue exitosa resulto exitoso");

    destruir_paquete(paquete);
    destruir_paquete(paquete2);


}

void escribir_archivo(char* nombre, int puntero_archivo, char* datos_a_escribir, int cantidad_bytes_para_escribir) {
    t_fcb* fcb = buscar_fcb_por_nombre(lista_fcbs, nombre);

    if (fcb == NULL)
        return;

    if (fcb->tamanio_archivo < puntero_archivo + cantidad_bytes_para_escribir) {
        log_error(logger_fileSystem, "No se puede escribir en el archivo %s porque los bytes a escribir superan el tamaño del archivo", nombre);
        free(fcb->nombre_archivo);
        free(fcb);
        return;
    }

    int archivoBloques = open(path_bloques, O_RDWR);
    int bytes_escritos = 0;
    int bytes_para_escribir_en_bloque_actual = 0;
    int bloque_leido;
    int puntero_en_bloque = 0;

    while (cantidad_bytes_para_escribir > 0) {
        int bloque_actual = ceil((double)puntero_archivo / (double)superBloque->tamanio_bloque);
        puntero_en_bloque  = puntero_archivo % superBloque->tamanio_bloque;
        if(puntero_en_bloque == 0) {
            bloque_actual += 1;
        }

        if ((superBloque->tamanio_bloque - puntero_en_bloque) > cantidad_bytes_para_escribir) {
            bytes_para_escribir_en_bloque_actual = cantidad_bytes_para_escribir;
        } else {
            bytes_para_escribir_en_bloque_actual = superBloque->tamanio_bloque - puntero_en_bloque;
        }

        if (bloque_actual == 1) {
            lseek(archivoBloques, (fcb->puntero_directo) * superBloque->tamanio_bloque + puntero_en_bloque, SEEK_SET);
            log_info(logger_fileSystem, "Acceso Bloque - Archivo: %s - Bloque Archivo: %d - Bloque File System %d ", nombre, bloque_actual, fcb->puntero_directo);
            char* datitos = string_substring_from(datos_a_escribir, bytes_escritos);
            write(archivoBloques, datitos, bytes_para_escribir_en_bloque_actual);
            puntero_archivo += bytes_para_escribir_en_bloque_actual;
            bytes_escritos += bytes_para_escribir_en_bloque_actual;
            cantidad_bytes_para_escribir -= bytes_para_escribir_en_bloque_actual;
            sleep(retardo / 1000);
        } else {
            log_info(logger_fileSystem, "Acceso Bloque - Archivo: %s - Bloque Archivo: 2 - Bloque File System %d ", nombre, fcb->puntero_indirecto);       
            lseek(archivoBloques, (int)fcb->puntero_indirecto * superBloque->tamanio_bloque + (bloque_actual - 2) * sizeof(uint32_t), SEEK_SET);
            read(archivoBloques, &bloque_leido, sizeof(uint32_t));
            log_info(logger_fileSystem, "Acceso Bloque - Archivo: %s - Bloque Archivo: %d - Bloque File System %d ", nombre, bloque_actual+1, bloque_leido);
            lseek(archivoBloques, bloque_leido * superBloque->tamanio_bloque + puntero_en_bloque, SEEK_SET);
            char* datitos = string_substring_from(datos_a_escribir, bytes_escritos);
            write(archivoBloques, datitos, bytes_para_escribir_en_bloque_actual);
            sleep(retardo / 1000);
            puntero_archivo += bytes_para_escribir_en_bloque_actual;
            bytes_escritos += bytes_para_escribir_en_bloque_actual;
            cantidad_bytes_para_escribir -= bytes_para_escribir_en_bloque_actual;
        }
    }

    sleep(retardo / 1000);
    close(archivoBloques);
}
